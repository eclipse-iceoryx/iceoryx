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
/// @brief static storage class to allocate memory for objects not yet known
///        this storage is not aware of any underlying type

/// @note we can define optimized_storage (or dynamic_storage) with a similar interface
///       but other allocation policies and use them where we need to store objects
///       with some interchangable storage policy (e.g. in storable_function)
///       optimized_storage would have a dynamic memory fallback when static memory is
///       insufficent
template <uint64_t Capacity, uint64_t Align = 1>
class static_storage
{
  private:
    alignas(Align) uint8_t m_bytes[Capacity];
    void* m_ptr{nullptr};

    static constexpr uint64_t align_delta(uint64_t align, uint64_t alignTarget);

  public:
    static_storage() = default;

    ~static_storage();

    // it is not supposed to be copied or moved for now
    // (construct a new one explicitly and populate it instead)
    // note: move would just be  copy and copy has the problem of
    // that it would just can perform a memcpy, regardless of what is stored
    // leading to potentially missed dtor calls
    static_storage(const static_storage&) = delete;
    static_storage& operator=(const static_storage&) = delete;
    static_storage(static_storage&&) = delete;
    static_storage& operator=(static_storage&&) = delete;

    /// @brief check whether the type T will fit in the buffer statically at compile time
    template <typename T>
    static constexpr bool fits_statically();

    /// @brief provide static memory for an object of type T
    /// @note  compilation fails if static memory is insufficient
    template <typename T>
    T* allocate();

    /// @brief provide align aligned memory with a specific size
    void* allocate(uint64_t align, uint64_t size);

    /// @brief mark the static memory as unused
    /// @note no dtor of the stored type is called (we cannot know the type)
    ///       nor is it overwritten
    void deallocate();

    /// @brief set the managed static memory to all zeros
    void clear();
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/static_storage.inl"
#endif // IOX_UTILS_STATIC_STORAGE_HPP
