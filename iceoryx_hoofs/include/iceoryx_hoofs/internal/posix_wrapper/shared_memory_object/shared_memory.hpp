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
#ifndef IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP

#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/file_management_interface.hpp"
#include "iox/filesystem.hpp"
#include "iox/optional.hpp"
#include "iox/string.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
class SharedMemoryObject;
enum class SharedMemoryError
{
    EMPTY_NAME,
    INVALID_FILE_NAME,
    INSUFFICIENT_PERMISSIONS,
    DOES_EXIST,
    PROCESS_LIMIT_OF_OPEN_FILES_REACHED,
    SYSTEM_LIMIT_OF_OPEN_FILES_REACHED,
    DOES_NOT_EXIST,
    NOT_ENOUGH_MEMORY_AVAILABLE,
    REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE,
    PATH_IS_A_DIRECTORY,
    TOO_MANY_SYMBOLIC_LINKS,
    NO_FILE_RESIZE_SUPPORT,
    NO_RESIZE_SUPPORT,
    INVALID_FILEDESCRIPTOR,
    INCOMPATIBLE_OPEN_AND_ACCESS_MODE,
    UNKNOWN_ERROR
};

/// @brief Shared memory file descriptor type
using shm_handle_t = int;

/// @brief Creates a bare metal shared memory object with the posix functions
///        shm_open, shm_unlink etc.
///        It must be used in combination with MemoryMap (or manual mmap calls)
///        to gain access to the created/opened shared memory
class SharedMemory : public FileManagementInterface<SharedMemory>
{
  public:
    static constexpr uint64_t NAME_SIZE = platform::IOX_MAX_SHM_NAME_LENGTH;
    static constexpr int INVALID_HANDLE = -1;
    using Name_t = string<NAME_SIZE>;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&&) noexcept;
    SharedMemory& operator=(SharedMemory&&) noexcept;
    ~SharedMemory() noexcept;

    /// @brief returns the file handle of the shared memory
    shm_handle_t getHandle() const noexcept;

    /// @brief this class has the ownership of the shared memory when the shared
    ///        memory was created by this class. This is the case when this class
    ///        was successful created with EXCLUSIVE_CREATE, PURGE_AND_CREATE or OPEN_OR_CREATE
    ///        and the shared memory was created. If an already available shared memory
    ///        is opened then this class does not have the ownership.
    bool hasOwnership() const noexcept;

    /// @brief removes shared memory with a given name from the system
    /// @param[in] name name of the shared memory
    /// @return true if the shared memory was removed, false if the shared memory did not exist and
    ///         SharedMemoryError when the underlying shm_unlink call failed.
    static expected<bool, SharedMemoryError> unlinkIfExist(const Name_t& name) noexcept;

    friend class SharedMemoryBuilder;

  private:
    SharedMemory(const Name_t& name, const shm_handle_t handle, const bool hasOwnership) noexcept;

    bool unlink() noexcept;
    bool close() noexcept;
    void destroy() noexcept;
    void reset() noexcept;

    static SharedMemoryError errnoToEnum(const int32_t errnum) noexcept;

    friend struct FileManagementInterface<SharedMemory>;
    shm_handle_t get_file_handle() const noexcept;

    Name_t m_name;
    shm_handle_t m_handle{INVALID_HANDLE};
    bool m_hasOwnership{false};
};

class SharedMemoryBuilder
{
    /// @brief A valid file name for the shared memory with the restriction that
    ///        no leading dot is allowed since it is not compatible with every
    ///        file system
    IOX_BUILDER_PARAMETER(SharedMemory::Name_t, name, "")

    /// @brief Defines if the memory should be mapped read only or with write access.
    ///        A read only memory section will cause a segmentation fault when written to.
    IOX_BUILDER_PARAMETER(AccessMode, accessMode, AccessMode::READ_ONLY)

    /// @brief Defines how the shared memory is acquired
    IOX_BUILDER_PARAMETER(OpenMode, openMode, OpenMode::OPEN_EXISTING)

    /// @brief Defines the access permissions of the shared memory
    IOX_BUILDER_PARAMETER(access_rights, filePermissions, perms::none)

    /// @brief Defines the size of the shared memory
    IOX_BUILDER_PARAMETER(uint64_t, size, 0U)

  public:
    /// @brief creates a valid SharedMemory object. If the construction failed the expected
    ///        contains an enum value describing the error.
    /// @return expected containing SharedMemory on success otherwise SharedMemoryError
    expected<SharedMemory, SharedMemoryError> create() noexcept;
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP
