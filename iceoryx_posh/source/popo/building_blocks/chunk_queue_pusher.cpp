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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace popo
{
ChunkQueuePusher::ChunkQueuePusher(cxx::not_null<MemberType_t* const> chunkQueueDataPtr) noexcept
    : m_chunkQueueDataPtr(chunkQueueDataPtr)
{
}

const ChunkQueuePusher::MemberType_t* ChunkQueuePusher::getMembers() const noexcept
{
    return m_chunkQueueDataPtr;
}

ChunkQueuePusher::MemberType_t* ChunkQueuePusher::getMembers() noexcept
{
    return m_chunkQueueDataPtr;
}

cxx::expected<ChunkQueueError> ChunkQueuePusher::push(mepoo::SharedChunk chunk) noexcept
{
    ChunkTuple chunkTupleIn(chunk.releaseWithRelativePtr());

    auto pushRet = getMembers()->m_queue.push(chunkTupleIn);

    if (pushRet.has_error())
    {
        // Inform the ChunkQueuePopper that our push failed
        getMembers()->m_queueHasOverflown = true;
        return cxx::error<ChunkQueueError>(ChunkQueueError::QUEUE_OVERFLOW);
    }
    else
    {
        // drop the chunk if one is returned by a safe overflow
        if ((*pushRet).has_value())
        {
            auto chunkTupleOut = **pushRet;
            auto chunkManagement =
                iox::relative_ptr<mepoo::ChunkManagement>(chunkTupleOut.m_chunkOffset, chunkTupleOut.m_segmentId);
            // this will release the chunk
            auto returnedChunk = mepoo::SharedChunk(chunkManagement);
        }

        if (getMembers()->m_semaphoreAttached.load(std::memory_order_acquire) && getMembers()->m_semaphore)
        {
            getMembers()->m_semaphore->post();
        }

        return cxx::success<void>();
    }
}

} // namespace popo
} // namespace iox
