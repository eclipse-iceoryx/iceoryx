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
    NAME_TOO_LONG,
    UNABLE_TO_OPEN_HANDLE,
    UNDEFINED
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
    using FileName_t = cxx::string<100>;

    /// @brief
    FileLock(FileName_t name) noexcept;
    
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    FileLock(FileLock&& rhs) noexcept = default;
    FileLock& operator=(FileLock&& rhs) noexcept = default;

    ~FileLock() noexcept;

  private:
    int32_t m_fd;

  private:
    friend class DesignPattern::Creation<FileLock, FileLockError>;

    /// @brief
    /// @param[in]
    FileLock() noexcept;

    FileLockError errnoToEnum(const int errnoValue) const noexcept;
};
} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_FILE_LOCK_HPP
