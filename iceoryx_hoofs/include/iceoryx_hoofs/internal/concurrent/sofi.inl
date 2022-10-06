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
#ifndef IOX_HOOFS_CONCURRENT_SOFI_INL
#define IOX_HOOFS_CONCURRENT_SOFI_INL

#include "iceoryx_hoofs/internal/concurrent/sofi.hpp"

namespace iox
{
namespace concurrent
{
template <class ValueType, uint64_t CapacityValue>
uint64_t SoFi<ValueType, CapacityValue>::capacity() const noexcept
{
    return m_size - INTERNAL_SIZE_ADD_ON;
}

template <class ValueType, uint64_t CapacityValue>
uint64_t SoFi<ValueType, CapacityValue>::size() const noexcept
{
    uint64_t readPosition{0};
    uint64_t writePosition{0};
    do
    {
        readPosition = m_readPosition.load(std::memory_order_relaxed);
        writePosition = m_writePosition.load(std::memory_order_relaxed);
    } while (m_writePosition.load(std::memory_order_relaxed) != writePosition
             || m_readPosition.load(std::memory_order_relaxed) != readPosition);

    return writePosition - readPosition;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::setCapacity(const uint64_t newSize) noexcept
{
    uint64_t newInternalSize = newSize + INTERNAL_SIZE_ADD_ON;
    if (empty() && (newInternalSize <= INTERNAL_SOFI_SIZE))
    {
        m_size = newInternalSize;

        m_readPosition.store(0, std::memory_order_release);
        m_writePosition.store(0, std::memory_order_release);

        return true;
    }

    return false;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::empty() const noexcept
{
    uint64_t currentReadPosition{0};
    bool isEmpty{false};

    do
    {
        /// @todo iox-#1695 read before write since the writer increments the aba counter!!!
        /// @todo iox-#1695 write doc with example!!!
        currentReadPosition = m_readPosition.load(std::memory_order_acquire);
        uint64_t currentWritePosition = m_writePosition.load(std::memory_order_acquire);

        isEmpty = (currentWritePosition == currentReadPosition);
        // we need compare without exchange
    } while (!(currentReadPosition == m_readPosition.load(std::memory_order_acquire)));

    return isEmpty;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::pop(ValueType& valueOut) noexcept
{
    return popIf(valueOut, [](ValueType) { return true; });
}

template <class ValueType, uint64_t CapacityValue>
template <typename Verificator_T>
inline bool SoFi<ValueType, CapacityValue>::popIf(ValueType& valueOut, const Verificator_T& verificator) noexcept
{
    uint64_t currentReadPosition = m_readPosition.load(std::memory_order_acquire);
    uint64_t nextReadPosition{0};

    bool popWasSuccessful{true};
    do
    {
        if (currentReadPosition == m_writePosition.load(std::memory_order_acquire))
        {
            nextReadPosition = currentReadPosition;
            popWasSuccessful = false;
        }
        else
        {
            // we use memcpy here, since the copy assignment is not thread safe in general (we might have an overflow in
            // the push thread and invalidates the object while the copy is running and therefore works on an
            // invalid object); memcpy is also not thread safe, but we discard the object anyway and read it
            // again if its overwritten in between; this is only relevant for types larger than pointer size
            // assign the user data
            std::memcpy(&valueOut, &m_data[currentReadPosition % m_size], sizeof(ValueType));

            /// @brief first we need to peak valueOut if it is fitting the condition and then we have to verify
            ///        if valueOut is not am invalid object, this could be the case if the read position has
            ///        changed
            if (m_readPosition.load(std::memory_order_relaxed) == currentReadPosition && !verificator(valueOut))
            {
                popWasSuccessful = false;
                nextReadPosition = currentReadPosition;
            }
            else
            {
                nextReadPosition = currentReadPosition + 1U;
                popWasSuccessful = true;
            }
        }

        // compare and swap
        // if(m_readPosition == currentReadPosition)
        //     m_readPosition = l_next_aba_read_pos
        // else
        //     currentReadPosition = m_readPosition
        // Assign m_aba_read_p to next readable location
    } while (!m_readPosition.compare_exchange_weak(
        currentReadPosition, nextReadPosition, std::memory_order_acq_rel, std::memory_order_acquire));

    return popWasSuccessful;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::push(const ValueType& valueIn, ValueType& valueOut) noexcept
{
    constexpr bool SOFI_OVERFLOW{false};

    uint64_t currentWritePosition = m_writePosition.load(std::memory_order_relaxed);
    uint64_t nextWritePosition = currentWritePosition + 1U;

    m_data[currentWritePosition % m_size] = valueIn;
    m_writePosition.store(nextWritePosition, std::memory_order_release);

    uint64_t currentReadPosition = m_readPosition.load(std::memory_order_acquire);

    // check if there is a free position for the next push
    if (nextWritePosition < currentReadPosition + m_size)
    {
        return !SOFI_OVERFLOW;
    }

    // this is an overflow situation, which means that the next push has no free position, therefore the oldest value
    // needs to be passed back to the caller

    uint64_t nextReadPosition = currentReadPosition + 1U;

    // we need to update the read position
    // a) it works, then we need to pass the overflow value back
    // b) it doesn't work, which means that the pop thread already took the value in the meantime an no further action
    // is required
    // memory order success is memory_order_acq_rel
    //   - this is to prevent the reordering of m_writePosition.store(...) after the increment of the m_readPosition
    //     - in case of an overflow, this might result in the pop thread getting one element less than the capacity of
    //       the SoFi if the push thread is suspended in between this two statements
    //     - it's still possible to get more elements than the capacity, but this is an inherent issue with concurrent
    //       queues and cannot be prevented since there can always be a push during a pop operation
    //   - another issue might be that two consecutive pushes (not concurrent) happen on different CPU cores without
    //     synchronization, then the memory also needs to be synchronized for the overflow case
    // memory order failure is memory_order_relaxed since there is no further synchronization needed if there is no
    // overflow
    if (m_readPosition.compare_exchange_strong(
            currentReadPosition, nextReadPosition, std::memory_order_acq_rel, std::memory_order_relaxed))
    {
        std::memcpy(&valueOut, &m_data[currentReadPosition % m_size], sizeof(ValueType));
        return SOFI_OVERFLOW;
    }

    return !SOFI_OVERFLOW;
}


} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_SOFI_INL
