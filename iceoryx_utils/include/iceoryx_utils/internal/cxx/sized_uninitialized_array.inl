// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_CXX_SIZED_UNINITIALIZED_ARRAY_INL
#define IOX_UTILS_CXX_SIZED_UNINITIALIZED_ARRAY_INL

#include <iceoryx_utils/cxx/sized_uninitialized_array.hpp>
#include <iostream>

namespace iox
{
namespace cxx
{
// Generic implementation for Capacity > 0
template <typename T, uint64_t Capacity>
inline uint64_t SizedUninitializedArray<T, Capacity>::size() const noexcept
{
    return m_size;
}

template <typename T, uint64_t Capacity>
inline void SizedUninitializedArray<T, Capacity>::set_size(uint64_t newSize) noexcept
{
    m_size = newSize;
}

template <typename T, uint64_t Capacity>
inline bool SizedUninitializedArray<T, Capacity>::empty() const noexcept
{
    return (m_size == 0U);
}

template <typename T, uint64_t Capacity>
inline bool SizedUninitializedArray<T, Capacity>::full() const noexcept
{
    return (m_size >= Capacity);
}

// Specialization for Capacity 0, where m_size is not needed
template <typename T>
class SizedUninitializedArray<T, 0U> : public UninitializedArray<T, 0U>
{
  public:
    inline uint64_t size() const noexcept
    {
        return 0u;
    }

    inline void set_size(uint64_t newSize [[gnu::unused]]) noexcept
    {
    }

    inline bool empty() const noexcept
    {
        return true;
    }

    inline bool full() const noexcept
    {
        return true;
    }
};

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_SIZED_UNINITIALIZED_ARRAY_INL
