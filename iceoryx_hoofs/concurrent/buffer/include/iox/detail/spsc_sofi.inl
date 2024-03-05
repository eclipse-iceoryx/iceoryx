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

#ifndef IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_INL
#define IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_INL

#include "iox/detail/spsc_sofi.hpp"

namespace iox
{
namespace concurrent
{
template <class ValueType, uint64_t CapacityValue>
inline uint64_t SpscSofi<ValueType, CapacityValue>::capacity() const noexcept
{
    return m_size - INTERNAL_CAPACITY_ADDON;
}

template <class ValueType, uint64_t CapacityValue>
inline std::pair<uint64_t, uint64_t> SpscSofi<ValueType, CapacityValue>::getReadWritePositions() const noexcept
{
    uint64_t readPosition{0};
    uint64_t writePosition{0};
    do
    {
        readPosition = m_readPosition.load(std::memory_order_relaxed);
        writePosition = m_writePosition.load(std::memory_order_relaxed);

        // The while loop is needed to avoid the following scenarios:
        // 1. Implementation to get the size: size = m_writePosition - m_readPosition;
        //   - consumer reads m_writePosition
        //   - consumer thread gets suspended
        //   - producer pushes 100 times
        //   - consumer reads m_readPosition
        //   => m_readPosition will be past m_writePosition and one would get a negative size (or the positive unsigned
        //   equivalent)
        // 2. Implementation to get the size: readPosition = m_readPosition; size = m_writePosition -  readPosition;
        //   - consumer stores m_readPosition in readPosition
        //   - consumer thread gets suspended
        //   - producer pushes 100 times
        //   - consumer reads m_writePosition
        //   => m_writePosition will be past readPosition + Capacity and one would get a size which is much larger than
        //   the capacity
        // ===========================================
        // Note: it is still possible to return a size that is not up-to-date anymore but at least
        // the returned size is logically valid
    } while (m_writePosition.load(std::memory_order_relaxed) != writePosition
             || m_readPosition.load(std::memory_order_relaxed) != readPosition);

    return {readPosition, writePosition};
}


template <class ValueType, uint64_t CapacityValue>
inline uint64_t SpscSofi<ValueType, CapacityValue>::size() const noexcept
{
    auto [readPosition, writePosition] = getReadWritePositions();
    return writePosition - readPosition;
}

template <class ValueType, uint64_t CapacityValue>
inline bool SpscSofi<ValueType, CapacityValue>::setCapacity(const uint64_t newSize) noexcept
{
    uint64_t newInternalSize = newSize + INTERNAL_CAPACITY_ADDON;
    if (empty() && (newInternalSize <= INTERNAL_SPSC_SOFI_CAPACITY))
    {
        m_size = newInternalSize;

        m_readPosition.store(0, std::memory_order_release);
        m_writePosition.store(0, std::memory_order_release);

        return true;
    }

    return false;
}

template <class ValueType, uint64_t CapacityValue>
inline bool SpscSofi<ValueType, CapacityValue>::empty() const noexcept
{
    auto [readPost, writePos] = getReadWritePositions();
    return readPost == writePos;
}

template <class ValueType, uint64_t CapacityValue>
inline bool SpscSofi<ValueType, CapacityValue>::pop(ValueType& valueOut) noexcept
{
    // Memory synchronization is not needed but we need to prevent operation reordering to avoid the following scenario
    // where the CPU reorder the load of m_readPosition and m_writePosition:
    // 0. Initial situation (the queue is full)
    // |----|--B--|--C--|
    // ^     ^
    // w=3  r=1
    // 1. The consumer thread loads m_writePosition => 3
    // |----|--B--|--C--|
    // ^     ^
    // w=3  r=1
    // 2. The producer thread pushes two times
    // |--D--|--E--|-----|
    // ^           ^
    // r=3        w=5
    // 3. The consumer thread loads m_readPosition => 3. The pop method returns false
    // => Whereas the queue was full, pop returned false giving the impression that the queue if empty
    // TODO: To which release/store statement does it correspond?
    uint64_t currentReadPos = m_readPosition.load(std::memory_order_acquire);

    do
    {
        // SYNC POINT READ: m_data
        // See explanation of the corresponding synchronization point in push()
        if (currentReadPos == m_writePosition.load(std::memory_order_acquire))
        {
            return false;
            // We don't need to check if read has changed, as it is enough to know that the empty state
            // was valid in the past. The same race can also happen after the while loop and before the
            // return operation
        }
        // we use memcpy here, to ensure that there is no logic in copying the data
        std::memcpy(&valueOut, &m_data[currentReadPos % m_size], sizeof(ValueType));

        // We need to check if m_readPosition hasn't changed otherwise valueOut might be corrupted
        // Memory order relaxed is enough as:
        // - synchronization is not needed with m_readPosition
        // - there is no operation reordering possible
        // =============================================
        // ABA problem: m_readPosition is an uint64_t. Assuming a thread is pushing at a rate of 1 GHz
        // while this thread is blocked, we would still need more than 500 years to overflow
        // m_readPosition and encounter the ABA problem
    } while (!m_readPosition.compare_exchange_weak(
        currentReadPos, currentReadPos + 1U, std::memory_order_relaxed, std::memory_order_relaxed));

    return true;
}

template <class ValueType, uint64_t CapacityValue>
inline bool SpscSofi<ValueType, CapacityValue>::push(const ValueType& valueIn, ValueType& valueOut) noexcept
{
    constexpr bool SOFI_OVERFLOW{false};

    // SYNC POINT READ: m_data
    // We need to synchronize data to avoid the following scenario:
    // 1. A thread calls push() and updates data with a new value
    // 2. Another thread calls push() and enters the overflow case. The data could be read before
    // any synchronization
    uint64_t currentWritePos = m_writePosition.load(std::memory_order_acquire);
    uint64_t nextWritePos = currentWritePos + 1U;

    m_data[currentWritePos % m_size] = valueIn;
    // SYNC POINT WRITE: m_data
    // We need to make sure that writing the value happens before incrementing the
    // m_writePosition otherwise the following scenario can happen:
    // 1. m_writePosition is increased (but the value has not been written yet)
    // 2. Another thread calls pop(): we check if the queue is empty => no (e.g. m_writePosition == 1
    // and m_readPosition == 0)
    // 3. In pop(), a data race can occur
    // With memory_order_release, this cannot happen as it is guaranteed that writing the data
    // happens before incrementing m_writePosition
    // =======================================
    // Note that the following situation can still happen (but is not a problem):
    // 1. A value is written (m_writePosition hasn't been incremented yet)
    // 2. Another thread calls pop(): we check if the queue is empty => yes (e.g. m_writePosition ==
    // m_readPosition == 0 )
    // 3. An element was already stored so we could have popped the element
    m_writePosition.store(nextWritePos, std::memory_order_release);

    // Memory order relaxed is enough since:
    // - synchronization is not needed with m_readPosition
    // - operation reordering:
    //    - cannot move below if statement otherwise, the code won't compile
    //    - if it moves above, we might get an outdated read position that will be caught by the
    //    compare_exchange check
    uint64_t currentReadPos = m_readPosition.load(std::memory_order_relaxed);

    // Check if queue is full: since we have an extra element (INTERNAL_CAPACITY_ADD_ON), we need to
    // check if there is a free position for the *next* write position
    if (nextWritePos < currentReadPos + m_size)
    {
        return !SOFI_OVERFLOW;
    }

    // This is an overflow situation so we will need to read the overwritten value
    // however, it could be that pop() was called in the meantime, i.e. m_readPosition was increased.
    // Memory order relaxed is enough for both success and failure cases since:
    //  - synchronization is not needed with m_readPosition
    //  - operation reordering cannot happen
    // ======================================
    // ABA problem: m_readPosition is an uint64_t. Assuming a thread is popping at a rate of 1 GHz while
    // this thread is blocked, we would still need more than 500 years to overflow m_readPosition and
    // encounter the ABA problem
    if (m_readPosition.compare_exchange_strong(
            currentReadPos, currentReadPos + 1U, std::memory_order_relaxed, std::memory_order_relaxed))
    {
        // Since INTERNAL_SOFI_CAPACITY = CapacityValue + 1, it can happen that we return more
        // elements than the CapacityValue by calling push and pop concurrently (in case of an
        // overflow). This is an inherent behavior with concurrent queues. Scenario example
        // (CapacityValue = 2):
        // 0. Initial situation (before the call to push)
        // |--A--|--B--|----|
        // ^           ^
        // r=0        w=2
        // 1. Thread 1, pushes a new value and increases m_readPosition (overflow situation)
        // |--A--|--B--|--C--|
        // ^     ^
        // w=3, r=1
        // 2. Now, thread 1 is interrupted and another thread pops as many elements as possible
        // 3. pop() -> returns B (First value returned by pop)
        // |--A--|-(B)-|--C--|
        // ^           ^
        // w=3        r=2
        // 4. pop() -> returns C (Second value returned by pop)
        // |--A--|-(B)-|-(C)-|
        // ^
        // w=3, r=3
        // 5. pop() -> nothing to return
        // 6. Finally, thread 1 resumes and returns A (Third value [additional value] returned by
        // push)
        // |-(A)-|-(B)-|-(C)-|
        // ^
        // w=3, r=3
        std::memcpy(&valueOut, &m_data[currentReadPos % m_size], sizeof(ValueType));
        return SOFI_OVERFLOW;
    }

    return !SOFI_OVERFLOW;
}

} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_INL
