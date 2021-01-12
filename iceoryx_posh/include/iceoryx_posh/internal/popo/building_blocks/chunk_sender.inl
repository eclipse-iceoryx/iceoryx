// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_INL

namespace iox
{
namespace popo
{
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
ChunkSender<ChunkSenderDataType>::tryAllocate(const uint32_t payloadSize, const UniquePortId originId) noexcept
{
    // use the chunk stored in m_lastChunk if there is one, there is no other owner and the new payload still fits in it
    const uint32_t neededChunkSize = getMembers()->m_memoryMgr->sizeWithChunkHeaderStruct(payloadSize);

    if (getMembers()->m_lastChunk && getMembers()->m_lastChunk.hasNoOtherOwners()
        && (getMembers()->m_lastChunk.getChunkHeader()->chunkSize >= neededChunkSize))
    {
        if (getMembers()->m_chunksInUse.insert(getMembers()->m_lastChunk))
        {
            getMembers()->m_lastChunk.getChunkHeader()->payloadSize = payloadSize;
            return cxx::success<mepoo::ChunkHeader*>(getMembers()->m_lastChunk.getChunkHeader());
        }
        else
        {
            return cxx::error<AllocationError>(AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);
        }
    }
    else
    {
        // BEGIN of critical section, chunk will be lost if process gets hard terminated in between
        // get a new chunk
        mepoo::SharedChunk chunk = getMembers()->m_memoryMgr->getChunk(payloadSize);

        if (chunk)
        {
            // if the application allocated too much chunks, return no more chunks
            if (getMembers()->m_chunksInUse.insert(chunk))
            {
                // END of critical section, chunk will be lost if process gets hard terminated in between
                chunk.getChunkHeader()->originId = originId;
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
            return cxx::error<AllocationError>(AllocationError::RUNNING_OUT_OF_CHUNKS);
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
inline void ChunkSender<ChunkSenderDataType>::send(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // BEGIN of critical section, chunk will be lost if process gets hard terminated in between
    if (getChunkReadyForSend(chunkHeader, chunk))
    {
        this->deliverToAllStoredQueues(chunk);
        getMembers()->m_lastChunk = chunk;
    }
    // END of critical section, chunk will be lost if process gets hard terminated in between
}

template <typename ChunkSenderDataType>
inline void ChunkSender<ChunkSenderDataType>::pushToHistory(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // BEGIN of critical section, chunk will be lost if process gets hard terminated in between
    if (getChunkReadyForSend(chunkHeader, chunk))
    {
        this->addToHistoryWithoutDelivery(chunk);
        getMembers()->m_lastChunk = chunk;
    }
    // END of critical section, chunk will be lost if process gets hard terminated in between
}

template <typename ChunkSenderDataType>
inline cxx::optional<const mepoo::ChunkHeader*> ChunkSender<ChunkSenderDataType>::tryGetPreviousChunk() const noexcept
{
    if (getMembers()->m_lastChunk)
    {
        return getMembers()->m_lastChunk.getChunkHeader();
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
    getMembers()->m_lastChunk = nullptr;
}

template <typename ChunkSenderDataType>
inline bool ChunkSender<ChunkSenderDataType>::getChunkReadyForSend(const mepoo::ChunkHeader* const chunkHeader,
                                                                   mepoo::SharedChunk& chunk) noexcept
{
    if (getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        chunk.getChunkHeader()->sequenceNumber = getMembers()->m_sequenceNumber++;
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
