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

#include "iceoryx_hoofs/log_ng/platform_building_blocks/console_logger.hpp"
#include "iceoryx_hoofs/platform/time.hpp"

#include <cstdio>
#include <cstring>
#include <ctime>

namespace iox
{
namespace pbb
{
// NOLINTJUSTIFICATION see at declaration in header
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<LogLevel> ConsoleLogger::m_activeLogLevel{LogLevel::INFO};

// NOLINTJUSTIFICATION see at declaration in header
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
thread_local char ConsoleLogger::m_buffer[ConsoleLogger::NULL_TERMINATED_BUFFER_SIZE]{0};

thread_local uint32_t ConsoleLogger::m_bufferWriteIndex{0U};
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

LogLevel ConsoleLogger::getLogLevel() noexcept
{
    return m_activeLogLevel.load(std::memory_order_relaxed);
}

// NOLINTJUSTIFICATION member access is required
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConsoleLogger::setLogLevel(const LogLevel logLevel) noexcept
{
    m_activeLogLevel.store(logLevel, std::memory_order_relaxed);
}

void ConsoleLogger::createLogMessageHeader(const char* file,
                                           const int line,
                                           const char* function,
                                           LogLevel logLevel) noexcept
{
    // TODO check all pointer for nullptr

    timespec timestamp{0, 0};
    if (clock_gettime(CLOCK_REALTIME, &timestamp) != 0)
    {
        timestamp = {0, 0};
        // intentionally do nothing since a timestamp from 01.01.1970 already indicates  an issue with the clock
    }

    time_t time = timestamp.tv_sec;
// TODO move to platform
#if defined(_WIN32)
    // seems to be thread-safe on Windows
    auto* timeInfo = localtime(&time);
#else
    // NOLINTJUSTIFICATION will be initialized with the call to localtime_r in the statement after the declaration
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    struct tm calendarData;

    // TODO check whether localtime_s would be the better solution
    auto* timeInfo = localtime_r(&time, &calendarData);
#endif
    if (timeInfo == nullptr)
    {
        // TODO an error occurred; what to do next? return? don't use the timestamp? call error handler?
    }

    // NOLINTJUSTIFICATION this is used to get the size for the buffer where strftime writes the local time
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    constexpr const char TIME_FORMAT[]{"2002-02-20 22:02:02"};
    constexpr uint32_t ZERO_TERMINATION{1U};
    constexpr uint32_t YEAR_1M_PROBLEM{2U}; // in case iceoryx is still in use, please change to 3
    constexpr auto TIMESTAMP_BUFFER_SIZE{ConsoleLogger::bufferSize(TIME_FORMAT) + YEAR_1M_PROBLEM + ZERO_TERMINATION};

    // NOLINTJUSTIFICATION required for strftime and safe since array bounds are taken into account
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char timestampString[TIMESTAMP_BUFFER_SIZE]{0};
    auto strftimeRetVal = strftime(&timestampString[0],
                                   TIMESTAMP_BUFFER_SIZE - 1, // TODO check whether the -1 is required
                                   "%Y-%m-%d %H:%M:%S",
                                   timeInfo);
    if (strftimeRetVal == 0)
    {
        // TODO an error occurred; what to do next? return? don't use the timestamp? call error handler?
    }

    constexpr auto MILLISECS_PER_SECOND{1000};
    auto milliseconds = timestamp.tv_nsec % MILLISECS_PER_SECOND;

    // TODO do we also want to always log the iceoryx version and commit sha? Maybe do that only in
    // `initLogger` with LogDebug

    unused(file);
    unused(line);
    unused(function);
    // << "\033[0;90m " << file << ':' << line << " ‘" << function << "’";

    // TODO check whether snprintf_s would be the better solution
    // TODO double check whether snprintf is correctly used


    // NOLINTJUSTIFICATION the user will only call this via a macro which ensures that the index won't be out of bounds
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
    // NOLINTJUSTIFICATION snprintf required to populate char array so that it can be flushed in one piece
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
    auto retVal = snprintf(&m_buffer[0],
                           NULL_TERMINATED_BUFFER_SIZE,
                           "\033[0;90m%s.%03ld %s%s\033[m: ",
                           &timestampString[0],
                           milliseconds,
                           LOG_LEVEL_COLOR[static_cast<uint8_t>(logLevel)],
                           LOG_LEVEL_TEXT[static_cast<uint8_t>(logLevel)]);
    // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
    if (retVal >= 0)
    {
        m_bufferWriteIndex = static_cast<uint32_t>(retVal);
    }
    else
    {
        // TODO an error occurred; what to do next? call error handler?
    }
}

void ConsoleLogger::flush() noexcept
{
    if (std::puts(&m_buffer[0]) < 0)
    {
        // TODO an error occurred; what to do next? call error handler?
    }
    assumeFlushed();
}

// NOLINTJUSTIFICATION member access is required
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
LogBuffer ConsoleLogger::getLogBuffer() const noexcept
{
    return LogBuffer{&m_buffer[0], m_bufferWriteIndex};
}

// NOLINTJUSTIFICATION member access is required
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConsoleLogger::assumeFlushed() noexcept
{
    m_buffer[0] = 0;
    m_bufferWriteIndex = 0;
}

// NOLINTJUSTIFICATION member access is required
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConsoleLogger::logString(const char* message) noexcept
{
    auto retVal =
        // NOLINTJUSTIFICATION snprintf required to populate char array so that it can be flushed in one piece
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
        // NOLINTJUSTIFICATION it is ensured that the index cannot be out of bounds
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        snprintf(&m_buffer[m_bufferWriteIndex],
                 NULL_TERMINATED_BUFFER_SIZE - m_bufferWriteIndex,
                 "%s",
                 message); // TODO do we need to check whether message is null-terminated at a reasonable length?
                           // NOLINTEND(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
    if (retVal >= 0)
    {
        m_bufferWriteIndex += static_cast<uint32_t>(retVal);
    }
    else
    {
        // TODO an error occurred; what to do next? call error handler?
    }
}

void ConsoleLogger::logI64Dec(const int64_t value) noexcept
{
    logArithmetik(value, "%li");
}
void ConsoleLogger::logU64Dec(const uint64_t value) noexcept
{
    logArithmetik(value, "%lu");
}
void ConsoleLogger::logU64Hex(const uint64_t value) noexcept
{
    logArithmetik(value, "%x");
}
void ConsoleLogger::logU64Oct(const uint64_t value) noexcept
{
    logArithmetik(value, "%o");
}

void ConsoleLogger::initLogger(const LogLevel) noexcept
{
    // nothing to do in the base implementation
}
} // namespace pbb
} // namespace iox
