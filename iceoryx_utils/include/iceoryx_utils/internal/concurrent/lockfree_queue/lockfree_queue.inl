// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/cxx/optional.hpp"

#include <utility>

namespace iox
{
namespace concurrent
{
template <typename ElementType, uint64_t Capacity>
LockFreeQueue<ElementType, Capacity>::LockFreeQueue() noexcept
    : m_freeIndices(IndexQueue<Capacity>::ConstructFull)
    , m_usedIndices(IndexQueue<Capacity>::ConstructEmpty)
{
}

template <typename ElementType, uint64_t Capacity>
constexpr uint64_t LockFreeQueue<ElementType, Capacity>::capacity() const noexcept
{
    return Capacity;
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::tryPush(const ElementType& value) noexcept
{
    BufferIndex index;

    if (!m_freeIndices.pop(index))
    {
        return false; // detected full queue, value is unchanged (as demanded by const)
    }

    writeBufferAt(index, value); // const& version is called

    m_usedIndices.push(index);

    return true; // value was copied into the queue and is unchanged
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::tryPush(ElementType&& value) noexcept
{
    BufferIndex index;

    if (!m_freeIndices.pop(index))
    {
        return false; // detected full queue
    }

    writeBufferAt(index, std::forward<ElementType>(value)); //&& version is called

    m_usedIndices.push(index);

    return true;
}

template <typename ElementType, uint64_t Capacity>
template <typename T>
iox::cxx::optional<ElementType> LockFreeQueue<ElementType, Capacity>::pushImpl(T&& value) noexcept
{
    cxx::optional<ElementType> evictedValue;

    BufferIndex index;

    while (!m_freeIndices.pop(index))
    {
        // only pop the index if the queue is still full
        // note, this leads to issues if an index is lost
        // (only possible due to an application crash)
        // then the queue can never be full and we may never leave if no one calls a concurrent pop
        // A quick remedy is not to use a conditional pop such as popIfFull here, but a normal one.
        // However, then it can happen that due to a concurrent pop it was not really necessary to
        // evict a value (i.e. we may needlessly lose values in rare cases)
        // Whether there is another acceptable solution needs to be explored.
        if (m_usedIndices.popIfFull(index))
        {
            evictedValue = readBufferAt(index);
            break;
        }
        // if m_usedIndices was not full we try again (m_freeIndices should contain an index in this case)
        // note that it is theoretically possible to be unsuccessful indefinitely
        // (and thus we would have an infinite loop)
        // but this requires a timing of concurrent pushes and pops which is exceptionally unlikely in practice
    }

    // if we removed from a full queue via popIfFull it might not be full anymore when a concurrent pop occurs

    writeBufferAt(index, value); //&& version is called due to explicit conversion via std::move

    m_usedIndices.push(index);

    return evictedValue; // value was moved into the queue, if a value was evicted to do so return it
}

template <typename ElementType, uint64_t Capacity>
iox::cxx::optional<ElementType> LockFreeQueue<ElementType, Capacity>::push(const ElementType& value) noexcept
{
    return pushImpl(std::forward<const ElementType>(value));
}

template <typename ElementType, uint64_t Capacity>
iox::cxx::optional<ElementType> LockFreeQueue<ElementType, Capacity>::push(ElementType&& value) noexcept
{
    return pushImpl(std::forward<ElementType>(value));
}

template <typename ElementType, uint64_t Capacity>
iox::cxx::optional<ElementType> LockFreeQueue<ElementType, Capacity>::pop() noexcept
{
    BufferIndex index;

    if (!m_usedIndices.pop(index))
    {
        return cxx::nullopt; // detected empty queue
    }

    auto result = readBufferAt(index);

    m_freeIndices.push(index);

    return result;
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::empty() const noexcept
{
    return m_usedIndices.empty();
}

template <typename ElementType, uint64_t Capacity>
uint64_t LockFreeQueue<ElementType, Capacity>::size() const noexcept
{
    return m_size.load(std::memory_order_relaxed);
}

template <typename ElementType, uint64_t Capacity>
cxx::optional<ElementType> LockFreeQueue<ElementType, Capacity>::readBufferAt(const BufferIndex& index)
{
    // also used for buffer synchronization
    m_size.fetch_sub(1u, std::memory_order_acquire);

    auto& element = m_buffer[index];
    cxx::optional<ElementType> result(std::move(element));
    element.~ElementType();
    return result;
}

template <typename ElementType, uint64_t Capacity>
template <typename T>
void LockFreeQueue<ElementType, Capacity>::writeBufferAt(const BufferIndex& index, T&& value)
{
    auto elementPtr = m_buffer.ptr(index);
    new (elementPtr) ElementType(std::forward<T>(value)); // move ctor invoked when available, copy ctor otherwise

    // also used for buffer synchronization
    m_size.fetch_add(1u, std::memory_order_release);
}

} // namespace concurrent
} // namespace iox
