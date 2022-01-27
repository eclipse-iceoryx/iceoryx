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
// It is crucial that objects of this type are relocatable.
template <typename T, uint64_t Capacity>
class TypedAllocator
{
    using index_t = decltype(Capacity);

    static constexpr index_t INVALID_INDEX = Capacity;

  public:
    TypedAllocator()
    {
    }

    // We do not need a specific dtor.
    // The dtor does not check whether all pointers were deallocated (the memory will just be gone).

    // This is ok since for regular memory allocation it is also an error not to free
    // something allocated.

    // convenience to ensure we get a valid object (like operator new)
    template <typename... Args>
    T* create(Args&&... args)
    {
        auto element = allocate();
        if (element)
        {
            return new (element) T(std::forward<Args>(args)...);
        }

        return nullptr;
    }

    void destroy(T* element)
    {
        if (element)
        {
            element->~T();
            deallocate(element);
        }
    }

    T* allocate()
    {
        auto maybeIndex = m_freeIndices.pop();
        if (maybeIndex.has_value())
        {
            return toPtr(*maybeIndex);
        }
        return nullptr;
    }

    void deallocate(T* element)
    {
        // no checking whether this is a pointer allocated by this allocator (
        // (not possible without serious overhead)
        m_freeIndices.push(toIndex(element));
    }

  private:
    using block_t = alignas(alignof(T)) uint8_t[sizeof(T)];
    using queue_t = iox::concurrent::IndexQueue<Capacity, index_t>;

    block_t m_blocks[Capacity];

    // NB: must be relocatable
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


#endif