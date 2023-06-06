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
#ifndef IOX_POSH_ROUDI_MEMORY_MEMORY_PROVIDER_HPP
#define IOX_POSH_ROUDI_MEMORY_MEMORY_PROVIDER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include "iox/expected.hpp"
#include "iox/not_null.hpp"
#include "iox/optional.hpp"
#include "iox/vector.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class MemoryBlock;

enum class MemoryProviderError
{
    /// attempt to add more memory blocks than the capacity allows
    MEMORY_BLOCKS_EXHAUSTED,
    /// an action was performed which requires memory blocks
    NO_MEMORY_BLOCKS_PRESENT,
    /// attempt to create memory although it already was created
    MEMORY_ALREADY_CREATED,
    /// generic error if memory creation failed
    MEMORY_CREATION_FAILED,
    /// attempt to create memory with an alignment bigger than the page size
    MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE,
    /// memory creation failed at allocating memory
    MEMORY_ALLOCATION_FAILED,
    /// memory creation failed at mapping memory
    MEMORY_MAPPING_FAILED,
    /// an action was performed which requires memory
    MEMORY_NOT_AVAILABLE,
    /// generic error if memory destruction failed
    MEMORY_DESTRUCTION_FAILED,
    /// memory destruction failed at deallocating memory
    MEMORY_DEALLOCATION_FAILED,
    /// memory destruction failed at unmapping memory
    MEMORY_UNMAPPING_FAILED,
    /// Setup or teardown of SIGBUS failed
    SIGACTION_CALL_FAILED,
};

/// @brief This class creates memory which is requested by the MemoryBlocks. Once the memory is available, this is
/// announced to the blocks, so that they can consume the memory for their needs. When the Memory is release, the blocks
/// will also called to handle this appropriately, e.g. calling the destructor of the underlying type. This class is an
/// interface with some default behavior and needs an implementation for real memory supply, e.g. a
/// PosixShmMemoryProvider.
class MemoryProvider
{
    friend class RouDiMemoryManager;

  public:
    MemoryProvider() noexcept = default;
    virtual ~MemoryProvider() noexcept;

    /// @note this is intentional not movable/copyable, since a pointer to the memory provider is registered at the
    /// RouDiMemoryManager and therefore an instance of a MemoryProvider must be pinned to memory
    MemoryProvider(const MemoryProvider&) = delete;
    MemoryProvider(MemoryProvider&&) = delete;
    MemoryProvider& operator=(const MemoryProvider&) = delete;
    MemoryProvider& operator=(MemoryProvider&&) = delete;

    /// @brief This function add a MemoryBlock to the list of memory requester
    /// @param [in] memoryBlock is a pointer to a user defined MemoryBlock
    /// @return an MemoryProviderError::MEMORY_BLOCKS_EXHAUSTED error if no further memory blocks can be added,
    /// otherwise success
    expected<void, MemoryProviderError> addMemoryBlock(not_null<MemoryBlock*> memoryBlock) noexcept;

    /// @brief With this call the memory requested by the MemoryBlocks need to be created. The function should be called
    /// from a MemoryManager which handles one or more MemoryProvider
    /// @return an MemoryProviderError if memory allocation was not successful, otherwise success
    expected<void, MemoryProviderError> create() noexcept;

    /// @brief This function announces the availability of the memory to the MemoryBlocks. The function should be called
    /// from a MemoryManager which handles one or more MemoryProvider
    void announceMemoryAvailable() noexcept;

    /// @brief This function destroys the previously allocated memory. Before the destruction, all MemoryBlocks are
    /// requested to handle this appropriately, e.g. call the destructor of the underlying type. The
    /// function should be called from a MemoryManager which handles one or more MemoryProvider
    /// @return an error if memory destruction was not successful, otherwise success
    expected<void, MemoryProviderError> destroy() noexcept;

    /// @brief This function provides the base address of the created memory
    /// @return an optional pointer to the base address of the created memory if the memory is available, otherwise a
    /// nullopt_t
    optional<void*> baseAddress() const noexcept;

    /// @brief This function provides the size of the created memory
    /// @return the size of the created memory
    uint64_t size() const noexcept;

    /// @brief This function provides the segment id of the relocatable memory segment which is owned by the
    /// MemoryProvider.
    /// @return an optional segment id for the created memory if the memory is available, otherwise nullopt_t
    optional<uint64_t> segmentId() const noexcept;

    /// @brief This function can be used to check if the requested memory is already available
    /// @return true if the requested memory is available, false otherwise
    bool isAvailable() const noexcept;

    /// @brief This function can be used to check if the availability of the memory was announced to the MemoryBlocks
    /// @return true if the availability of the memory was announced to the MemoryBlocks, false otherwise
    bool isAvailableAnnounced() const noexcept;

  protected:
    /// @brief This function needs to be implemented to provide the actual memory, e.g. in case of POSIX SHM, shm_open
    /// and mmap would need to be called in the implementation of this function
    /// @param [in] size is the size in bytes for the requested memory, the size should already be calculated according
    /// to the alignment requirements
    /// @param [in] alignment the required alignment for the memory
    /// @return the pointer of the begin of the created memory or a MemoryProviderError if the memory could not be
    /// created
    virtual expected<void*, MemoryProviderError> createMemory(const uint64_t size,
                                                              const uint64_t alignment) noexcept = 0;

    /// @brief This function needs to be implemented to free the actual memory, e.g. in case of POSIX SHM, shm_unlink
    /// and munmap would need to be called in the implementation of this function
    /// @return a MemoryProviderError if the destruction failed, otherwise success
    virtual expected<void, MemoryProviderError> destroyMemory() noexcept = 0;

    static const char* getErrorString(const MemoryProviderError error) noexcept;

  private:
    void* m_memory{nullptr};
    uint64_t m_size{0};
    uint64_t m_segmentId{0};
    bool m_memoryAvailableAnnounced{false};
    vector<MemoryBlock*, MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER> m_memoryBlocks;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_MEMORY_PROVIDER_HPP
