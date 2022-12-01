// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_platform/platform_correction.hpp"

#include <iostream>

namespace iox
{
BumpAllocator::BumpAllocator(void* const startAddress, const uint64_t length) noexcept
    : m_startAddress(static_cast<cxx::byte_t*>(startAddress))
    , m_length(length)
{
}

// NOLINTJUSTIFICATION allocation interface requires size and alignment as integral types
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void* BumpAllocator::allocate(const uint64_t size, const uint64_t alignment) noexcept
{
    cxx::Expects(size > 0);

    cxx::Expects(
        !m_allocationFinalized
        && "allocate() call after finalizeAllocation()! You are not allowed to acquire shared memory chunks anymore");
    // return a nullptr instead of terminate? Then this could be checked in SharedMemoryObject and the log messages
    // could be printed there...

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for low level pointer alignment
    uint64_t currentAddress = reinterpret_cast<uint64_t>(m_startAddress) + m_currentPosition;
    uint64_t alignedPosition = cxx::align(currentAddress, static_cast<uint64_t>(alignment));

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for low level pointer alignment
    alignedPosition -= reinterpret_cast<uint64_t>(m_startAddress);

    cxx::byte_t* l_returnValue = nullptr;

    if (m_length >= alignedPosition + size)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) low-level memory management
        l_returnValue = m_startAddress + alignedPosition;
        m_currentPosition = alignedPosition + size;
    }
    else
    {
        IOX_LOG(ERROR) << "Trying to allocate additional " << size << " bytes in the shared memory of capacity "
                       << m_length << " when there are already " << alignedPosition << " aligned bytes in use.";
        IOX_LOG(ERROR) << "Only " << m_length - alignedPosition << " bytes left.";

        cxx::Expects(false && "Not enough space left in shared memory");
    }

    return static_cast<void*>(l_returnValue);
}

void BumpAllocator::finalizeAllocation() noexcept
{
    m_allocationFinalized = true;
}

void BumpAllocator::deallocate() noexcept
{
    m_currentPosition = 0;
    m_allocationFinalized = false;
}

} // namespace iox
