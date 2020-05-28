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
    if (retVal)
    {
        auto chunkTupleOut = *retVal;
        auto chunkManagement =
            iox::relative_ptr<mepoo::ChunkManagement>(chunkTupleOut.m_chunkOffset, chunkTupleOut.m_segmentId);
        auto chunk = mepoo::SharedChunk(chunkManagement);
        return chunk;
    }
    else
    {
        return cxx::nullopt_t();
    }
}

bool ChunkQueuePopper::hasOverflown() noexcept
{
    if(getMembers()->m_queueHasOverflown)
    {
        getMembers()->m_queueHasOverflown = false;
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

void ChunkQueuePopper::setCapacity(const uint32_t newCapacity) noexcept
{
    getMembers()->m_queue.setCapacity(newCapacity);
}

uint64_t ChunkQueuePopper::capacity() noexcept
{
    return getMembers()->m_queue.capacity();
}

void ChunkQueuePopper::clear() noexcept
{
    do
    {
        auto retVal = getMembers()->m_queue.pop();
        if (retVal)
        {
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

cxx::expected<ChunkQueueError>
ChunkQueuePopper::attachSemaphore(mepoo::SharedPointer<posix::Semaphore> semaphore) noexcept
{
    if (isSemaphoreAttached())
    {
        LogWarn() << "Semaphore already set. Attaching the semaphore a second time will be ignored!";
        return cxx::error<ChunkQueueError>(ChunkQueueError::SEMAPHORE_ALREADY_SET);
    }
    else
    {
        getMembers()->m_semaphore = semaphore;
        getMembers()->m_semaphoreAttached.store(true, std::memory_order_release);
        return cxx::success<void>();
    }
}

bool ChunkQueuePopper::isSemaphoreAttached() noexcept
{
    return getMembers()->m_semaphoreAttached.load(std::memory_order_relaxed);
}

} // namespace popo
} // namespace iox
