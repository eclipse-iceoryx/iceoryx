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
#ifndef IOX_HOOFS_MEMORY_BUMP_ALLOCATOR_HPP
#define IOX_HOOFS_MEMORY_BUMP_ALLOCATOR_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"

#include <cstdint>

namespace iox
{

enum class BumpAllocatorError : uint8_t
{
    OUT_OF_MEMORY,
    REQUESTED_ZERO_SIZED_MEMORY
};

/// @brief A bump allocator for the memory provided in the ctor arguments
class BumpAllocator final
{
  public:
    /// @brief c'tor
    /// @param[in] startAddress of the memory this allocator manages
    /// @param[in] length of the memory this allocator manages
    BumpAllocator(void* const startAddress, const uint64_t length) noexcept;

    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator(BumpAllocator&&) noexcept = default;
    BumpAllocator& operator=(const BumpAllocator&) noexcept = delete;
    BumpAllocator& operator=(BumpAllocator&&) noexcept = default;
    ~BumpAllocator() noexcept = default;

    /// @brief allocates on the memory supplied with the ctor
    /// @param[in] size of the memory to allocate, must be greater than 0
    /// @param[in] alignment of the memory to allocate
    /// @return an expected containing a pointer to the memory if allocation was successful, otherwise
    /// BumpAllocatorError
    expected<void*, BumpAllocatorError> allocate(const uint64_t size, const uint64_t alignment) noexcept;

    /// @brief mark the memory as unused
    void deallocate() noexcept;

  private:
    uint64_t m_startAddress{0U};
    uint64_t m_length{0U};
    uint64_t m_currentPosition{0U};
};
} // namespace iox

#endif // IOX_HOOFS_MEMORY_BUMP_ALLOCATOR_HPP
