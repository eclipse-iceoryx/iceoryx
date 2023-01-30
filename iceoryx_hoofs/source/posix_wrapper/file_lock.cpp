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
#include "iceoryx_hoofs/platform/file.hpp"
#include "iceoryx_hoofs/platform/stat.hpp"
#include "iceoryx_hoofs/platform/unistd.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include "iceoryx_hoofs/platform/platform_correction.hpp"

namespace iox
{
namespace posix
{
constexpr const char FileLock::LOCK_FILE_SUFFIX[];

FileLock::FileLock(const FileName_t& name) noexcept
    : m_name(name)
    , m_fileLockPath(platform::IOX_LOCK_FILE_PATH_PREFIX + m_name + LOCK_FILE_SUFFIX)
{
    initializeFileLock().and_then([this]() { this->m_isInitialized = true; }).or_else([this](FileLockError& error) {
        this->m_isInitialized = false;
        this->m_errorValue = error;
    });
}

cxx::expected<FileLockError> FileLock::initializeFileLock() noexcept
{
    if (!cxx::isValidFilePath(m_name))
    {
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_NAME);
    }
    constexpr int CREATE_FILE_FOR_READ_WRITE = O_CREAT | O_RDWR;
    mode_t userReadWriteAccess = S_IRUSR | S_IWUSR;

    auto openCall = posixCall(iox_open)(m_fileLockPath.c_str(), CREATE_FILE_FOR_READ_WRITE, userReadWriteAccess)
                        .failureReturnValue(ERROR_CODE)
                        .evaluate()
                        .and_then([this](auto& r) { this->m_fd = r.value; });

    if (openCall.has_error())
    {
        return cxx::error<FileLockError>(convertErrnoToFileLockError(openCall.get_error().errnum));
    }

    auto lockCall = posixCall(iox_flock)(m_fd, LOCK_EX | LOCK_NB)
                        .failureReturnValue(ERROR_CODE)
                        .suppressErrorMessagesForErrnos(EWOULDBLOCK)
                        .evaluate();

    if (lockCall.has_error())
    {
        posixCall(iox_close)(m_fd).failureReturnValue(ERROR_CODE).evaluate().or_else([&](auto& result) {
            IOX_DISCARD_RESULT(this->convertErrnoToFileLockError(result.errnum));
            std::cerr << "Unable to close the file handle to the file lock \"" << m_fileLockPath << "\"" << std::endl;
        });

        // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
        return cxx::error<FileLockError>(convertErrnoToFileLockError(lockCall.get_error().errnum));
    }
    m_isInitialized = true;

    return cxx::success<>();
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
            std::cerr << "Unable to cleanup file lock \"" << m_fileLockPath
                      << "\" in the move constructor/move assingment operator" << std::endl;
        }

        CreationPattern_t::operator=(std::move(rhs));

        m_name = std::move(rhs.m_name);
        m_fileLockPath = std::move(rhs.m_fileLockPath);
        m_fd = std::move(rhs.m_fd);

        rhs.invalidate();
    }

    return *this;
}

FileLock::~FileLock() noexcept
{
    if (closeFileDescriptor().has_error())
    {
        std::cerr << "unable to cleanup file lock \"" << m_fileLockPath << "\" in the destructor" << std::endl;
    }
}

cxx::expected<FileLockError> FileLock::closeFileDescriptor() noexcept
{
    if (isInitialized() && (m_fd != INVALID_FD))
    {
        bool cleanupFailed = false;
        posixCall(iox_flock)(m_fd, LOCK_UN)
            .failureReturnValue(ERROR_CODE)
            .suppressErrorMessagesForErrnos(EWOULDBLOCK)
            .evaluate()
            .or_else([&](auto& result) {
                cleanupFailed = true;
                IOX_DISCARD_RESULT(this->convertErrnoToFileLockError(result.errnum));
                std::cerr << "Unable to unlock the file lock \"" << m_fileLockPath << "\"" << std::endl;
            });

        posixCall(iox_close)(m_fd).failureReturnValue(ERROR_CODE).evaluate().or_else([&](auto& result) {
            cleanupFailed = true;
            IOX_DISCARD_RESULT(this->convertErrnoToFileLockError(result.errnum));
            std::cerr << "Unable to close the file handle to the file lock \"" << m_fileLockPath << "\"" << std::endl;
        });

        posixCall(remove)(m_fileLockPath.c_str()).failureReturnValue(ERROR_CODE).evaluate().or_else([&](auto& result) {
            cleanupFailed = true;
            IOX_DISCARD_RESULT(this->convertErrnoToFileLockError(result.errnum));
            std::cerr << "Unable to remove the file lock \"" << m_fileLockPath << "\"" << std::endl;
        });

        invalidate();

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
    m_isInitialized = false;
    m_name.assign("");
    m_fileLockPath.assign("");
}

FileLockError FileLock::convertErrnoToFileLockError(const int32_t errnum) const noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "permission to access file denied \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::ACCESS_DENIED;
    }
    case EDQUOT:
    {
        std::cerr << "user disk quota exhausted for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::QUOTA_EXHAUSTED;
    }
    case EFAULT:
    {
        std::cerr << "outside address space error for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::ACCESS_DENIED;
    }
    case EFBIG:
    case EOVERFLOW:
    {
        std::cerr << "file \"" << m_fileLockPath << "\""
                  << " is too large to be openend" << std::endl;
        return FileLockError::FILE_TOO_LARGE;
    }
    case EINVAL:
    {
        std::cerr << "provided invalid characters in file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::INVALID_CHARACTERS_IN_FILE_NAME;
    }
    case ELOOP:
    {
        std::cerr << "too many symbolic links for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::INVALID_FILE_NAME;
    }
    case EMFILE:
    {
        std::cerr << "process limit reached for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::PROCESS_LIMIT;
    }
    case ENFILE:
    {
        std::cerr << "system limit reached for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::SYSTEM_LIMIT;
    }
    case ENODEV:
    {
        std::cerr << "permission to access file denied \"" << m_fileLockPath << "\"" << std::endl;
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
        std::cerr << "out of memory for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::OUT_OF_MEMORY;
    }
    case ENOSPC:
    {
        std::cerr << "Device has no space for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::QUOTA_EXHAUSTED;
    }
    case ENOSYS:
    {
        std::cerr << "open() not implemented for filesystem to \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::SYS_CALL_NOT_IMPLEMENTED;
    }
    case ENXIO:
    {
        std::cerr << "\"" << m_fileLockPath << "\""
                  << " is a special file and no corresponding device exists" << std::endl;
        return FileLockError::SPECIAL_FILE;
    }
    case EPERM:
    {
        std::cerr << "permission to access file denied \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::ACCESS_DENIED;
    }
    case EROFS:
    {
        std::cerr << "read only error for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::INVALID_FILE_NAME;
    }
    case ETXTBSY:
    {
        std::cerr << "write access requested for file \"" << m_fileLockPath << "\""
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
        std::cerr << "system limit for locks reached for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::SYSTEM_LIMIT;
    }
    case EIO:
    {
        std::cerr << "I/O for file \"" << m_fileLockPath << "\"" << std::endl;
        return FileLockError::I_O_ERROR;
    }
    default:
    {
        std::cerr << "internal logic error in file \"" << m_fileLockPath << "\" occurred" << std::endl;
        return FileLockError::INTERNAL_LOGIC_ERROR;
    }
    }
}

} // namespace posix
} // namespace iox
