// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include "ac3log/simplelogger.hpp"

#include <cassert>
#include <cstdint>

namespace iox
{
namespace popo
{
SenderPort::SenderPort(SenderPortData* const member)
    : BasePort(member)
{
}

cxx::optional<capro::CaproMessage> SenderPort::getCaProMessage()
{
    // get acivate reuest from user side
    const bool l_currentActivateRequest = getMembers()->m_activateRequested.load(std::memory_order_relaxed);

    const bool l_isActivated = getMembers()->m_active.load(std::memory_order_relaxed);

    if (l_currentActivateRequest && !l_isActivated)
    {
        getMembers()->m_active.store(true, std::memory_order_relaxed);

        capro::CaproMessage l_caproMessage(capro::CaproMessageType::OFFER, this->getMembers()->m_serviceDescription);
        if (getMembers()->m_receiverHandler.doesDeliverOnSubscribe())
        {
            l_caproMessage.m_subType = capro::CaproMessageSubType::FIELD;
        }
        else
        {
            l_caproMessage.m_subType = capro::CaproMessageSubType::EVENT;
        }
        return cxx::make_optional<capro::CaproMessage>(l_caproMessage);
    }
    else if (!l_currentActivateRequest && l_isActivated)
    {
        getMembers()->m_active.store(false, std::memory_order_relaxed);

        disconnectAllReceiver();

        capro::CaproMessage l_caproMessage(capro::CaproMessageType::STOP_OFFER,
                                           this->getMembers()->m_serviceDescription);
        return cxx::make_optional<capro::CaproMessage>(l_caproMessage);
    }
    else
    {
        // nothing to change
        return cxx::nullopt_t();
    }
}

cxx::optional<capro::CaproMessage> SenderPort::dispatchCaProMessage(capro::CaproMessage caProMessage)
{
    capro::CaproMessage l_responseMessage(capro::CaproMessageType::NACK,
                                          this->getMembers()->m_serviceDescription,
                                          capro::CaproMessageSubType::NOSUBTYPE,
                                          caProMessage.m_requestPort);

    if (getMembers()->m_active.load(std::memory_order_relaxed) && hasValidService(caProMessage))
    {
        if (capro::CaproMessageType::SUB == caProMessage.m_type)
        {
            if (connectReceiverPort(reinterpret_cast<ReceiverPortType::MemberType_t*>(caProMessage.m_requestPort)))
            {
                l_responseMessage.m_type = capro::CaproMessageType::ACK;
            }
        }
        else if (capro::CaproMessageType::UNSUB == caProMessage.m_type)
        {
            disconnectReceiverPort(reinterpret_cast<ReceiverPortType::MemberType_t*>(caProMessage.m_requestPort));

            l_responseMessage.m_type = capro::CaproMessageType::ACK;
        }
    }

    auto l_returnMessage =
        ReceiverPortType(reinterpret_cast<ReceiverPortType::MemberType_t*>(caProMessage.m_requestPort))
            .dispatchCaProMessage(l_responseMessage);
    cxx::Ensures(!l_returnMessage.has_value());

    return cxx::make_optional<capro::CaproMessage>(l_responseMessage);
}

// tidy up as good as possible
// This is called from RouDi and contract is that user process is no more running
void SenderPort::cleanup()
{
    // remove all chunks currently allocated by the application
    clearAllocatedChunkContainer();
    getMembers()->m_lastChunk = nullptr;
}

void SenderPort::setThroughput(const uint32_t payloadSize)
{
    getMembers()->m_activePayloadSize.store(payloadSize, std::memory_order_relaxed);
    getMembers()->m_throughput.payloadSize = payloadSize;
    getMembers()->m_throughput.chunkSize = getMembers()->m_memoryMgr->getMempoolChunkSizeForPayloadSize(payloadSize);
}

mepoo::ChunkHeader* SenderPort::reserveChunk(const uint32_t payloadSize, bool useDynamicPayloadSizes)
{
    if (!getMembers()->m_memoryMgr)
    {
        ERR_PRINTF("There is no shared memory available to allocate from! Terminating!");
        exit(EXIT_FAILURE);
    }
    /// @todo The chunk size should be set in the constructor
    /// this needs to be done in the upcoming refactoring
    auto activePayloadSize = getMembers()->m_activePayloadSize.load(std::memory_order_relaxed);
    if (activePayloadSize == 0 || (useDynamicPayloadSizes && payloadSize != activePayloadSize))
    {
        setThroughput(payloadSize);
    }
    else if (!useDynamicPayloadSizes && payloadSize != activePayloadSize)
    {
        errorHandler(Error::kPOSH__SENDERPORT_SAMPLE_SIZE_CHANGED_FOR_ACTIVE_PORT);
    }

    // if it is no field and we have a last chunk which is only owned by us, then use this chunk again
    if (!getMembers()->m_receiverHandler.doesDeliverOnSubscribe() && getMembers()->m_lastChunk
        && getMembers()->m_lastChunk.hasNoOtherOwners()
        && getMembers()->m_lastChunk.getChunkHeader()->m_info.m_usedSizeOfChunk
               >= getMembers()->m_memoryMgr->sizeWithChunkHeaderStruct(payloadSize))
    {
        if (pushToAllocatedChunkContainer(getMembers()->m_lastChunk))
        {
            getMembers()->m_lastChunk.getChunkHeader()->m_info.m_payloadSize = payloadSize;
            getMembers()->m_lastChunk.getChunkHeader()->m_info.m_usedSizeOfChunk =
                getMembers()->m_memoryMgr->sizeWithChunkHeaderStruct(payloadSize);
            return getMembers()->m_lastChunk.getChunkHeader();
        }
        else
        {
            assert(false && "Application allocates too much chunks");
            return nullptr;
        }
    }
    else
    {
        // get a new chunk
        mepoo::SharedChunk l_chunk = getMembers()->m_memoryMgr->getChunk(payloadSize);

        if (l_chunk)
        {
            // if the application allocated too much chunks, return no more chunks
            if (pushToAllocatedChunkContainer(l_chunk))
            {
                l_chunk.getChunkHeader()->m_info.m_payloadSize = payloadSize;
                return l_chunk.getChunkHeader();
            }
            else
            {
                // release the allocated chunk
                l_chunk = nullptr;
                assert(false && "Application allocates too much chunks");
                return nullptr;
            }
        }
        else
        {
            std::cerr << "Senderport [ service = " << getMembers()->m_serviceDescription.getServiceIDString()
                      << ", instance = " << getMembers()->m_serviceDescription.getInstanceIDString()
                      << ", event = " << getMembers()->m_serviceDescription.getEventIDString()
                      << " ] is unable to acquire a chunk of with payload size " << payloadSize << std::endl;
            errorHandler(Error::kPOSH__SENDERPORT_ALLOCATE_FAILED, [&] {
                std::cerr << ErrorHandler::ToString(Error::kPOSH__SENDERPORT_ALLOCATE_FAILED) << std::endl;
                std::cerr << std::endl;
                std::cerr << "\033[31mICEORYX ERROR!\033[m\n";
                std::cerr << std::endl;
                assert(l_chunk && "Pool is running out of chunks");
            });
            return nullptr;
        }
    }
}

void SenderPort::setThroughputDeliveryData(mepoo::ChunkInfo& chunkInfo, bool updateTimeInChunk)
{
    getMembers()->m_throughput.lastDeliveryTimestamp = getMembers()->m_throughput.currentDeliveryTimestamp;
    getMembers()->m_throughput.currentDeliveryTimestamp = mepoo::BaseClock::now();
    if (updateTimeInChunk)
    {
        chunkInfo.m_txTimestamp = getMembers()->m_throughput.currentDeliveryTimestamp;
    }

    getMembers()->m_throughput.sequenceNumber = getMembers()->m_sequenceNumber;
    getMembers()->m_throughputExchange.store(getMembers()->m_throughput, MemberType_t::ThreadContext::Application);
}

void SenderPort::deliverChunk(mepoo::ChunkHeader* const chunkHeader)
{
    bool l_isOffered = getMembers()->m_activateRequested.load(std::memory_order_relaxed);
    bool l_isField = getMembers()->m_receiverHandler.doesDeliverOnSubscribe();

    if (!l_isOffered && !l_isField)
    {
        // if not offered and no field, drop the chunk
        if (!deleteFromAllocatedChunkContainer(chunkHeader))
        {
            assert(false && "Application provided invalid chunk pointer to free");
        }
    }
    else
    {
        // we have to process this chunk

        // get chunk from allocated List
        mepoo::SharedChunk l_chunk(nullptr);

        if (popFromAllocatedChunkContainer(chunkHeader, l_chunk))
        {
            auto& chunkInfo = l_chunk.getChunkHeader()->m_info;
            if (!chunkInfo.m_externalSequenceNumber_bl)
            {
                chunkInfo.m_sequenceNumber = getMembers()->m_sequenceNumber;
                getMembers()->m_sequenceNumber++;
            }
            else
            {
                getMembers()->m_sequenceNumber++; // for Introspection, else nobody updates.
            }
            setThroughputDeliveryData(chunkInfo);
        }
        else
        {
            assert(false && "Application provided invalid chunk pointer to deliver");
        }

        if (l_isOffered && !l_isField)
        {
            // deliver the chunk and store the last chunk for recycling if it is free on next reserveChunk
            getMembers()->m_receiverHandler.appContext().deliverChunk(l_chunk);
            getMembers()->m_lastChunk = l_chunk;
        }
        else if (l_isOffered && l_isField)
        {
            // just deliver the chunk, we cannot recycle it as anytime someone could subscribe and the last chunk must
            // be provided
            getMembers()->m_receiverHandler.appContext().deliverChunk(l_chunk);
        }
        else
        {
            // a not offered field so we have to update the last chunk in the receiver handler
            getMembers()->m_receiverHandler.appContext().updateLastChunk(l_chunk);
        }
    }
}

void SenderPort::freeChunk(mepoo::ChunkHeader* const chunkHeader)
{
    if (!deleteFromAllocatedChunkContainer(chunkHeader))
    {
        assert(false && "Application provided invalid chunk pointer to free");
    }
}

void SenderPort::activate()
{
    if (!getMembers()->m_activateRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_activateRequested.store(true, std::memory_order_relaxed);

        if (getMembers()->m_receiverHandler.doesDeliverOnSubscribe()
            && !getMembers()->m_receiverHandler.appContext().hasLastChunk())
        {
            errorHandler(Error::kPOSH__SENDERPORT_ACTIVATE_FIELD_WITHOUT_DATA);
        }
    }
}

void SenderPort::deactivate()
{
    if (this->getMembers()->m_activateRequested.load(std::memory_order_relaxed))
    {
        this->getMembers()->m_activateRequested.store(false, std::memory_order_relaxed);

        getMembers()->m_activePayloadSize.store(0, std::memory_order_relaxed);
    }
}

bool SenderPort::hasSubscribers()
{
    return getMembers()->m_receiverHandler.appContext().hasReceivers();
}

void SenderPort::forwardChunk(mepoo::SharedChunk chunk)
{
    // @todo send limitations as for normal send?
    // if (this- getMembers().m_activateRequested.load(std::memory_order_relaxed) ||
    // getMembers().m_receiverHandler.doesDeliverOnSubscribe()) since we are a shadow port a normal send is not done! ->
    // we have to inc. seq.Nr. for introspection
    getMembers()->m_sequenceNumber++;
    setThroughputDeliveryData(chunk.getChunkHeader()->m_info, false);
    setThroughput(chunk.getChunkHeader()->m_info.m_payloadSize);
    getMembers()->m_receiverHandler.appContext().deliverChunk(chunk);
}

SenderPort::MemberType_t::Throughput SenderPort::getThroughput() const
{
    auto updatedValue = getMembers()->m_throughputExchange.take(MemberType_t::ThreadContext::RouDi);
    if (updatedValue.has_value())
    {
        getMembers()->m_throughputReadCache = updatedValue.value();
    }
    return getMembers()->m_throughputReadCache;
}

bool SenderPort::hasValidService(const capro::CaproMessage& caproMessage)
{
    if (caproMessage.m_serviceDescription == getMembers()->m_serviceDescription)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SenderPort::disconnectAllReceiver()
{
    getMembers()->m_receiverHandler.roudiContext().removeAll();
}

void SenderPort::enableDoDeliverOnSubscription()
{
    getMembers()->m_receiverHandler.appContext().enableDoDeliverOnSubscription();
}

bool SenderPort::doesDeliverOnSubscribe() const
{
    return getMembers()->m_receiverHandler.doesDeliverOnSubscribe();
}

bool SenderPort::isPortActive() const
{
    return getMembers()->m_active.load(std::memory_order_relaxed);
}

bool SenderPort::connectReceiverPort(ReceiverPortType::MemberType_t* const receiver)
{
    return getMembers()->m_receiverHandler.roudiContext().addNewReceiver(receiver);
}

void SenderPort::disconnectReceiverPort(ReceiverPortType::MemberType_t* const receiver)
{
    getMembers()->m_receiverHandler.roudiContext().removeReceiver(receiver);
}

bool SenderPort::pushToAllocatedChunkContainer(mepoo::SharedChunk chunk)
{
    return getMembers()->m_allocatedChunksList.insert(chunk);
}

bool SenderPort::popFromAllocatedChunkContainer(mepoo::ChunkHeader* chunkHeader, mepoo::SharedChunk& chunk)
{
    return getMembers()->m_allocatedChunksList.remove(chunkHeader, chunk);
}

bool SenderPort::deleteFromAllocatedChunkContainer(mepoo::ChunkHeader* chunkHeader)
{
    mepoo::SharedChunk l_chunk(nullptr);
    return getMembers()->m_allocatedChunksList.remove(chunkHeader, l_chunk);
}

void SenderPort::clearAllocatedChunkContainer()
{
    getMembers()->m_allocatedChunksList.cleanup();
}

uint32_t SenderPort::getMaxDeliveryFiFoCapacity()
{
    return getMembers()->m_receiverHandler.getMaxDeliveryFiFoCapacity();
}

const SenderPort::MemberType_t* SenderPort::getMembers() const
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}
SenderPort::MemberType_t* SenderPort::getMembers()
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

} // namespace popo
} // namespace iox
