// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_MEMORY_RELATIVE_POINTER_DATA_HPP
#define IOX_HOOFS_MEMORY_RELATIVE_POINTER_DATA_HPP

#include "iox/type_traits.hpp"

#include <cstdint>
#include <limits>

namespace iox
{
/// @brief This are the data for a relative pointer. To be able so safely be used in the shared memory and prevent torn
/// writes/reads, the class must not be larger than 64 bits and trivially copy-able.
class RelativePointerData
{
  public:
    using identifier_t = uint16_t;
    using offset_t = uint64_t;

    /// @brief Default constructed RelativePointerData which is logically equal to a nullptr
    constexpr RelativePointerData() noexcept = default;

    /// @brief constructs a RelativePointerData from a given offset and segment id
    /// @param[in] id is the unique id of the segment
    /// @param[in] offset is the offset within the segment
    constexpr RelativePointerData(identifier_t id, offset_t offset) noexcept;

    /// @brief Getter for the id which identifies the segment
    /// @return the id which identifies the segment
    identifier_t id() const noexcept;

    /// @brief Getter for the offset within the segment
    /// @return the offset
    offset_t offset() const noexcept;

    /// @brief Resets the pointer to a logically nullptr
    void reset() noexcept;

    /// @brief Checks if the pointer is logically a nullptr
    /// @return true if logically a nullptr otherwise false
    bool isLogicalNullptr() const noexcept;

    /// @note the maximum number of available ids
    static constexpr identifier_t ID_RANGE{std::numeric_limits<identifier_t>::max()};
    /// @note this represents the id of a logically nullptr
    static constexpr identifier_t NULL_POINTER_ID{ID_RANGE};
    /// @note the maximum number of valid ids
    static constexpr identifier_t MAX_VALID_ID{ID_RANGE - 1U};
    /// identifier_t is 16 bit and the offset consumes the remaining 48 bits -> offset range is 2^48 - 1
    static constexpr offset_t OFFSET_RANGE{(1ULL << 48U) - 1U};
    /// @note this represents the offset of a logically nullptr;
    static constexpr offset_t NULL_POINTER_OFFSET{OFFSET_RANGE};
    /// @note the maximum offset which can be represented
    static constexpr offset_t MAX_VALID_OFFSET{OFFSET_RANGE - 1U};
    /// @note the maximum allowed size of RelativePointerData
    static constexpr uint64_t MAX_ALLOWED_SIZE_OF_RELATIVE_POINTER_DATA{8U};

  private:
    /// @note offset in bits for storing and reading the id
    static constexpr uint64_t ID_BIT_SIZE{16U};
    /// @note internal representation of a nullptr
    static constexpr offset_t LOGICAL_NULLPTR{(NULL_POINTER_OFFSET << ID_BIT_SIZE) | NULL_POINTER_ID};
    uint64_t m_idAndOffset{LOGICAL_NULLPTR};
};

} // namespace iox

#include "iox/detail/relative_pointer_data.inl"

#endif // IOX_HOOFS_MEMORY_RELATIVE_POINTER_DATA_HPP
