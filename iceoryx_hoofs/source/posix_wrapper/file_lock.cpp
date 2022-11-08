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
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iceoryx_platform/errno.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/unistd.hpp"

#include "iceoryx_platform/platform_correction.hpp"

namespace iox
{
namespace posix
{
/// NOLINTJUSTIFICATION see declaration
/// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
constexpr const char FileLock::LOCK_FILE_SUFFIX[];

cxx::expected<FileLock, FileLockError> FileLockBuilder::create() noexcept
{
    if (!cxx::isValidFileName(m_name))
    {
        LogError() << "Unable to create FileLock since the name \"" << m_name << "\" is not a valid file name.";
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_NAME);
    }

    if (!cxx::isValidPathToDirectory(m_path))
    {
        LogError() << "Unable to create FileLock since the path \"" << m_path << "\" is not a valid path.";
        return cxx::error<FileLockError>(FileLockError::INVALID_PATH);
    }

    FileLock::FilePath_t fileLockPath = m_path;

    if (!cxx::doesEndWithPathSeparator(fileLockPath))
    {
        fileLockPath.unsafe_append(iox::platform::IOX_PATH_SEPARATORS[0]);
    }

    fileLockPath.unsafe_append(m_name);
    fileLockPath.unsafe_append(FileLock::LOCK_FILE_SUFFIX);

    auto openCall = posixCall(iox_open)(fileLockPath.c_str(),
                                        convertToOflags(AccessMode::READ_ONLY, OpenMode::OPEN_OR_CREATE),
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
        posixCall(iox_close)(fileDescriptor).failureReturnValue(-1).evaluate().or_else([&](auto& result) {
            IOX_DISCARD_RESULT(FileLock::convertErrnoToFileLockError(result.errnum, fileLockPath));
            LogError() << "Unable to close file lock \"" << fileLockPath
                       << "\" in error related cleanup during initialization.";
        });

        //  possible errors in iox_close() are masked and we inform the user about the actual error
        return cxx::error<FileLockError>(
            FileLock::convertErrnoToFileLockError(lockCall.get_error().errnum, fileLockPath));
    }

    return cxx::success<FileLock>(FileLock(fileDescriptor, fileLockPath));
}

FileLock::FileLock(const int32_t fileDescriptor, const FilePath_t& path) noexcept
    : m_fd{fileDescriptor}
    , m_fileLockPath{path}
{
}

FileLock::FileLock(FileLock&& rhs) noexcept
{
    *this = std::move(rhs);
}

FileLock& FileLock::operator=(FileLock&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (closeFileDescriptor().has_error())
        {
            LogError() << "Unable to cleanup file lock \"" << m_fileLockPath
                       << "\" in the move constructor/move assingment operator";
        }

        m_fileLockPath = std::move(rhs.m_fileLockPath);
        m_fd = rhs.m_fd;

        rhs.invalidate();
    }

    return *this;
}

FileLock::~FileLock() noexcept
{
    if (closeFileDescriptor().has_error())
    {
        LogError() << "unable to cleanup file lock \"" << m_fileLockPath << "\" in the destructor";
    }
}

cxx::expected<FileLockError> FileLock::closeFileDescriptor() noexcept
{
    if (m_fd != INVALID_FD)
    {
        bool cleanupFailed = false;
        posixCall(iox_flock)(m_fd, static_cast<int>(LockOperation::UNLOCK))
            .failureReturnValue(-1)
            .suppressErrorMessagesForErrnos(EWOULDBLOCK)
            .evaluate()
            .or_else([&](auto& result) {
                cleanupFailed = true;
                IOX_DISCARD_RESULT(FileLock::convertErrnoToFileLockError(result.errnum, m_fileLockPath));
                LogError() << "Unable to unlock the file lock \"" << m_fileLockPath << "\"";
            });

        posixCall(iox_close)(m_fd).failureReturnValue(-1).evaluate().or_else([&](auto& result) {
            cleanupFailed = true;
            IOX_DISCARD_RESULT(FileLock::convertErrnoToFileLockError(result.errnum, m_fileLockPath));
            LogError() << "Unable to close the file handle to the file lock \"" << m_fileLockPath << "\"";
        });

        posixCall(remove)(m_fileLockPath.c_str()).failureReturnValue(-1).evaluate().or_else([&](auto& result) {
            cleanupFailed = true;
            IOX_DISCARD_RESULT(FileLock::convertErrnoToFileLockError(result.errnum, m_fileLockPath));
            LogError() << "Unable to remove the file lock \"" << m_fileLockPath << "\"";
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

// NOLINTJUSTIFICATION the function size is unavoidable due to the errno translation and the cognitive complexity
// results from the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
FileLockError FileLock::convertErrnoToFileLockError(const int32_t errnum, const FilePath_t& fileLockPath) noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        LogError() << "permission denied for file lock \"" << fileLockPath << "\"";
        return FileLockError::ACCESS_DENIED;
    }
    case EDQUOT:
    {
        LogError() << "user disk quota exhausted for file lock \"" << fileLockPath << "\"";
        return FileLockError::QUOTA_EXHAUSTED;
    }
    case EFAULT:
    {
        LogError() << "outside address space error for file lock \"" << fileLockPath << "\"";
        return FileLockError::ACCESS_DENIED;
    }
    case EFBIG:
    case EOVERFLOW:
    {
        LogError() << "file lock \"" << fileLockPath << "\""
                   << " is too large to be openend";
        return FileLockError::FILE_TOO_LARGE;
    }
    case ELOOP:
    {
        LogError() << "too many symbolic links for file lock \"" << fileLockPath << "\"";
        return FileLockError::INVALID_FILE_NAME;
    }
    case EMFILE:
    {
        LogError() << "process limit reached for file lock \"" << fileLockPath << "\"";
        return FileLockError::PROCESS_LIMIT;
    }
    case ENFILE:
    {
        LogError() << "system limit reached for file lock \"" << fileLockPath << "\"";
        return FileLockError::SYSTEM_LIMIT;
    }
    case ENODEV:
    {
        LogError() << "permission to access file lock denied \"" << fileLockPath << "\"";
        return FileLockError::ACCESS_DENIED;
    }
    case ENOENT:
    {
        LogError() << "directory \"" << &platform::IOX_LOCK_FILE_PATH_PREFIX[0] << "\""
                   << " does not exist.";
        return FileLockError::NO_SUCH_DIRECTORY;
    }
    case ENOMEM:
    {
        LogError() << "out of memory for file lock \"" << fileLockPath << "\"";
        return FileLockError::OUT_OF_MEMORY;
    }
    case ENOSPC:
    {
        LogError() << "Device has no space for file lock \"" << fileLockPath << "\"";
        return FileLockError::QUOTA_EXHAUSTED;
    }
    case ENOSYS:
    {
        LogError() << "open() not implemented for filesystem to \"" << fileLockPath << "\"";
        return FileLockError::SYS_CALL_NOT_IMPLEMENTED;
    }
    case ENXIO:
    {
        LogError() << "\"" << fileLockPath << "\""
                   << " is a special file and no corresponding device exists";
        return FileLockError::SPECIAL_FILE;
    }
    case EPERM:
    {
        LogError() << "permission denied to file lock \"" << fileLockPath << "\"";
        return FileLockError::ACCESS_DENIED;
    }
    case EROFS:
    {
        LogError() << "read only error for file lock \"" << fileLockPath << "\"";
        return FileLockError::INVALID_FILE_NAME;
    }
    case ETXTBSY:
    {
        LogError() << "write access requested for file lock \"" << fileLockPath << "\""
                   << " in use";
        return FileLockError::FILE_IN_USE;
    }
    case EWOULDBLOCK:
    {
        // no error message needed since this is a normal use case
        return FileLockError::LOCKED_BY_OTHER_PROCESS;
    }
    case ENOLCK:
    {
        LogError() << "system limit for locks reached for file lock \"" << fileLockPath << "\"";
        return FileLockError::SYSTEM_LIMIT;
    }
    case EIO:
    {
        LogError() << "I/O for file lock \"" << fileLockPath << "\"";
        return FileLockError::I_O_ERROR;
    }
    default:
    {
        LogError() << "internal logic error in file lock \"" << fileLockPath << "\" occurred";
        return FileLockError::INTERNAL_LOGIC_ERROR;
    }
    }
}

} // namespace posix
} // namespace iox
