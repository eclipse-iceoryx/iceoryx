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

#include "iox/logging.hpp"

namespace iox::log::internal
{
// NOLINTJUSTIFICATION Not used directly but as a function pointer to set the backend
// NOLINTNEXTLINE(readability-function-size)
void platform_log_backend(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg)
{
    auto level = LogLevel::Trace;
    switch (log_level)
    {
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_OFF:
        level = LogLevel::Off;
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_FATAL:
        level = LogLevel::Fatal;
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_ERROR:
        level = LogLevel::Error;
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_WARN:
        level = LogLevel::Warn;
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_INFO:
        level = LogLevel::Info;
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_DEBUG:
        level = LogLevel::Debug;
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_TRACE:
        level = LogLevel::Trace;
        break;
    default:
        level = LogLevel::Trace;
    }
    IOX_LOG_INTERNAL(file, line, function, level, msg);
}
} // namespace iox::log::internal
