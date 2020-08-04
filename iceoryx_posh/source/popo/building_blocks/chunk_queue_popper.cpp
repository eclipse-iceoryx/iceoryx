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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace popo
{
ChunkQueuePopper::ChunkQueuePopper(cxx::not_null<MemberType_t* const> chunkQueueDataPtr) noexcept
    : m_chunkQueueDataPtr(chunkQueueDataPtr)
{
}

const ChunkQueuePopper::MemberType_t* ChunkQueuePopper::getMembers() const noexcept
{
    return m_chunkQueueDataPtr;
}

ChunkQueuePopper::MemberType_t* ChunkQueuePopper::getMembers() noexcept
{
    return m_chunkQueueDataPtr;
}

cxx::optional<mepoo::SharedChunk> ChunkQueuePopper::pop() noexcept
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

bool ChunkQueuePopper::hasOverflown() noexcept
{
    if (getMembers()->m_queueHasOverflown.load(std::memory_order_relaxed))
    {
        getMembers()->m_queueHasOverflown.store(false, std::memory_order_relaxed);
        return true;
    }
    return false;
}

bool ChunkQueuePopper::empty() noexcept
{
    return getMembers()->m_queue.empty();
}

uint64_t ChunkQueuePopper::size() noexcept
{
    return getMembers()->m_queue.size();
}

void ChunkQueuePopper::setCapacity(const uint64_t newCapacity) noexcept
{
    /// @todo fix getCapacity and setCapacity issue in queues (uint32 vs uint64)
    // this needs to be properly fixed by harmonizing the types across the functions, but currently this cast is also
    // sufficient
    getMembers()->m_queue.setCapacity(
        static_cast<std::remove_const<decltype(MemberType_t::MAX_CAPACITY)>::type>(newCapacity));
}

uint64_t ChunkQueuePopper::getCurrentCapacity() const noexcept
{
    return getMembers()->m_queue.capacity();
}

uint64_t ChunkQueuePopper::getMaximumCapacity() const noexcept
{
    return MemberType_t::MAX_CAPACITY;
}

void ChunkQueuePopper::clear() noexcept
{
    do
    {
        auto retVal = getMembers()->m_queue.pop();
        if (retVal.has_value())
        {
            // PRQA S 4117 4 # d'tor of SharedChunk will release the memory, so RAII has the side effect here
            auto chunkTupleOut = *retVal;
            auto chunkManagement =
                iox::relative_ptr<mepoo::ChunkManagement>(chunkTupleOut.m_chunkOffset, chunkTupleOut.m_segmentId);
            auto chunk = mepoo::SharedChunk(chunkManagement);
        }
        else
        {
            break;
        }
    } while (true);
}

bool ChunkQueuePopper::attachConditionVariableSignaler(ConditionVariableData* conditionVariableDataPtr) noexcept
{
    /// @todo Add lock guard here, use smart_lock
    if (isConditionVariableSignalerAttached())
    {
        LogWarn() << "Condition variable signaler already set. Attaching a second time will be ignored!";
        return false;
    }
    else
    {
        getMembers()->m_conditionVariableDataPtr = conditionVariableDataPtr;
        getMembers()->m_conditionVariableAttached.store(true, std::memory_order_release);
        return true;
    }
}

bool ChunkQueuePopper::detachConditionVariableSignaler() noexcept
{
    /// @todo Add lock guard here, use smart_lock
    if (isConditionVariableSignalerAttached())
    {
        getMembers()->m_conditionVariableDataPtr = nullptr;
        getMembers()->m_conditionVariableAttached.store(false, std::memory_order_release);
        return true;
    }
    else
    {
        LogWarn() << "Condition variable signaler not set yet.";
        return false;
    }
}

bool ChunkQueuePopper::isConditionVariableSignalerAttached() const noexcept
{
    return getMembers()->m_conditionVariableAttached.load(std::memory_order_relaxed);
}

} // namespace popo
} // namespace iox
