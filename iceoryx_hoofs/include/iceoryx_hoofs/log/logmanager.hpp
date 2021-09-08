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
#ifndef IOX_HOOFS_LOG_LOGMANAGER_HPP
#define IOX_HOOFS_LOG_LOGMANAGER_HPP

#include "iceoryx_hoofs/log/logcommon.hpp"
#include "iceoryx_hoofs/log/logger.hpp"

#include <atomic>
#include <map>
#include <string>

namespace iox
{
namespace log
{
enum class LogLevelOutput : uint8_t
{
    kDisplayLogLevel = 0,
    kHideLogLevel
};

class LogManager
{
  public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static LogManager& GetLogManager() noexcept;
    static Logger&
    // NOLINTNEXTLINE(readability-identifier-naming)
    CreateLogContext(const std::string& ctxId,
                     const std::string& ctxDescription,
                     const LogLevel appDefLogLevel) noexcept;

    ~LogManager() noexcept = default;

    LogManager(const LogManager&) = delete;
    LogManager(LogManager&&) = delete;
    LogManager& operator=(const LogManager&) = delete;
    LogManager& operator=(LogManager&&) = delete;

    // NOLINTNEXTLINE(readability-identifier-naming)
    LogLevel DefaultLogLevel() const noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    void SetDefaultLogLevel(const LogLevel logLevel,
                            const LogLevelOutput logLevelOutput = LogLevelOutput::kDisplayLogLevel) noexcept;

    // NOLINTNEXTLINE(readability-identifier-naming)
    LogMode DefaultLogMode() const noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    void SetDefaultLogMode(const LogMode logMode) noexcept;

  protected:
    LogManager() noexcept = default;

  private:
    std::atomic<LogLevel> m_defaultLogLevel{LogLevel::kVerbose};
    std::atomic<LogMode> m_defaultLogMode{LogMode::kConsole};

    std::map<std::string, Logger> m_loggers;
};

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_LOG_LOGMANAGER_HPP
