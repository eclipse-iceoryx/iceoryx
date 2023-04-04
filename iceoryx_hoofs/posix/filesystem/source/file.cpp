// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/file.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/attributes.hpp"
#include "iceoryx_platform/fcntl.hpp"

namespace iox
{
expected<File, FileCreationError> FileBuilder::create(const posix::OpenMode open_mode) noexcept
{
    switch (open_mode)
    {
    case posix::OpenMode::OPEN_EXISTING:
    {
        return this->open();
    }
    case posix::OpenMode::PURGE_AND_CREATE:
    {
        File::remove(m_name);
        IOX_FALLTHROUGH
    }
    case posix::OpenMode::EXCLUSIVE_CREATE:
    {
        return this->open_impl(true, posix::OpenMode::EXCLUSIVE_CREATE);
    }
    case posix::OpenMode::OPEN_OR_CREATE:
    {
        auto result = this->open_impl(false, posix::OpenMode::OPEN_EXISTING);
        if (!result.has_error() || result.get_error() != FileCreationError::DoesNotExist)
        {
            return result;
        }

        return this->open_impl(true, posix::OpenMode::EXCLUSIVE_CREATE);
    }
    }
}

expected<File, FileCreationError> FileBuilder::open_impl(const bool print_error_on_non_existing_file,
                                                         const posix::OpenMode open_mode) noexcept
{
    auto result = posix::posixCall(iox_open)(m_name.as_string().c_str(),
                                             posix::convertToOflags(m_access_mode, open_mode),
                                             static_cast<iox_mode_t>(m_permissions.value()))
                      .failureReturnValue(-1)
                      .evaluate();

    if (!result.has_error())
    {
        return iox::success<File>(File(result.value().value));
    }

    switch (result.get_error().errnum)
    {
    case EACCES:
        IOX_LOG(ERROR) << "Unable to open/create file due to insufficient permissions.";
        return iox::error<FileCreationError>(FileCreationError::PermissionDenied);
    case EPERM:
        IOX_LOG(ERROR) << "Unable to open/create file due to insufficient permissions.";
        return iox::error<FileCreationError>(FileCreationError::PermissionDenied);
    case EINTR:
        IOX_LOG(ERROR) << "Unable to open/create file since an interrupt signal was received.";
        return iox::error<FileCreationError>(FileCreationError::Interrupt);
    case EISDIR:
        IOX_LOG(ERROR) << "Unable to open/create file since it is actually a directory.";
        return iox::error<FileCreationError>(FileCreationError::IsDirectory);
    case ELOOP:
        IOX_LOG(ERROR) << "Unable to open/create file since too many symbolic links were encountered.";
        return iox::error<FileCreationError>(FileCreationError::TooManySymbolicLinksEncountered);
    case EMFILE:
        IOX_LOG(ERROR) << "Unable to open/create file since the process limit of open file descriptors was reached.";
        return iox::error<FileCreationError>(FileCreationError::ProcessLimitOfOpenFileDescriptorsReached);
    case ENFILE:
        IOX_LOG(ERROR) << "Unable to open/create file since the system limit of open file descriptors was reached.";
        return iox::error<FileCreationError>(FileCreationError::SystemLimitOfOpenFileDescriptorsReached);
    case ENOENT:
        if (print_error_on_non_existing_file)
        {
            IOX_LOG(ERROR) << "Unable to open file since the file does not exist.";
        }
        return iox::error<FileCreationError>(FileCreationError::DoesNotExist);
    case ENOMEM:
        IOX_LOG(ERROR) << "Unable to open/create file due to insufficient memory.";
        return iox::error<FileCreationError>(FileCreationError::InsufficientMemory);
    case EOVERFLOW:
        IOX_LOG(ERROR) << "Unable to open/create file since it is too large.";
        return iox::error<FileCreationError>(FileCreationError::FileTooLarge);
    case ETXTBSY:
        IOX_LOG(ERROR) << "Unable to open/create file since it is currently in use.";
        return iox::error<FileCreationError>(FileCreationError::CurrentlyInUse);
    case EEXIST:
        IOX_LOG(ERROR) << "Unable to create file since it already exists.";
        return iox::error<FileCreationError>(FileCreationError::AlreadyExists);
    default:
        IOX_LOG(ERROR) << "Unable to open/create file due to insufficient permissions.";
        return iox::error<FileCreationError>(FileCreationError::UnknownError);
    }

    return iox::success<File>(File(result->value));
}

expected<File, FileCreationError> FileBuilder::open() noexcept
{
    return this->open_impl(true, posix::OpenMode::OPEN_EXISTING);
}

File::File(const int file_descriptor) noexcept
    : m_file_descriptor{file_descriptor}
{
}

expected<bool, FileAccessError> File::does_exist(const FilePath& file) noexcept
{
    auto result = posix::posixCall(iox_access)(file.as_string().c_str(), F_OK).failureReturnValue(-1).evaluate();

    if (!result.has_error())
    {
        return iox::success<bool>(true);
    }

    switch (result.get_error().errnum)
    {
    case EACCES:
        IOX_LOG(ERROR) << "Unable to determine if file exists due to insufficient permissions.";
        return iox::error<FileAccessError>(FileAccessError::InsufficientPermissions);
    case ENOENT:
        return iox::success<bool>(false);
    case ELOOP:
        IOX_LOG(ERROR) << "Unable to determine if file exists due to too many symbolic links.";
        return iox::error<FileAccessError>(FileAccessError::TooManySymbolicLinksEncountered);
    case EIO:
        IOX_LOG(ERROR) << "Unable to determine if file exists due to an IO failure.";
        return iox::error<FileAccessError>(FileAccessError::IoFailure);
    case ENOMEM:
        IOX_LOG(ERROR) << "Unable to determine if file exists due insufficient kernel memory.";
        return iox::error<FileAccessError>(FileAccessError::InsufficientKernelMemory);
    default:
        IOX_LOG(ERROR) << "Unable to determine if file exists since an unknown error occurred.";
        return iox::error<FileAccessError>(FileAccessError::UnknownError);
    }
}

expected<FileRemoveError> File::remove(const FilePath& file) noexcept
{
}
} // namespace iox
