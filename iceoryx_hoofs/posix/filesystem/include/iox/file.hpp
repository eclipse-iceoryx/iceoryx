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

#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iox/builder.hpp"
#include "iox/file_management_interface.hpp"
#include "iox/file_path.hpp"
#include "iox/filesystem.hpp"

#ifndef IOX_HOOFS_POSIX_FILESYSTEM_FILE_HPP
#define IOX_HOOFS_POSIX_FILESYSTEM_FILE_HPP

namespace iox
{
using AccessMode = posix::AccessMode;

enum class FileCreationError
{
    PermissionDenied,
    Interrupt,
    IsDirectory,
    TooManySymbolicLinksEncountered,
    ProcessLimitOfOpenFileDescriptorsReached,
    SystemLimitOfOpenFileDescriptorsReached,
    DoesNotExist,
    AlreadyExists,
    InsufficientMemory,
    FileTooLarge,
    CurrentlyInUse,
    UnknownError
};

enum class FileReadError
{
    OffsetFailure,
    Interrupt,
    FileUnsuitableForReading,
    IoFailure,
    OperationWouldBlock,
    IsDirectory,
    UnknownError
};

enum class FileWriteError
{
    OffsetFailure,
    OperationWouldBlock,
    DiskQuotaExhausted,
    FileSizeExceedsMaximumSupportedSize,
    Interrupt,
    FileUnsuitableForWriting,
    IoFailure,
    NoSpaceLeftOnDevice,
    PreventedByFileSeal,
    UnknownError
};

enum class FileAccessError
{
    InsufficientPermissions,
    TooManySymbolicLinksEncountered,
    IoFailure,
    InsufficientKernelMemory,
    UnknownError
};

enum class FileRemoveError
{
    PermissionDenied,
    CurrentlyInUse,
    IoFailure,
    TooManySymbolicLinksEncountered,
    InsufficientKernelMemory,
    IsDirectory,
    ReadOnlyFilesystem,
    UnknownError
};

enum class FileOffsetError
{
    FileOffsetOverflow,
    OffsetBeyondFileLimits,
    SeekingNotSupportedByFileType,
    OffsetAtWrongPosition,
    UnknownError,
};

class File : public FileManagementInterface<File>
{
  public:
    int get_file_handle() const noexcept;

    expected<uint64_t, FileReadError> read(uint8_t* const buffer, const uint64_t buffer_len) const noexcept;
    expected<uint64_t, FileReadError>
    read_at(const uint64_t offset, uint8_t* const buffer, const uint64_t buffer_len) const noexcept;

    expected<uint64_t, FileWriteError> write(uint8_t* const buffer, const uint64_t buffer_len) const noexcept;
    expected<uint64_t, FileWriteError>
    write_at(const uint64_t offset, uint8_t* const buffer, const uint64_t buffer_len) const noexcept;

    static expected<bool, FileAccessError> does_exist(const FilePath& file) noexcept;
    static expected<bool, FileRemoveError> remove(const FilePath& file) noexcept;

  private:
    friend class FileBuilder;
    explicit File(const int file_descriptor) noexcept;

    expected<FileOffsetError> set_offset(const uint64_t offset) const noexcept;

  private:
    int m_file_descriptor{0};
};

class FileBuilder
{
    IOX_BUILDER_PARAMETER(FilePath, name, FilePath::create("").value())
    IOX_BUILDER_PARAMETER(Ownership, owner, Ownership::from_process())
    IOX_BUILDER_PARAMETER(access_rights, permissions, perms::owner_read)
    IOX_BUILDER_PARAMETER(AccessMode, access_mode, AccessMode::READ_ONLY)

  public:
    expected<File, FileCreationError> create(const posix::OpenMode open_mode) noexcept;
    expected<File, FileCreationError> open() noexcept;

  private:
    expected<File, FileCreationError> open_impl(const bool print_error_on_non_existing_file,
                                                const posix::OpenMode open_mode) noexcept;
};
} // namespace iox

#endif
