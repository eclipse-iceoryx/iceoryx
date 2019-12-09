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


namespace iox
{
namespace concurrent
{
template <class ValueType, uint32_t CapacityValue>
SoFi<ValueType, CapacityValue>::SoFi() noexcept
{
}

template <class ValueType, uint32_t CapacityValue>
uint64_t SoFi<ValueType, CapacityValue>::capacity() const noexcept
{
    return m_size - INTERNAL_SIZE_ADD_ON;
}

template <class ValueType, uint32_t CapacityValue>
uint64_t SoFi<ValueType, CapacityValue>::size() const noexcept
{
    uint64_t readPosition;
    uint64_t writePosition;
    do
    {
        readPosition = m_readPosition.load(std::memory_order_relaxed);
        writePosition = m_writePosition.load(std::memory_order_relaxed);
    } while (m_writePosition.load(std::memory_order_relaxed) != writePosition
             || m_readPosition.load(std::memory_order_relaxed) != readPosition);

    return writePosition - readPosition;
}

template <class ValueType, uint32_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::resize(const uint32_t newSize) noexcept
{
    uint64_t newInternalSize = newSize + INTERNAL_SIZE_ADD_ON;
    if (empty() and (newInternalSize <= INTERNAL_SOFI_SIZE))
    {
        m_size = newInternalSize;

        m_readPosition.store(0u, std::memory_order_release);
        m_writePosition.store(0u, std::memory_order_release);

        return true;
    }

    return false;
}

template <class ValueType, uint32_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::empty() const noexcept
{
    uint64_t currentReadPosition;
    bool isEmpty;

    do
    {
        /// @todo read before write since the writer increments the aba counter!!!
        /// @todo write doc with example!!!
        currentReadPosition = m_readPosition.load(std::memory_order_acquire);
        uint64_t currentWritePosition = m_writePosition.load(std::memory_order_acquire);

        isEmpty = (currentWritePosition == currentReadPosition);
        // we need compare without exchange
    } while (!(currentReadPosition == m_readPosition.load(std::memory_order_acquire)));

    return isEmpty;
}

template <class ValueType, uint32_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::pop(ValueType& valueOut) noexcept
{
    return popIf(valueOut, [](ValueType) { return true; });
}

template <class ValueType, uint32_t CapacityValue>
template <typename Verificator_T>
inline bool SoFi<ValueType, CapacityValue>::popIf(ValueType& valueOut, const Verificator_T& verificator) noexcept
{
    uint64_t currentReadPosition = m_readPosition.load(std::memory_order_acquire);
    uint64_t nextReadPosition;

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
            std::memcpy(&valueOut, &m_data[static_cast<int32_t>(currentReadPosition) % m_size], sizeof(ValueType));

            /// @brief first we need to peak valueOut if it is fitting the condition and then we have to verify
            ///        if valueOut is not am invalid object, this could be the case if the read position has
            ///        changed
            if (m_readPosition.load(std::memory_order_relaxed) == currentReadPosition && verificator(valueOut) == false)
            {
                popWasSuccessful = false;
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

template <class ValueType, uint32_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::push(const ValueType& valueOut, ValueType& f_paramOut_r) noexcept
{
    bool hasOverflow = false;

    uint64_t currentWritePosition = m_writePosition.load(std::memory_order_relaxed);
    uint64_t nextWritePosition = currentWritePosition + 1U;

    uint64_t currentReadPosition = m_readPosition.load(std::memory_order_acquire);
    uint64_t nextReadPosition;

    do
    {
        // buffer overflow detection
        if (nextWritePosition < currentReadPosition + m_size)
        {
            hasOverflow = false;
            break;
        }

        nextReadPosition = currentReadPosition + 1U;

        hasOverflow = true;

        // compare and swap
        // if(m_readPosition == currentReadPosition)
        //     m_readPosition = l_next_aba_read_pos
        // else
        //     currentReadPosition = m_readPosition
    } while (!m_readPosition.compare_exchange_weak(
        currentReadPosition, nextReadPosition, std::memory_order_acq_rel, std::memory_order_acquire));

    // no atomic synchronization required because writer can always
    // read his own data
    if (hasOverflow)
    {
        std::memcpy(&f_paramOut_r, &m_data[static_cast<int32_t>(currentReadPosition) % m_size], sizeof(ValueType));
    }

    m_data[static_cast<int32_t>(currentWritePosition) % m_size] = valueOut;
    m_writePosition.store(nextWritePosition, std::memory_order_release);

    return !hasOverflow;
}


} // namespace concurrent
} // namespace iox
