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
template <typename ElementType, uint64_t Capacity>
struct FirstElementZeroed
{
    // NOLINTJUSTIFICATION required by low level UnitializedArray building block and encapsulated in abstraction
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    using element_t = cxx::byte_t[sizeof(ElementType)];
    alignas(ElementType) element_t value[Capacity]{{0}};
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
};

template <typename ElementType, uint64_t Capacity>
struct UninitializedBuffer
{
    // NOLINTJUSTIFICATION required by low level UnitializedArray building block and encapsulated in abstraction
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    using element_t = cxx::byte_t[sizeof(ElementType)];
    alignas(ElementType) element_t value[Capacity];
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
};

template <typename ElementType,
          uint64_t Capacity,
          typename index_t = uint64_t,
          template <typename, uint64_t> class Buffer = UninitializedBuffer>
class UninitializedArray
{
  public:
    constexpr UninitializedArray() noexcept = default;

    constexpr ElementType& operator[](const index_t index) noexcept;

    constexpr const ElementType& operator[](const index_t index) const noexcept;

    constexpr ElementType* ptr(const index_t index) noexcept;

    constexpr const ElementType* ptr(const index_t index) const noexcept;

    static constexpr uint64_t capacity() noexcept;

  private:
    constexpr ElementType* toPtr(index_t index) const noexcept;

    Buffer<ElementType, Capacity> m_buffer;
};

} // namespace containers
} // namespace iox

#include "iceoryx_hoofs/internal/containers/uninitialized_array.inl"

#endif // IOX_HOOFS_CONTAINERS_UNINITIALIZED_ARRAY_HPP
