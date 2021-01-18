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

#ifndef ICEORYX_UTILS_CXX_SIZED_UNINITIALIZED_ARRAY_HPP
#define ICEORYX_UTILS_CXX_SIZED_UNINITIALIZED_ARRAY_HPP

#include "iceoryx_utils/cxx/uninitialized_array.hpp"
#include <cstdint>

namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
class SizedUninitializedArray;

template <typename T>
class SizedUninitializedArray<T, 0U>;

template <typename T, uint64_t Capacity>
class SizedUninitializedArray : public UninitializedArray<T, Capacity>
{
  public:
    SizedUninitializedArray() = default;

    /// @brief returns the number of elements which are currently stored in the
    ///         vector
    uint64_t size() const noexcept;

    /// @brief update the size
    /// @param [in] newSize new number of elements which are currently stored in the
    ///         vector.
    void set_size(uint64_t newSize) noexcept;

  private:
    uint64_t m_size{0u};
};

} // namespace cxx
} // namespace iox

#include <iceoryx_utils/internal/cxx/sized_uninitialized_array.inl>

#endif /* ICEORYX_UTILS_CXX_SIZED_UNINITIALIZED_ARRAY_HPP */
