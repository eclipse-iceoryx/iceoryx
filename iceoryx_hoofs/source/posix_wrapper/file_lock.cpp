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

#include "iceoryx_hoofs/posix_wrapper/file_lock.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/platform/errno.hpp"
#include "iceoryx_hoofs/platform/fcntl.hpp"
#include "iceoryx_hoofs/platform/stat.hpp"
#include "iceoryx_hoofs/platform/unistd.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/types.hpp"

#include "iceoryx_hoofs/platform/platform_correction.hpp"

namespace iox
{
namespace posix
{
constexpr const char FileLock::LOCK_FILE_SUFFIX[];

cxx::expected<FileLock, FileLockError> FileLockBuilder::create() noexcept
{
    if (!cxx::isValidFileName(m_name))
    {
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_NAME);
    }

    if (!cxx::isValidFilePath(m_path))
    {
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_PATH);
    }

    FileLock::FilePath_t fileLockPath = m_path;

    if (!cxx::doesEndWithPathSeparator(fileLockPath))
    {
        fileLockPath.unsafe_append(iox::platform::IOX_PATH_SEPARATORS[0]);
    }

    fileLockPath.unsafe_append(m_name);
    fileLockPath.unsafe_append(FileLock::LOCK_FILE_SUFFIX);

    auto openCall = posixCall(iox_open)(fileLockPath.c_str(),
                                        convertToOflags(AccessMode::READ_ONLY, OpenMode::EXCLUSIVE_CREATE),
                                        static_cast<mode_t>(m_permission))
                        .failureReturnValue(-1)
                        .evaluate();

    if (openCall.has_error())
    {
        return cxx::error<FileLockError>(
            FileLock::convertErrnoToFileLockError(openCall.get_error().errnum, fileLockPath));
    }

    auto fileDescriptor = openCall.value().value;
    auto lockCall = posixCall(iox_flock)(fileDescriptor, static_cast<int>(FileLock::LockOperation::LOCK))
                        .failureReturnValue(-1)
                        .suppressErrorMessagesForErrnos(EWOULDBLOCK)
                        .evaluate();

    if (lockCall.has_error())
    {
        FileLock::closeFileDescriptor(fileDescriptor, fileLockPath).or_else([](auto) {
            std::cerr << "Unable to close file lock in error related cleanup during initialization." << std::endl;
        });
        // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
        return cxx::error<FileLockError>(
            FileLock::convertErrnoToFileLockError(lockCall.get_error().errnum, fileLockPath));
    }

    return cxx::success<FileLock>(FileLock(fileDescriptor, fileLockPath));
}

FileLock::FileLock(FileLock&& rhs) noexcept
{
    *this = std::move(rhs);
}

FileLock& FileLock::operator=(FileLock&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (closeFileDescriptor(m_fd, m_fileLockPath).has_error())
        {
            std::cerr << "Unable to cleanup file lock \"" << m_fileLockPath
                      << "\" in the move constructor/move assingment operator" << std::endl;
        }

        m_fileLockPath = std::move(rhs.m_fileLockPath);
        m_fd = std::move(rhs.m_fd);

        rhs.invalidate();
    }

    return *this;
}

FileLock::~FileLock() noexcept
{
    if (closeFileDescriptor(m_fd, m_fileLockPath).has_error())
    {
        std::cerr << "unable to cleanup file lock \"" << m_fileLockPath << "\" in the destructor" << std::endl;
    }
}

cxx::expected<FileLockError> FileLock::closeFileDescriptor(const int32_t fileDescriptor,
                                                           const FilePath_t& fileLockPath) noexcept
{
    if (fileDescriptor != INVALID_FD)
    {
        bool cleanupFailed = false;
        posixCall(iox_flock)(fileDescriptor, static_cast<int>(LockOperation::UNLOCK))
            .failureReturnValue(-1)
            .suppressErrorMessagesForErrnos(EWOULDBLOCK)
            .evaluate()
            .or_else([&](auto& result) {
                cleanupFailed = true;
                IOX_DISCARD_RESULT(FileLock::convertErrnoToFileLockError(result.errnum, fileLockPath));
                std::cerr << "Unable to unlock the file lock \"" << fileLockPath << "\"" << std::endl;
            });

        posixCall(iox_close)(fileDescriptor).failureReturnValue(-1).evaluate().or_else([&](auto& result) {
            cleanupFailed = true;
            IOX_DISCARD_RESULT(FileLock::convertErrnoToFileLockError(result.errnum, fileLockPath));
            std::cerr << "Unable to close the file handle to the file lock \"" << fileLockPath << "\"" << std::endl;
        });

        posixCall(remove)(fileLockPath.c_str()).failureReturnValue(-1).evaluate().or_else([&](auto& result) {
            cleanupFailed = true;
            IOX_DISCARD_RESULT(FileLock::convertErrnoToFileLockError(result.errnum, fileLockPath));
            std::cerr << "Unable to remove the file lock \"" << fileLockPath << "\"" << std::endl;
        });

        if (cleanupFailed)
        {
            return cxx::error<FileLockError>(FileLockError::INTERNAL_LOGIC_ERROR);
        }
    }
    return cxx::success<>();
}

void FileLock::invalidate() noexcept
{
    m_fd = INVALID_FD;
    m_fileLockPath.assign("");
}

FileLockError FileLock::convertErrnoToFileLockError(const int32_t errnum, const FilePath_t& fileLockPath) noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "permission to access file denied \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::ACCESS_DENIED;
    }
    case EDQUOT:
    {
        std::cerr << "user disk quota exhausted for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::QUOTA_EXHAUSTED;
    }
    case EFAULT:
    {
        std::cerr << "outside address space error for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::ACCESS_DENIED;
    }
    case EFBIG:
    case EOVERFLOW:
    {
        std::cerr << "file \"" << fileLockPath << "\""
                  << " is too large to be openend" << std::endl;
        return FileLockError::FILE_TOO_LARGE;
    }
    case ELOOP:
    {
        std::cerr << "too many symbolic links for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::INVALID_FILE_NAME;
    }
    case EMFILE:
    {
        std::cerr << "process limit reached for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::PROCESS_LIMIT;
    }
    case ENFILE:
    {
        std::cerr << "system limit reached for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::SYSTEM_LIMIT;
    }
    case ENODEV:
    {
        std::cerr << "permission to access file denied \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::ACCESS_DENIED;
    }
    case ENOENT:
    {
        std::cerr << "directory \"" << platform::IOX_LOCK_FILE_PATH_PREFIX << "\""
                  << " does not exist. Please create it as described in the filesystem hierarchy standard."
                  << std::endl;
        return FileLockError::NO_SUCH_DIRECTORY;
    }
    case ENOMEM:
    {
        std::cerr << "out of memory for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::OUT_OF_MEMORY;
    }
    case ENOSPC:
    {
        std::cerr << "Device has no space for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::QUOTA_EXHAUSTED;
    }
    case ENOSYS:
    {
        std::cerr << "open() not implemented for filesystem to \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::SYS_CALL_NOT_IMPLEMENTED;
    }
    case ENXIO:
    {
        std::cerr << "\"" << fileLockPath << "\""
                  << " is a special file and no corresponding device exists" << std::endl;
        return FileLockError::SPECIAL_FILE;
    }
    case EPERM:
    {
        std::cerr << "permission to access file denied \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::ACCESS_DENIED;
    }
    case EROFS:
    {
        std::cerr << "read only error for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::INVALID_FILE_NAME;
    }
    case ETXTBSY:
    {
        std::cerr << "write access requested for file \"" << fileLockPath << "\""
                  << " in use" << std::endl;
        return FileLockError::FILE_IN_USE;
    }
    case EWOULDBLOCK:
    {
        // no error message needed since this is a normal use case
        return FileLockError::LOCKED_BY_OTHER_PROCESS;
    }
    case ENOLCK:
    {
        std::cerr << "system limit for locks reached for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::SYSTEM_LIMIT;
    }
    case EIO:
    {
        std::cerr << "I/O for file \"" << fileLockPath << "\"" << std::endl;
        return FileLockError::I_O_ERROR;
    }
    default:
    {
        std::cerr << "internal logic error in file \"" << fileLockPath << "\" occurred" << std::endl;
        return FileLockError::INTERNAL_LOGIC_ERROR;
    }
    }
}

} // namespace posix
} // namespace iox
