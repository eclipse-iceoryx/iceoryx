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
#ifndef IOX_HOOFS_POSIX_WRAPPER_FILE_LOCK_HPP
#define IOX_HOOFS_POSIX_WRAPPER_FILE_LOCK_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/design_pattern/creation.hpp"

namespace iox
{
namespace posix
{
enum class FileLockError
{
    INVALID_FILE_NAME,
    LOCKED_BY_OTHER_PROCESS,
    ACCESS_DENIED,
    QUOTA_EXHAUSTED,
    INVALID_CHARACTERS_IN_FILE_NAME,
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
///    iox::posix::FileLock::create(nameOfmyLock)
///        .and_then([] { std::cout << "We aquired the lock!" << std::endl; })
///        .or_else([](auto& error) {
///            if (error == FileLockError::LOCKED_BY_OTHER_PROCESS)
///            {
///                std::cout << "Some other process is running and holds the lock!" << std::endl;
///            }
///        });
/// @endcode
class FileLock : public DesignPattern::Creation<FileLock, FileLockError>
{
  public:
    static constexpr int32_t ERROR_CODE = -1;
    static constexpr int32_t INVALID_FD = -1;
    static constexpr const char LOCK_FILE_SUFFIX[] = ".lock";
    static constexpr uint64_t FILENAME_LENGTH = platform::IOX_MAX_FILENAME_LENGTH
                                                - sizeof(platform::IOX_LOCK_FILE_PATH_PREFIX) / sizeof(char)
                                                - sizeof(LOCK_FILE_SUFFIX) / sizeof(char);

    using FileName_t = cxx::string<FILENAME_LENGTH>;
    using PathName_t = cxx::string<platform::IOX_MAX_PATH_LENGTH>;

    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    FileLock(FileLock&& rhs) noexcept;
    FileLock& operator=(FileLock&& rhs) noexcept;

    ~FileLock() noexcept;

  private:
    int32_t m_fd{INVALID_FD};
    FileName_t m_name;
    PathName_t m_fileLockPath;

    /// @brief c'tor
    /// @param[in] name of the created file lock in PATH_PREFIX
    explicit FileLock(const FileName_t& name) noexcept;

    void invalidate() noexcept;

    cxx::expected<FileLockError> initializeFileLock() noexcept;
    FileLockError convertErrnoToFileLockError(const int32_t errnum) const noexcept;
    cxx::expected<FileLockError> closeFileDescriptor() noexcept;

    friend class DesignPattern::Creation<FileLock, FileLockError>;
};
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_FILE_LOCK_HPP
