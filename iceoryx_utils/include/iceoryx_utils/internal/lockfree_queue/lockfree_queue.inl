// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
template <typename ElementType, uint64_t Capacity>
LockFreeQueue<ElementType, Capacity>::LockFreeQueue() noexcept
    : m_freeIndices(IndexQueue<Capacity>::ConstructFull)
    , m_usedIndices(IndexQueue<Capacity>::ConstructEmpty)
{
}

template <typename ElementType, uint64_t Capacity>
constexpr uint64_t LockFreeQueue<ElementType, Capacity>::capacity() noexcept
{
    return Capacity;
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::tryPush(const ElementType& value) noexcept
{
    UniqueIndex index = m_freeIndices.pop();

    if (!index.isValid())
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
    UniqueIndex index = m_freeIndices.pop();

    if (!index.isValid())
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

    UniqueIndex index = m_freeIndices.pop();

    while (!index.isValid())
    {
        // only pop the index if the queue is still full
        index = m_usedIndices.popIfFull();
        if (index.isValid())
        {
            evictedValue = readBufferAt(index);
            break;
        }
        // if m_usedIndices was not full we try again (m_freeIndices should contain an index in this case)
        // note that it is theoretically possible to be unsuccessful indefinitely
        // (and thus we would have an infinite loop)
        // but this requires a timing of concurrent pushes and pops which is exceptionally unlikely in practice

        index = m_freeIndices.pop();
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
    UniqueIndex index = m_usedIndices.pop();

    if (!index.isValid())
    {
        return cxx::nullopt_t(); // detected empty queue
    }

    auto result = readBufferAt(index);

    m_freeIndices.push(index);

    return result;
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::empty()
{
    return m_usedIndices.empty();
}

template <typename ElementType, uint64_t Capacity>
uint64_t LockFreeQueue<ElementType, Capacity>::size()
{
    return m_size.load(std::memory_order_relaxed);
}

template <typename ElementType, uint64_t Capacity>
cxx::optional<ElementType> LockFreeQueue<ElementType, Capacity>::readBufferAt(const UniqueIndex& index)
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
void LockFreeQueue<ElementType, Capacity>::writeBufferAt(const UniqueIndex& index, T&& value)
{
    auto elementPtr = m_buffer.ptr(index);
    new (elementPtr) ElementType(std::forward<T>(value)); // move ctor invoked when available, copy ctor otherwise

    // also used for buffer synchronization
    m_size.fetch_add(1u, std::memory_order_release);
}

} // namespace iox