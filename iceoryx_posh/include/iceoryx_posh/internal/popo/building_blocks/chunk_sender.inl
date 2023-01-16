// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_INL

#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"

namespace iox
{
namespace cxx
{
template <>
constexpr popo::AllocationError
from<mepoo::MemoryManager::Error, popo::AllocationError>(const mepoo::MemoryManager::Error error) noexcept
{
    switch (error)
    {
    case mepoo::MemoryManager::Error::NO_MEMPOOLS_AVAILABLE:
        return popo::AllocationError::NO_MEMPOOLS_AVAILABLE;
    case mepoo::MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE:
        return popo::AllocationError::NO_MEMPOOLS_AVAILABLE;
    case mepoo::MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS:
        return popo::AllocationError::RUNNING_OUT_OF_CHUNKS;
    }
    return popo::AllocationError::UNDEFINED_ERROR;
}
} // namespace cxx

namespace popo
{
inline constexpr const char* asStringLiteral(const AllocationError value) noexcept
{
    switch (value)
    {
    case AllocationError::UNDEFINED_ERROR:
        return "AllocationError::UNDEFINED_ERROR";
    case AllocationError::NO_MEMPOOLS_AVAILABLE:
        return "AllocationError::NO_MEMPOOLS_AVAILABLE";
    case AllocationError::RUNNING_OUT_OF_CHUNKS:
        return "AllocationError::RUNNING_OUT_OF_CHUNKS";
    case AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL:
        return "AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL";
    case AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER:
        return "AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER";
    case AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER:
        return "AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER";
    }

    return "[Undefined AllocationError]";
}

inline std::ostream& operator<<(std::ostream& stream, AllocationError value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

inline log::LogStream& operator<<(log::LogStream& stream, AllocationError value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

template <typename ChunkSenderDataType>
inline ChunkSender<ChunkSenderDataType>::ChunkSender(cxx::not_null<MemberType_t* const> chunkSenderDataPtr) noexcept
    : Base_t(static_cast<typename ChunkSenderDataType::ChunkDistributorData_t* const>(chunkSenderDataPtr))
{
}

template <typename ChunkSenderDataType>
inline const typename ChunkSender<ChunkSenderDataType>::MemberType_t*
ChunkSender<ChunkSenderDataType>::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(Base_t::getMembers());
}

template <typename ChunkSenderDataType>
inline typename ChunkSender<ChunkSenderDataType>::MemberType_t* ChunkSender<ChunkSenderDataType>::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(Base_t::getMembers());
}

template <typename ChunkSenderDataType>
inline cxx::expected<mepoo::ChunkHeader*, AllocationError>
ChunkSender<ChunkSenderDataType>::tryAllocate(const UniquePortId originId,
                                              const uint32_t userPayloadSize,
                                              const uint32_t userPayloadAlignment,
                                              const uint32_t userHeaderSize,
                                              const uint32_t userHeaderAlignment) noexcept
{
    // use the chunk stored in m_lastChunkUnmanaged if:
    //   - there is a valid chunk
    //   - there is no other owner
    //   - the new user-payload still fits in it
    const auto chunkSettingsResult =
        mepoo::ChunkSettings::create(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);
    if (chunkSettingsResult.has_error())
    {
        return cxx::error<AllocationError>(AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER);
    }

    const auto& chunkSettings = chunkSettingsResult.value();
    const uint32_t requiredChunkSize = chunkSettings.requiredChunkSize();

    auto& lastChunkUnmanaged = getMembers()->m_lastChunkUnmanaged;
    mepoo::ChunkHeader* lastChunkChunkHeader =
        lastChunkUnmanaged.isNotLogicalNullptrAndHasNoOtherOwners() ? lastChunkUnmanaged.getChunkHeader() : nullptr;

    if (lastChunkChunkHeader && (lastChunkChunkHeader->chunkSize() >= requiredChunkSize))
    {
        auto sharedChunk = lastChunkUnmanaged.cloneToSharedChunk();
        if (getMembers()->m_chunksInUse.insert(sharedChunk))
        {
            auto chunkSize = lastChunkChunkHeader->chunkSize();
            lastChunkChunkHeader->~ChunkHeader();
            new (lastChunkChunkHeader) mepoo::ChunkHeader(chunkSize, chunkSettings);
            lastChunkChunkHeader->setOriginId(originId);
            return cxx::success<mepoo::ChunkHeader*>(lastChunkChunkHeader);
        }
        else
        {
            return cxx::error<AllocationError>(AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);
        }
    }
    else
    {
        // BEGIN of critical section, chunk will be lost if the process terminates in this section
        // get a new chunk
        auto getChunkResult = getMembers()->m_memoryMgr->getChunk(chunkSettings);

        if (!getChunkResult.has_error())
        {
            auto& chunk = getChunkResult.value();

            // if the application allocated too much chunks, return no more chunks
            if (getMembers()->m_chunksInUse.insert(chunk))
            {
                // END of critical section
                chunk.getChunkHeader()->setOriginId(originId);
                return cxx::success<mepoo::ChunkHeader*>(chunk.getChunkHeader());
            }
            else
            {
                // release the allocated chunk
                chunk = nullptr;
                return cxx::error<AllocationError>(AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);
            }
        }
        else
        {
            /// @todo iox-#1012 use cxx::error<E2>::from(E1); once available
            return cxx::error<AllocationError>(cxx::into<AllocationError>(getChunkResult.get_error()));
        }
    }
}

template <typename ChunkSenderDataType>
inline void ChunkSender<ChunkSenderDataType>::release(const mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // PRQA S 4127 1 # d'tor of SharedChunk will release the memory, we do not have to touch the returned chunk
    if (!getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        errorHandler(Error::kPOPO__CHUNK_SENDER_INVALID_CHUNK_TO_FREE_FROM_USER, nullptr, ErrorLevel::SEVERE);
    }
}

template <typename ChunkSenderDataType>
inline uint64_t ChunkSender<ChunkSenderDataType>::send(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    uint64_t numberOfReceiverTheChunkWasDelivered{0};
    mepoo::SharedChunk chunk(nullptr);
    // BEGIN of critical section, chunk will be lost if the process terminates in this section
    if (getChunkReadyForSend(chunkHeader, chunk))
    {
        numberOfReceiverTheChunkWasDelivered = this->deliverToAllStoredQueues(chunk);

        getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
        getMembers()->m_lastChunkUnmanaged = chunk;
    }
    // END of critical section

    return numberOfReceiverTheChunkWasDelivered;
}

template <typename ChunkSenderDataType>
inline bool ChunkSender<ChunkSenderDataType>::sendToQueue(mepoo::ChunkHeader* const chunkHeader,
                                                          const cxx::UniqueId uniqueQueueId,
                                                          const uint32_t lastKnownQueueIndex) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // BEGIN of critical section, chunk will be lost if the process terminates in this section
    if (getChunkReadyForSend(chunkHeader, chunk))
    {
        auto deliveryResult = this->deliverToQueue(uniqueQueueId, lastKnownQueueIndex, chunk);

        getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
        getMembers()->m_lastChunkUnmanaged = chunk;

        return !deliveryResult.has_error();
    }
    // END of critical section

    return false;
}

template <typename ChunkSenderDataType>
inline void ChunkSender<ChunkSenderDataType>::pushToHistory(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // BEGIN of critical section, chunk will be lost if the process terminates in this section
    if (getChunkReadyForSend(chunkHeader, chunk))
    {
        this->addToHistoryWithoutDelivery(chunk);

        getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
        getMembers()->m_lastChunkUnmanaged = chunk;
    }
    // END of critical section
}

template <typename ChunkSenderDataType>
inline cxx::optional<const mepoo::ChunkHeader*> ChunkSender<ChunkSenderDataType>::tryGetPreviousChunk() const noexcept
{
    if (!getMembers()->m_lastChunkUnmanaged.isLogicalNullptr())
    {
        return getMembers()->m_lastChunkUnmanaged.getChunkHeader();
    }
    else
    {
        return cxx::nullopt_t();
    }
}

template <typename ChunkSenderDataType>
inline void ChunkSender<ChunkSenderDataType>::releaseAll() noexcept
{
    getMembers()->m_chunksInUse.cleanup();
    this->cleanup();
    getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
}

template <typename ChunkSenderDataType>
inline bool ChunkSender<ChunkSenderDataType>::getChunkReadyForSend(const mepoo::ChunkHeader* const chunkHeader,
                                                                   mepoo::SharedChunk& chunk) noexcept
{
    if (getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        chunk.getChunkHeader()->setSequenceNumber(getMembers()->m_sequenceNumber++);
        return true;
    }
    else
    {
        errorHandler(Error::kPOPO__CHUNK_SENDER_INVALID_CHUNK_TO_SEND_FROM_USER, nullptr, ErrorLevel::SEVERE);
        return false;
    }
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_INL
