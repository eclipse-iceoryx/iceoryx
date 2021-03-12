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

#include <cstdint>
#include <iostream>
#include <limits>

namespace iox
{
/// @todo restructure and split into multiple files along with an implementation cpp for non-template code
class RelocatablePointer
{
    template <typename T>
    friend class relocatable_ptr;

  public:
    using offset_t = std::ptrdiff_t;

    RelocatablePointer() noexcept;

    explicit RelocatablePointer(const void* ptr) noexcept;

    RelocatablePointer(const RelocatablePointer& other) noexcept;

    RelocatablePointer(RelocatablePointer&& other) noexcept;

    RelocatablePointer& operator=(const RelocatablePointer& other) noexcept;

    RelocatablePointer& operator=(const void* rawPtr) noexcept;

    RelocatablePointer& operator=(RelocatablePointer&& other) noexcept;

    const void* operator*() const noexcept;

    operator bool() const noexcept;

    bool operator!() const noexcept;

    void* get() const noexcept;

    offset_t getOffset() const noexcept;

    static constexpr offset_t NULL_POINTER_OFFSET = std::numeric_limits<offset_t>::max();

  protected:
    offset_t m_offset{NULL_POINTER_OFFSET};

    offset_t computeOffset(const void* ptr) const noexcept;

    void* computeRawPtr() const noexcept;
};

/// @brief typed version so we can use operator->
template <typename T>
class relocatable_ptr : public RelocatablePointer
{
  public:
    relocatable_ptr() noexcept;

    relocatable_ptr(const T* ptr) noexcept;

    relocatable_ptr(const RelocatablePointer& other) noexcept;

    relocatable_ptr(T* rawPtr) noexcept;

    relocatable_ptr& operator=(const RelocatablePointer& other) noexcept;

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
