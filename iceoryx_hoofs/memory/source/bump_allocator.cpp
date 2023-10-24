// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/bump_allocator.hpp"
#include "iceoryx_platform/platform_correction.hpp"
#include "iox/logging.hpp"
#include "iox/memory.hpp"

#include <iostream>

namespace iox
{
BumpAllocator::BumpAllocator(void* const startAddress, const uint64_t length) noexcept
    // AXIVION Next Construct AutosarC++19_03-A5.2.4, AutosarC++19_03-M5.2.9 : required for low level memory management
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    : m_startAddress(reinterpret_cast<uint64_t>(startAddress))
    , m_length(length)
{
}

// NOLINTJUSTIFICATION allocation interface requires size and alignment as integral types
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
expected<void*, BumpAllocatorError> BumpAllocator::allocate(const uint64_t size, const uint64_t alignment) noexcept
{
    if (size == 0)
    {
        IOX_LOG(WARN, "Cannot allocate memory of size 0.");
        return err(BumpAllocatorError::REQUESTED_ZERO_SIZED_MEMORY);
    }

    const uint64_t currentAddress{m_startAddress + m_currentPosition};
    uint64_t alignedPosition{align(currentAddress, alignment)};

    alignedPosition -= m_startAddress;

    void* allocation{nullptr};

    const uint64_t nextPosition{alignedPosition + size};
    if (m_length >= nextPosition)
    {
        // AXIVION Next Construct AutosarC++19_03-A5.2.4 : required for low level memory management
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
        allocation = reinterpret_cast<void*>(m_startAddress + alignedPosition);
        m_currentPosition = nextPosition;
    }
    else
    {
        IOX_LOG(WARN,
                "Trying to allocate additional " << size << " bytes in the memory of capacity " << m_length
                                                 << " when there are already " << alignedPosition
                                                 << " aligned bytes in use.\n Only " << m_length - alignedPosition
                                                 << " bytes left.");
        return err(BumpAllocatorError::OUT_OF_MEMORY);
    }

    return ok(allocation);
}

void BumpAllocator::deallocate() noexcept
{
    m_currentPosition = 0;
}
} // namespace iox
