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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP
#define IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP

#include "base_relative_ptr.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace iox
{
namespace rp
{
template <typename T>
class RelativePointer : public BaseRelativePointer
{
  public:
    RelativePointer(ptr_t ptr, id_t id) noexcept;

    RelativePointer(offset_t offset, id_t id) noexcept;

    RelativePointer(ptr_t ptr = nullptr) noexcept;

    RelativePointer(const BaseRelativePointer& other) noexcept;

    RelativePointer& operator=(const BaseRelativePointer& other) noexcept;

    RelativePointer& operator=(ptr_t ptr) noexcept;

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, U&>::type operator*() noexcept;

    T* operator->() noexcept;

    template <typename U = T>
    typename std::enable_if<!std::is_void<U>::value, const U&>::type operator*() const noexcept;

    T* operator->() const noexcept;

    T* get() const noexcept;

    operator T*() const noexcept;

    bool operator==(T* const ptr) const noexcept;

    bool operator!=(T* const ptr) const noexcept;
};

} // namespace rp
} // namespace iox

#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.inl"

#endif // IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_PTR_HPP
