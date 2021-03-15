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

#ifndef IOX_UTILS_RELOCATABLE_POINTER_BASE_RELOCATABLE_PTR_HPP
#define IOX_UTILS_RELOCATABLE_POINTER_BASE_RELOCATABLE_PTR_HPP

#include <cstdint>
#include <limits>

namespace iox
{
class BaseRelocatablePointer
{
    template <typename T>
    friend class relocatable_ptr;

  public:
    using offset_t = std::ptrdiff_t;

    BaseRelocatablePointer() noexcept;

    explicit BaseRelocatablePointer(const void* ptr) noexcept;

    BaseRelocatablePointer(const BaseRelocatablePointer& other) noexcept;

    BaseRelocatablePointer(BaseRelocatablePointer&& other) noexcept;

    BaseRelocatablePointer& operator=(const BaseRelocatablePointer& other) noexcept;

    BaseRelocatablePointer& operator=(const void* rawPtr) noexcept;

    BaseRelocatablePointer& operator=(BaseRelocatablePointer&& other) noexcept;

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

} // namespace iox

#endif // IOX_UTILS_RELOCATABLE_POINTER_BASE_RELOCATABLE_PTR_HPP
