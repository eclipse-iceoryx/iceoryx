// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_ALLOCATOR_HPP
#define IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_ALLOCATOR_HPP

#include <cstdint>
namespace iox
{
namespace posix
{
class SharedMemoryObject;

class Allocator
{
    using byte_t = uint8_t;


  public:
    static constexpr uint64_t MEMORY_ALIGNMENT = 32;
    Allocator(void* const f_startAddress, const uint64_t f_length) noexcept;

    Allocator(const Allocator&) = delete;
    Allocator(Allocator&&) noexcept = default;
    Allocator& operator=(const Allocator&) noexcept = delete;
    Allocator& operator=(Allocator&&) = default;
    ~Allocator() = default;

    void* allocate(const uint64_t f_size, const uint64_t f_alignment = MEMORY_ALIGNMENT) noexcept;

  protected:
    friend class SharedMemoryObject;
    void finalizeAllocation() noexcept;

  private:
    byte_t* m_startAddress{nullptr};
    uint64_t m_length{0u};
    uint64_t m_currentPosition = 0u;
    bool m_allocationFinalized = false;
};
} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_ALLOCATOR_HPP
