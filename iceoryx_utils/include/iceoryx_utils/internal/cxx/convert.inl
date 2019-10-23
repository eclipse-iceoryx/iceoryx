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
    if (strlen(v) != 1)
    {
        std::cerr << v << " is not a char" << std::endl;
        return false;
    }
    dest = v[0];
    return true;
}

template <>
inline bool convert::fromString<CString100>(const char* v, CString100& dest)
{
    dest = CString100(v);
    return true;
}

inline bool convert::stringIsNumber(const char* v, const NumberType type)
{
    if (v[0] == '\0')
        return false;

    bool hasDot = false;

    for (unsigned int i = 0; v[i] != '\0'; ++i)
    {
        if (v[i] >= 48 && v[i] <= 57) // 48 == ascii 0, 57 == ascii 9
        {
            continue;
        }
        else if (type != NumberType::UNSIGNED_INTEGER && i == 0
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
inline bool convert::fromString<unsigned int>(const char* v, unsigned int& dest)
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

    if (call.getReturnValue() > UINT_MAX)
    {
        std::cerr << call.getReturnValue() << " too large, unsigned integer overflow" << std::endl;
        return false;
    }

    dest = static_cast<unsigned int>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<unsigned short>(const char* v, unsigned short& dest)
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

    if (call.getReturnValue() > USHRT_MAX)
    {
        std::cerr << call.getReturnValue() << " too large, unsigned short overflow" << std::endl;
        return false;
    }

    dest = static_cast<unsigned short>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<short>(const char* v, short& dest)
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

    if (call.getReturnValue() > SHRT_MAX || call.getReturnValue() < SHRT_MIN)
    {
        std::cerr << call.getReturnValue() << " too large, short integer overflow" << std::endl;
        return false;
    }

    dest = static_cast<short>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<unsigned long>(const char* v, unsigned long& dest)
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

    dest = call.getReturnValue();
    return true;
}

template <>
inline bool convert::fromString<unsigned long long>(const char* v, unsigned long long& dest)
{
    if (!stringIsNumberWithErrorMessage(v, NumberType::UNSIGNED_INTEGER))
    {
        return false;
    }

    auto call = makeSmartC(strtoull, ReturnMode::PRE_DEFINED_ERROR_CODE, {ULLONG_MAX}, {}, v, nullptr, 10);
    if (call.hasErrors())
    {
        return false;
    }

    dest = call.getReturnValue();
    return true;
}

template <>
inline bool convert::fromString<int>(const char* v, int& dest)
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

    if (call.getReturnValue() < INT_MIN || call.getReturnValue() > INT_MAX)
    {
        std::cerr << call.getReturnValue() << " too large, integer overflow!" << std::endl;
        return false;
    }

    dest = static_cast<int>(call.getReturnValue());
    return true;
}

template <>
inline bool convert::fromString<long>(const char* v, long& dest)
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

    dest = call.getReturnValue();
    return true;
}

template <>
inline bool convert::fromString<long long>(const char* v, long long& dest)
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

    dest = call.getReturnValue();
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
