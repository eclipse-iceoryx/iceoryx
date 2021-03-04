// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/concurrent/resizeable_lockfree_queue.hpp"

namespace iox
{
namespace concurrent
{
template <typename ElementType, uint64_t MaxCapacity>
ResizeableLockFreeQueue<ElementType, MaxCapacity>::ResizeableLockFreeQueue(const uint64_t initialCapacity) noexcept
{
    setCapacity(initialCapacity);
}

template <typename ElementType, uint64_t MaxCapacity>
constexpr uint64_t ResizeableLockFreeQueue<ElementType, MaxCapacity>::maxCapacity() noexcept
{
    return MAX_CAPACITY;
}

template <typename ElementType, uint64_t MaxCapacity>
uint64_t ResizeableLockFreeQueue<ElementType, MaxCapacity>::capacity() const noexcept
{
    return m_capacity.load(std::memory_order_relaxed);
}


template <typename ElementType, uint64_t MaxCapacity>
bool ResizeableLockFreeQueue<ElementType, MaxCapacity>::setCapacity(const uint64_t newCapacity) noexcept
{
    auto removeHandler = [](const ElementType&) {};
    return setCapacity(newCapacity, removeHandler);
}

template <typename ElementType, uint64_t MaxCapacity>
template <typename Function, typename>
bool ResizeableLockFreeQueue<ElementType, MaxCapacity>::setCapacity(const uint64_t newCapacity,
                                                                    Function&& removeHandler) noexcept
{
    if (newCapacity > MAX_CAPACITY)
    {
        return false;
    }

    /// @note The vector m_unusedIndices is protected by the atomic flag, but this also means dying during a resize
    /// will prevent further resizes. This is not a problem for the use case were only the dying receiver itself
    /// requests the resize. I.e. resize is lockfree, but it assumes that a concurrent resize will always
    /// eventually complete (which is true when the application does not die and the relevant thread is
    /// scheduled eventually. The latter is the case for any OS and mandatory for a Realtime OS.

    if (m_resizeInProgress.test_and_set(std::memory_order_acquire))
    {
        // at most one resize can be in progress at any time
        return false;
    }

    auto cap = capacity();

    while (cap != newCapacity)
    {
        if (cap < newCapacity)
        {
            auto toIncrease = newCapacity - cap;
            increaseCapacity(toIncrease); // return value does not matter, we check the capacity later
        }
        else
        {
            auto toDecrease = cap - newCapacity;
            decreaseCapacity(toDecrease, removeHandler); // return value does not matter, we check the capacity later
        }

        cap = capacity();
    }

    // sync everything related to capacity change, e.g. the new capacity stored in m_capacity
    m_resizeInProgress.clear(std::memory_order_release);
    return true;
}

template <typename ElementType, uint64_t MaxCapacity>
uint64_t ResizeableLockFreeQueue<ElementType, MaxCapacity>::increaseCapacity(const uint64_t toIncrease) noexcept
{
    // we can be sure this is not called concurrently due to the m_resizeInProgress flag
    //(this must be ensured as the vector is modified)
    uint64_t increased = 0U;
    while (increased < toIncrease)
    {
        if (m_unusedIndices.empty())
        {
            // no indices left to increase capacity
            return increased;
        }
        ++increased;
        m_capacity.fetch_add(1U);
        Base::m_freeIndices.push(m_unusedIndices.back());
        m_unusedIndices.pop_back();
    }

    return increased;
}

template <typename ElementType, uint64_t MaxCapacity>
template <typename Function>
uint64_t ResizeableLockFreeQueue<ElementType, MaxCapacity>::decreaseCapacity(const uint64_t toDecrease,
                                                                             Function&& removeHandler) noexcept
{
    uint64_t decreased = 0U;
    while (decreased < toDecrease)
    {
        BufferIndex index;
        while (decreased < toDecrease)
        {
            if (!Base::m_freeIndices.pop(index))
            {
                break;
            }

            m_unusedIndices.push_back(index);
            ++decreased;
            if (m_capacity.fetch_sub(1U) == 1U)
            {
                // we reached capacity 0 and cannot further decrease it
                return decreased;
            }
        }

        // no free indices, try the used ones
        while (decreased < toDecrease)
        {
            // remark: just calling pop to create free space is not sufficent in a concurrent scenario
            // we want to make sure no one else gets the index once we have it
            if (!tryGetUsedIndex(index))
            {
                // try the free ones again
                break;
            }

            auto result = Base::readBufferAt(index);
            removeHandler(result.value());
            m_unusedIndices.push_back(index);

            ++decreased;
            if (m_capacity.fetch_sub(1U) == 1U)
            {
                // we reached capacity 0 and cannot further decrease it
                return decreased;
            }
        }
    }
    return decreased;
}

template <typename ElementType, uint64_t MaxCapacity>
bool ResizeableLockFreeQueue<ElementType, MaxCapacity>::tryGetUsedIndex(BufferIndex& index) noexcept
{
    /// @note: we have a problem here if we lose an index entirely, since the queue
    /// can then never be full again (or, more generally contain capacity indices)
    /// to lessen this problem, we could use a regular pop if we fail too often here
    /// instead of a variation of popIfFull (which will never work then)

    return Base::m_usedIndices.popIfSizeIsAtLeast(capacity(), index);
}

template <typename ElementType, uint64_t MaxCapacity>
iox::cxx::optional<ElementType>
ResizeableLockFreeQueue<ElementType, MaxCapacity>::push(const ElementType& value) noexcept
{
    return pushImpl(std::forward<const ElementType>(value));
}

template <typename ElementType, uint64_t MaxCapacity>
iox::cxx::optional<ElementType> ResizeableLockFreeQueue<ElementType, MaxCapacity>::push(ElementType&& value) noexcept
{
    return pushImpl(std::forward<ElementType>(value));
}

template <typename ElementType, uint64_t MaxCapacity>
template <typename T>
iox::cxx::optional<ElementType> ResizeableLockFreeQueue<ElementType, MaxCapacity>::pushImpl(T&& value) noexcept
{
    cxx::optional<ElementType> evictedValue;

    BufferIndex index;

    while (!Base::m_freeIndices.pop(index))
    {
        if (tryGetUsedIndex(index))
        {
            evictedValue = Base::readBufferAt(index);
            break;
        }
        // if m_usedIndices was not full we try again (m_freeIndices should contain an index in this case)
        // note that it is theoretically possible to be unsuccessful indefinitely
        // (and thus we would have an infinite loop)
        // but this requires a timing of concurrent pushes and pops which is exceptionally unlikely in practice
    }

    // if we removed from a full queue via popIfFull it might not be full anymore when a concurrent pop occurs

    Base::writeBufferAt(index, value);

    Base::m_usedIndices.push(index);

    return evictedValue; // value was moved into the queue, if a value was evicted to do so return it
}

} // namespace concurrent
} // namespace iox
