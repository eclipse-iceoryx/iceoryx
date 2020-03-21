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

#include <limits>
namespace iox
{
namespace cxx
{
template <typename Source>
inline typename std::enable_if<!std::is_convertible<Source, std::string>::value, std::string>::type
convert::toString(const Source& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template <typename Source>
inline typename std::enable_if<std::is_convertible<Source, std::string>::value, std::string>::type
convert::toString(const Source& t)
{
    return t;
}

template <typename Destination>
inline typename std::enable_if<std::is_convertible<const char*, Destination>::value, bool>::type
fromString(const char* v, Destination& dest)
{
    dest = Destination(v);
    return true;
}

template <>
inline bool convert::fromString<std::string>(const char* v, std::string& dest)
{
    dest = std::string(v);
    return true;
}

template <>
inline bool convert::fromString<char>(const char* v, char& dest)
{
    if (strlen(v) != 1u)
    {
        std::cerr << v << " is not a char" << std::endl;
        return false;
    }
    dest = v[0];
    return true;
}

template <>
inline bool convert::fromString<string<100>>(const char* v, string<100>& dest)
{
    dest = string<100>(TruncateToCapacity, v);
    return true;
}

inline bool convert::stringIsNumber(const char* v, const NumberType type)
{
    if (v[0] == '\0')
        return false;

    bool hasDot = false;

    for (uint32_t i = 0u; v[i] != '\0'; ++i)
    {
        if (v[i] >= 48 && v[i] <= 57) // 48 == ascii 0, 57 == ascii 9
        {
            continue;
        }
        else if (type != NumberType::UNSIGNED_INTEGER && i == 0u
                 && (v[i] == 43 || v[i] == 45)) // 43 == ascii +, 45 == ascii -
        {
            continue;
        }
        else if (type == NumberType::FLOAT && !hasDot && v[i] == 46) // 46 == ascii .
        {
            hasDot = true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

inline bool convert::stringIsNumberWithErrorMessage(const char* v, const NumberType type)
{
    if (!stringIsNumber(v, type))
    {
        std::cerr << v << " is not ";
        switch (type)
        {
        case NumberType::FLOAT: {
            std::cerr << "a float";
            break;
        }
        case NumberType::INTEGER: {
            std::cerr << "a signed integer";
            break;
        }
        case NumberType::UNSIGNED_INTEGER: {
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
inline bool convert::fromString<float>(const char* v, float& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::FLOAT))
    {
        return false;
    }

    auto call = makeSmartC(strtof, ReturnMode::PRE_DEFINED_ERROR_CODE, {HUGE_VALF, -HUGE_VALF}, {}, v, nullptr);

    if (call.hasErrors())
    {
        return false;
    }

    dest = call.getReturnValue();
    return true;
}

template <>
inline bool convert::fromString<double>(const char* v, double& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::FLOAT))
    {
        return false;
    }

    auto call = makeSmartC(strtod, ReturnMode::PRE_DEFINED_ERROR_CODE, {HUGE_VAL, -HUGE_VAL}, {}, v, nullptr);

    if (call.hasErrors())
    {
        return false;
    }

    dest = call.getReturnValue();
    return true;
}

template <>
inline bool convert::fromString<long double>(const char* v, long double& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::FLOAT))
    {
        return false;
    }

    auto call = makeSmartC(strtold, ReturnMode::PRE_DEFINED_ERROR_CODE, {HUGE_VALL, -HUGE_VALL}, {}, v, nullptr);

    if (call.hasErrors())
    {
        return false;
    }

    dest = call.getReturnValue();
    return true;
}

template <>
inline bool convert::fromString<uint64_t>(const char* v, uint64_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoull, ReturnMode::PRE_DEFINED_ERROR_CODE, {ULLONG_MAX}, {}, v, nullptr, 10u);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<uint64_t>::max())
    {
        std::cerr << call.getReturnValue() << " too large, uint64_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint64_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<uint32_t>(const char* v, uint32_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoull, ReturnMode::PRE_DEFINED_ERROR_CODE, {ULLONG_MAX}, {}, v, nullptr, 10u);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<uint32_t>::max())
    {
        std::cerr << call.getReturnValue() << " too large, uint32_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint32_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<uint16_t>(const char* v, uint16_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoul, ReturnMode::PRE_DEFINED_ERROR_CODE, {ULONG_MAX}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<uint16_t>::max())
    {
        std::cerr << call.getReturnValue() << " too large, uint16_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint16_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<uint8_t>(const char* v, uint8_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoul, ReturnMode::PRE_DEFINED_ERROR_CODE, {ULONG_MAX}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<uint8_t>::max())
    {
        std::cerr << call.getReturnValue() << " too large, uint8_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<uint8_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<int64_t>(const char* v, int64_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoll, ReturnMode::PRE_DEFINED_ERROR_CODE, {LLONG_MAX, LLONG_MIN}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<int64_t>::max()
        || call.getReturnValue() < std::numeric_limits<int64_t>::min())
    {
        std::cerr << call.getReturnValue() << " is out of range, int64_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int64_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<int32_t>(const char* v, int32_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoll, ReturnMode::PRE_DEFINED_ERROR_CODE, {LLONG_MAX, LLONG_MIN}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<int32_t>::max()
        || call.getReturnValue() < std::numeric_limits<int32_t>::min())
    {
        std::cerr << call.getReturnValue() << " is out of range, int32_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int32_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<int16_t>(const char* v, int16_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtol, ReturnMode::PRE_DEFINED_ERROR_CODE, {LONG_MAX, LONG_MIN}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<int16_t>::max()
        || call.getReturnValue() < std::numeric_limits<int16_t>::min())
    {
        std::cerr << call.getReturnValue() << " is out of range, int16_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int16_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<int8_t>(const char* v, int8_t& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtol, ReturnMode::PRE_DEFINED_ERROR_CODE, {LONG_MAX, LONG_MIN}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    if (call.getReturnValue() > std::numeric_limits<int8_t>::max()
        || call.getReturnValue() < std::numeric_limits<int8_t>::min())
    {
        std::cerr << call.getReturnValue() << " is out of range, int8_t overflow" << std::endl;
        return false;
    }

    dest = static_cast<int8_t>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<bool>(const char* v, bool& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoul, ReturnMode::PRE_DEFINED_ERROR_CODE, {ULONG_MAX}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    dest = static_cast<bool>(call.getReturnValue());
    return true;
}

} // namespace cxx
} // namespace iox
