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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
ChunkReceiver::ChunkReceiver(MemberType_t* const chunkReceiverDataPtr) noexcept
    : ChunkQueuePopper(chunkReceiverDataPtr)
{
}

const ChunkReceiver::MemberType_t* ChunkReceiver::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(ChunkQueuePopper::getMembers());
}

ChunkReceiver::MemberType_t* ChunkReceiver::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(ChunkQueuePopper::getMembers());
}

cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiverError> ChunkReceiver::get() noexcept
{
    auto popRet = this->pop();

    if (popRet.has_value())
    {
        auto sharedChunk = *popRet;

        // if the application holds too many chunks, don't provide more
        if (getMembers()->m_chunksInUse.insert(sharedChunk))
        {
            return cxx::success<cxx::optional<const mepoo::ChunkHeader*>>(sharedChunk.getChunkHeader());
        }
        else
        {
            // release the chunk
            sharedChunk = nullptr;
            return cxx::error<ChunkReceiverError>(ChunkReceiverError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL);
        }
    }
    else
    {
        // no new chunk
        return cxx::success<cxx::optional<const mepoo::ChunkHeader*>>(cxx::nullopt_t());
    }
}

void ChunkReceiver::release(const mepoo::ChunkHeader* chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    if (!getMembers()->m_chunksInUse.remove(chunkHeader, chunk))
    {
        errorHandler(Error::kPOPO__CHUNK_RECEIVER_INVALID_CHUNK_TO_RELEASE_FROM_USER, nullptr, ErrorLevel::SEVERE);
    }
}

void ChunkReceiver::releaseAll() noexcept
{
    getMembers()->m_chunksInUse.cleanup();
    this->clear();
}

} // namespace popo
} // namespace iox
