// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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
#ifndef ICEORYX_UTILS_CXX_UNINITIALIZED_ARRAY_HPP
#define ICEORYX_UTILS_CXX_UNINITIALIZED_ARRAY_HPP

#include <cstdint>

namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
class uninitialized_array
{
  public:
    using value_type = T;
    using size_type = decltype(Capacity);

    uninitialized_array() = default;

    /// @brief return the pointer to the underlying array
    /// @return pointer to underlying array
    ///          If the vector is empty it returns nullptr
    T* data() noexcept;

    /// @brief return the const pointer to the underlying array
    /// @return const pointer to underlying array
    const T* data() const noexcept;


    /// @brief returns the capacity, which was given via the template argument,
    ///         i.e., the maximum number of elements it can hold
    uint64_t capacity() const noexcept;


  protected:
    using element_t = uint8_t[Capacity == 0 ? 1 : sizeof(T)];

  private:
    alignas(Capacity == 0 ? 1 : alignof(T)) element_t m_data[Capacity == 0 ? 1 : Capacity];
};

} // namespace cxx
} // namespace iox

#include <iceoryx_utils/internal/cxx/uninitialized_array.inl>

#endif /* ICEORYX_UTILS_CXX_UNINITIALIZED_ARRAY_HPP */
