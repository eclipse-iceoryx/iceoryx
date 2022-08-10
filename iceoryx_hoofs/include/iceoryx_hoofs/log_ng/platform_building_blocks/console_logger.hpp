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

#ifndef IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_HPP
#define IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_HPP

#include "iceoryx_hoofs/log_ng/platform_building_blocks/logcommon.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>

namespace iox
{
namespace pbb
{
class ConsoleLogger
{
  public:
    static LogLevel getLogLevel() noexcept;

    void setLogLevel(const LogLevel logLevel) noexcept;

    virtual ~ConsoleLogger() = default;

    ConsoleLogger(const ConsoleLogger&) = delete;
    ConsoleLogger(ConsoleLogger&&) = delete;

    ConsoleLogger& operator=(const ConsoleLogger&) = delete;
    ConsoleLogger& operator=(ConsoleLogger&&) = delete;

  protected:
    ConsoleLogger() noexcept = default;

    virtual void initLogger(const LogLevel) noexcept;

    virtual void
    createLogMessageHeader(const char* file, const int line, const char* function, LogLevel logLevel) noexcept;

    virtual void flush() noexcept;

    LogBuffer getLogBuffer() const noexcept;

    void assumeFlushed() noexcept;

    void logString(const char* message) noexcept;

    // TODO add addBool(const bool), ...
    void logI64Dec(const int64_t value) noexcept;
    void logU64Dec(const uint64_t value) noexcept;
    void logU64Hex(const uint64_t value) noexcept;
    void logU64Oct(const uint64_t value) noexcept;

  private:
    template <uint32_t N>
    // NOLINTJUSTIFICATION safe access is guaranteed since the char array is not accessed but only the size is obtained
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    inline static constexpr uint32_t bufferSize(const char (&)[N]) noexcept;

    template <typename T>
    inline void unused(T&&) const noexcept;

    template <typename T>
    inline void logArithmetik(const T value, const char* format) noexcept;

  private:
    static constexpr uint32_t BUFFER_SIZE{1024}; // TODO compile time option?
    static constexpr uint32_t NULL_TERMINATED_BUFFER_SIZE{BUFFER_SIZE + 1};

    // NOLINTJUSTIFICATION needed for the functionality and a private member of the class
    // NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
    static std::atomic<LogLevel> m_activeLogLevel; // initialized in corresponding cpp file

    // NOLINTJUSTIFICATION safe access is guaranteed since the char array is wrapped inside the class
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    thread_local static char m_buffer[NULL_TERMINATED_BUFFER_SIZE];
    thread_local static uint32_t m_bufferWriteIndex; // initialized in corresponding cpp file
    // NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
    // TODO thread local storage with thread id
};

} // namespace pbb
} // namespace iox

#include "iceoryx_hoofs/internal/log_ng/platform_building_blocks/console_logger.inl"

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_HPP
