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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_POPPER_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_POPPER_INL

#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace popo
{
template <typename ChunkQueueDataType>
inline ChunkQueuePopper<ChunkQueueDataType>::ChunkQueuePopper(
    cxx::not_null<MemberType_t* const> chunkQueueDataPtr) noexcept
    : m_chunkQueueDataPtr(chunkQueueDataPtr)
{
}

template <typename ChunkQueueDataType>
inline const typename ChunkQueuePopper<ChunkQueueDataType>::MemberType_t*
ChunkQueuePopper<ChunkQueueDataType>::getMembers() const noexcept
{
    return m_chunkQueueDataPtr;
}

template <typename ChunkQueueDataType>
inline typename ChunkQueuePopper<ChunkQueueDataType>::MemberType_t*
ChunkQueuePopper<ChunkQueueDataType>::getMembers() noexcept
{
    return m_chunkQueueDataPtr;
}

template <typename ChunkQueueDataType>
inline cxx::optional<mepoo::SharedChunk> ChunkQueuePopper<ChunkQueueDataType>::pop() noexcept
{
    auto retVal = getMembers()->m_queue.pop();

    // check if queue had an element that was poped and return if so
    if (retVal.has_value())
    {
        auto chunkTupleOut = *retVal;
        auto chunkManagement =
            iox::relative_ptr<mepoo::ChunkManagement>(chunkTupleOut.m_chunkOffset, chunkTupleOut.m_segmentId);
        auto chunk = mepoo::SharedChunk(chunkManagement);
        return cxx::make_optional<mepoo::SharedChunk>(chunk);
    }
    else
    {
        return cxx::nullopt_t();
    }
}

template <typename ChunkQueueDataType>
inline bool ChunkQueuePopper<ChunkQueueDataType>::hasOverflown() noexcept
{
    if (getMembers()->m_queueHasOverflown.load(std::memory_order_relaxed))
    {
        getMembers()->m_queueHasOverflown.store(false, std::memory_order_relaxed);
        return true;
    }
    return false;
}

template <typename ChunkQueueDataType>
inline bool ChunkQueuePopper<ChunkQueueDataType>::empty() noexcept
{
    return getMembers()->m_queue.empty();
}

template <typename ChunkQueueDataType>
inline uint64_t ChunkQueuePopper<ChunkQueueDataType>::size() noexcept
{
    return getMembers()->m_queue.size();
}

template <typename ChunkQueueDataType>
inline void ChunkQueuePopper<ChunkQueueDataType>::setCapacity(const uint64_t newCapacity) noexcept
{
    getMembers()->m_queue.setCapacity(newCapacity);
}

template <typename ChunkQueueDataType>
inline uint64_t ChunkQueuePopper<ChunkQueueDataType>::getCurrentCapacity() const noexcept
{
    return getMembers()->m_queue.capacity();
}

template <typename ChunkQueueDataType>
inline uint64_t ChunkQueuePopper<ChunkQueueDataType>::getMaximumCapacity() const noexcept
{
    return MemberType_t::MAX_CAPACITY;
}

template <typename ChunkQueueDataType>
inline void ChunkQueuePopper<ChunkQueueDataType>::clear() noexcept
{
    while (auto maybeChunkTuple = getMembers()->m_queue.pop())
    {
        // PRQA S 4117 4 # d'tor of SharedChunk will release the memory, so RAII has the side effect here
        auto chunkTupleOut = maybeChunkTuple.value();
        auto chunkManagement =
            iox::relative_ptr<mepoo::ChunkManagement>(chunkTupleOut.m_chunkOffset, chunkTupleOut.m_segmentId);
        auto chunk = mepoo::SharedChunk(chunkManagement);
    }
}

template <typename ChunkQueueDataType>
inline bool
ChunkQueuePopper<ChunkQueueDataType>::attachConditionVariable(ConditionVariableData* conditionVariableDataPtr) noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    if (isConditionVariableAttached())
    {
        LogWarn() << "Condition variable signaler already set. Attaching a second time will be ignored!";
        return false;
    }
    else
    {
        getMembers()->m_conditionVariableDataPtr = conditionVariableDataPtr;
        return true;
    }
}

template <typename ChunkQueueDataType>
inline bool ChunkQueuePopper<ChunkQueueDataType>::detachConditionVariable() noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    if (isConditionVariableAttached())
    {
        getMembers()->m_conditionVariableDataPtr = nullptr;
        return true;
    }
    else
    {
        LogWarn() << "Condition variable signaler not set yet.";
        return false;
    }
}

template <typename ChunkQueueDataType>
inline bool ChunkQueuePopper<ChunkQueueDataType>::isConditionVariableAttached() const noexcept
{
    return getMembers()->m_conditionVariableDataPtr;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_PUSHER_INL
