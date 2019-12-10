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

#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include "cassert"

namespace iox
{
namespace popo
{
// BEGIN REGION_ROUDI // /* access from RouDi------------------------------------

ReceiverPort::ReceiverPort(ReceiverPortData* f_member)
    : BasePort(f_member)
{
}

cxx::optional<capro::CaproMessage> ReceiverPort::getCaProMessage()
{
    // get subscribe request from user side
    const bool l_currentSubscribeRequest = getMembers()->m_subscribeRequested.load(std::memory_order_relaxed);

    const auto l_currentSubscribeState = getMembers()->m_subscriptionState.load(std::memory_order_relaxed);

    if (l_currentSubscribeRequest && (SubscribeState::NOT_SUBSCRIBED == l_currentSubscribeState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::SUBSCRIBE_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage l_caproMessage(capro::CaproMessageType::SUB, BasePort::getMembers()->m_serviceDescription);
        l_caproMessage.m_requestPort = this->getMembers();

        return cxx::make_optional<capro::CaproMessage>(l_caproMessage);
    }
    else if (!l_currentSubscribeRequest && (SubscribeState::SUBSCRIBED == l_currentSubscribeState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::UNSUBSCRIBE_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage l_caproMessage(capro::CaproMessageType::UNSUB,
                                           BasePort::getMembers()->m_serviceDescription);
        l_caproMessage.m_requestPort = this->getMembers();

        return cxx::make_optional<capro::CaproMessage>(l_caproMessage);
    }
    else if (!l_currentSubscribeRequest && (SubscribeState::WAIT_FOR_OFFER == l_currentSubscribeState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::NOT_SUBSCRIBED, std::memory_order_relaxed);
        return cxx::nullopt_t();
    }
    else
    {
        // nothing to change
        return cxx::nullopt_t();
    }
}

cxx::optional<capro::CaproMessage> ReceiverPort::dispatchCaProMessage(capro::CaproMessage f_caProMessage)
{
    const auto l_currentSubscribeState = getMembers()->m_subscriptionState.load(std::memory_order_relaxed);

    if ((capro::CaproMessageType::OFFER == f_caProMessage.m_type)
        && (SubscribeState::WAIT_FOR_OFFER == l_currentSubscribeState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::SUBSCRIBE_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage l_caproMessage(capro::CaproMessageType::SUB, BasePort::getMembers()->m_serviceDescription);
        l_caproMessage.m_requestPort = this->getMembers();

        return cxx::make_optional<capro::CaproMessage>(l_caproMessage);
    }
    else if ((capro::CaproMessageType::STOP_OFFER == f_caProMessage.m_type)
             && (SubscribeState::SUBSCRIBED == l_currentSubscribeState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::WAIT_FOR_OFFER, std::memory_order_relaxed);

        return cxx::nullopt_t();
    }
    else if (capro::CaproMessageType::ACK == f_caProMessage.m_type)
    {
        if (SubscribeState::SUBSCRIBE_REQUESTED == l_currentSubscribeState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::SUBSCRIBED, std::memory_order_relaxed);
        }
        else if (SubscribeState::UNSUBSCRIBE_REQUESTED == l_currentSubscribeState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::NOT_SUBSCRIBED, std::memory_order_relaxed);
        }
        else
        {
            // error wrong protocol
        }

        return cxx::nullopt_t();
    }
    else if (capro::CaproMessageType::NACK == f_caProMessage.m_type)
    {
        if (SubscribeState::SUBSCRIBE_REQUESTED == l_currentSubscribeState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::WAIT_FOR_OFFER, std::memory_order_relaxed);
        }
        else if (SubscribeState::UNSUBSCRIBE_REQUESTED == l_currentSubscribeState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::NOT_SUBSCRIBED, std::memory_order_relaxed);
        }
        else
        {
            // error wrong protocol
        }

        return cxx::nullopt_t();
    }
    else
    {
        // error wrong protocol
        return cxx::nullopt_t();
    }
}

// tidy up as good as possible
// This is called from RouDi and contract is that user process is no more running
void ReceiverPort::cleanup() noexcept
{
    // unsubscribe from sender if subscribed
    unsubscribe();

    // remove all new chunks from the delivery FiFo
    clearDeliveryFiFo();

    // release all the chunks currently held by the no more present user process
    getMembers()->m_deliveredChunkList.cleanup();
}
// END REGION__ROUDI

// BEGIN REGION__APPLICATION // /* access from Application-------------------------------
void ReceiverPort::subscribe(const bool f_autoResubscribe, const uint32_t f_deliverySize)
{
    (void)f_autoResubscribe;
    subscribe(f_deliverySize);
}

void ReceiverPort::subscribe(const uint32_t f_deliverySize)
{
    if (!getMembers()->m_subscribeRequested.load(std::memory_order_relaxed))
    {
        // start with new chunks, drop old ones that could be in the queue. And we need an empty queue for resize
        clearDeliveryFiFo();

        // @todo: Add error handling for the cases where resize on non-empty
        // deliveryFiFo could fail in case the size exceeds INTERNAL_SOFI_SIZE.
        // Senders ports will have no response in the deliver call while we unsubscribe or
        // resubscribe
        // @todo limitation to only resize when initial subscribe
        getMembers()->m_deliveryFiFo.resize(f_deliverySize);

        getMembers()->m_subscribeRequested.store(true, std::memory_order_relaxed);
    }
}

void ReceiverPort::unsubscribe()
{
    if (getMembers()->m_subscribeRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_subscribeRequested.store(false, std::memory_order_relaxed);
    }
}

bool ReceiverPort::isSubscribed() const
{
    const auto l_currentSubscribeState = getMembers()->m_subscriptionState.load(std::memory_order_relaxed);
    return ((SubscribeState::SUBSCRIBED == l_currentSubscribeState)
            || (SubscribeState::UNSUBSCRIBE_REQUESTED == l_currentSubscribeState));
}

SubscribeState ReceiverPort::getSubscribeState() const
{
    return getMembers()->m_subscriptionState.load(std::memory_order_relaxed);
}

bool ReceiverPort::getChunk(const mepoo::ChunkHeader*& f_chunkHeader) noexcept
{
    mepoo::SharedChunk l_chunk;

    if (getMembers()->m_deliveryFiFo.pop(l_chunk))
    {
        // store the chunk that is provided to the user side
        if (getMembers()->m_deliveredChunkList.insert(l_chunk))
        {
            f_chunkHeader = l_chunk.getChunkHeader();
            return true;
        }
        else
        {
            // release the received chunk
            l_chunk = nullptr;
            assert(false && "deliveredChunkList overflow");
            return false;
        }
    }
    else
    {
        // no new chunk
        return false;
    }
}

// direct access to the received chunks
bool ReceiverPort::getSharedChunk(mepoo::SharedChunk& f_chunk)
{
    return getMembers()->m_deliveryFiFo.pop(f_chunk);
}

bool ReceiverPort::newData() noexcept
{
    return !getMembers()->m_deliveryFiFo.empty();
}

bool ReceiverPort::releaseChunk(const mepoo::ChunkHeader* f_chunkHeader)
{
    mepoo::SharedChunk l_chunk(nullptr);

    if (getMembers()->m_deliveredChunkList.remove(f_chunkHeader, l_chunk))
    {
        return true;
    }
    else
    {
        assert(false && "Application provided invalid chunk pointer to free");
        return false;
    }
}

void ReceiverPort::clearDeliveryFiFo()
{
    mepoo::SharedChunk l_crap{nullptr};
    while (getMembers()->m_deliveryFiFo.pop(l_crap))
    {
    }
}

/* expects an initialized POSIX semaphore, stored in shared memory! */
void ReceiverPort::SetCallbackReferences(posix::Semaphore* f_callbackSemaphore,
                                         std::atomic<ChunksCounterType>*) noexcept
{
    ///  @todo is this a method that will be called concurrent?
    std::lock_guard<mutex_t> g(*getMembers()->m_chunkSendCallbackMutex);

    assert((getMembers()->m_chunkSendCallbackActive.load(std::memory_order_relaxed) == false)
           && "SetCallbackReferences: callback semaphore already set - please Unset first.");
    getMembers()->m_chunkSendSemaphore = f_callbackSemaphore;
    getMembers()->m_chunkSendCallbackActive.store(true, std::memory_order_release);
}

void ReceiverPort::UnsetCallbackReferences() noexcept
{
    std::lock_guard<mutex_t> g(*getMembers()->m_chunkSendCallbackMutex);

    getMembers()->m_chunkSendCallbackActive.store(false, std::memory_order_release);
    getMembers()->m_chunkSendSemaphore = nullptr;
}

bool ReceiverPort::AreCallbackReferencesSet()
{
    return getMembers()->m_chunkSendCallbackActive.load(std::memory_order_relaxed);
}

// offer a 'local' semaphore, stored in shared memory, that can be used with
// 'SetReceiveSemaphore(sem_t*)'
posix::Semaphore* ReceiverPort::GetShmSemaphore()
{
    if (getMembers()->m_shmSemaphore.has_error())
    {
        getMembers()->m_shmSemaphore = std::move(posix::Semaphore::create(&getMembers()->m_shmSemaphoreHandle, 0));
        if (getMembers()->m_shmSemaphore.has_error())
        {
            return nullptr;
        }
    }
    return &getMembers()->m_shmSemaphore.get_value();
}

bool ReceiverPort::deliver(mepoo::SharedChunk f_chunk_p) noexcept
{
    if (SubscribeState::SUBSCRIBED == getMembers()->m_subscriptionState.load(std::memory_order_relaxed))
    {
        // check this first since it will be the case occuring most often
        // we can continue below with the regular delivery logic
        // (this is why this statement block is empty)
    }
    else if (getMembers()->m_subscribeRequested.load(std::memory_order_relaxed))
    {
        // if a subscribe request is out and the sender delivers a chunk it is
        // possible that roudi put the receiver in the receiver list of the
        // sender but did not yet set the receiver state to subscribed. this is
        // some kind of race and the solution is to set the subscription state
        // to subscribed. We encountered an error where the receiver got a
        // chunk, the receiverCallback was activated but since the receiver
        // was not yet subscribed no chunks where available for the user.
        getMembers()->m_subscriptionState.store(SubscribeState::SUBSCRIBED, std::memory_order_relaxed);
    }
    else
    {
        // state was neither SUBSCRIPTION_REQUESTED nor SUBSCRIBED, do nothing
        return false;
    }

    mepoo::SharedChunk l_chunk{nullptr};

    getMembers()->m_deliveryFiFo.push(std::move(f_chunk_p), l_chunk);

    // check for registered event callback handler - trigger if existing
    // note that we also call in the overflow case of a push above
    if (getMembers()->m_chunkSendCallbackActive.load(std::memory_order_acquire))
    {
        if (getMembers()->m_chunkSendSemaphore != nullptr)
        {
            getMembers()->m_chunkSendSemaphore->post();
        }
    }

    return true;
}

uint64_t ReceiverPort::getDeliveryFiFoCapacity() const
{
    return getMembers()->m_deliveryFiFo.getCapacity();
}

uint64_t ReceiverPort::getDeliveryFiFoSize() const
{
    return getMembers()->m_deliveryFiFo.getSize();
}

const ReceiverPort::MemberType_t* ReceiverPort::getMembers() const
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ReceiverPort::MemberType_t* ReceiverPort::getMembers()
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

} // namespace popo
} // namespace iox
