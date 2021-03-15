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

#ifndef IOX_UTILS_RELOCATABLE_POINTER_RELOCATABLE_PTR_HPP
#define IOX_UTILS_RELOCATABLE_POINTER_RELOCATABLE_PTR_HPP

#include "base_relocatable_ptr.hpp"

namespace iox
{
/// @brief typed version so we can use operator->
template <typename T>
class RelocatablePointer : public BaseRelocatablePointer
{
  public:
    RelocatablePointer() noexcept;

    RelocatablePointer(const T* ptr) noexcept;

    RelocatablePointer(const BaseRelocatablePointer& other) noexcept;

    RelocatablePointer(T* rawPtr) noexcept;

    RelocatablePointer& operator=(const BaseRelocatablePointer& other) noexcept;

    T& operator*() noexcept;

    T* operator->() noexcept;

    const T& operator*() const noexcept;

    const T* operator->() const noexcept;

    T& operator[](uint64_t index) noexcept;

    operator T*() const noexcept;
};
} // namespace iox

#include "iceoryx_utils/internal/relocatable_pointer/relocatable_ptr.inl"

#endif // IOX_UTILS_RELOCATABLE_POINTER_RELOCATABLE_PTR_HPP
