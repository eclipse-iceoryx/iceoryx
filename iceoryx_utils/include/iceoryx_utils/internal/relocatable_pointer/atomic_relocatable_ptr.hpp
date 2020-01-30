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

#pragma once

#include <atomic>
#include <limits>

namespace iox
{
///@brief minimalistic relocatable pointer that can be written and read atomically
/// and can be stored safely in shared memory.
/// As the basic relocatable_ptr, it must point to something in the same shared memory segment as itself
/// since the internally used offset must be an invariant different across adress spaces.
/// Rationale: the default relocatable_ptr cannot be used in an atomic since the copy ctor is nontrivial.
template <typename T>
class atomic_relocatable_ptr
{
  public:
    using offset_t = std::ptrdiff_t;
    static constexpr offset_t NULL_POINTER_OFFSET = std::numeric_limits<offset_t>::max();

    atomic_relocatable_ptr(const T* ptr = nullptr);

    ///@todo: can be implemented when needed, note that the offset must be recomputed during the move/copy
    atomic_relocatable_ptr(const atomic_relocatable_ptr&) = delete;
    atomic_relocatable_ptr& operator=(const atomic_relocatable_ptr& other) = delete;

    atomic_relocatable_ptr(atomic_relocatable_ptr&& other) = delete;
    atomic_relocatable_ptr& operator=(atomic_relocatable_ptr&& other) = delete;

    // minimal set of required operators, can be extended later

    atomic_relocatable_ptr& operator=(const T* ptr) noexcept;

    T* operator->() const noexcept;

    T& operator*() const noexcept;

    operator T*() const noexcept;

  private:
    std::atomic<offset_t> m_offset{NULL_POINTER_OFFSET};

    inline T* computeRawPtr() const;

    inline offset_t computeOffset(const void* ptr) const;
};

} // namespace iox

#include "iceoryx_utils/internal/relocatable_pointer/atomic_relocatable_ptr.inl"