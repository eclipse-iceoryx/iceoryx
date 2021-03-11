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
#ifndef IOX_UTILS_POSIX_WRAPPER_FILE_LOCK_HPP
#define IOX_UTILS_POSIX_WRAPPER_FILE_LOCK_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/design_pattern/creation.hpp"

#include <cstring>

namespace iox
{
namespace posix
{
enum class FileLockError
{
    INVALID_STATE,
    LOCKED_BY_OTHER_PROCESS,
    ACCESS_DENIED,
    INVALID_FILE_NAME,
    FILE_EXISTS,
    INTERRUPTED_BY_SIGNAL,
    QUOTA_EXHAUSTED,
    INVALID_ARGUMENTS,
    SYSTEM_LIMIT,
    PROCESS_LIMIT,
    NO_SUCH_FILE,
    SPECIAL_FILE,
    TEMP_FILE_NOT_SUPPORTED,
    FILE_TOO_LARGE,
    FILE_IN_USE,
    INVALID_FILE_DESCRIPTOR,
    OUT_OF_MEMORY,
    INTERNAL_LOGIC_ERROR,
};

constexpr char PATH_PREFIX[] = "/var/lock/";

/// @brief Posix file lock C++ Wrapping class
/// @code
///     auto lock = posix::FileLock::Create();
/// @endcode
class FileLock : public DesignPattern::Creation<FileLock, FileLockError>
{
  public:
    static constexpr int32_t ERROR_CODE = -1;
    static constexpr int32_t INVALID_FD = -1;
    using FileName_t = cxx::string<100>;

    /// @brief
    FileLock(FileName_t name) noexcept;

    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    FileLock(FileLock&& rhs) noexcept;
    FileLock& operator=(FileLock&& rhs) noexcept;

    ~FileLock() noexcept;

  private:
    int32_t m_fd{INVALID_FD};
    FileName_t m_name{""};

    friend class DesignPattern::Creation<FileLock, FileLockError>;

    FileLock() noexcept;
    cxx::expected<FileLockError> initializeFileLock() noexcept;
    cxx::error<FileLockError> createErrorFromErrnum(const int32_t errnum) const noexcept;
    cxx::expected<FileLockError> closeFileDescriptor() noexcept;
    cxx::expected<FileLockError> destroy() noexcept;

    FileLockError errnoToEnum(const int errnoValue) const noexcept;
};
} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_FILE_LOCK_HPP
