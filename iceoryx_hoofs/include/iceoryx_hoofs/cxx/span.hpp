// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_SPAN_HPP
#define IOX_HOOFS_CXX_SPAN_HPP

#include <cstdint>

namespace iox
{
namespace cxx
{
template <typename T>
class span
{
  public:
    span() noexcept = default;
    span(T* const data, const uint64_t size) noexcept;

    T& operator[](const uint64_t index) noexcept;
    const T& operator[](const uint64_t index) const noexcept;

    uint64_t size() const noexcept;

  private:
    T* m_data{nullptr};
    uint64_t m_size{0U};
};
} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/span.inl"

#endif
