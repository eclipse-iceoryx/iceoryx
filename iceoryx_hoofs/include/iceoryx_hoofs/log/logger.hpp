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
#ifndef IOX_HOOFS_LOG_LOGGER_HPP
#define IOX_HOOFS_LOG_LOGGER_HPP

#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/log/logcommon.hpp"
#include "iceoryx_hoofs/log/logstream.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <string>

namespace iox
{
namespace log
{
/// @todo for asynchronous logging, make the logger an active object according to Herb Sutter
/// https://herbsutter.com/2010/07/12/effective-concurrency-prefer-using-active-objects-instead-of-naked-threads/

class Logger
{
    friend class LogManager;
    /// @todo LogStream needs to call Log(); do we want to make Log() public?
    friend class LogStream;

  public:
    Logger(Logger&& other) noexcept;
    Logger& operator=(Logger&& rhs) noexcept;

    Logger(const Logger& other) = delete;
    Logger& operator=(const Logger& rhs) = delete;

    /// @brief Getter method for the current LogLevel
    /// @return the current LogLevel
    // NOLINTNEXTLINE(readability-identifier-naming)
    LogLevel GetLogLevel() const noexcept;

    /// @brief Sets the LogLevel for the Logger
    /// @param[in] logLevel to be set
    // NOLINTNEXTLINE(readability-identifier-naming)
    void SetLogLevel(const LogLevel logLevel) noexcept;

    /// @brief Sets the LogLevel to the given level for the lifetime of the GenericRAII object and then sets it back to
    /// the previous one
    /// @param[in] logLevel to be set temporarily
    /// @return a scope guard which resets the LogLevel to the value at the time when this method was called
    // NOLINTNEXTLINE(readability-identifier-naming)
    cxx::GenericRAII SetLogLevelForScope(const LogLevel logLevel) noexcept;

    // NOLINTNEXTLINE(readability-identifier-naming)
    void SetLogMode(const LogMode logMode) noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    bool IsEnabled(const LogLevel logLevel) const noexcept;

    // NOLINTNEXTLINE(readability-identifier-naming)
    LogStream LogFatal() noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    LogStream LogError() noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    LogStream LogWarn() noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    LogStream LogInfo() noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    LogStream LogDebug() noexcept;
    // NOLINTNEXTLINE(readability-identifier-naming)
    LogStream LogVerbose() noexcept;

  protected:
    Logger(const std::string& ctxId, const std::string& ctxDescription, const LogLevel appLogLevel) noexcept;

    // virtual because of Logger_Mock
    // NOLINTNEXTLINE(readability-identifier-naming)
    virtual void Log(const LogEntry& entry) const noexcept;

  private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    static void Print(const LogEntry& entry) noexcept;

    std::atomic<LogLevel> m_logLevel{LogLevel::kVerbose};
    std::atomic<LogLevel> m_logLevelPredecessor{LogLevel::kVerbose};
    std::atomic<LogMode> m_logMode{LogMode::kConsole};
};

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_LOG_LOGGER_HPP
