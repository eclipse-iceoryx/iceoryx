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

#ifndef IOX_HOOFS_LOG_LOGSTREAM_INL
#define IOX_HOOFS_LOG_LOGSTREAM_INL

#include "iceoryx_hoofs/log/logstream.hpp"

#include <string>

namespace iox
{
namespace log
{
template <typename T>
template <typename>
constexpr LogHex<T>::LogHex(const T value) noexcept
    : m_value(value)
{
}

template <typename T, typename>
inline constexpr LogHex<T> hex(const T value) noexcept
{
    return LogHex<T>(value);
}

inline LogHex<uint64_t> hex(const void* const ptr) noexcept
{
    // JUSTIFICATION needed to print the pointer
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return LogHex<uint64_t>(reinterpret_cast<uint64_t>(ptr));
}

template <typename T>
template <typename>
inline constexpr LogOct<T>::LogOct(const T value) noexcept
    : m_value(value)
{
}

template <typename T, typename>
inline constexpr LogOct<T> oct(const T value) noexcept
{
    return LogOct<T>(value);
}

inline LogStream::LogStream(const char* file, const int line, const char* function, LogLevel logLevel) noexcept
{
    m_logger.createLogMessageHeader(file, line, function, logLevel);
}

inline LogStream::~LogStream() noexcept
{
    flush();
}

inline void LogStream::flush() noexcept
{
    if (!m_flushed)
    {
        m_logger.flush();
        m_flushed = true;
    }
}

inline LogStream& LogStream::self() noexcept
{
    return *this;
}

inline LogStream& LogStream::operator<<(const char* cstr) noexcept
{
    m_logger.logString(cstr);
    m_flushed = false;
    return *this;
}

inline LogStream& LogStream::operator<<(const std::string& str) noexcept
{
    m_logger.logString(str.c_str());
    m_flushed = false;
    return *this;
}

template <typename T, typename std::enable_if_t<std::is_signed<T>::value, int>>
inline LogStream& LogStream::operator<<(const T val) noexcept
{
    m_logger.logI64Dec(val);
    m_flushed = false;
    return *this;
}

template <typename T, typename std::enable_if_t<std::is_unsigned<T>::value, int>>
inline LogStream& LogStream::operator<<(const T val) noexcept
{
    m_logger.logU64Dec(val);
    m_flushed = false;
    return *this;
}

template <typename T, typename std::enable_if_t<std::is_signed<T>::value, int>>
inline LogStream& LogStream::operator<<(const LogHex<T>&& val) noexcept
{
    m_logger.logString("0x");
    m_logger.logU64Hex(static_cast<uint64_t>(val.m_value));
    m_flushed = false;
    return *this;
}

template <typename T, typename std::enable_if_t<std::is_unsigned<T>::value, int>>
inline LogStream& LogStream::operator<<(const LogHex<T>&& val) noexcept
{
    m_logger.logString("0x");
    m_logger.logU64Hex(val.m_value);
    m_flushed = false;
    return *this;
}

template <typename T, typename std::enable_if_t<std::is_signed<T>::value, int>>
inline LogStream& LogStream::operator<<(const LogOct<T>&& val) noexcept
{
    m_logger.logString("0o");
    m_logger.logU64Oct(static_cast<uint64_t>(val.m_value));
    m_flushed = false;
    return *this;
}

template <typename T, typename std::enable_if_t<std::is_unsigned<T>::value, int>>
inline LogStream& LogStream::operator<<(const LogOct<T>&& val) noexcept
{
    m_logger.logString("0o");
    m_logger.logU64Oct(val.m_value);
    m_flushed = false;
    return *this;
}

template <typename Callable, typename>
inline LogStream& LogStream::operator<<(const Callable&& c)
{
    return c(*this);
}

inline LogStream& LogStream::operator<<(const LogLevel value) noexcept
{
    // TODO re-add asStringLiteral
    // m_logger.logString(asStringLiteral(value));
    m_logger.logU64Dec(static_cast<uint64_t>(value));
    return *this;
}

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_LOG_LOGSTREAM_INL
