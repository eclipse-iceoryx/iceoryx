// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_MEMORY_MEMORY_BLOCK_HPP
#define IOX_POSH_ROUDI_MEMORY_MEMORY_BLOCK_HPP

#include "iox/not_null.hpp"
#include "iox/optional.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
/// @brief The MemoryBlock is a container for general purpose memory. It is used to request some memory from a
/// MemoryProvider, which can be POSIX SHM, the stack or something completely different. To be able to use the
/// container, some functions need to be implemented. For most use cases the GenericMemoryBlock can be used, which is a
/// templated class and implements the most common case.
class MemoryBlock
{
    friend class MemoryProvider;

  public:
    MemoryBlock() noexcept = default;
    virtual ~MemoryBlock() noexcept = default;

    /// @note this is intentional not movable/copyable, since a pointer to the memory block is registered at a
    /// MemoryProvider and therefore an instance of a MemoryBlock must be pinned to memory
    MemoryBlock(const MemoryBlock&) = delete;
    MemoryBlock(MemoryBlock&&) = delete;
    MemoryBlock& operator=(const MemoryBlock&) = delete;
    MemoryBlock& operator=(MemoryBlock&&) = delete;

    /// @brief This function provides the size of the required memory for the underlying data. It is needed for the
    /// MemoryProvider to calculate the total size of memory.
    /// @return the required memory as multiple of the alignment
    virtual uint64_t size() const noexcept = 0;

    /// @brief This function provides the alignment of the memory for the underlying data. This information is needed
    /// for the MemoryProvider
    /// @return the alignment of the underlying data.
    virtual uint64_t alignment() const noexcept = 0;

    /// @brief This function provides the pointer to the requested memory.
    /// @return an optional pointer to a memory block with the requested size and alignment if the memory is available,
    /// otherwise a nullopt_t
    optional<void*> memory() const noexcept;

  protected:
    /// @brief The MemoryProvider calls this either when MemoryProvider::destroy is called or in its destructor.
    /// @note This function can be called multiple times. Make sure that the implementation can handle this.
    virtual void destroy() noexcept = 0;

    /// @brief This function is called once the memory is available and is therefore the earliest possibility to use the
    /// memory.
    /// @param [in] memory pointer to a valid memory block, the same one that the memory() member function would return
    virtual void onMemoryAvailable(not_null<void*> memory) noexcept;

  private:
    void* m_memory{nullptr};
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_MEMORY_BLOCK_HPP
