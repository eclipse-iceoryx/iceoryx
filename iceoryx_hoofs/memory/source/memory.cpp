// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iox/memory.hpp"

#include <cstdlib>

namespace iox
{
void* alignedAlloc(const size_t alignment, const size_t size) noexcept
{
    // -1 == since the max alignment addition is alignment - 1 otherwise the
    // memory is already aligned and we have to do nothing
    // low-level memory management, no other approach then to use malloc to acquire heap memory
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-pro-type-reinterpret-cast,hicpp-no-malloc,cppcoreguidelines-no-malloc)
    auto memory = reinterpret_cast<size_t>(std::malloc(size + alignment + sizeof(void*) - 1));
    if (memory == 0)
    {
        return nullptr;
    }
    size_t alignedMemory = align(memory + sizeof(void*), alignment);
    assert(alignedMemory >= memory + 1);
    // low-level memory management, we have to store the actual start of the memory a position before the
    // returned aligned address to be able to release the actual memory address again with free when we
    // only get the aligned address
    // NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
    reinterpret_cast<void**>(alignedMemory)[-1] = reinterpret_cast<void*>(memory);

    // we have to return a void pointer to the aligned memory address
    // NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<void*>(alignedMemory);
}

void alignedFree(void* const memory) noexcept
{
    if (memory != nullptr)
    {
        // low-level memory management
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc,
        // cppcoreguidelines-pro-bounds-pointer-arithmetic)
        // NOLINTNEXTLINE
        std::free(reinterpret_cast<void**>(memory)[-1]);
    }
}
} // namespace iox
