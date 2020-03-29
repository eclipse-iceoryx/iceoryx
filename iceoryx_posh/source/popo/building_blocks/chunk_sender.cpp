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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
ChunkSender::ChunkSender(MemberType_t* const chunkSenderDataPtr) noexcept
    : ChunkDistributor(chunkSenderDataPtr)
{
}

const ChunkSender::MemberType_t* ChunkSender::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(ChunkDistributor::getMembers());
}

ChunkSender::MemberType_t* ChunkSender::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(ChunkDistributor::getMembers());
}

cxx::expected<mepoo::ChunkHeader*, ChunkSenderError> ChunkSender::allocate(const uint32_t payloadSize) noexcept
{
    // use the chunk stored in m_lastChunk if there is one, there is no other owner and the new payload fits in this
    // chunk
    if (getMembers()->m_lastChunk && getMembers()->m_lastChunk.hasNoOtherOwners()
        && getMembers()->m_lastChunk.getChunkHeader()->m_info.m_usedSizeOfChunk
               >= getMembers()->m_memoryMgr->sizeWithChunkHeaderStruct(payloadSize))
    {
        if (getMembers()->m_chunksInUse.insert(getMembers()->m_lastChunk))
        {
            getMembers()->m_lastChunk.getChunkHeader()->m_info.m_payloadSize = payloadSize;
            getMembers()->m_lastChunk.getChunkHeader()->m_info.m_usedSizeOfChunk =
                getMembers()->m_memoryMgr->sizeWithChunkHeaderStruct(payloadSize);
            return cxx::success<mepoo::ChunkHeader*>(getMembers()->m_lastChunk.getChunkHeader());
        }
        else
        {
            return cxx::error<ChunkSenderError>(ChunkSenderError::TOO_MANY_CHUKS_ALLOCATED_IN_PARALLEL);
        }
    }
    else
    {
        // START of critical section, chunk will be lost if process gets hard terminated in between
        // get a new chunk
        mepoo::SharedChunk chunk = getMembers()->m_memoryMgr->getChunk(payloadSize);

        if (chunk)
        {
            // if the application allocated too much chunks, return no more chunks
            if (getMembers()->m_chunksInUse.insert(chunk))
            {
                // STOP of critical section, chunk will be lost if process gets hard terminated in between
                return cxx::success<mepoo::ChunkHeader*>(chunk.getChunkHeader());
            }
            else
            {
                // release the allocated chunk
                chunk = nullptr;
                return cxx::error<ChunkSenderError>(ChunkSenderError::TOO_MANY_CHUKS_ALLOCATED_IN_PARALLEL);
            }
        }
        else
        {
            return cxx::error<ChunkSenderError>(ChunkSenderError::RUNNING_OUT_OF_CHUNKS);
        }
    }
}

void ChunkSender::free(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    if (!getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        errorHandler(Error::kPOPO__CHUNK_SENDER_INVALID_CHUNK_TO_FREE_FROM_USER, nullptr, ErrorLevel::SEVERE);
    }
}

void ChunkSender::send(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // START of critical section, chunk will be lost if process gets hard terminated in between
    if (getChunkReadyForSend(chunkHeader, chunk))
    {
        this->deliverToAllStoredQueues(chunk);
        getMembers()->m_lastChunk = chunk;
    }
    // STOP of critical section, chunk will be lost if process gets hard terminated in between
}

void ChunkSender::pushToHistory(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // START of critical section, chunk will be lost if process gets hard terminated in between
    if (getChunkReadyForSend(chunkHeader, chunk))
    {
        this->addToHistoryWithoutDelivery(chunk);
        getMembers()->m_lastChunk = chunk;
    }
    // STOP of critical section, chunk will be lost if process gets hard terminated in between
}

bool ChunkSender::getChunkReadyForSend(mepoo::ChunkHeader* chunkHeader, mepoo::SharedChunk& chunk) noexcept
{
    if (getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        auto& chunkInfo = chunk.getChunkHeader()->m_info;
        if (!chunkInfo.m_externalSequenceNumber_bl)
        {
            chunkInfo.m_sequenceNumber = getMembers()->m_sequenceNumber;
            getMembers()->m_sequenceNumber++;
        }
        else
        {
            getMembers()->m_sequenceNumber++; // for Introspection, else nobody updates.
        }
        return true;
    }
    else
    {
        errorHandler(Error::kPOPO__CHUNK_SENDER_INVALID_CHUNK_TO_SEND_FROM_USER, nullptr, ErrorLevel::SEVERE);
        return false;
    }
}

void ChunkSender::cleanup() noexcept
{
    getMembers()->m_chunksInUse.cleanup();
    this->clearHistory();
    getMembers()->m_lastChunk = nullptr;
}

} // namespace popo
} // namespace iox
