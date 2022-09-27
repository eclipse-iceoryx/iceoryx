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

#ifndef IOX_HOOFS_RELOCATABLE_POINTER_RELATIVE_POINTER_HPP
#define IOX_HOOFS_RELOCATABLE_POINTER_RELATIVE_POINTER_HPP

#include "iceoryx_hoofs/internal/relocatable_pointer/base_relative_pointer.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace iox
{
namespace rp
{
/// @brief Typed version so we can use operator->
template <typename T>
class RelativePointer : public BaseRelativePointer
{
  public:
    using const_ptr_t = const T*;
    using ptr_t = T*;

    /// @brief Default constructs a RelativePointer as a logical nullptr
    RelativePointer() noexcept = default;

    /// @brief Constructs a RelativePointer pointing to the same pointee as ptr in a segment identified by id
    /// @param[in] ptr The pointer whose pointee shall be the same for this
    /// @param[in] id Is the unique id of the segment
    RelativePointer(ptr_t ptr, id_t id) noexcept;

    /// @brief Constructs a RelativePointer from a given offset and segment id
    /// @param[in] offset Is the offset
    /// @param[in] id Is the unique id of the segment
    RelativePointer(offset_t offset, id_t id) noexcept;

    /// @brief Constructs a RelativePointer pointing to the same pointee as ptr
    /// @param[in] ptr the pointer whose pointee shall be the same for this
    explicit RelativePointer(ptr_t ptr) noexcept;

    /// @brief Assigns the RelativePointer to point to the same pointee as ptr
    /// @param[in] ptr The pointer whose pointee shall be the same for this
    /// @return Reference to self
    RelativePointer& operator=(ptr_t ptr) noexcept;

    /// @brief Dereferencing operator which returns a reference to the underlying object
    /// @tparam U a template parameter to enable the dereferencing operator only for non-void T
    /// @return A reference to the underlying object
    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, const U&>::type operator*() const noexcept;

    /// @brief Access to the underlying object. If the RelativePointer does not point to anything the
    /// application terminates.
    /// @return A pointer to the underlying object
    T* operator->() const noexcept;

    /// @brief Access the underlying object.
    /// @return A pointer to the underlying object
    T* get() const noexcept;

    /// @brief Converts the RelativePointer to bool
    /// @return Bool which contains true if the RelativePointer contains a pointer
    explicit operator bool() const noexcept;

    /// @brief Checks if this and ptr point to the same pointee
    /// @param[in] ptr Is the pointer whose pointee is compared with this' pointee
    /// @return True if the pointees are equal, otherwise false
    bool operator==(T* const ptr) const noexcept;

    /// @brief Checks if this and ptr point not to the same pointee
    /// @param[in] ptr Is the pointer whose pointee is compared with this' pointee
    /// @return True if the pointees are not equal, otherwise false
    bool operator!=(T* const ptr) const noexcept;
};

} // namespace rp
} // namespace iox

#include "iceoryx_hoofs/internal/relocatable_pointer/relative_pointer.inl"

#endif // IOX_HOOFS_RELOCATABLE_POINTER_RELATIVE_POINTER_HPP
