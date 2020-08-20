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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_PUSHER_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_PUSHER_INL

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_signaler.hpp"

namespace iox
{
namespace popo
{
template <typename ChunkQueueDataType>
inline ChunkQueuePusher<ChunkQueueDataType>::ChunkQueuePusher(
    cxx::not_null<MemberType_t* const> chunkQueueDataPtr) noexcept
    : m_chunkQueueDataPtr(chunkQueueDataPtr)
{
}

template <typename ChunkQueueDataType>
inline const typename ChunkQueuePusher<ChunkQueueDataType>::MemberType_t*
ChunkQueuePusher<ChunkQueueDataType>::getMembers() const noexcept
{
    return m_chunkQueueDataPtr;
}

template <typename ChunkQueueDataType>
inline typename ChunkQueuePusher<ChunkQueueDataType>::MemberType_t*
ChunkQueuePusher<ChunkQueueDataType>::getMembers() noexcept
{
    return m_chunkQueueDataPtr;
}

template <typename ChunkQueueDataType>
inline cxx::expected<ChunkQueueError> ChunkQueuePusher<ChunkQueueDataType>::push(mepoo::SharedChunk chunk) noexcept
{
    ChunkTuple chunkTupleIn(chunk.releaseWithRelativePtr());

    auto pushRet = getMembers()->m_queue.push(chunkTupleIn);

    if (pushRet.has_error())
    {
        // Inform the ChunkQueuePopper that our push failed
        getMembers()->m_queueHasOverflown.store(true, std::memory_order_relaxed);
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
            /// we have to set this to true to inform the higher levels that there
            /// was a chunk lost
            getMembers()->m_queueHasOverflown.store(true, std::memory_order_relaxed);
        }

        {
            typename MemberType_t::LockGuard_t lock(*getMembers());
            if (getMembers()->m_conditionVariableDataPtr)
            {
                ConditionVariableSignaler condVarSignaler(getMembers()->m_conditionVariableDataPtr.get());
                condVarSignaler.notifyOne();
            }
        }

        return cxx::success<void>();
    }
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_PUSHER_INL
