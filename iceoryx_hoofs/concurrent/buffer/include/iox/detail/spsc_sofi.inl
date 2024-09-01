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
    auto [readPosition, writePosition] = getReadWritePositions();
    return readPosition == writePosition;
}

template <class ValueType, uint64_t CapacityValue>
inline bool SpscSofi<ValueType, CapacityValue>::pop(ValueType& valueOut) noexcept
{
    uint64_t nextReadPosition{0};
    bool popWasSuccessful{true};
    // Memory order relaxed is enough since:
    // - no synchronization needed for m_readPosition
    // - if m_writePosition is loaded before m_readPosition and m_readPosition changed, it will be detected by the
    // compare_exchange loop
    uint64_t currentReadPosition = m_readPosition.load(std::memory_order_relaxed);

    do
    {
        // SYNC POINT READ: m_data
        // See explanation of the corresponding synchronization point in push()
        if (currentReadPosition == m_writePosition.load(std::memory_order_acquire))
        {
            nextReadPosition = currentReadPosition;
            popWasSuccessful = false;
            // We cannot just return false (i.e. we need to continue the loop) to avoid the following situation:
            // 0. Initial situation (the queue is full)
            // |----|--B--|--C--|
            // ^    ^
            // w=3 r=1
            // 1. The consumer thread loads m_writePosition => 3
            // |----|--B--|--C--|
            // ^     ^
            // w=3  r=1
            // 2. The producer thread pushes two times
            // |--D--|--E--|-----|
            // ^           ^
            // r=3        w=5
            // 3. The consumer thread loads m_readPosition => 3 The pop method returns false
            // => Whereas the queue was full, pop returned false giving the impression that the queue if empty
        }
        else
        {
            // we use memcpy here, to ensure that there is no logic in copying the data
            std::memcpy(&valueOut, &m_data[currentReadPosition % m_size], sizeof(ValueType));
            nextReadPosition = currentReadPosition + 1U;
            popWasSuccessful = true;

            // We need to check if m_readPosition hasn't changed otherwise valueOut might be corrupted
            // =============================================
            // While memory synchronization is not needed for m_readPosition, we need to ensure that the
            // 'memcpy' happens before updating m_readPosition.
            // Corresponding m_readPosition load/acquire is in the CAS loop of push method
            // =============================================
            // ABA problem: m_readPosition is an uint64_t. Assuming a thread is pushing at a rate of 1 GHz
            // while this thread is blocked, we would still need more than 500 years to overflow
            // m_readPosition and encounter the ABA problem
        }
    } while (!m_readPosition.compare_exchange_weak(
        currentReadPosition, nextReadPosition, std::memory_order_acq_rel, std::memory_order_acquire));

    return popWasSuccessful;
}

template <class ValueType, uint64_t CapacityValue>
inline bool SpscSofi<ValueType, CapacityValue>::push(const ValueType& valueIn, ValueType& valueOut) noexcept
{
    constexpr bool SOFI_OVERFLOW{false};

    // Memory order relaxed is enough since:
    // - no synchronization needed as we are loading a value only modified in this method and this method cannot be
    // accessed concurrently
    // - the operation cannot move below without observable changes
    uint64_t currentWritePosition = m_writePosition.load(std::memory_order_relaxed);
    uint64_t nextWritePosition = currentWritePosition + 1U;

    m_data[currentWritePosition % m_size] = valueIn;
    // SYNC POINT WRITE: m_data
    // We need to make sure that writing the value happens before incrementing the
    // m_writePosition otherwise the following scenario can happen:
    // 1. m_writePosition is increased (but the value has not been written yet)
    // 2. The consumer thread calls pop(): we check if the queue is empty => no
    // 3. In pop(), when we read a value a data race can occur
    // With memory_order_release, this cannot happen as it is guaranteed that writing the data
    // happens before incrementing m_writePosition
    // =======================================
    // Note that the following situation can still happen (but, although it is an inherent race with
    // concurrent algorithms, it is not a data race and therefore not a problem):
    // 1. There is an empty queue
    // 2. A push operation is in progress, the value has been written but 'm_writePosition' was not
    //  yet advanced
    // 3. The consumer thread performs a pop operation and the check for an empty queue is true
    // resulting in a failed pop
    // 4. The push operation is finished by advancing m_writePos and synchronizing the memory
    // 5. The consumer thread missed the chance to pop the element in the blink of an eye
    m_writePosition.store(nextWritePosition, std::memory_order_release);

    // Memory order relaxed is enough since:
    // - no synchronization needed when loading
    // - the operation cannot move below without observable changes
    uint64_t currentReadPosition = m_readPosition.load(std::memory_order_relaxed);

    // Check if queue is full: since we have an extra element (INTERNAL_CAPACITY_ADD_ON), we need to
    // check if there is a free position for the *next* write position
    if (nextWritePosition < currentReadPosition + m_size)
    {
        return !SOFI_OVERFLOW;
    }

    // This is an overflow situation so we will need to read the overwritten value
    // however, it could be that pop() was called in the meantime, i.e. m_readPosition was increased.
    // Memory order success needs to be memory_order_acq_rel to prevent the reordering of
    // m_writePosition.store(...) after the increment of the m_readPosition. Otherwise, in case of
    // an overflow, this might result in the pop thread getting one element less than the capacity
    // of the SoFi if the push thread is suspended in between this two statements.
    // It's still possible to get more elements than the capacity, but this is an inherent issue
    // with concurrent queues and cannot be prevented since there can always be a push during a pop
    // operation.
    // Another issue might be that two consecutive pushes (not concurrent) happen on different CPU
    // cores without synchronization, then the memory also needs to be synchronized for the overflow
    // case.
    // Memory order failure needs to be memory_order_acquire to match the corresponding m_readPosition store/release in
    // the CAS loop of the pop method
    // ======================================
    // ABA problem: m_readPosition is an uint64_t. Assuming a thread is popping at a rate of 1 GHz while
    // this thread is blocked, we would still need more than 500 years to overflow m_readPosition and
    // encounter the ABA problem
    if (m_readPosition.compare_exchange_strong(
            currentReadPosition, currentReadPosition + 1U, std::memory_order_acq_rel, std::memory_order_acquire))
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
        std::memcpy(&valueOut, &m_data[currentReadPosition % m_size], sizeof(ValueType));
        return SOFI_OVERFLOW;
    }

    return !SOFI_OVERFLOW;
}

} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_BUFFER_SPSC_SOFI_INL
