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
#ifndef IOX_HOOFS_MEMORY_BUMP_ALLOCATOR_HPP
#define IOX_HOOFS_MEMORY_BUMP_ALLOCATOR_HPP

#include "iceoryx_hoofs/iceoryx_hoofs_types.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
class SharedMemoryObject;
}

class BumpAllocator
{
  public:
    /// @brief A bump allocator for the memory provided in the ctor arguments
    /// @param[in] startAddress of the memory this allocator manages
    /// @param[in] length of the memory this allocator manages
    BumpAllocator(void* const startAddress, const uint64_t length) noexcept;

    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator(BumpAllocator&&) noexcept = default;
    BumpAllocator& operator=(const BumpAllocator&) noexcept = delete;
    BumpAllocator& operator=(BumpAllocator&&) noexcept = default;
    ~BumpAllocator() noexcept = default;

    /// @brief allocates on the memory supplied with the ctor
    /// @param[in] size of the memory to allocate
    /// @param[in] alignment of the memory to allocate
    /// @note May terminate if out of memory or finalizeAllocation() was called before
    void* allocate(const uint64_t size, const uint64_t alignment) noexcept;

  protected:
    friend class posix::SharedMemoryObject;
    // make free function; destructive move?
    void finalizeAllocation() noexcept;

  private:
    cxx::byte_t* m_startAddress{nullptr};
    uint64_t m_length{0U};
    uint64_t m_currentPosition = 0U;
    bool m_allocationFinalized = false;
};
} // namespace iox

#endif // IOX_HOOFS_POSIX_MEMORY_BUMP_ALLOCATOR_HPP
