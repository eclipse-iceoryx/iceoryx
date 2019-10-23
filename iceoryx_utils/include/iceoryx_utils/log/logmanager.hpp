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

#include "iceoryx_utils/log/logcommon.hpp"
#include "iceoryx_utils/log/logger.hpp"

#include <atomic>
#include <map>
#include <string>

namespace iox
{
namespace log
{
class LogManager
{
  public:
    static LogManager& GetLogManager();
    static Logger& CreateLogContext(std::string ctxId, std::string ctxDescription, LogLevel appDefLogLevel) noexcept;

    ~LogManager() = default;

    LogManager(const LogManager&) = delete;
    LogManager(LogManager&&) = delete;
    LogManager& operator=(const LogManager&) = delete;
    LogManager& operator=(LogManager&&) = delete;

    LogLevel DefaultLogLevel() const noexcept;
    void SetDefaultLogLevel(const LogLevel logLevel) noexcept;

    LogMode DefaultLogMode() const noexcept;
    void SetDefaultLogMode(const LogMode logMode) noexcept;

  protected:
    LogManager() = default;

  private:
    std::atomic<LogLevel> m_defaultLogLevel{LogLevel::kVerbose};
    std::atomic<LogMode> m_defaultLogMode{LogMode::kConsole};

    std::map<std::string, Logger> m_loggers;
};

} // namespace log
} // namespace iox

