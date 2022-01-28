// Copyright (c) 2021-2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_DATA_STRUCTURES_TYPED_ALLOCATOR_HPP
#define IOX_HOOFS_DATA_STRUCTURES_TYPED_ALLOCATOR_HPP

#include <stdint.h>

#include "iceoryx_hoofs/internal/concurrent/lockfree_queue/index_queue.hpp"

namespace iox
{
namespace cxx
{
/// @brief An allocator for objects of a specific type T.
///        Supports allocation of initialized objects and aligned raw memory where objects
///        of type T can be constructed.
///        All allocate and deallocate operations have O(1) complexity.
///        Wink-out: if T does not require a destructor call (non-RAII object, e.g a struct of PODs)
///        or have not been initialized then it is legal to call the destructor of the allocator
///        *without* deallocating or destroying outstanding allocations for added efficiency.
/// @tparam T object type to allocate
/// @tparam Capacity maximum number of objects of T to be allocated at the same time
/// @note relocatable, i.e. logical state can be copied using memcpy
/// @note interface is thread-safe, lock-free
template <typename T, uint64_t Capacity>
class TypedAllocator
{
    using index_t = decltype(Capacity);

    static constexpr index_t INVALID_INDEX = Capacity;

  public:
    TypedAllocator() = default;
    TypedAllocator(const TypedAllocator&) = delete;
    TypedAllocator(TypedAllocator&&) = delete;
    TypedAllocator& operator=(const TypedAllocator&) = delete;
    TypedAllocator& operator=(TypedAllocator&&) = delete;

    /// @brief Allocate memory for an object of type T.
    /// @return T-aligned pointer to object if memory could be obtained, nullptr otherwise
    /// @note thread-safe, lock-free
    T* allocate();

    /// @brief Deallocate memory for an object of type T.
    /// @param element element to be deallocated, must have been obtained by allocate before
    /// @note thread-safe, lock-free
    /// @note There is no efficient way to check whether this element is legal to be deallocated (i.e. has
    ///       been obtained with allocate by this allocator and was not yet dellocated).
    ///       The user is therefore responsible to do so.
    void deallocate(T* element);

    // NB: We do not need a specific dtor.
    // The dtor does not check whether all pointers were deallocated (wink out - the memory will just be gone).
    // This is ok since for regular memory allocation it is also an error not to free something allocated.

    /// @brief Allocate memory for an object of type T and construct it with the given arguments in this memory
    /// @param args construction arguments
    /// @return pointer to object if memory could be obtained, nullptr otherwise
    /// @note thread-safe, lock-free
    /// @note equivalent to allocate followed by emplacement new
    template <typename... Args>
    T* create(Args&&... args);

    /// @brief Destroy an object previously obtained with create and deallocate its memory for further
    /// @param element element to be destroyed
    /// @note thread-safe, lock-free
    /// @note There is no efficient way to check whether this element is legal to be destroyed (i.e. has
    ///       been obtained by this allocator and was not yet destroyed).
    ///       The user is therefore responsible to do so.
    void destroy(T* element);

  private:
    // NB: types must be relocatable
    using block_t = alignas(alignof(T)) uint8_t[sizeof(T)];
    using queue_t = iox::concurrent::IndexQueue<Capacity, index_t>;

    block_t m_blocks[Capacity];
    queue_t m_freeIndices{queue_t::ConstructFull};

    T* toPtr(index_t index)
    {
        return reinterpret_cast<T*>(&m_blocks[index]);
    }

    index_t toIndex(void* ptr)
    {
        block_t* block = reinterpret_cast<block_t*>(ptr);
        index_t index = block - &m_blocks[0];
        return index;
    }
};

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/data_structures/typed_allocator.inl"

#endif // IOX_HOOFS_DATA_STRUCTURES_TYPED_ALLOCATOR_HPP
