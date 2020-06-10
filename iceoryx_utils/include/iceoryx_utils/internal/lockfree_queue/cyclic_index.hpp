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

#ifndef IOX_UTILS_LOCKFREE_QUEUE_CYCLIC_INDEX_HPP
#define IOX_UTILS_LOCKFREE_QUEUE_CYCLIC_INDEX_HPP

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

    explicit CyclicIndex(ValueType value = 0) noexcept;

    CyclicIndex(ValueType index, ValueType cycle) noexcept;

    CyclicIndex(const CyclicIndex&) = default;
    CyclicIndex(CyclicIndex&&) = default;
    CyclicIndex& operator=(const CyclicIndex&) = default;
    CyclicIndex& operator=(CyclicIndex&&) = default;

    ValueType getIndex() const noexcept;

    ValueType getCycle() const noexcept;

    CyclicIndex operator+(const ValueType value) const noexcept;

    CyclicIndex next() const noexcept;

    bool isOneCycleBehind(const CyclicIndex& other) const noexcept;

  private:
    ValueType m_value{0};
};

} // namespace iox

#include "cyclic_index.inl"

#endif // IOX_UTILS_LOCKFREE_QUEUE_CYCLIC_INDEX_HPP