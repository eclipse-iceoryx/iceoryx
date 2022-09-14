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

#include "iceoryx_hoofs/log/platform_building_blocks/logcommon.hpp"

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

    static void setLogLevel(const LogLevel logLevel) noexcept;

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

    void logBool(const bool value) noexcept;

    template <typename T, typename std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
    void logDec(const T val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, int> = 0>
    void logHex(const T val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, int> = 0>
    void logOct(const T val) noexcept;

  private:
    template <uint32_t N>
    // NOLINTJUSTIFICATION safe access is guaranteed since the char array is not accessed but only the size is obtained
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    inline static constexpr uint32_t bufferSize(const char (&)[N]) noexcept;

    template <typename T>
    inline void unused(T&&) const noexcept;

    template <typename T>
    inline void logArithmetic(const T value, const char* format) noexcept;

    struct ThreadLocalData
    {
        ThreadLocalData() noexcept = default;
        ~ThreadLocalData() = default;

        ThreadLocalData(const ThreadLocalData&) = delete;
        ThreadLocalData(ThreadLocalData&&) = delete;

        ThreadLocalData& operator=(const ThreadLocalData&) = delete;
        ThreadLocalData& operator=(ThreadLocalData&&) = delete;

        /// @todo iox-#1345 this could be made a compile time option
        static constexpr uint32_t BUFFER_SIZE{1024};
        static constexpr uint32_t NULL_TERMINATED_BUFFER_SIZE{BUFFER_SIZE + 1};

        // NOLINTJUSTIFICATION safe access is guaranteed since the char array is wrapped inside the class
        // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
        char buffer[NULL_TERMINATED_BUFFER_SIZE];
        uint32_t bufferWriteIndex; // initialized in corresponding cpp file
        /// @todo iox-#1345 add thread local storage with thread id and print it in the log messages
    };

    static ThreadLocalData& getThreadLocalData();

  private:
    // NOLINTJUSTIFICATION needed for the functionality and a private member of the class
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    static std::atomic<LogLevel> m_activeLogLevel; // initialized in corresponding cpp file
};

} // namespace pbb
} // namespace iox

#include "iceoryx_hoofs/internal/log/platform_building_blocks/console_logger.inl"

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_HPP
