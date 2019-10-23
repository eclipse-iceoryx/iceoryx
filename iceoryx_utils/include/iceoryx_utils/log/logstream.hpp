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

#include <bitset>
#include <chrono>
#include <sstream>
#include <string>
#include <iostream>

namespace iox
{
namespace log
{
// helper struct for SFINAE of LogStream& operator<<
struct LogHex
{
};

struct LogHex8 : private LogHex
{
    uint8_t value;
    constexpr LogHex8(uint8_t value)
        : value(value)
    {
    }
};

struct LogHex16 : private LogHex
{
    uint16_t value;
    constexpr LogHex16(uint16_t value)
        : value(value)
    {
    }
};
struct LogHex32 : private LogHex
{
    uint32_t value;
    constexpr LogHex32(uint32_t value)
        : value(value)
    {
    }
};
struct LogHex64 : private LogHex
{
    uint64_t value;
    constexpr LogHex64(uint64_t value)
        : value(value)
    {
    }
};

// helper struct for SFINAE of LogStream& operator<<
struct LogBin
{
};

struct LogBin8 : private LogBin
{
    uint8_t value;
    constexpr LogBin8(uint8_t value)
        : value(value)
    {
    }
};
struct LogBin16 : private LogBin
{
    uint16_t value;
    constexpr LogBin16(uint16_t value)
        : value(value)
    {
    }
};
struct LogBin32 : private LogBin
{
    uint32_t value;
    constexpr LogBin32(uint32_t value)
        : value(value){};
};
struct LogBin64 : private LogBin
{
    uint64_t value;
    constexpr LogBin64(uint64_t value)
        : value(value)
    {
    }
};
struct LogRawBuffer
{
    const uint8_t* data;
    uint8_t size;
};

class Logger;

class LogStream
{
  public:
    LogStream(Logger& logger, LogLevel logLevel = LogLevel::kWarn);

    virtual ~LogStream();

    void Flush();

    LogStream& operator<<(const char* cstr) noexcept;

    LogStream& operator<<(const std::string& str) noexcept;

    template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
    LogStream& operator<<(const T val) noexcept
    {
        m_logEntry.message.append(std::to_string(val));
        m_flushed = false;
        return *this;
    }

    template <typename T, typename std::enable_if<std::is_base_of<LogHex, T>::value, int>::type = 0>
    LogStream& operator<<(const T val) noexcept
    {
        std::stringstream ss;
        // the '+val' is there to not interpret the uint8_t as char and print the character instead of the hex value
        ss << "0x" << std::hex << +val.value;
        m_logEntry.message.append(ss.str());
        m_flushed = false;
        return *this;
    }

    template <typename T, typename std::enable_if<std::is_base_of<LogBin, T>::value, int>::type = 0>
    LogStream& operator<<(const T val) noexcept
    {
        m_logEntry.message.append("0b");
        m_logEntry.message.append(std::bitset<std::numeric_limits<decltype(val.value)>::digits>(val.value).to_string());
        m_flushed = false;
        return *this;
    }

    LogStream& operator<<(const LogRawBuffer& value) noexcept;

  private:
    Logger& m_logger;
    bool m_flushed{false};
    LogEntry m_logEntry;
};

LogStream& operator<<(LogStream& out, LogLevel value) noexcept;

} // namespace log
} // namespace iox

