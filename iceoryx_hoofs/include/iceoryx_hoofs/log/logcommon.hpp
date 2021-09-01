// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_LOG_LOGCOMMON_HPP
#define IOX_HOOFS_LOG_LOGCOMMON_HPP

#include <chrono>
#include <string>

namespace iox
{
namespace log
{
enum class LogLevel : uint8_t
{
    kOff = 0,
    kFatal,
    kError,
    kWarn,
    kInfo,
    kDebug,
    kVerbose
};

enum class LogMode : uint8_t
{
    kRemote = 0x01,
    kFile = 0x02,
    kConsole = 0x04
};

constexpr const char* LogLevelColor[] = {
    "",                 // nothing
    "\033[0;1;97;41m",  // bold bright white on red
    "\033[0;1;31;103m", // bold red on light yellow
    "\033[0;1;93m",     // bold bright yellow
    "\033[0;1;92m",     // bold bright green
    "\033[0;1;96m",     // bold bright cyan
    "\033[0;1;36m",     // bold cyan
};

constexpr const char* LogLevelText[] = {
    "[  Off  ]", // nothing
    "[ Fatal ]", // bold bright white on red
    "[ Error ]", // bold red on light yellow
    "[Warning]", // bold bright yellow
    "[ Info  ]", // bold bright green
    "[ Debug ]", // bold bright cyan
    "[Verbose]", // bold cyan
};

LogMode operator|(LogMode lhs, LogMode rhs) noexcept;
LogMode& operator|=(LogMode& lhs, LogMode rhs) noexcept;
LogMode operator&(LogMode lhs, LogMode rhs) noexcept;
LogMode& operator&=(LogMode& lhs, LogMode rhs) noexcept;

struct LogEntry
{
    LogLevel level{LogLevel::kVerbose};
    std::chrono::milliseconds time{0};
    std::string message;
};

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_LOG_LOGCOMMON_HPP
