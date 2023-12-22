// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_CYCLIC_INDEX_HPP
#define IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_CYCLIC_INDEX_HPP

#include <cstdint>
#include <limits>
#include <type_traits>

namespace iox
{
namespace concurrent
{
/// @brief index structure that can contain logical values 0, ..., CycleLength-1
/// but also stores an internal cycle counter to be used in compare_exchange
template <uint64_t CycleLength, typename ValueType = uint64_t>
class CyclicIndex
{
  public:
    static_assert(std::is_unsigned<ValueType>::value, "ValueType must be an unsigned integral type");
    static_assert(CycleLength >= 1U, "CycleLength must be >= 1");

    using value_t = ValueType;

    static constexpr ValueType MAX_INDEX = CycleLength - 1U;
    static constexpr ValueType MAX_VALUE = std::numeric_limits<ValueType>::max();

    // assumes MAX_VALUE >= CycleLength, otherwise we could not fit in even one cycle
    static constexpr ValueType MAX_CYCLE = MAX_VALUE / CycleLength;

    static constexpr ValueType INDEX_AT_MAX_VALUE = MAX_VALUE % CycleLength;
    static constexpr ValueType OVERFLOW_START_INDEX = (INDEX_AT_MAX_VALUE + 1U) % CycleLength;

    static_assert(CycleLength < MAX_VALUE / 2U, "CycleLength is too large, need at least one bit for cycle");
    static_assert(CycleLength > 0, "CycleLength must be > 0");

    explicit CyclicIndex(ValueType value = 0U) noexcept;

    CyclicIndex(ValueType index, ValueType cycle) noexcept;

    ~CyclicIndex() = default;

    CyclicIndex(const CyclicIndex&) noexcept = default;
    CyclicIndex(CyclicIndex&&) noexcept = default;
    CyclicIndex& operator=(const CyclicIndex&) noexcept = default;
    CyclicIndex& operator=(CyclicIndex&&) noexcept = default;

    ValueType getIndex() const noexcept;

    ValueType getCycle() const noexcept;

    ValueType getValue() const noexcept;

    CyclicIndex operator+(const ValueType value) const noexcept;

    CyclicIndex next() const noexcept;

    bool isOneCycleBehind(const CyclicIndex& other) const noexcept;

    /// @note The difference will be negative if lhs < rhs (lhs is this) and
    /// its absolute value fits into an int64_t, otherwise it
    /// will be positive and follow the rules of modular arithmetic of unsigned types
    /// This is intended and includes the case were rhs is "very close to 0" and
    /// and lhs is "close" to the MAX of uint64_t (MAX=2^64-1). Here close means that
    /// the real absolute difference would be larger than 2^63.
    /// This is excactly the right behaviour to deal with a (theoretically possible)
    /// overflow of lhs and can be seen as lhs being interpreted as MAX + its actual value.
    /// In this case, lhs - rhs is positive even though lhs < rhs.
    int64_t operator-(const CyclicIndex<CycleLength, ValueType>& rhs) const noexcept;

  private:
    ValueType m_value{0U};
};

} // namespace concurrent
} // namespace iox

#include "iox/detail/mpmc_lockfree_queue/cyclic_index.inl"

#endif // IOX_HOOFS_CONCURRENT_BUFFER_MPMC_LOCKFREE_QUEUE_CYCLIC_INDEX_HPP
