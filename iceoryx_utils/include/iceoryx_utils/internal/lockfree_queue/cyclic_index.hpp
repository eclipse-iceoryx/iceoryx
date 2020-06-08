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

#pragma once

#include <limits>
#include <stdint.h>
#include <type_traits>


namespace iox
{
/// @brief index structure that can contain logical values 0, ..., CycleLength-1
/// but also stores an internal cycle counter to be used in compare_exchange
template <uint64_t CycleLength, typename ValueType = uint64_t>
class CyclicIndex
{
  public:
    using value_t = ValueType;

    static constexpr ValueType MAX_INDEX = CycleLength - 1;
    static constexpr ValueType MAX_VALUE = std::numeric_limits<ValueType>::max();

    // assumes MAX_VALUE >= CycleLength, otherwise we could not fit in even one cycle
    static constexpr ValueType MAX_CYCLE = MAX_VALUE / CycleLength;

    static constexpr ValueType INDEX_AT_MAX_VALUE = MAX_VALUE % CycleLength;
    static constexpr ValueType OVERFLOW_START_INDEX = (INDEX_AT_MAX_VALUE + 1) % CycleLength;

    static_assert(CycleLength < MAX_VALUE / 2, "CycleLength is too large, need at least one bit for cycle");
    static_assert(CycleLength > 0, "CycleLength must be > 0");

    explicit CyclicIndex(ValueType value = 0) noexcept
        : m_value(value)
    {
    }

    CyclicIndex(ValueType index, ValueType cycle) noexcept
        : CyclicIndex(index + cycle * CycleLength)
    {
    }

    CyclicIndex(const CyclicIndex&) = default;
    CyclicIndex(CyclicIndex&&) = default;
    CyclicIndex& operator=(const CyclicIndex&) = default;
    CyclicIndex& operator=(CyclicIndex&&) = default;

    ValueType getIndex() const noexcept
    {
        return m_value % CycleLength;
    }

    ValueType getCycle() const noexcept
    {
        return m_value / CycleLength;
    }

    CyclicIndex operator+(const ValueType value) const noexcept
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

    CyclicIndex next() const noexcept
    {
        if (m_value == MAX_VALUE)
        {
            return CyclicIndex(OVERFLOW_START_INDEX);
        }
        return CyclicIndex(m_value + 1);
    }

    bool isOneCycleBehind(const CyclicIndex& other) const noexcept
    {
        auto thisCycle = this->getCycle();
        auto otherCycle = other.getCycle();

        if (thisCycle == MAX_CYCLE)
        {
            return otherCycle == 0;
        }
        return (thisCycle + 1 == otherCycle);
    }

  private:
    ValueType m_value{0};
};
} // namespace iox