// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_BUFFER_SPSC_FIFO_INL
#define IOX_HOOFS_CONCURRENT_BUFFER_SPSC_FIFO_INL

#include "iox/detail/spsc_fifo.hpp"

namespace iox
{
namespace concurrent
{
template <class ValueType, uint64_t Capacity>
inline bool SpscFifo<ValueType, Capacity>::push(const ValueType& value) noexcept
{
    if (is_full())
    {
        return false;
    }
    // Memory order relaxed is enough since:
    // - there is no concurrent access to this method
    // - the load statement cannot be reordered with writing m_data otherwise it would not compile
    auto currentWritePos = m_writePos.load(std::memory_order_relaxed);
    m_data[currentWritePos % Capacity] = value;

    // SYNC POINT WRITE: m_data
    // We need to make sure that writing the value happens before incrementing the
    // m_writePos otherwise the following scenario can happen:
    // 1. m_writePos is increased (but the value has not been written yet)
    // 2. Another thread calls pop(): we check if the queue is empty => no
    // 3. In pop(), when we read a value, a data race can occur when at the same time a value is
    // written by push. With memory_order_release, this cannot happen as it is guaranteed that
    // writing the data happens before incrementing m_writePos. Note that the following scenario
    // can still happen (but this is not a problem):
    // 1. A value is written (m_writePos hasn't been incremented yet)
    // 2. Another thread calls pop(): we check if the queue is empty => yes
    // 3. An element was already stored so we could have popped the element
    m_writePos.store(currentWritePos + 1, std::memory_order_release);
    return true;
}

template <class ValueType, uint64_t Capacity>
inline bool SpscFifo<ValueType, Capacity>::is_full() const noexcept
{
    return m_writePos.load(std::memory_order_relaxed) == m_readPos.load(std::memory_order_relaxed) + Capacity;
}


template <class ValueType, uint64_t Capacity>
inline uint64_t SpscFifo<ValueType, Capacity>::size() const noexcept
{
    return m_writePos.load(std::memory_order_relaxed) - m_readPos.load(std::memory_order_relaxed);
}
template <class ValueType, uint64_t Capacity>
inline constexpr uint64_t SpscFifo<ValueType, Capacity>::capacity() noexcept
{
    return Capacity;
}

template <class ValueType, uint64_t Capacity>
inline bool SpscFifo<ValueType, Capacity>::empty() const noexcept
{
    return m_readPos.load(std::memory_order_relaxed) == m_writePos.load(std::memory_order_relaxed);
}


template <class ValueType, uint64_t Capacity>
inline optional<ValueType> SpscFifo<ValueType, Capacity>::pop() noexcept
{
    // Memory order relaxed is enough since:
    // - there is no concurrent access to this method
    // - the load statement cannot be reordered with the isEmpty check otherwise it would not
    // compile
    auto currentReadPos = m_readPos.load(std::memory_order_relaxed);

    bool isEmpty = (currentReadPos ==
                     // SYNC POINT READ: m_data
                     // See explanation of the corresponding sync point.
                     // As a consequence, we are not allowed to use the empty method
                     // since we have to sync with m_writePos in the push method
                     m_writePos.load(std::memory_order_acquire));
    if (isEmpty)
    {
        return nullopt_t();
    }

    ValueType out = m_data[currentReadPos % Capacity];

    // We need to make sure that reading the value happens before incrementing the m_readPos
    // otherwise the following can happen:
    // 1. We increment m_readPos (but the value hasn't been read yet)
    // 2. Another thread calls push(): we check if the queue is full => no
    // 3. In push(), a data race can occur
    // With memory_order_acq_rel,
    // m_readPos must be increased after reading the popped value otherwise
    // it is possible that the popped value is overwritten by push while being read
    // (e.g. when the queue is full, i.e. m_readPos == m_writePos)
    // Note that the following situation can still happen but is not a problem:
    // 1. We read the value
    // 2. Another thread calls push(): we check if the queue is full => yes
    // 3. The queue was not really full as the value was already read
    // Memory order acq_rel to avoid:
    // - reading m_data to happen after the fence (rel)
    // - incrementing m_readPos before the fence (acq)
    std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
    m_readPos.store(currentReadPos + 1, std::memory_order_relaxed);
    return out;
}
} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_BUFFER_SPSC_FIFO_INL
