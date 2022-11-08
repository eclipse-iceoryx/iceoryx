// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGGER_HPP
#define IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGGER_HPP

#include "iceoryx_hoofs/log/platform_building_blocks/logcommon.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>

namespace iox
{
namespace log
{
class LogStream;
} // namespace log

namespace pbb
{
template <uint32_t N>
// NOLINTJUSTIFICATION required for C-style string comparison; safety guaranteed by strncmp
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
bool equalStrings(const char* lhs, const char (&rhs)[N]) noexcept;

/// @brief Tries to get the log level from the 'IOX_LOG_LEVEL' env variable or uses the specified one if the env
/// variable is not set
/// @param[in] logLevel is the log level to be used when the env variable is not set
/// @note The function uses 'getenv' which is not thread safe and can result in undefined behavior when it is called
/// from multiple threads or the env variable is changed while the function holds a pointer to the data. For this reason
/// the function should only be used in the startup phase of the application and only in the main thread.
LogLevel logLevelFromEnvOr(const LogLevel logLevel) noexcept;

template <typename BaseLogger>
class Logger : public BaseLogger
{
  public:
    friend class log::LogStream;

    Logger() = default;

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    ~Logger() = default;

    static Logger& get() noexcept;

    static void init(const LogLevel logLevel = logLevelFromEnvOr(LogLevel::INFO)) noexcept;

    static void setActiveLogger(Logger& newLogger) noexcept;

  private:
    static Logger& activeLogger(Logger* newLogger = nullptr) noexcept;

    void initLoggerInternal(const LogLevel logLevel) noexcept;

  private:
    std::atomic<bool> m_isActive{true};
    std::atomic<bool> m_isFinalized{false};
};

} // namespace pbb
} // namespace iox

#include "iceoryx_hoofs/internal/log/platform_building_blocks/logger.inl"

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGGER_HPP
