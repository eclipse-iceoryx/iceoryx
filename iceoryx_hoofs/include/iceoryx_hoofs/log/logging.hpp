// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_LOG_LOGGING_HPP
#define IOX_HOOFS_LOG_LOGGING_HPP

#include "iceoryx_hoofs/log/logstream.hpp"
namespace iox
{
namespace log
{
namespace internal
{
/// @brief Convenience function for the IOX_LOG_INTERNAL macro
inline bool isLogLevelActive(LogLevel logLevel)
{
    return (logLevel) <= platform::MINIMAL_LOG_LEVEL
           && (platform::IGNORE_ACTIVE_LOG_LEVEL || (logLevel) <= Logger::getLogLevel());
}

/// @brief Only for internal usage
// NOLINTJUSTIFICATION cannot be realized with templates or constexpr functions due to the the source location intrinsic
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define IOX_LOG_INTERNAL(file, line, function, level)                                                                  \
    if (iox::log::internal::isLogLevelActive(level))                                                                   \
    iox::log::LogStream(file, line, function, level).self()

} // namespace internal

/// @brief Macro for logging
/// @param[in] level is the log level to be used for the log message
/// @code
///     IOX_LOG(INFO) << "Hello World";
/// @endcode
// NOLINTJUSTIFICATION __PRETTY_FUNCTION__ would work better in lambdas but especially with templates the resulting
// string is too large; we also get the file name and the line of the invocation which should be sufficient for
// debugging
// NOLINTBEGIN(bugprone-lambda-function-name)
// NOLINTJUSTIFICATION needed for source code location, safely wrapped in macro
// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay,-warnings-as-errors)
#define IOX_LOG(level) IOX_LOG_INTERNAL(__FILE__, __LINE__, __FUNCTION__, iox::log::LogLevel::level)
// NOLINTEND(bugprone-lambda-function-name)

// NOLINTEND(cppcoreguidelines-macro-usage)

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_LOG_LOGGING_HPP
