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

#ifndef IOX_UTILS_STORAGE_HPP
#define IOX_UTILS_STORAGE_HPP

#include <cstring>
#include <memory>
#include <stdint.h>

namespace iox
{
namespace cxx
{
// simple storage class that allocates on the stack or heap
// can only store one object, whose type is determined at runtime
// can/will be fine tuned later (just proof of concept, we need the same storage size for all different types
// but cannot use dynamic memory in the final stage)

template <uint64_t Capacity, uint64_t Align = 1>
class optimized_storage
{
  private:
    alignas(Align) uint8_t m_bytes[Capacity];
    void* m_ptr{nullptr};
    uint64_t m_dynamic{0};


  public:
    template <typename T>
    T* allocate()
    {
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
            // std::cout << "static storage is large enough" << std::endl;
            return m_ptr;
        }

        // does not fit, allocate on heap (could also return nullptr)

        space = size + align;
        m_ptr = malloc(space);
        if (!m_ptr)
        {
            return nullptr;
        }

        m_dynamic = space;
        auto ptr = m_ptr;
        std::align(align, size, ptr, space);
        return ptr;
    }

    // note: no dtor of the stored type is called (we cannot know the type)
    void deallocate()
    {
        if (m_dynamic > 0)
        {
            free(m_ptr);
            m_dynamic = 0;
        }
        m_ptr = nullptr;
    }

    void clear()
    {
        std::memset(m_bytes, 0, sizeof(m_bytes));
        if (m_dynamic > 0)
        {
            std::memset(m_ptr, 0, m_dynamic);
        }
    }

    optimized_storage() = default;

    // todo: check whether we need intricate copy behavior
    // memcpy is not really advised, depending what is stored
    optimized_storage(const optimized_storage&) = default;
    optimized_storage& operator=(const optimized_storage&) = default;

    // todo: check move behavior
    optimized_storage(optimized_storage&&) = default;
    optimized_storage& operator=(optimized_storage&&) = default;

    ~optimized_storage()
    {
        deallocate();
    }
};

// static storage class which cannot use dynamic memory as a fallback
template <uint64_t Capacity, uint64_t Align = 1>
class static_storage
{
  private:
    alignas(Align) uint8_t m_bytes[Capacity];
    void* m_ptr{nullptr};
    bool m_dynamic{false};

    static constexpr uint64_t align_delta(uint64_t align, uint64_t alignTarget)
    {
        auto r = align % alignTarget;

        // if r != 0 we are not aligned and need to add this ammount to an align
        // aligned address to be aligned with alignTarget
        return r != 0 ? alignTarget - r : 0;
    }

  public:
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
    void deallocate()
    {
        if (m_dynamic)
        {
            free(m_ptr);
            m_dynamic = false;
        }
        m_ptr = nullptr;
    }

    void clear()
    {
        std::memset(m_bytes, 0, sizeof(m_bytes));
    }

    static_storage() = default;

    ///@ todo: check whether we need more intricate copy/move behavior
    static_storage(const static_storage&) = default;
    static_storage& operator=(const static_storage&) = default;

    static_storage(static_storage&&) = default;
    static_storage& operator=(static_storage&&) = default;

    ~static_storage()
    {
        deallocate();
    }
};

} // namespace cxx
} // namespace iox
#endif // IOX_UTILS_STORAGE_HPP
