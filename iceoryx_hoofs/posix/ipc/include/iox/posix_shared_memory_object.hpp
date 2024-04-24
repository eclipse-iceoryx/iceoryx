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

#ifndef IOX_HOOFS_POSIX_ICP_POSIX_SHARED_MEMORY_OBJECT_HPP
#define IOX_HOOFS_POSIX_ICP_POSIX_SHARED_MEMORY_OBJECT_HPP

#include "iceoryx_platform/stat.hpp"
#include "iox/builder.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/detail/posix_memory_map.hpp"
#include "iox/detail/posix_shared_memory.hpp"
#include "iox/file_management_interface.hpp"
#include "iox/filesystem.hpp"
#include "iox/optional.hpp"

#include <cstdint>

namespace iox
{

enum class PosixSharedMemoryObjectError : uint8_t
{
    SHARED_MEMORY_CREATION_FAILED,
    MAPPING_SHARED_MEMORY_FAILED,
    UNABLE_TO_VERIFY_MEMORY_SIZE,
    REQUESTED_SIZE_EXCEEDS_ACTUAL_SIZE,
    INTERNAL_LOGIC_FAILURE,
};

enum class PosixSharedMemoryAllocationError : uint8_t
{
    REQUESTED_MEMORY_AFTER_FINALIZED_ALLOCATION,
    NOT_ENOUGH_MEMORY,
    REQUESTED_ZERO_SIZED_MEMORY

};

class PosixSharedMemoryObjectBuilder;

/// @brief Creates a shared memory segment and maps it into the process space.
///        One can use optionally the allocator to acquire memory.
class PosixSharedMemoryObject : public FileManagementInterface<PosixSharedMemoryObject>
{
  public:
    using Builder = PosixSharedMemoryObjectBuilder;

    static constexpr const void* const NO_ADDRESS_HINT = nullptr;
    PosixSharedMemoryObject(const PosixSharedMemoryObject&) = delete;
    PosixSharedMemoryObject& operator=(const PosixSharedMemoryObject&) = delete;
    PosixSharedMemoryObject(PosixSharedMemoryObject&&) noexcept = default;
    PosixSharedMemoryObject& operator=(PosixSharedMemoryObject&&) noexcept = default;
    ~PosixSharedMemoryObject() noexcept = default;

    /// @brief Returns start- or base-address of the shared memory.
    const void* getBaseAddress() const noexcept;

    /// @brief Returns start- or base-address of the shared memory.
    void* getBaseAddress() noexcept;

    /// @brief Returns the underlying file handle of the shared memory
    shm_handle_t getFileHandle() const noexcept;

    /// @brief True if the shared memory has the ownership. False if an already
    ///        existing shared memory was opened.
    bool hasOwnership() const noexcept;

    friend class PosixSharedMemoryObjectBuilder;

  private:
    PosixSharedMemoryObject(detail::PosixSharedMemory&& sharedMemory, detail::PosixMemoryMap&& memoryMap) noexcept;

    friend struct FileManagementInterface<PosixSharedMemoryObject>;
    shm_handle_t get_file_handle() const noexcept;

  private:
    detail::PosixSharedMemory m_sharedMemory;
    detail::PosixMemoryMap m_memoryMap;
};

class PosixSharedMemoryObjectBuilder
{
    /// @brief A valid file name for the shared memory with the restriction that
    ///        no leading dot is allowed since it is not compatible with every
    ///        file system
    IOX_BUILDER_PARAMETER(detail::PosixSharedMemory::Name_t, name, "")

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
    expected<PosixSharedMemoryObject, PosixSharedMemoryObjectError> create() noexcept;
};
} // namespace iox

#endif // IOX_HOOFS_POSIX_ICP_POSIX_SHARED_MEMORY_OBJECT_HPP
