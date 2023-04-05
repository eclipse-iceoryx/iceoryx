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

expected<bool, FileRemoveError> File::remove(const FilePath& file) noexcept
{
    auto result = posix::posixCall(iox_unlink)(file.as_string().c_str()).failureReturnValue(-1).evaluate();

    if (!result.has_error())
    {
        return iox::success<bool>(true);
    }

    switch (result.get_error().errnum)
    {
    case ENOENT:
        return iox::success<bool>(false);
    case EPERM:
        IOX_FALLTHROUGH
    case EACCES:
        IOX_LOG(ERROR) << "Unable to remove file due to insufficient permissions.";
        return iox::error<FileRemoveError>(FileRemoveError::PermissionDenied);
    case EBUSY:
        IOX_LOG(ERROR) << "Unable to remove file since it is currently in use.";
        return iox::error<FileRemoveError>(FileRemoveError::CurrentlyInUse);
    case EIO:
        IOX_LOG(ERROR) << "Unable to remove file due to an IO failure.";
        return iox::error<FileRemoveError>(FileRemoveError::IoFailure);
    case ELOOP:
        IOX_LOG(ERROR) << "Unable to remove file due to too many symbolic links.";
        return iox::error<FileRemoveError>(FileRemoveError::TooManySymbolicLinksEncountered);
    case ENOMEM:
        IOX_LOG(ERROR) << "Unable to remove file due to insufficient kernel memory.";
        return iox::error<FileRemoveError>(FileRemoveError::InsufficientKernelMemory);
    case EISDIR:
        IOX_LOG(ERROR) << "Unable to remove file since it is a directory.";
        return iox::error<FileRemoveError>(FileRemoveError::IsDirectory);
    case EROFS:
        IOX_LOG(ERROR) << "Unable to remove file since it resides on a read-only file system.";
        return iox::error<FileRemoveError>(FileRemoveError::ReadOnlyFilesystem);
    default:
        IOX_LOG(ERROR) << "Unable to remove file since an unknown error occurred.";
        return iox::error<FileRemoveError>(FileRemoveError::UnknownError);
    }
}

expected<FileOffsetError> File::set_offset(const uint64_t offset) const noexcept
{
    auto result = posix::posixCall(iox_lseek)(m_file_descriptor, static_cast<iox_off_t>(offset), IOX_SEEK_SET)
                      .failureReturnValue(-1)
                      .evaluate();

    if (!result.has_error())
    {
        if (result->value == static_cast<iox_off_t>(offset))
        {
            return iox::success<>();
        }

        IOX_LOG(ERROR) << "Unable to set file offset position since it set to the wrong offset position.";
        return iox::error<FileOffsetError>(FileOffsetError::OffsetAtWrongPosition);
    }


    switch (result.get_error().errnum)
    {
    case EINVAL:
        IOX_FALLTHROUGH
    case ENXIO:
        IOX_LOG(ERROR) << "Unable to set file offset position since it beyond the file limits.";
        return iox::error<FileOffsetError>(FileOffsetError::OffsetBeyondFileLimits);
    case EOVERFLOW:
        IOX_LOG(ERROR)
            << "Unable to set file offset position since the file is too large and the offset would overflow.";
        return iox::error<FileOffsetError>(FileOffsetError::FileOffsetOverflow);
    case EPIPE:
        IOX_LOG(ERROR) << "Unable to set file offset position since seeking is not supported by the file type.";
        return iox::error<FileOffsetError>(FileOffsetError::SeekingNotSupportedByFileType);
    default:
        IOX_LOG(ERROR) << "Unable to remove file since an unknown error occurred.";
        return iox::error<FileOffsetError>(FileOffsetError::UnknownError);
    }
}

expected<uint64_t, FileReadError> File::read(uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    return read_at(0, buffer, buffer_len);
}

expected<uint64_t, FileReadError>
File::read_at(const uint64_t offset, uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    if (set_offset(offset).has_error())
    {
        IOX_LOG(ERROR) << "Unable to read from file since the offset could not be set.";
        return iox::error<FileReadError>(FileReadError::OffsetFailure);
    }

    auto result = posix::posixCall(iox_read)(m_file_descriptor, buffer, buffer_len).failureReturnValue(-1).evaluate();

    if (!result.has_error())
    {
        return iox::success<uint64_t>(result->value);
    }

    switch (result.get_error().errnum)
    {
    case EAGAIN:
        IOX_LOG(ERROR) << "Unable to read from file since the operation would block.";
        return iox::error<FileReadError>(FileReadError::OperationWouldBlock);
    case EINTR:
        IOX_LOG(ERROR) << "Unable to read from file since an interrupt signal was received.";
        return iox::error<FileReadError>(FileReadError::Interrupt);
    case EINVAL:
        IOX_LOG(ERROR) << "Unable to read from file since is unsuitable for reading.";
        return iox::error<FileReadError>(FileReadError::FileUnsuitableForReading);
    case EIO:
        IOX_LOG(ERROR) << "Unable to read from file since an IO failure occurred.";
        return iox::error<FileReadError>(FileReadError::IoFailure);
    case EISDIR:
        IOX_LOG(ERROR) << "Unable to read from file since it is a directory.";
        return iox::error<FileReadError>(FileReadError::IsDirectory);
    default:
        IOX_LOG(ERROR) << "Unable to read from file since an unknown error occurred.";
        return iox::error<FileReadError>(FileReadError::UnknownError);
    }
}

expected<uint64_t, FileWriteError> File::write(uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    return write_at(0, buffer, buffer_len);
}

expected<uint64_t, FileWriteError>
File::write_at(const uint64_t offset, uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    if (set_offset(offset).has_error())
    {
        IOX_LOG(ERROR) << "Unable to write to file since the offset could not be set.";
        return iox::error<FileWriteError>(FileWriteError::OffsetFailure);
    }

    auto result = posix::posixCall(iox_write)(m_file_descriptor, buffer, buffer_len).failureReturnValue(-1).evaluate();

    if (!result.has_error())
    {
        return iox::success<uint64_t>(result->value);
    }

    switch (result.get_error().errnum)
    {
    case EAGAIN:
        IOX_LOG(ERROR) << "Unable to write to file since the operation would block.";
        return iox::error<FileWriteError>(FileWriteError::OperationWouldBlock);
    case EDQUOT:
        IOX_LOG(ERROR) << "Unable to write to file since the users disk quota has been exhausted.";
        return iox::error<FileWriteError>(FileWriteError::DiskQuotaExhausted);
    case EFBIG:
        IOX_LOG(ERROR) << "Unable to write to file since file size exceeds the maximum supported size.";
        return iox::error<FileWriteError>(FileWriteError::FileSizeExceedsMaximumSupportedSize);
    case EINTR:
        IOX_LOG(ERROR) << "Unable to write to file since an interrupt signal occurred.";
        return iox::error<FileWriteError>(FileWriteError::Interrupt);
    case EINVAL:
        IOX_LOG(ERROR) << "Unable to write to file since the file is unsuitable for writing.";
        return iox::error<FileWriteError>(FileWriteError::FileUnsuitableForWriting);
    case ENOSPC:
        IOX_LOG(ERROR) << "Unable to write to file since there is no space left on target.";
        return iox::error<FileWriteError>(FileWriteError::NoSpaceLeftOnDevice);
    case EPERM:
        IOX_LOG(ERROR) << "Unable to write to file since the operation was prevented by a file seal.";
        return iox::error<FileWriteError>(FileWriteError::PreventedByFileSeal);
    default:
        IOX_LOG(ERROR) << "Unable to write to file since an unknown error has occurred.";
        return iox::error<FileWriteError>(FileWriteError::UnknownError);
    }
}
} // namespace iox
