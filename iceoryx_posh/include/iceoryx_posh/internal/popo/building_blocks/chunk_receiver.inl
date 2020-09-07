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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_RECEIVER_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_RECEIVER_INL

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
template <typename ChunkQueuePopperType>
inline ChunkReceiver<ChunkQueuePopperType>::ChunkReceiver(
    cxx::not_null<MemberType_t* const> chunkReceiverDataPtr) noexcept
    : ChunkQueuePopperType(static_cast<typename ChunkQueuePopperType::MemberType_t*>(chunkReceiverDataPtr))
{
}

template <typename ChunkQueuePopperType>
inline const typename ChunkReceiver<ChunkQueuePopperType>::MemberType_t*
ChunkReceiver<ChunkQueuePopperType>::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(ChunkQueuePopperType::getMembers());
}

template <typename ChunkQueuePopperType>
inline typename ChunkReceiver<ChunkQueuePopperType>::MemberType_t*
ChunkReceiver<ChunkQueuePopperType>::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(ChunkQueuePopperType::getMembers());
}

template <typename ChunkQueuePopperType>
inline cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiveError>
ChunkReceiver<ChunkQueuePopperType>::get() noexcept
{
    auto popRet = this->pop();

    if (popRet.has_value())
    {
        auto sharedChunk = *popRet;

        // if the application holds too many chunks, don't provide more
        if (getMembers()->m_chunksInUse.insert(sharedChunk))
        {
            return cxx::success<cxx::optional<const mepoo::ChunkHeader*>>(
                const_cast<const mepoo::ChunkHeader*>(sharedChunk.getChunkHeader()));
        }
        else
        {
            // release the chunk
            sharedChunk = nullptr;
            return cxx::error<ChunkReceiveError>(ChunkReceiveError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL);
        }
    }
    else
    {
        // no new chunk
        return cxx::success<cxx::optional<const mepoo::ChunkHeader*>>(cxx::nullopt_t());
    }
}

template <typename ChunkQueuePopperType>
inline void ChunkReceiver<ChunkQueuePopperType>::release(const mepoo::ChunkHeader* const chunkHeader) noexcept
{
    mepoo::SharedChunk chunk(nullptr);
    // PRQA S 4127 1 # d'tor of SharedChunk will release the memory, we do not have to touch the returned chunk
    if (!getMembers()->m_chunksInUse.remove(chunkHeader, chunk)) // PRQA S 4127
    {
        errorHandler(Error::kPOPO__CHUNK_RECEIVER_INVALID_CHUNK_TO_RELEASE_FROM_USER, nullptr, ErrorLevel::SEVERE);
    }
}

template <typename ChunkQueuePopperType>
inline void ChunkReceiver<ChunkQueuePopperType>::releaseAll() noexcept
{
    getMembers()->m_chunksInUse.cleanup();
    this->clear();
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_RECEIVER_INL
