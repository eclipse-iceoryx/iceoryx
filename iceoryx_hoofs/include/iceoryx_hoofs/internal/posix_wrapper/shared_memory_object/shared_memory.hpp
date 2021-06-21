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
#ifndef IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/design_pattern/creation.hpp"
#include "iceoryx_hoofs/platform/mman.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
class SharedMemoryObject;
enum class AccessMode : uint64_t
{
    READ_ONLY = 0U,
    READ_WRITE = 1U
};
static constexpr const char* ACCESS_MODE_STRING[] = {"AccessMode::READ_ONLY", "AccessMode::READ_WRITE"};

/// @brief describes how the shared memory is opened or created
enum class OpenMode : uint64_t
{
    /// @brief creates the shared memory, if it exists already the construction will fail
    EXCLUSIVE_CREATE = 0U,
    /// @brief creates the shared memory, if it exists it will be deleted and recreated
    PURGE_AND_CREATE = 1U,
    /// @brief creates the shared memory, if it does not exist otherwise it opens it
    OPEN_OR_CREATE = 2U,
    /// @brief opens the shared memory, if it does not exist it will fail
    OPEN_EXISTING = 3U
};
static constexpr const char* OPEN_MODE_STRING[] = {
    "OpenMode::EXCLUSIVE_CREATE", "OpenMode::PURGE_AND_CREATE", "OpenMode::OPEN_OR_CREATE", "OpenMode::OPEN_EXISTING"};

enum class SharedMemoryError
{
    INVALID_STATE,
    EMPTY_NAME,
    NAME_WITHOUT_LEADING_SLASH,
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
    UNKNOWN_ERROR
};

/// @brief Creates a bare metal shared memory object with the posix functions
///        shm_open, shm_unlink etc.
///        It must be used in combination with MemoryMap (or manual mmap calls)
//         to gain access to the created/opened shared memory
class SharedMemory : public DesignPattern::Creation<SharedMemory, SharedMemoryError>
{
  public:
    static constexpr uint64_t NAME_SIZE = platform::IOX_MAX_SHM_NAME_LENGTH;
    static constexpr int INVALID_HANDLE = -1;
    using Name_t = cxx::string<NAME_SIZE>;

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&&) noexcept;
    SharedMemory& operator=(SharedMemory&&) noexcept;
    ~SharedMemory() noexcept;

    /// @brief returns the file handle of the shared memory
    int32_t getHandle() const noexcept;

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
    static cxx::expected<bool, SharedMemoryError> unlinkIfExist(const Name_t& name) noexcept;

    friend class DesignPattern::Creation<SharedMemory, SharedMemoryError>;

  private:
    /// @brief constructs or opens existing shared memory
    /// @param[in] name the name of the shared memory, must start with a leading /
    /// @param[in] accessMode defines if the shared memory is mapped read only or with read write rights
    /// @param[in] openMode states how the shared memory is created/opened
    /// @param[in] permissions the permissions the shared memory should have
    /// @param[in] size the size in bytes of the shared memory
    SharedMemory(const Name_t& name,
                 const AccessMode accessMode,
                 const OpenMode openMode,
                 const mode_t permissions,
                 const uint64_t size) noexcept;

    bool
    open(const AccessMode accessMode, const OpenMode openMode, const mode_t permissions, const uint64_t size) noexcept;
    bool unlink() noexcept;
    bool close() noexcept;
    void destroy() noexcept;
    void reset() noexcept;
    static int getOflagsFor(const AccessMode accessMode, const OpenMode openMode) noexcept;

    static SharedMemoryError errnoToEnum(const int32_t errnum) noexcept;

    Name_t m_name;
    int m_handle{INVALID_HANDLE};
    bool m_hasOwnership{false};
};
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_SHARED_MEMORY_OBJECT_SHARED_MEMORY_HPP
