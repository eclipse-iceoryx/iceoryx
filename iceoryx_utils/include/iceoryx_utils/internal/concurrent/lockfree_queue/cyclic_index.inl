// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType>::CyclicIndex(ValueType value) noexcept
    : m_value(value)
{
}

template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType>::CyclicIndex(ValueType index, ValueType cycle) noexcept
    : CyclicIndex(index + cycle * CycleLength)
{
}

template <uint64_t CycleLength, typename ValueType>
ValueType CyclicIndex<CycleLength, ValueType>::getIndex() const noexcept
{
    return m_value % CycleLength;
}

template <uint64_t CycleLength, typename ValueType>
ValueType CyclicIndex<CycleLength, ValueType>::getCycle() const noexcept
{
    return m_value / CycleLength;
}

template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType> CyclicIndex<CycleLength, ValueType>::operator+(const ValueType value) const noexcept
{
    // if we were at this value, we would have no overflow, i.e. when m_value is larger there is an overflow
    auto delta = MAX_VALUE - value;
    if (delta < m_value)
    {
        // overflow, rare case (overflow by m_value - delta)
        // we need to compute the correct index and cycle we are in after overflow
        // note that we could also limit the max value to always start at OVERFLOW_START_INDEX = 0,
        // but this has other drawbacks (and the overflow will not occur often if at all with 64 bit)
        delta = m_value - delta - 1;
        return CyclicIndex(OVERFLOW_START_INDEX + delta);
    }

    // no overflow, regular case
    return CyclicIndex(m_value + value);
}

template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType> CyclicIndex<CycleLength, ValueType>::next() const noexcept
{
    if (m_value == MAX_VALUE)
    {
        return CyclicIndex(OVERFLOW_START_INDEX);
    }
    return CyclicIndex(m_value + 1);
}

template <uint64_t CycleLength, typename ValueType>
bool CyclicIndex<CycleLength, ValueType>::isOneCycleBehind(const CyclicIndex& other) const noexcept
{
    auto thisCycle = this->getCycle();
    auto otherCycle = other.getCycle();

    if (thisCycle == MAX_CYCLE)
    {
        return otherCycle == 0;
    }
    return (thisCycle + 1 == otherCycle);
}
} // namespace concurrent
} // namespace iox