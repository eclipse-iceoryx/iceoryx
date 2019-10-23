// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

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

LogMode operator|(LogMode lhs, LogMode rhs);
LogMode& operator|=(LogMode& lhs, LogMode rhs);
LogMode operator&(LogMode lhs, LogMode rhs);
LogMode& operator&=(LogMode& lhs, LogMode rhs);

struct LogEntry
{
    LogLevel level{LogLevel::kVerbose};
    std::chrono::milliseconds time{0};
    std::string message;
};

} // namespace log
} // namespace iox

