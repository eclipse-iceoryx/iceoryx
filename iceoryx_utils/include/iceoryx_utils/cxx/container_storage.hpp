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

#ifndef ICEORYX_UTILS_CXX_SIZE_VALUE_HPP
#define ICEORYX_UTILS_CXX_SIZE_VALUE_HPP

#include "iceoryx_utils/cxx/uninitialized_array.hpp"
#include <cstdint>

namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
class container_storage;

template <typename T>
class container_storage<T, 0U>;

template <typename T, uint64_t Capacity>
class container_storage : public uninitialized_array<T, Capacity>
{
  public:
    container_storage() = default;

    /// @brief returns the number of elements which are currently stored in the
    ///         vector
    uint64_t size() const noexcept;

    /// @brief update the size
    /// @param [in] newSize new number of elements which are currently stored in the
    ///         vector.
    void set_size(uint64_t newSize) noexcept;

    /// @brief returns whether the data structure is empty
    /// @return true, if it contains no elements, false otherwise
    bool empty() const noexcept;

    /// @brief returns whether the data structure is completely full
    /// @return true, if filled with max_size() elements, false otherwise
    bool full() const noexcept;

  private:
    uint64_t m_size{0u};
};

} // namespace cxx
} // namespace iox

#include <iceoryx_utils/internal/cxx/container_storage.inl>

#endif /* ICEORYX_UTILS_CXX_SIZE_VALUE_HPP */
