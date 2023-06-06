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
#include "iceoryx_platform/errno.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/stdio.hpp"
#include "iox/filesystem.hpp"

namespace iox
{
expected<File, FileCreationError> FileBuilder::create(const FilePath& name) noexcept
{
    if (m_open_mode == posix::OpenMode::PURGE_AND_CREATE)
    {
        if (File::remove(name).has_error())
        {
            IOX_LOG(ERROR) << "Unable to purge and open file \"" << name.as_string()
                           << "\" since the file could not be removed";
            return err(FileCreationError::CannotBePurged);
        }
    }

    return this->open(name);
}

expected<File, FileCreationError> FileBuilder::open(const FilePath& name) noexcept
{
    auto result = posix::posixCall(iox_open)(name.as_string().c_str(),
                                             posix::convertToOflags(m_access_mode, m_open_mode),
                                             static_cast<iox_mode_t>(m_permissions.value()))
                      .failureReturnValue(-1)
                      .evaluate();

    if (!result.has_error())
    {
        File file(result.value().value, m_access_mode);
        const auto perms = file.get_permissions();
        if (perms.has_error())
        {
            IOX_LOG(ERROR) << "Unable to acquire the permissions of the file.";
            return err(FileCreationError::PermissionDenied);
        }

        if (m_access_mode == posix::AccessMode::READ_ONLY || m_access_mode == posix::AccessMode::READ_WRITE)
        {
            if ((perms->value() & perms::owner_read.value()) == 0)
            {
                IOX_LOG(ERROR) << "Unable to open/create file due to insufficient read permissions.";
                return err(FileCreationError::PermissionDenied);
            }
        }

        if (m_access_mode == posix::AccessMode::WRITE_ONLY || m_access_mode == posix::AccessMode::READ_WRITE)
        {
            if ((perms->value() & perms::owner_write.value()) == 0)
            {
                IOX_LOG(ERROR) << "Unable to open/create file due to insufficient write permissions.";
                return err(FileCreationError::PermissionDenied);
            }
        }

        return ok(std::move(file));
    }

    switch (result.error().errnum)
    {
    case EACCES:
        IOX_LOG(ERROR) << "Unable to open/create file due to insufficient permissions.";
        return err(FileCreationError::PermissionDenied);
    case EPERM:
        IOX_LOG(ERROR) << "Unable to open/create file due to insufficient permissions.";
        return err(FileCreationError::PermissionDenied);
    case EINTR:
        IOX_LOG(ERROR) << "Unable to open/create file since an interrupt signal was received.";
        return err(FileCreationError::Interrupt);
    case EISDIR:
        IOX_LOG(ERROR) << "Unable to open/create file since it is actually a directory.";
        return err(FileCreationError::IsDirectory);
    case ELOOP:
        IOX_LOG(ERROR) << "Unable to open/create file since too many symbolic links were encountered.";
        return err(FileCreationError::TooManySymbolicLinksEncountered);
    case EMFILE:
        IOX_LOG(ERROR) << "Unable to open/create file since the process limit of open file descriptors was reached.";
        return err(FileCreationError::ProcessLimitOfOpenFileDescriptorsReached);
    case ENFILE:
        IOX_LOG(ERROR) << "Unable to open/create file since the system limit of open file descriptors was reached.";
        return err(FileCreationError::SystemLimitOfOpenFileDescriptorsReached);
    case ENOENT:
        IOX_LOG(ERROR) << "Unable to open file since the file does not exist.";
        return err(FileCreationError::DoesNotExist);
    case ENOMEM:
        IOX_LOG(ERROR) << "Unable to open/create file due to insufficient memory.";
        return err(FileCreationError::InsufficientMemory);
    case EOVERFLOW:
        IOX_LOG(ERROR) << "Unable to open/create file since it is too large.";
        return err(FileCreationError::FileTooLarge);
    case ETXTBSY:
        IOX_LOG(ERROR) << "Unable to open/create file since it is currently in use.";
        return err(FileCreationError::CurrentlyInUse);
    case EEXIST:
        IOX_LOG(ERROR) << "Unable to create file since it already exists.";
        return err(FileCreationError::AlreadyExists);
    default:
        IOX_LOG(ERROR) << "Unable to open/create file since an unknown error occurred (" << result.error().errnum
                       << ").";
        return err(FileCreationError::UnknownError);
    }
}

File::File(const int file_descriptor, const posix::AccessMode access_mode) noexcept
    : m_file_descriptor{file_descriptor}
    , m_access_mode{access_mode}
{
}

File::File(File&& rhs) noexcept
    : m_file_descriptor{rhs.m_file_descriptor}
    , m_access_mode{rhs.m_access_mode}
{
    rhs.m_file_descriptor = INVALID_FILE_DESCRIPTOR;
}

File& File::operator=(File&& rhs) noexcept
{
    if (this != &rhs)
    {
        close_fd();
        m_file_descriptor = rhs.m_file_descriptor;
        m_access_mode = rhs.m_access_mode;
        rhs.m_file_descriptor = INVALID_FILE_DESCRIPTOR;
    }

    return *this;
}

File::~File() noexcept
{
    close_fd();
}

void File::close_fd() noexcept
{
    if (m_file_descriptor == INVALID_FILE_DESCRIPTOR)
    {
        return;
    }

    auto result = posix::posixCall(iox_close)(m_file_descriptor).failureReturnValue(-1).evaluate();
    m_file_descriptor = INVALID_FILE_DESCRIPTOR;

    if (!result.has_error())
    {
        return;
    }

    switch (result.error().errnum)
    {
    case EBADF:
        IOX_LOG(FATAL) << "This should never happen! Unable to close file since the file descriptor is invalid.";
        break;
    case EINTR:
        IOX_LOG(FATAL) << "This should never happen! Unable to close file since an interrupt signal was received.";
        break;
    case EIO:
        IOX_LOG(FATAL) << "This should never happen! Unable to close file due to an IO failure.";
        break;
    default:
        IOX_LOG(FATAL) << "This should never happen! Unable to close file due to an unknown error ("
                       << result.error().errnum << ").";
        break;
    }

    cxx::EnsuresWithMsg(false, "Unable to close file descriptor due to a corrupted file descriptor.");
}

expected<bool, FileAccessError> File::does_exist(const FilePath& file) noexcept
{
    auto result = posix::posixCall(iox_access)(file.as_string().c_str(), F_OK).failureReturnValue(-1).evaluate();

    if (!result.has_error())
    {
        return ok(true);
    }

    switch (result.error().errnum)
    {
    case EACCES:
        IOX_LOG(ERROR) << "Unable to determine if file exists due to insufficient permissions.";
        return err(FileAccessError::InsufficientPermissions);
    case ENOENT:
        return ok(false);
    case ELOOP:
        IOX_LOG(ERROR) << "Unable to determine if file exists due to too many symbolic links.";
        return err(FileAccessError::TooManySymbolicLinksEncountered);
    case EIO:
        IOX_LOG(ERROR) << "Unable to determine if file exists due to an IO failure.";
        return err(FileAccessError::IoFailure);
    case ENOMEM:
        IOX_LOG(ERROR) << "Unable to determine if file exists due insufficient kernel memory.";
        return err(FileAccessError::InsufficientKernelMemory);
    default:
        IOX_LOG(ERROR) << "Unable to determine if file exists since an unknown error occurred ("
                       << result.error().errnum << ").";
        return err(FileAccessError::UnknownError);
    }
}

expected<bool, FileRemoveError> File::remove(const FilePath& file) noexcept
{
    auto result = posix::posixCall(iox_remove)(file.as_string().c_str())
                      .failureReturnValue(-1)
                      .suppressErrorMessagesForErrnos(ENOENT)
                      .evaluate();

    if (!result.has_error())
    {
        return ok(true);
    }

    switch (result.error().errnum)
    {
    case ENOENT:
        return ok(false);
    case EPERM:
        IOX_FALLTHROUGH;
    case EACCES:
        IOX_LOG(ERROR) << "Unable to remove file due to insufficient permissions.";
        return err(FileRemoveError::PermissionDenied);
    case EBUSY:
        IOX_LOG(ERROR) << "Unable to remove file since it is currently in use.";
        return err(FileRemoveError::CurrentlyInUse);
    case EIO:
        IOX_LOG(ERROR) << "Unable to remove file due to an IO failure.";
        return err(FileRemoveError::IoFailure);
    case ELOOP:
        IOX_LOG(ERROR) << "Unable to remove file due to too many symbolic links.";
        return err(FileRemoveError::TooManySymbolicLinksEncountered);
    case ENOMEM:
        IOX_LOG(ERROR) << "Unable to remove file due to insufficient kernel memory.";
        return err(FileRemoveError::InsufficientKernelMemory);
    case EISDIR:
        IOX_LOG(ERROR) << "Unable to remove file since it is a directory.";
        return err(FileRemoveError::IsDirectory);
    case EROFS:
        IOX_LOG(ERROR) << "Unable to remove file since it resides on a read-only file system.";
        return err(FileRemoveError::ReadOnlyFilesystem);
    default:
        IOX_LOG(ERROR) << "Unable to remove file since an unknown error occurred (" << result.error().errnum << ").";
        return err(FileRemoveError::UnknownError);
    }
}

expected<void, FileOffsetError> File::set_offset(const uint64_t offset) const noexcept
{
    auto result = posix::posixCall(iox_lseek)(m_file_descriptor, static_cast<iox_off_t>(offset), IOX_SEEK_SET)
                      .failureReturnValue(-1)
                      .evaluate();

    if (!result.has_error())
    {
        if (result->value == static_cast<iox_off_t>(offset))
        {
            return ok();
        }

        IOX_LOG(ERROR) << "Unable to set file offset position since it set to the wrong offset position.";
        return err(FileOffsetError::OffsetAtWrongPosition);
    }


    switch (result.error().errnum)
    {
    case EINVAL:
        IOX_FALLTHROUGH;
    case ENXIO:
        IOX_LOG(ERROR) << "Unable to set file offset position since it is beyond the file limits.";
        return err(FileOffsetError::OffsetBeyondFileLimits);
    case EOVERFLOW:
        IOX_LOG(ERROR)
            << "Unable to set file offset position since the file is too large and the offset would overflow.";
        return err(FileOffsetError::FileOffsetOverflow);
    case ESPIPE:
        IOX_LOG(ERROR) << "Unable to set file offset position since seeking is not supported by the file type.";
        return err(FileOffsetError::SeekingNotSupportedByFileType);
    default:
        IOX_LOG(ERROR) << "Unable to remove file since an unknown error occurred (" << result.error().errnum << ").";
        return err(FileOffsetError::UnknownError);
    }
}

expected<uint64_t, FileReadError> File::read(uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    return read_at(0, buffer, buffer_len);
}

expected<uint64_t, FileReadError>
File::read_at(const uint64_t offset, uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    if (m_access_mode == posix::AccessMode::WRITE_ONLY)
    {
        IOX_LOG(ERROR) << "Unable to read from file since it is opened for writing only.";
        return err(FileReadError::NotOpenedForReading);
    }

    if (set_offset(offset).has_error())
    {
        IOX_LOG(ERROR) << "Unable to read from file since the offset could not be set.";
        return err(FileReadError::OffsetFailure);
    }

    auto result = posix::posixCall(iox_read)(m_file_descriptor, buffer, buffer_len).failureReturnValue(-1).evaluate();

    if (!result.has_error())
    {
        return ok(static_cast<uint64_t>(result->value));
    }

    switch (result.error().errnum)
    {
    case EAGAIN:
        IOX_LOG(ERROR) << "Unable to read from file since the operation would block.";
        return err(FileReadError::OperationWouldBlock);
    case EINTR:
        IOX_LOG(ERROR) << "Unable to read from file since an interrupt signal was received.";
        return err(FileReadError::Interrupt);
    case EINVAL:
        IOX_LOG(ERROR) << "Unable to read from file since it is unsuitable for reading.";
        return err(FileReadError::FileUnsuitableForReading);
    case EIO:
        IOX_LOG(ERROR) << "Unable to read from file since an IO failure occurred.";
        return err(FileReadError::IoFailure);
    case EISDIR:
        IOX_LOG(ERROR) << "Unable to read from file since it is a directory.";
        return err(FileReadError::IsDirectory);
    default:
        IOX_LOG(ERROR) << "Unable to read from file since an unknown error occurred (" << result.error().errnum << ").";
        return err(FileReadError::UnknownError);
    }
}

expected<uint64_t, FileWriteError> File::write(const uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    return write_at(0, buffer, buffer_len);
}

expected<uint64_t, FileWriteError>
File::write_at(const uint64_t offset, const uint8_t* const buffer, const uint64_t buffer_len) const noexcept
{
    if (m_access_mode == posix::AccessMode::READ_ONLY)
    {
        IOX_LOG(ERROR) << "Unable to write to file since it is opened for reading only.";
        return err(FileWriteError::NotOpenedForWriting);
    }

    if (set_offset(offset).has_error())
    {
        IOX_LOG(ERROR) << "Unable to write to file since the offset could not be set.";
        return err(FileWriteError::OffsetFailure);
    }

    auto result = posix::posixCall(iox_write)(m_file_descriptor, buffer, buffer_len).failureReturnValue(-1).evaluate();

    if (!result.has_error())
    {
        return ok(static_cast<uint64_t>(result->value));
    }

    switch (result.error().errnum)
    {
    case EAGAIN:
        IOX_LOG(ERROR) << "Unable to write to file since the operation would block.";
        return err(FileWriteError::OperationWouldBlock);
    case EDQUOT:
        IOX_LOG(ERROR) << "Unable to write to file since the users disk quota has been exhausted.";
        return err(FileWriteError::DiskQuotaExhausted);
    case EFBIG:
        IOX_LOG(ERROR) << "Unable to write to file since file size exceeds the maximum supported size.";
        return err(FileWriteError::FileSizeExceedsMaximumSupportedSize);
    case EINTR:
        IOX_LOG(ERROR) << "Unable to write to file since an interrupt signal occurred.";
        return err(FileWriteError::Interrupt);
    case EINVAL:
        IOX_LOG(ERROR) << "Unable to write to file since the file is unsuitable for writing.";
        return err(FileWriteError::FileUnsuitableForWriting);
    case ENOSPC:
        IOX_LOG(ERROR) << "Unable to write to file since there is no space left on target.";
        return err(FileWriteError::NoSpaceLeftOnDevice);
    case EPERM:
        IOX_LOG(ERROR) << "Unable to write to file since the operation was prevented by a file seal.";
        return err(FileWriteError::PreventedByFileSeal);
    case EIO:
        IOX_LOG(ERROR) << "Unable to write to file since an IO failure occurred.";
        return err(FileWriteError::IoFailure);
    default:
        IOX_LOG(ERROR) << "Unable to write to file since an unknown error has occurred (" << result.error().errnum
                       << ").";
        return err(FileWriteError::UnknownError);
    }
}

int File::get_file_handle() const noexcept
{
    return m_file_descriptor;
}
} // namespace iox
