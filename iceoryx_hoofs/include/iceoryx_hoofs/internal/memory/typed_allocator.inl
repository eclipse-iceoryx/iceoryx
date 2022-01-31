// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_MEMORY_TYPED_ALLOCATOR_INL
#define IOX_HOOFS_MEMORY_TYPED_ALLOCATOR_INL

#include "iceoryx_hoofs/memory/typed_allocator.hpp"

#include <stdint.h>

namespace iox
{
namespace memory
{
template <typename T, uint64_t Capacity>
T* TypedAllocator<T, Capacity>::allocate()
{
    auto maybeIndex = m_freeIndices.pop();
    if (maybeIndex.has_value())
    {
        return toPtr(*maybeIndex);
    }
    return nullptr;
}

template <typename T, uint64_t Capacity>
void TypedAllocator<T, Capacity>::deallocate(T* element)
{
    // no checking whether this is a pointer allocated by this allocator (
    // (not possible without serious overhead)
    m_freeIndices.push(toIndex(element));
}

template <typename T, uint64_t Capacity>
template <typename... Args>
T* TypedAllocator<T, Capacity>::create(Args&&... args)
{
    auto element = allocate();
    if (element)
    {
        return new (element) T(std::forward<Args>(args)...);
    }

    return nullptr;
}

template <typename T, uint64_t Capacity>
void TypedAllocator<T, Capacity>::destroy(T* element)
{
    if (element)
    {
        element->~T();
        deallocate(element);
    }
}

} // namespace memory
} // namespace iox

#endif // IOX_HOOFS_MEMORY_TYPED_ALLOCATOR_INL