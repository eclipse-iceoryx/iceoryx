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

#include "iceoryx_utils/posix_wrapper/file_lock.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/platform_correction.hpp"
#include "iceoryx_utils/platform/unistd.hpp"

#include <sys/file.h>

namespace iox
{
namespace posix
{
FileLock::FileLock(const FileName_t& name) noexcept
    : m_name(name)
{
    initializeFileLock().and_then([this]() { this->m_isInitialized = true; }).or_else([this](FileLockError& error) {
        this->m_isInitialized = false;
        this->m_errorValue = error;
    });
}

cxx::expected<FileLockError> FileLock::initializeFileLock() noexcept
{
    if (m_name.empty())
    {
        return cxx::error<FileLockError>(FileLockError::NO_FILE_NAME_PROVIDED);
    }
    PathName_t fullPath(PATH_PREFIX + m_name + ".lock");
    constexpr int createFileForReadWrite = O_CREAT | O_RDWR;
    mode_t userReadWriteAccess = S_IRUSR | S_IWUSR;

    auto openCall = cxx::makeSmartC(openFile,
                                    cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                    {ERROR_CODE},
                                    {},
                                    fullPath.c_str(),
                                    createFileForReadWrite,
                                    userReadWriteAccess);

    if (openCall.hasErrors())
    {
        return createErrorFromErrnum(openCall.getErrNum());
    }
    else
    {
        m_fd = openCall.getReturnValue();

        auto lockCall = cxx::makeSmartC(
            flock, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {EWOULDBLOCK}, m_fd, LOCK_EX | LOCK_NB);

        if (lockCall.hasErrors())
        {
            closeFileDescriptor();
            // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
            return createErrorFromErrnum(openCall.getErrNum());
        }
        else if (lockCall.getErrNum() == EWOULDBLOCK)
        {
            closeFileDescriptor();
            // possible errors in closeFileDescriptor() are masked and we inform the user about the actual error
            return cxx::error<FileLockError>(FileLockError::LOCKED_BY_OTHER_PROCESS);
        }
    }
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
        if (destroy().has_error())
        {
            std::cerr << "Unable to cleanup file lock \"" << m_name
                      << "\" in the move constructor/move assingment operator" << std::endl;
        }

        CreationPattern_t::operator=(std::move(rhs));

        m_name = std::move(rhs.m_name);
        m_fd = std::move(rhs.m_fd);

        rhs.m_name.assign("");
        rhs.m_fd = INVALID_FD;
    }

    return *this;
}

cxx::expected<FileLockError> FileLock::destroy() noexcept
{
    if (m_isInitialized)
    {
        return closeFileDescriptor();
    }

    return cxx::success<void>();
}

FileLock::~FileLock() noexcept
{
    closeFileDescriptor();
}

cxx::expected<FileLockError> FileLock::closeFileDescriptor() noexcept
{
    if (m_fd != INVALID_FD)
    {
        auto closeCall =
            cxx::makeSmartC(closePlatformFileHandle, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, m_fd);

        if (!closeCall.hasErrors())
        {
            m_fd = INVALID_FD;
            m_isInitialized = false;

            return cxx::success<void>();
        }
        else
        {
            return createErrorFromErrnum(closeCall.getErrNum());
        }
    }
    return cxx::success<>();
}

cxx::error<FileLockError> FileLock::createErrorFromErrnum(const int32_t errnum) const noexcept
{
    switch (errnum)
    {
    case EACCES:
    {
        std::cerr << "permission to access file denied \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::ACCESS_DENIED);
    }
    case EBUSY:
    {
        std::cerr << "O_EXCL provided but file \"" << m_name << "\""
                  << " is currently used" << std::endl;
        return cxx::error<FileLockError>(FileLockError::FILE_IN_USE);
    }
    case EDQUOT:
    {
        std::cerr << "user disk quota exhausted for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::QUOTA_EXHAUSTED);
    }
    case EEXIST:
    {
        std::cerr << "file \"" << m_name << "\""
                  << " already exists" << std::endl;
        return cxx::error<FileLockError>(FileLockError::FILE_EXISTS);
    }
    case EFAULT:
    {
        std::cerr << "outside address space error for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::ACCESS_DENIED);
    }
    case EFBIG:
    case EOVERFLOW:
    {
        std::cerr << "file \"" << m_name << "\""
                  << " is too large to be openend" << std::endl;
        return cxx::error<FileLockError>(FileLockError::FILE_TOO_LARGE);
    }
    case EINVAL:
    {
        std::cerr << "provided invalid arguments for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INVALID_ARGUMENTS);
    }
    case EISDIR:
    {
        std::cerr << "write access requested for directory \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INVALID_ARGUMENTS);
    }
    case ELOOP:
    {
        std::cerr << "too many symbolic links for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_NAME);
    }
    case EMFILE:
    {
        std::cerr << "process limit reached for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::PROCESS_LIMIT);
    }
    case ENAMETOOLONG:
    {
        std::cerr << "name too long for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_NAME);
    }
    case ENFILE:
    {
        std::cerr << "system limit reached for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::SYSTEM_LIMIT);
    }
    case ENODEV:
    {
        std::cerr << "permission to access file denied \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::ACCESS_DENIED);
    }
    case ENOENT:
    {
        std::cerr << "file \"" << m_name << "\""
                  << "does not exist" << std::endl;
        return cxx::error<FileLockError>(FileLockError::NO_SUCH_FILE);
    }
    case ENOMEM:
    {
        std::cerr << "out of memory for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::OUT_OF_MEMORY);
    }
    case ENOSPC:
    {
        std::cerr << "Device has no space for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::QUOTA_EXHAUSTED);
    }
    case ENOTDIR:
    {
        std::cerr << "not a directory error for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_NAME);
    }
    case ENXIO:
    {
        std::cerr << "\"" << m_name << "\""
                  << " is a special file and no corresponding device exists" << std::endl;
        return cxx::error<FileLockError>(FileLockError::SPECIAL_FILE);
    }
    case EOPNOTSUPP:
    {
        std::cerr << "filesystem does not support O_TMPFILE for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::TEMP_FILE_NOT_SUPPORTED);
    }
    case EPERM:
    {
        std::cerr << "permission to access file denied \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::ACCESS_DENIED);
    }
    case EBADF:
    {
        std::cerr << "invalid file descriptor for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_DESCRIPTOR);
    }
    case EROFS:
    {
        std::cerr << "read only error for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INVALID_FILE_NAME);
    }
    case ETXTBSY:
    {
        std::cerr << "write access requested for file \"" << m_name << "\""
                  << " in use" << std::endl;
        return cxx::error<FileLockError>(FileLockError::FILE_IN_USE);
    }
    case EWOULDBLOCK:
    {
        // no error message needed since this is a normal use case
        return cxx::error<FileLockError>(FileLockError::LOCKED_BY_OTHER_PROCESS);
    }
    case ENOLCK:
    {
        std::cerr << "system limit for locks reached for file \"" << m_name << "\"" << std::endl;
        return cxx::error<FileLockError>(FileLockError::SYSTEM_LIMIT);
    }
    default:
    {
        std::cerr << "internal logic error in file \"" << m_name << "\" occurred" << std::endl;
        return cxx::error<FileLockError>(FileLockError::INTERNAL_LOGIC_ERROR);
    }
    }
}

} // namespace posix
} // namespace iox
