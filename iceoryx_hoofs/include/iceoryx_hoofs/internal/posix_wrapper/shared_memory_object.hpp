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
#ifndef IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_HPP

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iox/builder.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/file_management_interface.hpp"
#include "iox/filesystem.hpp"
#include "iox/optional.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{

enum class SharedMemoryObjectError
{
    SHARED_MEMORY_CREATION_FAILED,
    MAPPING_SHARED_MEMORY_FAILED,
    UNABLE_TO_VERIFY_MEMORY_SIZE,
    REQUESTED_SIZE_EXCEEDS_ACTUAL_SIZE,
    INTERNAL_LOGIC_FAILURE,
};

enum class SharedMemoryAllocationError
{
    REQUESTED_MEMORY_AFTER_FINALIZED_ALLOCATION,
    NOT_ENOUGH_MEMORY,
    REQUESTED_ZERO_SIZED_MEMORY

};

class SharedMemoryObjectBuilder;

/// @brief Creates a shared memory segment and maps it into the process space.
///        One can use optionally the allocator to acquire memory.
class SharedMemoryObject : public FileManagementInterface<SharedMemoryObject>
{
  public:
    using Builder = SharedMemoryObjectBuilder;

    static constexpr const void* const NO_ADDRESS_HINT = nullptr;
    SharedMemoryObject(const SharedMemoryObject&) = delete;
    SharedMemoryObject& operator=(const SharedMemoryObject&) = delete;
    SharedMemoryObject(SharedMemoryObject&&) noexcept = default;
    SharedMemoryObject& operator=(SharedMemoryObject&&) noexcept = default;
    ~SharedMemoryObject() noexcept = default;

    /// @brief Returns start- or base-address of the shared memory.
    const void* getBaseAddress() const noexcept;

    /// @brief Returns start- or base-address of the shared memory.
    void* getBaseAddress() noexcept;

    /// @brief Returns the underlying file handle of the shared memory
    shm_handle_t getFileHandle() const noexcept;

    /// @brief True if the shared memory has the ownership. False if an already
    ///        existing shared memory was opened.
    bool hasOwnership() const noexcept;

    friend class SharedMemoryObjectBuilder;

  private:
    SharedMemoryObject(SharedMemory&& sharedMemory, MemoryMap&& memoryMap) noexcept;

    friend struct FileManagementInterface<SharedMemoryObject>;
    shm_handle_t get_file_handle() const noexcept;

  private:
    SharedMemory m_sharedMemory;
    MemoryMap m_memoryMap;
};

class SharedMemoryObjectBuilder
{
    /// @brief A valid file name for the shared memory with the restriction that
    ///        no leading dot is allowed since it is not compatible with every
    ///        file system
    IOX_BUILDER_PARAMETER(SharedMemory::Name_t, name, "")

    /// @brief Defines the size of the shared memory
    IOX_BUILDER_PARAMETER(uint64_t, memorySizeInBytes, 0U)

    /// @brief Defines if the memory should be mapped read only or with write access.
    ///        A read only memory section will cause a segmentation fault when written to.
    IOX_BUILDER_PARAMETER(AccessMode, accessMode, AccessMode::READ_ONLY)

    /// @brief Defines how the shared memory is acquired
    IOX_BUILDER_PARAMETER(OpenMode, openMode, OpenMode::OPEN_EXISTING)

    /// @brief If this is set to a non null address create will try to map the shared
    ///        memory to the provided address. Since it is a hint, this mapping can
    ///        fail. The .getBaseAddress() method of the SharedMemoryObject returns
    ///        the actual mapped base address.
    IOX_BUILDER_PARAMETER(optional<const void*>, baseAddressHint, nullopt)

    /// @brief Defines the access permissions of the shared memory
    IOX_BUILDER_PARAMETER(access_rights, permissions, perms::none)

  public:
    expected<SharedMemoryObject, SharedMemoryObjectError> create() noexcept;
};
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_HPP
