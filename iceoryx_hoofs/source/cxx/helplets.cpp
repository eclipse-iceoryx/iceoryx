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

#include "iceoryx_hoofs/cxx/helplets.hpp"

namespace iox
{
namespace cxx
{
// AXIVION DISABLE STYLE AutosarC++19_03-A5.2.4 : allow reinterpret_cast for the whole file
void* alignedAlloc(const uint64_t alignment, const uint64_t size) noexcept
{
    // -1 == since the max alignment addition is alignment - 1 otherwise the
    // memory is already aligned and we have to do nothing
    // low-level memory management
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc, hicpp-use-auto)
    // AXIVION Next Line AutosarC++19_03-M5.2.9 : Pointer conversion is safe here.
    // AXIVION Next Line AutosarC++19_03-A18.5.1 : Dynamic memory management function is required.
    const uint64_t memory = reinterpret_cast<uint64_t>(malloc((size + alignment + sizeof(void*)) - 1));
    if (memory == 0)
    {
        return nullptr;
    }
    
    const uint64_t alignedMemory = align(memory + sizeof(void*), alignment);
    assert(alignedMemory >= (memory + 1));
    // low-level memory management
    // NOLINTNEXTLINE(performance-no-int-to-ptr, cppcoreguidelines-pro-bounds-pointer-arithmetic)
    // AXIVION Next Line AutosarC++19_03-M5.2.8 : Cast required for conversion and has no side-effects.
    reinterpret_cast<void**>(alignedMemory)[-1] = reinterpret_cast<void*>(memory);

    // NOLINTNEXTLINE(performance-no-int-to-ptr) low-level memory management
    return reinterpret_cast<void*>(alignedMemory);
}

// AXIVION Construct AutosarC++19_03-M7.1.2 : idiomatic for deallocate
void alignedFree(void* const memory) noexcept
{
    if (memory != nullptr)
    {
        // low-level memory management
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc,
        // cppcoreguidelines-pro-bounds-pointer-arithmetic)
        // NOLINTNEXTLINE
        // AXIVION Next Line AutosarC++19_03-M5.2.8 : Cast required for conversion and has no side-effects.
        // AXIVION Next Line AutosarC++19_03-A18.5.1 : Dynamic memory management function is required.
        free(reinterpret_cast<void**>(memory)[-1]);
    }
}
// AXIVION ENABLE STYLE AutosarC++19_03-A5.2.4
} // namespace cxx
} // namespace iox
