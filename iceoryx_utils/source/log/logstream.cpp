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

#include "iceoryx_utils/log/logstream.hpp"

#include "iceoryx_utils/log/logger.hpp"
#include "iceoryx_utils/log/logging.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>

namespace iox
{
namespace log
{
LogStream::LogStream(Logger& logger, LogLevel logLevel)
    : m_logger(logger)
{
    m_logEntry.level = logLevel;
    /// @todo do we want to do this only when loglevel is higher than global loglevel?
    auto timePoint = std::chrono::high_resolution_clock::now();
    m_logEntry.time = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
}

LogStream::~LogStream()
{
    Flush();
}

void LogStream::Flush()
{
    /// @todo do we want to send the log to the logger even if the loglevel is lower than the global log level?
    if (!m_flushed)
    {
        m_flushed = true;
        m_logger.Log(m_logEntry);
        m_logEntry.message.clear();
        /// @todo do we need to reset the m_logTime? maybe just print a counter with each flush?
    }
}

LogStream& LogStream::operator<<(const char* cstr) noexcept
{
    m_logEntry.message.append(cstr);
    m_flushed = false;
    return *this;
}

LogStream& LogStream::operator<<(const std::string& str) noexcept
{
    m_logEntry.message.append(str);
    m_flushed = false;
    return *this;
}

LogStream& operator<<(LogStream& out, LogLevel value) noexcept
{
    switch (value)
    {
    case LogLevel::kOff:
        return (out << "Off");
    case LogLevel::kFatal:
        return (out << "Fatal");
    case LogLevel::kError:
        return (out << "Error");
    case LogLevel::kWarn:
        return (out << "Warn");
    case LogLevel::kInfo:
        return (out << "Info");
    case LogLevel::kDebug:
        return (out << "Debug");
    case LogLevel::kVerbose:
        return (out << "Verbose");
    default:
        return (out << "Off");
    }
}

LogStream& LogStream::operator<<(const LogRawBuffer& value) noexcept
{
    std::stringstream ss;
    ss << "0x[";
    ss << std::hex << std::setfill('0');
    for (int8_t i = 0; i < value.size; ++i)
    {
        // the '+value' is there to not interpret the uint8_t as char and print the character instead of the hex value
        ss << (i > 0 ? " " : "") << std::setw(2) << +value.data[i];
    }
    ss << "]";
    m_logEntry.message.append(ss.str());
    m_flushed = false;
    return *this;
}

} // namespace log
} // namespace iox
