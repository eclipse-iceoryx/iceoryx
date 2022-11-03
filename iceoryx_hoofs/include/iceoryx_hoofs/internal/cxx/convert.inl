// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_CONVERT_INL
#define IOX_HOOFS_CXX_CONVERT_INL

#include "iceoryx_hoofs/cxx/convert.hpp"

namespace iox
{
namespace cxx
{
///@brief specialization for  uint8_t and int8_t is required  since uint8_t is unsigned char and int8_t is signed char
/// and stringstream will not convert these to string as it is already a character.
template <>
inline typename std::enable_if<!std::is_convertible<uint8_t, std::string>::value, std::string>::type
convert::toString(const uint8_t& t) noexcept
{
    return toString(static_cast<uint16_t>(t));
}

template <>
inline typename std::enable_if<!std::is_convertible<int8_t, std::string>::value, std::string>::type
convert::toString(const int8_t& t) noexcept
{
    return toString(static_cast<int16_t>(t));
}


template <typename Source>
inline typename std::enable_if<!std::is_convertible<Source, std::string>::value, std::string>::type
convert::toString(const Source& t) noexcept
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template <typename Source>
inline typename std::enable_if<std::is_convertible<Source, std::string>::value, std::string>::type
convert::toString(const Source& t) noexcept
{
    return t;
}

template <typename Destination>
inline typename std::enable_if<std::is_convertible<const char*, Destination>::value, bool>::type
fromString(const char* v, Destination& dest) noexcept
{
    dest = Destination(v);
    return true;
}

template <>
inline bool convert::fromString<std::string>(const char* v, std::string& dest) noexcept
{
    dest = std::string(v);
    return true;
}

template <>
inline bool convert::fromString<char>(const char* v, char& dest) noexcept
{
    if (strlen(v) != 1U)
    {
        std::cerr << v << " is not a char" << std::endl;
        return false;
    }

    /// @NOLINTJUSTIFICATION encapsulated in abstraction
    /// @NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    dest = v[0];
    return true;
}

/// @NOLINTJUSTIFICATION @todo iox-#260 convert must be refactored
/// @NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
template <>
inline bool convert::fromString<string<100>>(const char* v, string<100>& dest) noexcept
{
    dest = string<100>(TruncateToCapacity, v, strnlen(v, string<100>::capacity()));
    return true;
}
/// @NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

inline bool convert::stringIsNumber(const char* v, const NumberType type) noexcept
{
    /// @NOLINTJUSTIFICATION encapsulated in abstraction
    /// @NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (v[0] == '\0')
    {
        return false;
    }

    bool hasDot = false;

    for (uint32_t i = 0U; v[i] != '\0'; ++i)
    {
        if (v[i] >= '0' && v[i] <= '9')
        {
            continue;
        }

        if (type != NumberType::UNSIGNED_INTEGER && i == 0U && (v[i] == '+' || v[i] == '-'))
        {
            continue;
        }

        if (type == NumberType::FLOAT && !hasDot && v[i] == '.')
        {
            hasDot = true;
        }
        else
        {
            return false;
        }
    }
    /// @NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    return true;
}

inline bool convert::stringIsNumberWithErrorMessage(const char* v, const NumberType type) noexcept
{
    if (!stringIsNumber(v, type))
    {
        std::cerr << v << " is not ";
        switch (type)
        {
        case NumberType::FLOAT:
        {
            std::cerr << "a float";
            break;
        }
        case NumberType::INTEGER:
        {
            std::cerr << "a signed integer";
            break;
        }
        case NumberType::UNSIGNED_INTEGER:
        {
            std::cerr << "an unsigned integer";
            break;
        }
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template <>
inline bool convert::fromString<float>(const char* v, float& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::FLOAT))
    {
        return false;
    }

    return !posix::posixCall(strtof)(v, nullptr)
                .failureReturnValue(HUGE_VALF, -HUGE_VALF)
                .evaluate()
                .and_then([&](auto& r) { dest = r.value; })
                .has_error();
}

template <>
inline bool convert::fromString<double>(const char* v, double& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::FLOAT))
    {
        return false;
    }

    return !posix::posixCall(strtod)(v, nullptr)
                .failureReturnValue(HUGE_VAL, -HUGE_VAL)
                .evaluate()
                .and_then([&](auto& r) { dest = r.value; })
                .has_error();
}

template <>
inline bool convert::fromString<long double>(const char* v, long double& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::FLOAT))
    {
        return false;
    }

    return !posix::posixCall(strtold)(v, nullptr)
                .failureReturnValue(HUGE_VALL, -HUGE_VALL)
                .evaluate()
                .and_then([&](auto& r) { dest = r.value; })
                .has_error();
}

template <>
inline bool convert::fromString<uint64_t>(const char* v, uint64_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = posix::posixCall(strtoull)(v, nullptr, STRTOULL_BASE).failureReturnValue(ULLONG_MAX).evaluate();

    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<uint64_t>::max())
    {
        std::cerr << call->value << " too large, uint64_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint64_t>(call->value);
    return true;
}

#ifdef __APPLE__
/// introduced for mac os since unsigned long is not uint64_t despite it has the same size
/// who knows why ¯\_(ツ)_/¯
template <>
inline bool convert::fromString<unsigned long>(const char* v, unsigned long& dest) noexcept
{
    uint64_t temp{0};
    bool retVal = fromString(v, temp);
    dest = temp;
    return retVal;
}
#endif

template <>
inline bool convert::fromString<uint32_t>(const char* v, uint32_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = posix::posixCall(strtoull)(v, nullptr, STRTOULL_BASE).failureReturnValue(ULLONG_MAX).evaluate();

    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<uint32_t>::max())
    {
        std::cerr << call->value << " too large, uint32_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint32_t>(call->value);
    return true;
}

template <>
inline bool convert::fromString<uint16_t>(const char* v, uint16_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = posix::posixCall(strtoul)(v, nullptr, STRTOULL_BASE).failureReturnValue(ULONG_MAX).evaluate();

    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<uint16_t>::max())
    {
        std::cerr << call->value << " too large, uint16_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint16_t>(call->value);
    return true;
}

template <>
inline bool convert::fromString<uint8_t>(const char* v, uint8_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = posix::posixCall(strtoul)(v, nullptr, STRTOULL_BASE).failureReturnValue(ULONG_MAX).evaluate();

    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<uint8_t>::max())
    {
        std::cerr << call->value << " too large, uint8_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint8_t>(call->value);
    return true;
}

template <>
inline bool convert::fromString<int64_t>(const char* v, int64_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call =
        posix::posixCall(strtoll)(v, nullptr, STRTOULL_BASE).failureReturnValue(LLONG_MAX, LLONG_MIN).evaluate();
    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<int64_t>::max() || call->value < std::numeric_limits<int64_t>::min())
    {
        std::cerr << call->value << " is out of range, int64_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int64_t>(call->value);
    return true;
}

template <>
inline bool convert::fromString<int32_t>(const char* v, int32_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call =
        posix::posixCall(strtoll)(v, nullptr, STRTOULL_BASE).failureReturnValue(LLONG_MAX, LLONG_MIN).evaluate();
    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<int32_t>::max() || call->value < std::numeric_limits<int32_t>::min())
    {
        std::cerr << call->value << " is out of range, int32_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int32_t>(call->value);
    return true;
}

template <>
inline bool convert::fromString<int16_t>(const char* v, int16_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call = posix::posixCall(strtol)(v, nullptr, STRTOULL_BASE).failureReturnValue(LONG_MAX, LONG_MIN).evaluate();
    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<int16_t>::max() || call->value < std::numeric_limits<int16_t>::min())
    {
        std::cerr << call->value << " is out of range, int16_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int16_t>(call->value);
    return true;
}

template <>
inline bool convert::fromString<int8_t>(const char* v, int8_t& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call = posix::posixCall(strtol)(v, nullptr, STRTOULL_BASE).failureReturnValue(LONG_MAX, LONG_MIN).evaluate();
    if (call.has_error())
    {
        return false;
    }

    if (call->value > std::numeric_limits<int8_t>::max() || call->value < std::numeric_limits<int8_t>::min())
    {
        std::cerr << call->value << " is out of range, int8_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int8_t>(call->value);
    return true;
}

template <>
inline bool convert::fromString<bool>(const char* v, bool& dest) noexcept
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    return !posix::posixCall(strtoul)(v, nullptr, STRTOULL_BASE)
                .failureReturnValue(ULONG_MAX)
                .evaluate()
                .and_then([&](auto& r) { dest = static_cast<bool>(r.value); })
                .has_error();
}

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_CONVERT_INL
