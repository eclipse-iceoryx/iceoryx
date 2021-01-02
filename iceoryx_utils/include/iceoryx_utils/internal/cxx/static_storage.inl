// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_CXX_STATIC_STORAGE_INL
#define IOX_UTILS_CXX_STATIC_STORAGE_INL

#include "iceoryx_utils/internal/cxx/static_storage.hpp"

namespace iox
{
namespace cxx
{
template <uint64_t Capacity, uint64_t Align>
constexpr uint64_t static_storage<Capacity, Align>::align_delta(uint64_t align, uint64_t alignTarget)
{
    auto r = align % alignTarget;

    // if r != 0 we are not aligned and need to add this amount to an align
    // aligned address to be aligned with alignTarget
    return r != 0 ? alignTarget - r : 0;
}

template <uint64_t Capacity, uint64_t Align>
static_storage<Capacity, Align>::~static_storage()
{
    deallocate();
}

template <uint64_t Capacity, uint64_t Align>
template <typename T>
constexpr bool static_storage<Capacity, Align>::fits_statically()
{
    return sizeof(T) + align_delta(alignof(m_bytes), alignof(T)) <= Capacity;
}

template <uint64_t Capacity, uint64_t Align>
template <typename T>
T* static_storage<Capacity, Align>::allocate()
{
    static_assert(fits_statically<T>(), "type does not fit into static storage");
    return reinterpret_cast<T*>(allocate(alignof(T), sizeof(T)));
}

template <uint64_t Capacity, uint64_t Align>
void* static_storage<Capacity, Align>::allocate(uint64_t align, uint64_t size)
{
    if (m_ptr)
    {
        return nullptr; // cannot allocate, already in use
    }

    uint64_t space = Capacity;
    m_ptr = m_bytes;
    if (std::align(align, size, m_ptr, space))
    {
        // fits, ptr was potentially modified to reflect alignent
        return m_ptr;
    }

    // does not fit
    return nullptr;
}

template <uint64_t Capacity, uint64_t Align>
void static_storage<Capacity, Align>::deallocate()
{
    m_ptr = nullptr;
}

template <uint64_t Capacity, uint64_t Align>
void static_storage<Capacity, Align>::clear()
{
    std::memset(m_bytes, 0, Capacity);
}


} // namespace cxx
} // namespace iox


#endif // IOX_UTILS_STATIC_STORAGE_INL