// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_PUSHER_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_PUSHER_INL


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
inline void ChunkQueuePusher<ChunkQueueDataType>::push(mepoo::SharedChunk chunk) noexcept
{
    ChunkTuple chunkTupleIn(chunk.releaseWithRelativePtr());

    auto pushRet = getMembers()->m_queue.push(chunkTupleIn);


    // drop the chunk if one is returned by an overflow
    if (pushRet.has_value())
    {
        auto chunkTupleOut = pushRet.value();
        auto chunkManagement =
            relative_ptr<mepoo::ChunkManagement>(chunkTupleOut.m_chunkOffset, chunkTupleOut.m_segmentId);
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
            if (getMembers()->m_eventVariableIndex)
            {
                EventNotifier(*reinterpret_cast<EventVariableData*>(getMembers()->m_conditionVariableDataPtr.get()),
                              *getMembers()->m_eventVariableIndex)
                    .notify();
            }
            else
            {
                ConditionVariableSignaler condVarSignaler(getMembers()->m_conditionVariableDataPtr.get());
                condVarSignaler.notifyOne();
            }
        }
    }
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_PUSHER_INL
