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
    // Memory order relaxed is enough since:
    // - there is no concurrent access to this method
    // - the load statement cannot be reordered with writing m_data otherwise there would be observable changes
    auto currentWritePos = m_writePos.load(std::memory_order_relaxed);

    // There is no need to sync the memory (no data is written) but we need the memory order acquire
    // to enforce the happens-before relationship of the matching store/release on m_readPos in the
    // pop method
    auto currentReadPos = m_readPos.load(std::memory_order_acquire);
    if (is_full(currentReadPos, currentWritePos))
    {
        return false;
    }
    m_data[currentWritePos % Capacity] = value;

    // SYNC POINT WRITE: m_data
    // We need to make sure that writing the value happens before incrementing the
    // m_writePos otherwise the following scenario can happen:
    // 1. m_writePos is increased (but the value has not been written yet)
    // 2. Another thread calls pop(): we check if the queue is empty => no
    // 3. In pop(), when we read a value, a data race can occur when at the same time a value is
    // written by push. With memory_order_release, this cannot happen as it is guaranteed that
    // writing the data happens before incrementing m_writePos. Note that the following scenario
    // can still happen (but, although it is an inherent race with concurrent algorithms, it is
    // not a data race and therefore not a problem):
    // 1. There is an empty queue
    // 2. A push operation is in progress, the value has been written but 'm_writePos' was not yet
    // advanced
    // 3. The consumer thread performs a pop operation and the check for an empty queue is true
    // resulting in a failed pop
    // 4. The push operation is finished by advancing m_writePos and synchronizing the memory
    // 5. The consumer thread missed the chance to pop the element in the blink of an eye
    m_writePos.store(currentWritePos + 1, std::memory_order_release);
    return true;
}

template <class ValueType, uint64_t Capacity>
inline bool SpscFifo<ValueType, Capacity>::is_full(uint64_t currentReadPos, uint64_t currentWritePos) const noexcept
{
    return currentWritePos == currentReadPos + Capacity;
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
    // - the load statement cannot be reordered with the isEmpty check otherwise there would be observable changes
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

    // We need to make sure that reading the value happens before incrementing the m_readPos (hence release memory
    // order) otherwise the following can happen:
    // 1. We increment m_readPos (but the value hasn't been read yet)
    // 2. Another thread calls push(): we check if the queue is full => no
    // 3. In push(), a data race can occur
    // Note that the following situation can still happen (but, although it is an inherent race with
    // concurrent algorithms, it is not a data race and therefore not a problem):
    // 1. There is a full queue
    // 2. A pop operation is in progress, the value has been read but m_readPos was not yet advanced
    // 3. The producer thread performs a push operation and the check for a full queue is true resulting in a fail push
    // 4. The read operation is finished by advancing m_readPos and synchronizing the memory
    // 5. The producer thread missed the chance to push an element in the blink of an eye
    m_readPos.store(currentReadPos + 1, std::memory_order_release);
    return out;
}
} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_BUFFER_SPSC_FIFO_INL
