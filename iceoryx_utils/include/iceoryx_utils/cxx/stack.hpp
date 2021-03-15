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
#ifndef IOX_UTILS_CXX_STACK_HPP
#define IOX_UTILS_CXX_STACK_HPP

#include "iceoryx_utils/cxx/optional.hpp"
#include <cstdint>

namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
class stack
{
  public:
    cxx::optional<T> pop() noexcept;

    template <typename... Targs>
    bool push(Targs&&... args) noexcept;

    uint64_t size() noexcept;

    static constexpr uint64_t capacity() noexcept;

  private:
    using element_t = uint8_t[sizeof(T)];
    alignas(alignof(T)) element_t m_data[Capacity];
    uint64_t m_size = 0U;
};
} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/stack.inl"

#endif
