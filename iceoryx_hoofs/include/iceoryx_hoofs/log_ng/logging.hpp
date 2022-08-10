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

#include "iceoryx_hoofs/log_ng/logstream.hpp"
namespace iox
{
namespace log
{
/// @brief Only for internal usage
// NOLINTJUSTIFICATION cannot be realized with templates or constexpr functions due to the the source location intrinsic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define IOX_LOG_INTERNAL(file, line, function, level)                                                                  \
    if ((level) <= iox::log::Logger::minimalLogLevel()                                                                 \
        && (iox::log::Logger::ignoreLogLevel() || (level) <= iox::log::Logger::getLogLevel()                           \
            || iox::log::custom(file, function)))                                                                      \
    iox::log::LogStream(file, line, function, level).self()

/// @brief Macro for logging
/// @param[in] level is the log level to be used for the log message
/// @code
///     IOX_LOG(INFO) << "Hello World";
/// @endcode
// NOLINTJUSTIFICATION cannot be realized with templates or constexpr functions due to the the source location intrinsic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define IOX_LOG(level) IOX_LOG_INTERNAL(__FILE__, __LINE__, __FUNCTION__, iox::log::LogLevel::level)

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_LOG_LOGGING_HPP
