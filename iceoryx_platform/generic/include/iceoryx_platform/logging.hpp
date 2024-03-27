// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_PLATFORM_LOGGING_HPP
#define IOX_PLATFORM_LOGGING_HPP

enum IceoryxPlatformLogLevel
{
    IOX_PLATFORM_LOG_LEVEL_OFF = 0,
    IOX_PLATFORM_LOG_LEVEL_FATAL,
    IOX_PLATFORM_LOG_LEVEL_ERROR,
    IOX_PLATFORM_LOG_LEVEL_WARN,
    IOX_PLATFORM_LOG_LEVEL_INFO,
    IOX_PLATFORM_LOG_LEVEL_DEBUG,
    IOX_PLATFORM_LOG_LEVEL_TRACE
};

/// @brief Typedef to the platform log backend
/// @param[in] file should be the '__FILE__' compiler intrinsic
/// @param[in] line should be the '__LINE__' compiler intrinsic
/// @param[in] function should be the '__FUNCTION__' compiler intrinsic
/// @param[in] log_level is the log level to be used for the log message
/// @param[in] msg is the message to be logged
typedef void (*IceoryxPlatformLogBackend)(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg);

/// @brief Sets the logging backend to the provided function
/// @param[in] log_backend to be used
/// @note The log_backend must have a static lifetime and must be thread-safe
void iox_platform_set_log_backend(IceoryxPlatformLogBackend log_backend);

/// @note Implementation detail! Do not use directly
void iox_platform_detail_log(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg);

/// @note Implementation detail! Do not use directly
void iox_platform_detail_default_log_backend(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg);

// NOLINTJUSTIFICATION The functionality of the macro (obtaining the source location) cannot be achieved with C++17
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @brief Frontend for logging in the iceoryx platform
/// @param[in] log_level is the log level to be used for the log message
/// @param[in] msg is the message to be logged
#define IOX_PLATFORM_LOG(log_level, msg)                                                                               \
    iox_platform_detail_log(                                                                                           \
        __FILE__, __LINE__, static_cast<const char*>(__FUNCTION__), IceoryxPlatformLogLevel::log_level, msg)
// NOLINTEND(cppcoreguidelines-macro-usage)

#endif // IOX_PLATFORM_LOGGING_HPP
