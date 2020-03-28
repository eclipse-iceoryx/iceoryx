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
    : m_freeIndices(IndexQueue<Capacity>::ConstructFull::Policy)
    , m_usedIndices(IndexQueue<Capacity>::ConstructEmpty::Policy)
{
}

template <typename T, uint64_t Capacity>
constexpr uint64_t LockFreeQueue<T, Capacity>::capacity() noexcept
{
    return Capacity;
}

template <typename T, uint64_t Capacity>
T* LockFreeQueue<T, Capacity>::toPtr(index_t index) noexcept
{
    auto p = &(m_buffer[index * sizeof(T)]);
    return reinterpret_cast<T*>(p);
}


template <typename T, uint64_t Capacity>
bool LockFreeQueue<T, Capacity>::try_push(const T& value) noexcept
{
    index_t index;
    if (!m_freeIndices.pop(index))
    {
        return false; // detected full queue
    }

    auto ptr = toPtr(index);
    new (ptr) T(value);

    m_usedIndices.push(index);

    return true;
}


template <typename T, uint64_t Capacity>
iox::cxx::optional<T> LockFreeQueue<T, Capacity>::push(const T& value) noexcept
{
    index_t index;
    cxx::optional<T> result;

    while (!m_freeIndices.pop(index))
    {
        // only pop the index if the queue is still full
        if (m_usedIndices.popIfFull(index))
        {
            auto ptr = toPtr(index);
            result = std::move(*ptr);
            ptr->~T();
            break;
        }
        // if the queue was not full we try again ...
    }

    // if we removed from a full queue via popIfFull it might not be full anymore when a concurrent pop occurs

    auto ptr = toPtr(index);
    new (ptr) T(value);

    m_usedIndices.push(index);

    return result;
}

template <typename T, uint64_t Capacity>
iox::cxx::optional<T> LockFreeQueue<T, Capacity>::pop() noexcept
{
    index_t index;
    if (!m_usedIndices.pop(index))
    {
        return cxx::nullopt_t(); // detected empty queue
    }

    auto ptr = toPtr(index);
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
} // namespace iox