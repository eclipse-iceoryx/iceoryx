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

#ifndef IOX_HOOFS_POSIX_FILESYSTEM_FILE_LOCK_HPP
#define IOX_HOOFS_POSIX_FILESYSTEM_FILE_LOCK_HPP

#include "iceoryx_platform/file.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/filesystem.hpp"
#include "iox/string.hpp"

namespace iox
{
enum class FileLockError : uint8_t
{
    INVALID_FILE_NAME,
    INVALID_PATH,
    LOCKED_BY_OTHER_PROCESS,
    ACCESS_DENIED,
    QUOTA_EXHAUSTED,
    SYSTEM_LIMIT,
    PROCESS_LIMIT,
    NO_SUCH_DIRECTORY,
    SPECIAL_FILE,
    FILE_TOO_LARGE,
    FILE_IN_USE,
    OUT_OF_MEMORY,
    I_O_ERROR,
    SYS_CALL_NOT_IMPLEMENTED,
    INTERNAL_LOGIC_ERROR,
};

/// @brief Posix file lock C++ wrapping class
///        Following RAII, the lock is acquired on creation and released on destruction. Releasing the locks works even
///        if the process crashes with a segfault or using SIGKILL. 'lslocks' can be used to display all system-wide
///        locks (see man page)
/// @code
///   auto fileLock = iox::FileLockBuilder().name("myLockName")
///                                         .path("/tmp")
///                                         .permission(iox::perms::owner_all)
///                                         .create()
///                                         .expect("Oh no I couldn't create the lock");
/// @endcode
class FileLock
{
  public:
    static constexpr int32_t INVALID_FD = -1;
    // NOLINTJUSTIFICATION Required as a safe and null terminated compile time string literal
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    static constexpr const char LOCK_FILE_SUFFIX[] = ".lock";
    static constexpr uint64_t PATH_SEPARATOR_LENGTH = 1U;
    static constexpr uint64_t LOCK_FILE_SUFFIX_LENGTH = sizeof(LOCK_FILE_SUFFIX) / sizeof(char);

    static constexpr uint64_t FILENAME_LENGTH = platform::IOX_MAX_FILENAME_LENGTH - LOCK_FILE_SUFFIX_LENGTH;

    /// @brief A file name without any containing slash (path separator). Can be used by the user
    ///        and the LOCK_FILE_SUFFIX will be appended later.
    ///        For instance "myLock" (without .lock)
    using FileName_t = string<FILENAME_LENGTH>;
    /// @brief The full path to the file, including the file
    ///        For instance "/path/to/myLock.lock"
    using FilePath_t = string<platform::IOX_MAX_PATH_LENGTH>;
    /// @brief The directory to the file
    ///        For instance "/path/to/"
    using PathName_t =
        string<platform::IOX_MAX_PATH_LENGTH - PATH_SEPARATOR_LENGTH - (FILENAME_LENGTH + LOCK_FILE_SUFFIX_LENGTH)>;

    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    FileLock(FileLock&& rhs) noexcept;
    FileLock& operator=(FileLock&& rhs) noexcept;

    ~FileLock() noexcept;

  private:
    // NOLINTNEXTLINE(performance-enum-size) int32_t required for values from from file.h
    enum class LockOperation : int32_t
    {
        // NOLINTJUSTIFICATION abstracted in enum as value so that the user does not have to use bit operations
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        LOCK = LOCK_EX | LOCK_NB,
        UNLOCK = LOCK_UN
    };

    int32_t m_fd{INVALID_FD};
    FilePath_t m_fileLockPath;

  private:
    friend class FileLockBuilder;
    FileLock(const int32_t fileDescriptor, const FilePath_t& path) noexcept;

    void invalidate() noexcept;

    static FileLockError convertErrnoToFileLockError(const int32_t errnum, const FilePath_t& fileLockPath) noexcept;
    expected<void, FileLockError> closeFileDescriptor() noexcept;
};

class FileLockBuilder
{
    /// @brief Defines the file name of the lock, the suffix ".lock" will be
    ///        appended to the filename
    IOX_BUILDER_PARAMETER(FileLock::FileName_t, name, "")

    /// @brief Defines the path where the lock is stored. Uses the file lock path from the
    ///        corresponding platform as default.
    IOX_BUILDER_PARAMETER(FileLock::PathName_t, path, platform::IOX_LOCK_FILE_PATH_PREFIX)

    /// @brief Defines the access permissions of the file lock. If they are not
    ///        explicitly set they will be none
    IOX_BUILDER_PARAMETER(access_rights, permission, perms::none)

  public:
    /// @brief Creates a file lock
    /// @return a valid file lock or an FileLockError describing the error
    expected<FileLock, FileLockError> create() noexcept;
};
} // namespace iox

#endif // IOX_HOOFS_POSIX_FILESYSTEM_FILE_LOCK_HPP
