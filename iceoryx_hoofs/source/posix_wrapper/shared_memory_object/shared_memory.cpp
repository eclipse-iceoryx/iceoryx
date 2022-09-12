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
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/scope_guard.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/mman.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/unistd.hpp"

#include <bitset>
#include <cassert>
#include <limits>

namespace iox
{
namespace posix
{
cxx::string<SharedMemory::Name_t::capacity() + 1> addLeadingSlash(const SharedMemory::Name_t& name) noexcept
{
    cxx::string<SharedMemory::Name_t::capacity() + 1> nameWithLeadingSlash = "/";
    nameWithLeadingSlash.append(cxx::TruncateToCapacity, name);
    return nameWithLeadingSlash;
}

cxx::expected<SharedMemory, SharedMemoryError> SharedMemoryBuilder::create() noexcept
{
    auto printError = [this] {
        std::cerr << "Unable to create shared memory with the following properties [ name = " << m_name
                  << ", access mode = " << asStringLiteral(m_accessMode)
                  << ", open mode = " << asStringLiteral(m_openMode)
                  << ", mode = " << std::bitset<sizeof(mode_t)>(static_cast<mode_t>(m_filePermissions))
                  << ", sizeInBytes = " << m_size << " ]" << std::endl;
    };


    // on qnx the current working directory will be added to the /dev/shmem path if the leading slash is missing
    if (m_name.empty())
    {
        std::cerr << "No shared memory name specified!" << std::endl;
        return cxx::error<SharedMemoryError>(SharedMemoryError::EMPTY_NAME);
    }

    if (!cxx::isValidFileName(m_name))
    {
        std::cerr << "Shared memory requires a valid file name (not path) as name and \"" << m_name
                  << "\" is not a valid file name" << std::endl;
        return cxx::error<SharedMemoryError>(SharedMemoryError::INVALID_FILE_NAME);
    }

    auto nameWithLeadingSlash = addLeadingSlash(m_name);

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
                result = posixCall(iox_shm_open)(nameWithLeadingSlash.c_str(),
                                                 convertToOflags(m_accessMode, OpenMode::OPEN_EXISTING),
                                                 static_cast<mode_t>(m_filePermissions))
                             .failureReturnValue(SharedMemory::INVALID_HANDLE)
                             .evaluate();
                if (!result.has_error())
                {
                    constexpr bool HAS_NO_OWNERSHIP = false;
                    sharedMemoryFileHandle = result->value;
                    return cxx::success<SharedMemory>(SharedMemory(m_name, sharedMemoryFileHandle, HAS_NO_OWNERSHIP));
                }
            }

            printError();
            return cxx::error<SharedMemoryError>(SharedMemory::errnoToEnum(result.get_error().errnum));
        }
        sharedMemoryFileHandle = result->value;
    }

    const bool hasOwnership = (m_openMode == OpenMode::EXCLUSIVE_CREATE || m_openMode == OpenMode::PURGE_AND_CREATE
                               || m_openMode == OpenMode::OPEN_OR_CREATE);
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
                    std::cerr << "Unable to close filedescriptor (close failed) : " << r.getHumanReadableErrnum()
                              << " for SharedMemory \"" << m_name << "\"" << std::endl;
                });

            posixCall(iox_shm_unlink)(nameWithLeadingSlash.c_str())
                .failureReturnValue(SharedMemory::INVALID_HANDLE)
                .evaluate()
                .or_else([&](auto&) {
                    std::cerr << "Unable to remove previously created SharedMemory \"" << m_name
                              << "\". This may be a SharedMemory leak." << std::endl;
                });

            return cxx::error<SharedMemoryError>(SharedMemory::errnoToEnum(result->errnum));
        }
    }

    return cxx::success<SharedMemory>(SharedMemory(m_name, sharedMemoryFileHandle, hasOwnership));
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

cxx::expected<bool, SharedMemoryError> SharedMemory::unlinkIfExist(const Name_t& name) noexcept
{
    auto nameWithLeadingSlash = addLeadingSlash(name);

    auto result = posixCall(iox_shm_unlink)(nameWithLeadingSlash.c_str())
                      .failureReturnValue(INVALID_HANDLE)
                      .ignoreErrnos(ENOENT)
                      .evaluate();

    if (!result.has_error())
    {
        return cxx::success<bool>(result->errnum != ENOENT);
    }

    return cxx::error<SharedMemoryError>(errnoToEnum(result.get_error().errnum));
}

bool SharedMemory::unlink() noexcept
{
    if (m_hasOwnership)
    {
        auto unlinkResult = unlinkIfExist(m_name);
        if (unlinkResult.has_error() || !unlinkResult.value())
        {
            std::cerr << "Unable to unlink SharedMemory (shm_unlink failed)." << std::endl;
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
            std::cerr << "Unable to close SharedMemory filedescriptor (close failed) : " << r.getHumanReadableErrnum()
                      << std::endl;
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
        std::cerr << "No permission to modify, truncate or access the shared memory!" << std::endl;
        return SharedMemoryError::INSUFFICIENT_PERMISSIONS;
    case EPERM:
        std::cerr << "Resizing a file beyond its current size is not supported by the filesystem!" << std::endl;
        return SharedMemoryError::NO_RESIZE_SUPPORT;
    case EFBIG:
        std::cerr << "Requested Shared Memory is larger then the maximum file size." << std::endl;
        return SharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EINVAL:
        std::cerr << "Requested Shared Memory is larger then the maximum file size or the filedescriptor does not "
                     "belong to a regular file."
                  << std::endl;
        return SharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EBADF:
        std::cerr << "Provided filedescriptor is not a valid filedescriptor." << std::endl;
        return SharedMemoryError::INVALID_FILEDESCRIPTOR;
    case EEXIST:
        std::cerr << "A Shared Memory with the given name already exists." << std::endl;
        return SharedMemoryError::DOES_EXIST;
    case EISDIR:
        std::cerr << "The requested Shared Memory file is a directory." << std::endl;
        return SharedMemoryError::PATH_IS_A_DIRECTORY;
    case ELOOP:
        std::cerr << "Too many symbolic links encountered while traversing the path." << std::endl;
        return SharedMemoryError::TOO_MANY_SYMBOLIC_LINKS;
    case EMFILE:
        std::cerr << "Process limit of maximum open files reached." << std::endl;
        return SharedMemoryError::PROCESS_LIMIT_OF_OPEN_FILES_REACHED;
    case ENFILE:
        std::cerr << "System limit of maximum open files reached." << std::endl;
        return SharedMemoryError::SYSTEM_LIMIT_OF_OPEN_FILES_REACHED;
    case ENOENT:
        std::cerr << "Shared Memory does not exist." << std::endl;
        return SharedMemoryError::DOES_NOT_EXIST;
    case ENOMEM:
        std::cerr << "Not enough memory available to create shared memory." << std::endl;
        return SharedMemoryError::NOT_ENOUGH_MEMORY_AVAILABLE;
    default:
        std::cerr << "This should never happen! An unknown error occurred!" << std::endl;
        return SharedMemoryError::UNKNOWN_ERROR;
    }
}

} // namespace posix
} // namespace iox
