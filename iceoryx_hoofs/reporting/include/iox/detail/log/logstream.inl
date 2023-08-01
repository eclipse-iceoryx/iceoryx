// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_REPORTING_LOG_LOGSTREAM_INL
#define IOX_HOOFS_REPORTING_LOG_LOGSTREAM_INL

#include "iox/log/logstream.hpp"

#include <string>

namespace iox
{
// AXIVION Next Construct AutosarC++19_03-M2.10.1 : log is a sensible namespace for a logger; furthermore it is in the
// iox namespace and when used as function the compiler will complain
namespace log
{
template <typename T>
template <typename>
constexpr LogHex<T>::LogHex(const T value) noexcept
    : m_value(value)
{
}

// AXIVION Next Construct AutosarC++19_03-M17.0.3 : See at declaration in header
template <typename T, typename>
inline constexpr LogHex<T> hex(const T value) noexcept
{
    return LogHex<T>(value);
}

// AXIVION Next Construct AutosarC++19_03-M17.0.3 : See at declaration in header
inline LogHex<const void* const> hex(const void* const ptr) noexcept
{
    return LogHex<const void* const>(ptr);
}

template <typename T>
template <typename>
constexpr LogOct<T>::LogOct(const T value) noexcept
    : m_value(value)
{
}

// AXIVION Next Construct AutosarC++19_03-M17.0.3 : See at declaration in header
template <typename T, typename>
inline constexpr LogOct<T> oct(const T value) noexcept
{
    return LogOct<T>(value);
}

/// @todo iox-#1755 use something like 'source_location'
// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See at declaration in header
// NOLINTNEXTLINE(readability-function-size)
inline LogStream::LogStream(
    Logger& logger, const char* file, const int line, const char* function, LogLevel logLevel) noexcept
    : m_logger(logger)
{
    m_logger.createLogMessageHeader(file, line, function, logLevel);
}

// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See at declaration in header
inline LogStream::LogStream(const char* file, const int line, const char* function, LogLevel logLevel) noexcept
    : LogStream(Logger::get(), file, line, function, logLevel)
{
}

/// @todo iox-#1755 use something like 'source_location'
// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See at declaration in header
// NOLINTNEXTLINE(readability-function-size)
inline LogStream::LogStream(
    const char* file, const int line, const char* function, LogLevel logLevel, bool doFlush) noexcept
    : m_logger(Logger::get())
    , m_doFlush(doFlush)
{
    m_logger.createLogMessageHeader(file, line, function, logLevel);
}

inline LogStream::~LogStream() noexcept
{
    flush();
}

inline void LogStream::flush() noexcept
{
    if (!m_isFlushed && m_doFlush)
    {
        m_logger.flush();
        m_isFlushed = true;
    }
}

inline LogStream& LogStream::self() noexcept
{
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See at declaration in header
// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
inline LogStream& LogStream::operator<<(const char* cstr) noexcept
{
    m_logger.logString(cstr);
    m_isFlushed = false;
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
inline LogStream& LogStream::operator<<(const std::string& str) noexcept
{
    m_logger.logString(str.c_str());
    m_isFlushed = false;
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
inline LogStream& LogStream::operator<<(const bool val) noexcept
{
    m_logger.logBool(val);
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
template <typename T, typename std::enable_if_t<std::is_arithmetic<T>::value, bool>>
inline LogStream& LogStream::operator<<(const T val) noexcept
{
    m_logger.logDec(val);
    m_isFlushed = false;
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
template <typename T, typename std::enable_if_t<std::is_integral<T>::value, bool>>
inline LogStream& LogStream::operator<<(const LogHex<T> val) noexcept
{
    m_logger.logString("0x");
    m_logger.logHex(static_cast<typename std::make_unsigned<T>::type>(val.m_value));
    m_isFlushed = false;
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool>>
inline LogStream& LogStream::operator<<(const LogHex<T> val) noexcept
{
    m_logger.logHex(val.m_value);
    m_isFlushed = false;
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
inline LogStream& LogStream::operator<<(const LogHex<const void* const> val) noexcept
{
    m_logger.logHex(val.m_value);
    m_isFlushed = false;
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
template <typename T, typename std::enable_if_t<std::is_integral<T>::value, bool>>
inline LogStream& LogStream::operator<<(const LogOct<T> val) noexcept
{
    m_logger.logString("0o");
    m_logger.logOct(static_cast<typename std::make_unsigned<T>::type>(val.m_value));
    m_isFlushed = false;
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
template <typename Callable, typename>
inline LogStream& LogStream::operator<<(const Callable& c) noexcept
{
    return c(*this);
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
inline LogStream& LogStream::operator<<(const LogLevel value) noexcept
{
    m_logger.logString(asStringLiteral(value));
    return *this;
}

namespace internal
{
// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See at declaration in header
inline LogStreamOff::LogStreamOff(const char*, const int, const char*, LogLevel, bool) noexcept
{
}

inline LogStreamOff& LogStreamOff::self() noexcept
{
    return *this;
}

// AXIVION Next Construct AutosarC++19_03-M5.17.1 : This is not used as shift operator but as stream operator and does
// not require to implement '<<='
template <typename T>
inline LogStreamOff& LogStreamOff::operator<<(T&&) noexcept
{
    return *this;
}
} // namespace internal

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_REPORTING_LOG_LOGSTREAM_INL
