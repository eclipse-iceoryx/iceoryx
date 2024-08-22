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

#include "iox/detail/posix_shared_memory.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/mman.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/unistd.hpp"
#include "iox/filesystem.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"
#include "iox/scope_guard.hpp"

#include <cassert>

namespace iox
{
namespace detail
{
string<PosixSharedMemory::Name_t::capacity() + 1> addLeadingSlash(const PosixSharedMemory::Name_t& name) noexcept
{
    string<PosixSharedMemory::Name_t::capacity() + 1> nameWithLeadingSlash = "/";
    nameWithLeadingSlash.append(TruncateToCapacity, name);
    return nameWithLeadingSlash;
}

// NOLINTJUSTIFICATION the function size and cognitive complexity results from the error handling and the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
expected<PosixSharedMemory, PosixSharedMemoryError> PosixSharedMemoryBuilder::create() noexcept
{
    auto printError = [this] {
        IOX_LOG(ERROR,
                "Unable to create shared memory with the following properties [ name = "
                    << m_name << ", access mode = " << asStringLiteral(m_accessMode)
                    << ", open mode = " << asStringLiteral(m_openMode)
                    << ", mode = " << iox::log::oct(m_filePermissions.value()) << ", sizeInBytes = " << m_size << " ]");
    };


    // on qnx the current working directory will be added to the /dev/shmem path if the leading slash is missing
    if (m_name.empty())
    {
        IOX_LOG(ERROR, "No shared memory name specified!");
        return err(PosixSharedMemoryError::EMPTY_NAME);
    }

    if (!isValidFileName(m_name))
    {
        IOX_LOG(ERROR,
                "Shared memory requires a valid file name (not path) as name and \"" << m_name
                                                                                     << "\" is not a valid file name");
        return err(PosixSharedMemoryError::INVALID_FILE_NAME);
    }

    auto nameWithLeadingSlash = addLeadingSlash(m_name);

    bool hasOwnership = (m_openMode == OpenMode::EXCLUSIVE_CREATE || m_openMode == OpenMode::PURGE_AND_CREATE
                         || m_openMode == OpenMode::OPEN_OR_CREATE);

    if (hasOwnership && (m_accessMode == AccessMode::READ_ONLY))
    {
        IOX_LOG(ERROR,
                "Cannot create shared-memory file \"" << m_name << "\" in read-only mode. "
                                                      << "Initializing a new file requires write access");
        return err(PosixSharedMemoryError::INCOMPATIBLE_OPEN_AND_ACCESS_MODE);
    }

    // the mask will be applied to the permissions, therefore we need to set it to 0
    shm_handle_t sharedMemoryFileHandle = PosixSharedMemory::INVALID_HANDLE;
    mode_t umaskSaved = umask(0U);
    {
        ScopeGuard umaskGuard([&] { umask(umaskSaved); });

        if (m_openMode == OpenMode::PURGE_AND_CREATE)
        {
            IOX_DISCARD_RESULT(IOX_POSIX_CALL(iox_shm_unlink)(nameWithLeadingSlash.c_str())
                                   .failureReturnValue(PosixSharedMemory::INVALID_HANDLE)
                                   .ignoreErrnos(ENOENT)
                                   .evaluate());
        }

        auto result =
            IOX_POSIX_CALL(iox_shm_open)(
                nameWithLeadingSlash.c_str(),
                convertToOflags(m_accessMode,
                                (m_openMode == OpenMode::OPEN_OR_CREATE) ? OpenMode::EXCLUSIVE_CREATE : m_openMode),
                m_filePermissions.value())
                .failureReturnValue(PosixSharedMemory::INVALID_HANDLE)
                .suppressErrorMessagesForErrnos((m_openMode == OpenMode::OPEN_OR_CREATE) ? EEXIST : 0)
                .evaluate();
        if (result.has_error())
        {
            // if it was not possible to create the shm exclusively someone else has the
            // ownership and we just try to open it
            if (m_openMode == OpenMode::OPEN_OR_CREATE && result.error().errnum == EEXIST)
            {
                hasOwnership = false;
                result = IOX_POSIX_CALL(iox_shm_open)(nameWithLeadingSlash.c_str(),
                                                      convertToOflags(m_accessMode, OpenMode::OPEN_EXISTING),
                                                      m_filePermissions.value())
                             .failureReturnValue(PosixSharedMemory::INVALID_HANDLE)
                             .evaluate();
            }

            // Check again, as the if-block above may have changed 'result'
            if (result.has_error())
            {
                printError();
                return err(PosixSharedMemory::errnoToEnum(result.error().errnum));
            }
        }
        sharedMemoryFileHandle = result->value;
    }

    if (hasOwnership)
    {
        auto result = IOX_POSIX_CALL(iox_ftruncate)(sharedMemoryFileHandle, static_cast<off_t>(m_size))
                          .failureReturnValue(PosixSharedMemory::INVALID_HANDLE)
                          .evaluate();
        if (result.has_error())
        {
            printError();

            IOX_POSIX_CALL(iox_shm_close)
            (sharedMemoryFileHandle)
                .failureReturnValue(PosixSharedMemory::INVALID_HANDLE)
                .evaluate()
                .or_else([&](auto& r) {
                    IOX_LOG(ERROR,
                            "Unable to close filedescriptor (close failed) : "
                                << r.getHumanReadableErrnum() << " for SharedMemory \"" << m_name << "\"");
                });

            IOX_POSIX_CALL(iox_shm_unlink)
            (nameWithLeadingSlash.c_str())
                .failureReturnValue(PosixSharedMemory::INVALID_HANDLE)
                .evaluate()
                .or_else([&](auto&) {
                    IOX_LOG(ERROR,
                            "Unable to remove previously created SharedMemory \""
                                << m_name << "\". This may be a SharedMemory leak.");
                });

            return err(PosixSharedMemory::errnoToEnum(result.error().errnum));
        }
    }

    return ok(PosixSharedMemory(m_name, sharedMemoryFileHandle, hasOwnership));
}

PosixSharedMemory::PosixSharedMemory(const Name_t& name, const shm_handle_t handle, const bool hasOwnership) noexcept
    : m_name{name}
    , m_handle{handle}
    , m_hasOwnership{hasOwnership}
{
}

PosixSharedMemory::~PosixSharedMemory() noexcept
{
    destroy();
}

void PosixSharedMemory::destroy() noexcept
{
    close();
    unlink();
}

void PosixSharedMemory::reset() noexcept
{
    m_hasOwnership = false;
    m_name = Name_t();
    m_handle = INVALID_HANDLE;
}

PosixSharedMemory::PosixSharedMemory(PosixSharedMemory&& rhs) noexcept
{
    *this = std::move(rhs);
}

PosixSharedMemory& PosixSharedMemory::operator=(PosixSharedMemory&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();

        m_name = rhs.m_name;
        m_hasOwnership = rhs.m_hasOwnership;
        m_handle = rhs.m_handle;

        rhs.reset();
    }
    return *this;
}

shm_handle_t PosixSharedMemory::getHandle() const noexcept
{
    return m_handle;
}

shm_handle_t PosixSharedMemory::get_file_handle() const noexcept
{
    return m_handle;
}

bool PosixSharedMemory::hasOwnership() const noexcept
{
    return m_hasOwnership;
}

expected<bool, PosixSharedMemoryError> PosixSharedMemory::unlinkIfExist(const Name_t& name) noexcept
{
    auto nameWithLeadingSlash = addLeadingSlash(name);

    auto result = IOX_POSIX_CALL(iox_shm_unlink)(nameWithLeadingSlash.c_str())
                      .failureReturnValue(INVALID_HANDLE)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (result.has_error())
    {
        return err(errnoToEnum(result.error().errnum));
    }

    return ok(result->errnum != ENOENT);
}

bool PosixSharedMemory::unlink() noexcept
{
    if (m_hasOwnership)
    {
        auto unlinkResult = unlinkIfExist(m_name);
        if (unlinkResult.has_error() || !unlinkResult.value())
        {
            IOX_LOG(ERROR, "Unable to unlink SharedMemory (shm_unlink failed).");
            return false;
        }
        m_hasOwnership = false;
    }

    reset();
    return true;
}

bool PosixSharedMemory::close() noexcept
{
    if (m_handle != INVALID_HANDLE)
    {
        auto call =
            IOX_POSIX_CALL(iox_shm_close)(m_handle).failureReturnValue(INVALID_HANDLE).evaluate().or_else([](auto& r) {
                IOX_LOG(ERROR,
                        "Unable to close SharedMemory filedescriptor (close failed) : " << r.getHumanReadableErrnum());
            });

        m_handle = INVALID_HANDLE;
        return !call.has_error();
    }
    return true;
}

// NOLINTJUSTIFICATION the function size and cognitive complexity results from the error handling and the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
PosixSharedMemoryError PosixSharedMemory::errnoToEnum(const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EACCES:
        IOX_LOG(ERROR, "No permission to modify, truncate or access the shared memory!");
        return PosixSharedMemoryError::INSUFFICIENT_PERMISSIONS;
    case EPERM:
        IOX_LOG(ERROR, "Resizing a file beyond its current size is not supported by the filesystem!");
        return PosixSharedMemoryError::NO_RESIZE_SUPPORT;
    case EFBIG:
        IOX_LOG(ERROR, "Requested Shared Memory is larger then the maximum file size.");
        return PosixSharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EINVAL:
        IOX_LOG(ERROR,
                "Requested Shared Memory is larger then the maximum file size or the filedescriptor does not "
                "belong to a regular file.");
        return PosixSharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EBADF:
        IOX_LOG(ERROR, "Provided filedescriptor is not a valid filedescriptor.");
        return PosixSharedMemoryError::INVALID_FILEDESCRIPTOR;
    case EEXIST:
        IOX_LOG(ERROR, "A Shared Memory with the given name already exists.");
        return PosixSharedMemoryError::DOES_EXIST;
    case EISDIR:
        IOX_LOG(ERROR, "The requested Shared Memory file is a directory.");
        return PosixSharedMemoryError::PATH_IS_A_DIRECTORY;
    case ELOOP:
        IOX_LOG(ERROR, "Too many symbolic links encountered while traversing the path.");
        return PosixSharedMemoryError::TOO_MANY_SYMBOLIC_LINKS;
    case EMFILE:
        IOX_LOG(ERROR, "Process limit of maximum open files reached.");
        return PosixSharedMemoryError::PROCESS_LIMIT_OF_OPEN_FILES_REACHED;
    case ENFILE:
        IOX_LOG(ERROR, "System limit of maximum open files reached.");
        return PosixSharedMemoryError::SYSTEM_LIMIT_OF_OPEN_FILES_REACHED;
    case ENOENT:
        IOX_LOG(ERROR, "Shared Memory does not exist.");
        return PosixSharedMemoryError::DOES_NOT_EXIST;
    case ENOMEM:
        IOX_LOG(ERROR, "Not enough memory available to create shared memory.");
        return PosixSharedMemoryError::NOT_ENOUGH_MEMORY_AVAILABLE;
    default:
        IOX_LOG(ERROR, "This should never happen! An unknown error occurred!");
        return PosixSharedMemoryError::UNKNOWN_ERROR;
    }
}

} // namespace detail
} // namespace iox
