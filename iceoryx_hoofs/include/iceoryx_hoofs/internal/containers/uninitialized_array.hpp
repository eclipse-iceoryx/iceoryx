// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_HPP
#define IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_HPP

#include "iceoryx_hoofs/iceoryx_hoofs_types.hpp"

#include <cstdint>

namespace iox
{
namespace containers
{
/// @brief struct used as policy parameter in UninitializedArray to wrap an array with all elements zeroed
/// @tparam ElementType is the array type
/// @tparam Capacity is the array size
template <typename ElementType, uint64_t Capacity>
struct ZeroedBuffer
{
    // AXIVION Next Construct AutosarC++19_03-A18.1.1 : required by low level UninitializedArray building block and encapsulated in abstraction
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    using element_t = cxx::byte_t[sizeof(ElementType)];
    // AXIVION Next Construct AutosarC++19_03-A18.1.1 : required by low level UninitializedArray building block and encapsulated in abstraction
    alignas(ElementType) element_t value[Capacity]{};
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
};

/// @brief struct used as policy parameter in UninitializedArray to wrap an uninitialized array
/// @tparam ElementType is the array type
/// @tparam Capacity is the array size
template <typename ElementType, uint64_t Capacity>
struct NonZeroedBuffer
{
    // AXIVION Next Construct AutosarC++19_03-A18.1.1 : required by low level UninitializedArray building block and encapsulated in abstraction
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    using element_t = cxx::byte_t[sizeof(ElementType)];
    // AXIVION Next Construct AutosarC++19_03-A18.1.1 : required by low level UninitializedArray building block and encapsulated in abstraction
    alignas(ElementType) element_t value[Capacity];
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
};

/// @brief Wrapper class for a C-style array of type ElementType and size Capacity. Per default it is uninitialized but
/// all elements can be zeroed via template parameter ZeroedBuffer.
/// @tparam ElementType is the array type
/// @tparam Capacity is the array size
/// @tparam Buffer is the policy parameter to choose between an uninitialized, not zeroed array (=NonZeroedBuffer,
/// default) and an uninitialized array with all elements zeroed (=ZeroedBuffer)
/// @note Out of bounds access leads to undefined behavior
template <typename ElementType, uint64_t Capacity, template <typename, uint64_t> class Buffer = NonZeroedBuffer>
class UninitializedArray
{
    static_assert(Capacity > 0U, "The size of the UninitializedArray must be greater than 0!");

  public:
    using value_type = ElementType;
    using iterator = ElementType*;
    using const_iterator = const ElementType*;

    constexpr UninitializedArray() noexcept = default;
    UninitializedArray(const UninitializedArray&) = delete;
    UninitializedArray(UninitializedArray&&) = delete;
    UninitializedArray& operator=(const UninitializedArray&) = delete;
    UninitializedArray& operator=(UninitializedArray&&) = delete;
    ~UninitializedArray() = default;

    /// @brief returns a reference to the element stored at index
    /// @param[in] index position of the element to return
    /// @return reference to the element
    /// @note out of bounds access leads to undefined behavior
    constexpr ElementType& operator[](const uint64_t index) noexcept;

    /// @brief returns a const reference to the element stored at index
    /// @param[in] index position of the element to return
    /// @return const reference to the element
    /// @note out of bounds access leads to undefined behavior
    constexpr const ElementType& operator[](const uint64_t index) const noexcept;

    /// @brief returns an iterator to the beginning of the UninitializedArray
    iterator begin() noexcept;

    /// @brief returns a const iterator to the beginning of the UninitializedArray
    const_iterator begin() const noexcept;

    /// @brief returns an iterator to the end of the UninitializedArray
    iterator end() noexcept;

    /// @brief returns a const iterator to the end of the UninitializedArray
    const_iterator end() const noexcept;

    /// @brief returns the array capacity
    static constexpr uint64_t capacity() noexcept;

  private:
    Buffer<ElementType, Capacity> m_buffer;
};

} // namespace containers
} // namespace iox

#include "iceoryx_hoofs/internal/containers/uninitialized_array.inl"

#endif // IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_HPP
