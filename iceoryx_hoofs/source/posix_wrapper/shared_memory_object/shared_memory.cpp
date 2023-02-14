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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#include "iceoryx_hoofs/cxx/scope_guard.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/mman.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/unistd.hpp"

#include <bitset>
#include <cassert>

namespace iox
{
namespace posix
{
string<SharedMemory::Name_t::capacity() + 1> addLeadingSlash(const SharedMemory::Name_t& name) noexcept
{
    string<SharedMemory::Name_t::capacity() + 1> nameWithLeadingSlash = "/";
    nameWithLeadingSlash.append(TruncateToCapacity, name);
    return nameWithLeadingSlash;
}

expected<SharedMemory, SharedMemoryError> SharedMemoryBuilder::create() noexcept
{
    auto printError = [this] {
        IOX_LOG(ERROR) << "Unable to create shared memory with the following properties [ name = " << m_name
                       << ", access mode = " << asStringLiteral(m_accessMode)
                       << ", open mode = " << asStringLiteral(m_openMode)
                       << ", mode = " << std::bitset<sizeof(mode_t)>(static_cast<mode_t>(m_filePermissions)).to_string()
                       << ", sizeInBytes = " << m_size << " ]";
    };


    // on qnx the current working directory will be added to the /dev/shmem path if the leading slash is missing
    if (m_name.empty())
    {
        IOX_LOG(ERROR) << "No shared memory name specified!";
        return error<SharedMemoryError>(SharedMemoryError::EMPTY_NAME);
    }

    if (!isValidFileName(m_name))
    {
        IOX_LOG(ERROR) << "Shared memory requires a valid file name (not path) as name and \"" << m_name
                       << "\" is not a valid file name";
        return error<SharedMemoryError>(SharedMemoryError::INVALID_FILE_NAME);
    }

    auto nameWithLeadingSlash = addLeadingSlash(m_name);

    bool hasOwnership = (m_openMode == OpenMode::EXCLUSIVE_CREATE || m_openMode == OpenMode::PURGE_AND_CREATE
                         || m_openMode == OpenMode::OPEN_OR_CREATE);

    if (hasOwnership && (m_accessMode == AccessMode::READ_ONLY))
    {
        std::cerr << "Cannot create shared-memory file \"" << m_name << "\" in read-only mode. "
                  << "Initializing a new file requires write access" << std::endl;
        return error<SharedMemoryError>(SharedMemoryError::INCOMPATIBLE_OPEN_AND_ACCESS_MODE);
    }

    // the mask will be applied to the permissions, therefore we need to set it to 0
    int sharedMemoryFileHandle = SharedMemory::INVALID_HANDLE;
    mode_t umaskSaved = umask(0U);
    {
        cxx::ScopeGuard umaskGuard([&] { umask(umaskSaved); });

        if (m_openMode == OpenMode::PURGE_AND_CREATE)
        {
            IOX_DISCARD_RESULT(posixCall(iox_shm_unlink)(nameWithLeadingSlash.c_str())
                                   .failureReturnValue(SharedMemory::INVALID_HANDLE)
                                   .ignoreErrnos(ENOENT)
                                   .evaluate());
        }

        auto result =
            posixCall(iox_shm_open)(
                nameWithLeadingSlash.c_str(),
                convertToOflags(m_accessMode,
                                (m_openMode == OpenMode::OPEN_OR_CREATE) ? OpenMode::EXCLUSIVE_CREATE : m_openMode),
                static_cast<mode_t>(m_filePermissions))
                .failureReturnValue(SharedMemory::INVALID_HANDLE)
                .suppressErrorMessagesForErrnos((m_openMode == OpenMode::OPEN_OR_CREATE) ? EEXIST : 0)
                .evaluate();
        if (result.has_error())
        {
            // if it was not possible to create the shm exclusively someone else has the
            // ownership and we just try to open it
            if (m_openMode == OpenMode::OPEN_OR_CREATE && result.get_error().errnum == EEXIST)
            {
                hasOwnership = false;
                result = posixCall(iox_shm_open)(nameWithLeadingSlash.c_str(),
                                                 convertToOflags(m_accessMode, OpenMode::OPEN_EXISTING),
                                                 static_cast<mode_t>(m_filePermissions))
                             .failureReturnValue(SharedMemory::INVALID_HANDLE)
                             .evaluate();
            }

            // Check again, as the if-block above may have changed `result`
            if (result.has_error())
            {
                printError();
                return error<SharedMemoryError>(SharedMemory::errnoToEnum(result.get_error().errnum));
            }
        }
        sharedMemoryFileHandle = result->value;
    }

    if (hasOwnership)
    {
        auto result = posixCall(ftruncate)(sharedMemoryFileHandle, static_cast<int64_t>(m_size))
                          .failureReturnValue(SharedMemory::INVALID_HANDLE)
                          .evaluate();
        if (result.has_error())
        {
            printError();

            posixCall(iox_close)(sharedMemoryFileHandle)
                .failureReturnValue(SharedMemory::INVALID_HANDLE)
                .evaluate()
                .or_else([&](auto& r) {
                    IOX_LOG(ERROR) << "Unable to close filedescriptor (close failed) : " << r.getHumanReadableErrnum()
                                   << " for SharedMemory \"" << m_name << "\"";
                });

            posixCall(iox_shm_unlink)(nameWithLeadingSlash.c_str())
                .failureReturnValue(SharedMemory::INVALID_HANDLE)
                .evaluate()
                .or_else([&](auto&) {
                    IOX_LOG(ERROR) << "Unable to remove previously created SharedMemory \"" << m_name
                                   << "\". This may be a SharedMemory leak.";
                });

            return error<SharedMemoryError>(SharedMemory::errnoToEnum(result.get_error().errnum));
        }
    }

    return success<SharedMemory>(SharedMemory(m_name, sharedMemoryFileHandle, hasOwnership));
}

SharedMemory::SharedMemory(const Name_t& name, const int handle, const bool hasOwnership) noexcept
    : m_name{name}
    , m_handle{handle}
    , m_hasOwnership{hasOwnership}
{
}

SharedMemory::~SharedMemory() noexcept
{
    destroy();
}

void SharedMemory::destroy() noexcept
{
    close();
    unlink();
}

void SharedMemory::reset() noexcept
{
    m_hasOwnership = false;
    m_name = Name_t();
    m_handle = INVALID_HANDLE;
}

SharedMemory::SharedMemory(SharedMemory&& rhs) noexcept
{
    *this = std::move(rhs);
}

SharedMemory& SharedMemory::operator=(SharedMemory&& rhs) noexcept
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

int32_t SharedMemory::getHandle() const noexcept
{
    return m_handle;
}

bool SharedMemory::hasOwnership() const noexcept
{
    return m_hasOwnership;
}

expected<bool, SharedMemoryError> SharedMemory::unlinkIfExist(const Name_t& name) noexcept
{
    auto nameWithLeadingSlash = addLeadingSlash(name);

    auto result = posixCall(iox_shm_unlink)(nameWithLeadingSlash.c_str())
                      .failureReturnValue(INVALID_HANDLE)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (!result.has_error())
    {
        return success<bool>(result->errnum != ENOENT);
    }

    return error<SharedMemoryError>(errnoToEnum(result.get_error().errnum));
}

bool SharedMemory::unlink() noexcept
{
    if (m_hasOwnership)
    {
        auto unlinkResult = unlinkIfExist(m_name);
        if (unlinkResult.has_error() || !unlinkResult.value())
        {
            IOX_LOG(ERROR) << "Unable to unlink SharedMemory (shm_unlink failed).";
            return false;
        }
        m_hasOwnership = false;
    }

    reset();
    return true;
}

bool SharedMemory::close() noexcept
{
    if (m_handle != INVALID_HANDLE)
    {
        auto call = posixCall(iox_close)(m_handle).failureReturnValue(INVALID_HANDLE).evaluate().or_else([](auto& r) {
            IOX_LOG(ERROR) << "Unable to close SharedMemory filedescriptor (close failed) : "
                           << r.getHumanReadableErrnum();
        });

        m_handle = INVALID_HANDLE;
        return !call.has_error();
    }
    return true;
}

SharedMemoryError SharedMemory::errnoToEnum(const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EACCES:
        IOX_LOG(ERROR) << "No permission to modify, truncate or access the shared memory!";
        return SharedMemoryError::INSUFFICIENT_PERMISSIONS;
    case EPERM:
        IOX_LOG(ERROR) << "Resizing a file beyond its current size is not supported by the filesystem!";
        return SharedMemoryError::NO_RESIZE_SUPPORT;
    case EFBIG:
        IOX_LOG(ERROR) << "Requested Shared Memory is larger then the maximum file size.";
        return SharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EINVAL:
        IOX_LOG(ERROR) << "Requested Shared Memory is larger then the maximum file size or the filedescriptor does not "
                          "belong to a regular file.";
        return SharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EBADF:
        IOX_LOG(ERROR) << "Provided filedescriptor is not a valid filedescriptor.";
        return SharedMemoryError::INVALID_FILEDESCRIPTOR;
    case EEXIST:
        IOX_LOG(ERROR) << "A Shared Memory with the given name already exists.";
        return SharedMemoryError::DOES_EXIST;
    case EISDIR:
        IOX_LOG(ERROR) << "The requested Shared Memory file is a directory.";
        return SharedMemoryError::PATH_IS_A_DIRECTORY;
    case ELOOP:
        IOX_LOG(ERROR) << "Too many symbolic links encountered while traversing the path.";
        return SharedMemoryError::TOO_MANY_SYMBOLIC_LINKS;
    case EMFILE:
        IOX_LOG(ERROR) << "Process limit of maximum open files reached.";
        return SharedMemoryError::PROCESS_LIMIT_OF_OPEN_FILES_REACHED;
    case ENFILE:
        IOX_LOG(ERROR) << "System limit of maximum open files reached.";
        return SharedMemoryError::SYSTEM_LIMIT_OF_OPEN_FILES_REACHED;
    case ENOENT:
        IOX_LOG(ERROR) << "Shared Memory does not exist.";
        return SharedMemoryError::DOES_NOT_EXIST;
    case ENOMEM:
        IOX_LOG(ERROR) << "Not enough memory available to create shared memory.";
        return SharedMemoryError::NOT_ENOUGH_MEMORY_AVAILABLE;
    default:
        IOX_LOG(ERROR) << "This should never happen! An unknown error occurred!";
        return SharedMemoryError::UNKNOWN_ERROR;
    }
}

} // namespace posix
} // namespace iox
