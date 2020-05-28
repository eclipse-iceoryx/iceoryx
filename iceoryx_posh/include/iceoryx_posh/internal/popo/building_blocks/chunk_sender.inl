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

namespace iox
{
namespace popo
{
template <typename ChunkDistributorType>
inline ChunkSender<ChunkDistributorType>::ChunkSender(cxx::not_null<MemberType_t* const> chunkSenderDataPtr) noexcept
    : ChunkDistributorType(static_cast<typename ChunkDistributorType::MemberType_t* const>(chunkSenderDataPtr))
{
}

template <typename ChunkDistributorType>
inline const typename ChunkSender<ChunkDistributorType>::MemberType_t*
ChunkSender<ChunkDistributorType>::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(ChunkDistributorType::getMembers());
}

template <typename ChunkDistributorType>
inline typename ChunkSender<ChunkDistributorType>::MemberType_t*
ChunkSender<ChunkDistributorType>::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(ChunkDistributorType::getMembers());
}

template <typename ChunkDistributorType>
inline cxx::expected<mepoo::ChunkHeader*, ChunkSenderError>
ChunkSender<ChunkDistributorType>::allocate(const uint32_t payloadSize) noexcept
{
    // use the chunk stored in m_lastChunk if there is one, there is no other owner and the new payload still fits in it
    uint32_t neededChunkSize = getMembers()->m_memoryMgr->sizeWithChunkHeaderStruct(payloadSize);

    if (getMembers()->m_lastChunk && getMembers()->m_lastChunk.hasNoOtherOwners()
        && getMembers()->m_lastChunk.getChunkHeader()->m_info.m_totalSizeOfChunk >= neededChunkSize)
    {
        if (getMembers()->m_chunksInUse.insert(getMembers()->m_lastChunk))
        {
            getMembers()->m_lastChunk.getChunkHeader()->m_info.m_payloadSize = payloadSize;
            getMembers()->m_lastChunk.getChunkHeader()->m_info.m_usedSizeOfChunk = neededChunkSize;
            return cxx::success<mepoo::ChunkHeader*>(getMembers()->m_lastChunk.getChunkHeader());
        }
        else
        {
            return cxx::error<ChunkSenderError>(ChunkSenderError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);
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
                return cxx::success<mepoo::ChunkHeader*>(chunk.getChunkHeader());
            }
            else
            {
                // release the allocated chunk
                chunk = nullptr;
                return cxx::error<ChunkSenderError>(ChunkSenderError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);
            }
        }
        else
        {
            return cxx::error<ChunkSenderError>(ChunkSenderError::RUNNING_OUT_OF_CHUNKS);
        }
    }
}

template <typename ChunkDistributorType>
inline void ChunkSender<ChunkDistributorType>::free(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    if (!getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        errorHandler(Error::kPOPO__CHUNK_SENDER_INVALID_CHUNK_TO_FREE_FROM_USER, nullptr, ErrorLevel::SEVERE);
    }
}

template <typename ChunkDistributorType>
inline void ChunkSender<ChunkDistributorType>::send(mepoo::ChunkHeader* const chunkHeader) noexcept
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

template <typename ChunkDistributorType>
inline void ChunkSender<ChunkDistributorType>::pushToHistory(mepoo::ChunkHeader* const chunkHeader) noexcept
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

template <typename ChunkDistributorType>
inline cxx::optional<const mepoo::ChunkHeader*> ChunkSender<ChunkDistributorType>::getLast() const noexcept
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

template <typename ChunkDistributorType>
inline void ChunkSender<ChunkDistributorType>::releaseAll() noexcept
{
    getMembers()->m_chunksInUse.cleanup();
    this->cleanup();
    getMembers()->m_lastChunk = nullptr;
}

template <typename ChunkDistributorType>
inline bool ChunkSender<ChunkDistributorType>::getChunkReadyForSend(mepoo::ChunkHeader* chunkHeader,
                                                                    mepoo::SharedChunk& chunk) noexcept
{
    if (getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        auto& chunkInfo = chunk.getChunkHeader()->m_info;
        if (!chunkInfo.m_externalSequenceNumber_bl)
        {
            // if the sequence number is NOT set by the user, we take the one from the chunk sender for the chunk info
            chunkInfo.m_sequenceNumber = getMembers()->m_sequenceNumber;
            getMembers()->m_sequenceNumber++;
        }
        else
        {
            // if the seqence number in the chunk info is set by the user, we still increment the internal one as this
            // might be needed by midddleware spcific evaluation (like in introspection)
            getMembers()->m_sequenceNumber++;
        }
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
