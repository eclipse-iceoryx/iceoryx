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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef ICEORYX_UTILS_CXX_CONTAINER_STORAGE_INL
#define ICEORYX_UTILS_CXX_CONTAINER_STORAGE_INL

#include <iceoryx_utils/cxx/container_storage.hpp>
#include <iostream>

namespace iox
{
namespace cxx
{
// Generic implementation for Capacity > 0
template <typename T, uint64_t Capacity>
inline uint64_t container_storage<T, Capacity>::size() const noexcept
{
    return m_size;
}

template <typename T, uint64_t Capacity>
inline void container_storage<T, Capacity>::set_size(uint64_t newSize) noexcept
{
    if (newSize > Capacity)
    {
        std::cerr << "Illegal call of set_size(" << newSize << ") exceeds Capacity=" << Capacity << "." << std::endl;
        std::terminate();
    }
    m_size = newSize;
}

template <typename T, uint64_t Capacity>
inline bool container_storage<T, Capacity>::empty() const noexcept
{
    return (m_size == 0U);
}

template <typename T, uint64_t Capacity>
inline bool container_storage<T, Capacity>::full() const noexcept
{
    return (m_size >= Capacity);
}

// Memory-optimized specialization for Capacity 0
template <typename T>
inline uint64_t container_storage<T, 0U>::size() const noexcept
{
    return 0U;
}

template <typename T>
inline void container_storage<T, 0U>::set_size(uint64_t newSize) noexcept
{
    if (newSize != 0U)
    {
        std::cerr << "Illegal call of set_size(" << newSize << ") exceeds Capacity=0." << std::endl;
        std::terminate();
    }
}

template <typename T>
inline bool container_storage<T, 0U>::empty() const noexcept
{
    return true;
}

template <typename T>
inline bool container_storage<T, 0U>::full() const noexcept
{
    return true;
}
} // namespace cxx
} // namespace iox

#endif // ICEORYX_UTILS_CXX_CONTAINER_STORAGE_INL
