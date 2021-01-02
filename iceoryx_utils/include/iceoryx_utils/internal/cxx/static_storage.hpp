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

#ifndef IOX_UTILS_STATIC_STORAGE_HPP
#define IOX_UTILS_STATIC_STORAGE_HPP

#include <cstring>
#include <memory>
#include <stdint.h>

namespace iox
{
namespace cxx
{
/// @brief static storage class which cannot use dynamic memory as a fallback
///        this storage is not aware of any underlying type
template <uint64_t Capacity, uint64_t Align = 1>
class static_storage
{
  private:
    alignas(Align) uint8_t m_bytes[Capacity];
    void* m_ptr{nullptr};

    static constexpr uint64_t align_delta(uint64_t align, uint64_t alignTarget)
    {
        auto r = align % alignTarget;

        // if r != 0 we are not aligned and need to add this amount to an align
        // aligned address to be aligned with alignTarget
        return r != 0 ? alignTarget - r : 0;
    }

  public:
    static_storage() = default;

    ~static_storage()
    {
        deallocate();
    }

    // it is not supposed to be copied or moved for now
    // (construct a new one explicitly and populate it instead)
    // note: move would just be  copy and copy has the problem of
    // that it would just can perform a memcpy, regardless of what is stored
    // leading to potentially missed dtor calls
    static_storage(const static_storage&) = delete;
    static_storage& operator=(const static_storage&) = delete;
    static_storage(static_storage&&) = delete;
    static_storage& operator=(static_storage&&) = delete;

    // check whether the type T will fit in the buffer statically at compile time
    template <typename T>
    static constexpr bool fits_statically()
    {
        return sizeof(T) + align_delta(alignof(m_bytes), alignof(T)) <= Capacity;
    }

    template <typename T>
    T* allocate()
    {
        static_assert(fits_statically<T>(), "type does not fit into static storage");
        return reinterpret_cast<T*>(allocate(alignof(T), sizeof(T)));
    }

    void* allocate(uint64_t align, uint64_t size)
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

    /// @note no dtor of the stored type is called (we cannot know the type)
    ///       it is just marked as unused
    void deallocate()
    {
        m_ptr = nullptr;
    }

    void clear()
    {
        std::memset(m_bytes, 0, Capacity);
    }
};

} // namespace cxx
} // namespace iox
#endif // IOX_UTILS_STATIC_STORAGE_HPP
