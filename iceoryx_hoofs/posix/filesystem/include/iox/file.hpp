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
/// @brief Describes failures which can occur when a file is opened or created.
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
    CannotBePurged,
    UnknownError
};

/// @brief Describes failures which can occur when a file is read.
enum class FileReadError
{
    OffsetFailure,
    Interrupt,
    FileUnsuitableForReading,
    IoFailure,
    OperationWouldBlock,
    IsDirectory,
    NotOpenedForReading,
    UnknownError
};

/// @brief Describes failures which can occur when a file is written to.
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
    NotOpenedForWriting,
    UnknownError
};

/// @brief Describes failures which can occur when the files metadata is accessed.
enum class FileAccessError
{
    InsufficientPermissions,
    TooManySymbolicLinksEncountered,
    IoFailure,
    InsufficientKernelMemory,
    UnknownError
};

/// @brief Describes failures which can occur when a file is removed.
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

/// @brief Describes failures which can occur when a file offset is changed.
enum class FileOffsetError
{
    FileOffsetOverflow,
    OffsetBeyondFileLimits,
    SeekingNotSupportedByFileType,
    OffsetAtWrongPosition,
    UnknownError,
};

/// @brief Represents a file. It supports various read and write functionalities
///        and can verify the existance of a file as well as remove existing files.
class File : public FileManagementInterface<File>
{
  public:
    File(const File&) noexcept = delete;
    File& operator=(const File&) noexcept = delete;

    File(File&& rhs) noexcept;
    File& operator=(File&& rhs) noexcept;
    ~File() noexcept;

    /// @brief Returns the underlying native file handle.
    int get_file_handle() const noexcept;

    /// @brief Reads the contents of the file and writes it into the provided buffer.
    /// @param[in] buffer pointer to the memory.
    /// @param[in] buffer_len the storage size of the provided memory.
    /// @returns The amount of bytes read, at most buffer_len.
    expected<uint64_t, FileReadError> read(uint8_t* const buffer, const uint64_t buffer_len) const noexcept;

    /// @brief Reads the contents of the file from a given offset and writes it into the provided buffer.
    ///        If the offset is out of bounds it will read nothing.
    /// @param[in] offset starting point from which the reading shall begin.
    /// @param[in] buffer pointer to the memory.
    /// @param[in] buffer_len the storage size of the provided memory.
    /// @returns The amount of bytes read, at most buffer_len.
    expected<uint64_t, FileReadError>
    read_at(const uint64_t offset, uint8_t* const buffer, const uint64_t buffer_len) const noexcept;

    /// @brief Writes the provided buffer into the file.
    /// @param[in] buffer pointer to the memory which shall be stored inside the file.
    /// @param[in] buffer_len length of the memory.
    /// @returns The amount of bytes written, at most buffer_len.
    expected<uint64_t, FileWriteError> write(const uint8_t* const buffer, const uint64_t buffer_len) const noexcept;

    /// @brief Writes the provided buffer into the file starting from the given offset. If the offset
    ///        is out of bounds the file will be filled with zeroes until the offset.
    /// @param[in] offset starting point from which the writing shall begin.
    /// @param[in] buffer pointer to the memory which shall be stored inside the file.
    /// @param[in] buffer_len length of the memory.
    /// @returns The amount of bytes written, at most buffer_len.
    expected<uint64_t, FileWriteError>
    write_at(const uint64_t offset, const uint8_t* const buffer, const uint64_t buffer_len) const noexcept;

    /// @brief Returns true if the provided file exists, otherwise false.
    /// @param[in] file the path to the file
    static expected<bool, FileAccessError> does_exist(const FilePath& file) noexcept;

    /// @brief Removes an existing file.
    /// @param[in] file the path to the file which shall be removed
    /// @returns true if the file did exist and was removed, otherwise false
    static expected<bool, FileRemoveError> remove(const FilePath& file) noexcept;

  private:
    friend class FileBuilder;
    explicit File(const int file_descriptor, const posix::AccessMode access_mode) noexcept;
    void close_fd() noexcept;

    expected<void, FileOffsetError> set_offset(const uint64_t offset) const noexcept;

  private:
    static constexpr int INVALID_FILE_DESCRIPTOR{-1};

    int m_file_descriptor{INVALID_FILE_DESCRIPTOR};
    posix::AccessMode m_access_mode{posix::AccessMode::READ_ONLY};
};

class FileBuilder
{
  private:
    using AccessMode = posix::AccessMode;
    using OpenMode = posix::OpenMode;

    IOX_BUILDER_PARAMETER(Ownership, owner, Ownership::from_process())
    IOX_BUILDER_PARAMETER(access_rights, permissions, perms::owner_read)
    IOX_BUILDER_PARAMETER(AccessMode, access_mode, AccessMode::READ_ONLY)
    IOX_BUILDER_PARAMETER(OpenMode, open_mode, OpenMode::OPEN_EXISTING)

  public:
    expected<File, FileCreationError> create(const FilePath& name) noexcept;
    expected<File, FileCreationError> open(const FilePath& name) noexcept;
};
} // namespace iox

#endif
