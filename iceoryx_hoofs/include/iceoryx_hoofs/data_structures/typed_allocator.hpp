// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/concurrent/fifo.hpp"

namespace iox
{
// since all the blocks have a fixed size, allocation and deallocation takes constant time
// allocator can be used e.g. to create linked data structures
template <typename T, uint64_t Capacity>
class typed_allocator
{
  public:
    typed_allocator()
    {
        for (uint64_t i = 0; i < Capacity; ++i)
        {
            // push the pointers to elements, they are correctly aligned for the type
            auto element = reinterpret_cast<T*>(&blocks[i]);
            freeElements.push(element);
        }
    }

    // Te do not need a specific dtor
    // The dtor does not check whether all pointers were deallocated
    // (the memory will just be gone)
    // This is ok since for regular memory allocation it is also an error not to free
    // something allocated etc and the user must ensure the allocator outlives any allocation.

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
        auto maybeElement = freeElements.pop();
        if (maybeElement.has_value())
        {
            return *maybeElement;
        }
        return nullptr;
    }

    void deallocate(T* element)
    {
        // no checking whether this is a pointer allocated by this allocator (
        // (not possible without serious overhead)
        freeElements.push(element);
    }


  private:
    // TODO: correct alignas usage
    // if individual alignment is not possible use overallocation by alignof(T)
    using block = alignas(alignof(T)) uint8_t[sizeof(T)];

    block blocks[Capacity];

    // actually we just need a non-concurrent queue but do not have one
    // (if the allocator is just used inside non-concurrent classes)
    /// @todo simple non-concurrent queue?
    // could make queue template argument and have a concurrent and non-concurrent version
    iox::concurrent::FiFo<T*, Capacity> freeElements;
};

} // namespace iox


#endif