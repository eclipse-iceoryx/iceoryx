// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGCOMMON_INL
#define IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGCOMMON_INL

#include "iceoryx_hoofs/log/platform_building_blocks/logcommon.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>

namespace iox
{
namespace pbb
{
inline constexpr const char* asStringLiteral(const LogLevel value) noexcept
{
    switch (value)
    {
    case LogLevel::OFF:
        return "LogLevel::OFF";
    case LogLevel::FATAL:
        return "LogLevel::FATAL";
    case LogLevel::ERROR:
        return "LogLevel::ERROR";
    case LogLevel::WARN:
        return "LogLevel::WARN";
    case LogLevel::INFO:
        return "LogLevel::INFO";
    case LogLevel::DEBUG:
        return "LogLevel::DEBUG";
    case LogLevel::TRACE:
        return "LogLevel::TRACE";
    }

    return "[Undefined LogLevel]";
}

inline constexpr const char* logLevelDisplayColor(const LogLevel value) noexcept
{
    switch (value)
    {
    case LogLevel::OFF:
        return ""; // nothing
    case LogLevel::FATAL:
        return "\033[0;1;97;41m"; // bold bright white on red
    case LogLevel::ERROR:
        return "\033[0;1;31;103m"; // bold red on light yellow
    case LogLevel::WARN:
        return "\033[0;1;93m"; // bold bright yellow
    case LogLevel::INFO:
        return "\033[0;1;92m"; // bold bright green
    case LogLevel::DEBUG:
        return "\033[0;1;96m"; // bold bright cyan
    case LogLevel::TRACE:
        return "\033[0;1;36m"; // bold cyan
    }

    return ""; // nothing
}

inline constexpr const char* logLevelDisplayText(const LogLevel value) noexcept
{
    switch (value)
    {
    case LogLevel::OFF:
        return "[ Off ]";
    case LogLevel::FATAL:
        return "[Fatal]";
    case LogLevel::ERROR:
        return "[Error]";
    case LogLevel::WARN:
        return "[Warn ]";
    case LogLevel::INFO:
        return "[Info ]";
    case LogLevel::DEBUG:
        return "[Debug]";
    case LogLevel::TRACE:
        return "[Trace]";
    }

    return "[UNDEF]";
}

namespace internal
{
template <typename>
constexpr bool always_false_v{false};

template <typename T>
constexpr const char* logFormatDec()
{
    static_assert(always_false_v<T>, "This type is not supported for decimal output!");
    return nullptr;
}
template <>
inline constexpr const char* logFormatDec<signed char>()
{
    return "%hhi";
}
template <>
inline constexpr const char* logFormatDec<unsigned char>()
{
    return "%hhu";
}
template <>
inline constexpr const char* logFormatDec<short>()
{
    return "%hi";
}
template <>
inline constexpr const char* logFormatDec<unsigned short>()
{
    return "%hu";
}
template <>
inline constexpr const char* logFormatDec<int>()
{
    return "%i";
}
template <>
inline constexpr const char* logFormatDec<unsigned int>()
{
    return "%u";
}
template <>
inline constexpr const char* logFormatDec<long>()
{
    return "%li";
}
template <>
inline constexpr const char* logFormatDec<unsigned long>()
{
    return "%lu";
}
template <>
inline constexpr const char* logFormatDec<long long>()
{
    return "%lli";
}
template <>
inline constexpr const char* logFormatDec<unsigned long long>()
{
    return "%llu";
}
template <>
inline constexpr const char* logFormatDec<float>()
{
    return "%.5e";
}
template <>
inline constexpr const char* logFormatDec<double>()
{
    return "%.5le";
}
template <>
inline constexpr const char* logFormatDec<long double>()
{
    return "%.5Le";
}

template <typename T>
constexpr const char* logFormatHex()
{
    static_assert(always_false_v<T>, "This type is not supported for hexadecimal output!");
    return nullptr;
}
template <>
constexpr const char* logFormatHex<unsigned char>()
{
    return "%hhx";
}
template <>
constexpr const char* logFormatHex<unsigned short>()
{
    return "%hx";
}
template <>
constexpr const char* logFormatHex<unsigned int>()
{
    return "%x";
}
template <>
constexpr const char* logFormatHex<unsigned long>()
{
    return "%lx";
}
template <>
constexpr const char* logFormatHex<unsigned long long>()
{
    return "%llx";
}
template <>
constexpr const char* logFormatHex<float>()
{
    return "%a";
}
template <>
constexpr const char* logFormatHex<double>()
{
    return "%la";
}
template <>
constexpr const char* logFormatHex<long double>()
{
    return "%La";
}

template <typename T>
constexpr const char* logFormatOct()
{
    static_assert(always_false_v<T>, "This type is not supported for octal output!");
    return nullptr;
}
template <>
constexpr const char* logFormatOct<unsigned char>()
{
    return "%hho";
}
template <>
constexpr const char* logFormatOct<unsigned short>()
{
    return "%ho";
}
template <>
constexpr const char* logFormatOct<unsigned int>()
{
    return "%o";
}
template <>
constexpr const char* logFormatOct<unsigned long>()
{
    return "%lo";
}
template <>
constexpr const char* logFormatOct<unsigned long long>()
{
    return "%llo";
}
} // namespace internal

} // namespace pbb
} // namespace iox

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGCOMMON_INL
