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

namespace iox
{
template <typename T, uint64_t Capacity>
LockFreeQueue<T, Capacity>::LockFreeQueue() noexcept
    : m_freeIndices(IndexQueue<Capacity>::ConstructFull)
    , m_usedIndices(IndexQueue<Capacity>::ConstructEmpty)
{
}

template <typename T, uint64_t Capacity>
constexpr uint64_t LockFreeQueue<T, Capacity>::capacity() noexcept
{
    return Capacity;
}


template <typename T, uint64_t Capacity>
bool LockFreeQueue<T, Capacity>::try_push(const T& value) noexcept
{
    UniqueIndex index = m_freeIndices.pop();

    if (!index.is_valid())
    {
        return false; // detected full queue
    }

    auto ptr = m_buffer.ptr(index);
    new (ptr) T(value);

    // ensures that whenever an index is pushed into m_usedIndices, the corresponding value in m_buffer[index]
    // was written before
    releaseBufferChanges();

    m_usedIndices.push(index);

    return true;
}


template <typename T, uint64_t Capacity>
iox::cxx::optional<T> LockFreeQueue<T, Capacity>::push(const T& value) noexcept
{
    cxx::optional<T> result;

    UniqueIndex index = m_freeIndices.pop();
    while (!index.is_valid())
    {
        // only pop the index if the queue is still full
        index = m_usedIndices.popIfFull();
        if (index.is_valid())
        {
            // todo: private method and sync here?
            auto ptr = m_buffer.ptr(index);
            result = std::move(*ptr);
            ptr->~T();
            break;
        }
        // if the queue was not full we try again
        index = m_freeIndices.pop();
    }

    // if we removed from a full queue via popIfFull it might not be full anymore when a concurrent pop occurs

    auto ptr = m_buffer.ptr(index);
    new (ptr) T(value);

    // ensures that whenever an index is pushed into m_usedIndices, the corresponding value in m_buffer[index]
    // was written before
    releaseBufferChanges();

    m_usedIndices.push(index);

    return result;
}

template <typename T, uint64_t Capacity>
iox::cxx::optional<T> LockFreeQueue<T, Capacity>::pop() noexcept
{
    UniqueIndex index = m_usedIndices.pop();

    if (!index.is_valid())
    {
        return cxx::nullopt_t(); // detected empty queue
    }

    // combined with releaseChanges, this ensures that whenever an index is popped from m_usedIndices,
    // the corresponding value in m_buffer[index] was written to before
    acquireBufferChanges();

    auto ptr = m_buffer.ptr(index);
    cxx::optional<T> result(std::move(*ptr));
    ptr->~T();

    m_freeIndices.push(index);
    return result;
}

template <typename T, uint64_t Capacity>
bool LockFreeQueue<T, Capacity>::empty()
{
    return m_usedIndices.empty();
}

template <typename T, uint64_t Capacity>
void LockFreeQueue<T, Capacity>::acquireBufferChanges()
{
    std::atomic_thread_fence(std::memory_order_acquire);
}

template <typename T, uint64_t Capacity>
void LockFreeQueue<T, Capacity>::releaseBufferChanges()
{
    std::atomic_thread_fence(std::memory_order_release);
}
} // namespace iox