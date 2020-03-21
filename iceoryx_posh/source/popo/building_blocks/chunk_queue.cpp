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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace popo
{
ChunkQueue::ChunkQueue(MemberType_t* const chunkQueueDataPtr) noexcept
    : m_chunkQueueDataPtr(chunkQueueDataPtr)
{
}

const ChunkQueue::MemberType_t* ChunkQueue::getMembers() const noexcept
{
    return m_chunkQueueDataPtr;
}

ChunkQueue::MemberType_t* ChunkQueue::getMembers() noexcept
{
    return m_chunkQueueDataPtr;
}

bool ChunkQueue::push(mepoo::SharedChunk chunk) noexcept
{
    ChunkQueueData::ChunkTuple chunkTupleIn(chunk.releaseWithRelativePtr());

    return !getMembers()
                ->m_queue.push(chunkTupleIn)
                .on_success(
                    [this](cxx::expected<cxx::optional<ChunkQueueData::ChunkTuple>, cxx::VariantQueueError>& retVal) {
                        // drop the chunk if one is returned by a safe overflow
                        if (*retVal)
                        {
                            auto chunkTupleOut = **retVal;
                            auto chunkManagement = iox::relative_ptr<mepoo::ChunkManagement>(
                                chunkTupleOut.m_chunkOffset, chunkTupleOut.m_segmentId);
                            auto chunk = mepoo::SharedChunk(chunkManagement);
                        }

                        if (getMembers()->m_semaphoreAttached.load(std::memory_order_acquire)
                            && getMembers()->m_semaphore)
                        {
                            std::cout << "fubar" << std::endl;
                            getMembers()->m_semaphore->post();
                        }
                    })
                .has_error();
}

cxx::optional<mepoo::SharedChunk> ChunkQueue::pop() noexcept
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

bool ChunkQueue::empty() noexcept
{
    return getMembers()->m_queue.empty();
}

uint64_t ChunkQueue::size() noexcept
{
    return getMembers()->m_queue.size();
}

void ChunkQueue::setCapacity(const uint32_t newCapacity) noexcept
{
    getMembers()->m_queue.setCapacity(newCapacity);
}

uint64_t ChunkQueue::capacity() noexcept
{
    return getMembers()->m_queue.capacity();
}

void ChunkQueue::clear() noexcept
{
    while (getMembers()->m_queue.pop())
    {
    }
}

cxx::expected<ChunkQueueError> ChunkQueue::attachSemaphore(mepoo::SharedPointer<posix::Semaphore> semaphore) noexcept
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

bool ChunkQueue::isSemaphoreAttached() noexcept
{
    return getMembers()->m_semaphoreAttached.load(std::memory_order_relaxed);
}

} // namespace popo
} // namespace iox
