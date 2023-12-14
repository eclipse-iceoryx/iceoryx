// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
// Copyright (c) 2023 by Dennis Liu. All rights reserved.
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

#ifndef IOX_HOOFS_UTILITY_CONVERT_INL
#define IOX_HOOFS_UTILITY_CONVERT_INL

#include "iox/detail/convert.hpp"
#include "iox/logging.hpp"

namespace iox
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

template <typename Destination, typename std::enable_if_t<!is_iox_string<Destination>::value, int>>
inline iox::optional<Destination> convert::from_string(const char* v) noexcept
{
    return iox::optional<Destination>(Destination(v));
}

template <>
inline iox::optional<char> convert::from_string<char>(const char* v) noexcept
{
    if (strlen(v) != 1U)
    {
        IOX_LOG(DEBUG, v << " is not a char");
        return iox::nullopt;
    }

    /// @NOLINTJUSTIFICATION encapsulated in abstraction
    /// @NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return iox::optional<char>(v[0]);
}

template <typename IoxString, typename std::enable_if_t<is_iox_string<IoxString>::value, int>>
inline iox::optional<IoxString> convert::from_string(const char* v) noexcept
{
    if (strlen(v) > GetCapacity<IoxString>::value)
    {
        return iox::nullopt;
    }
    return iox::optional<IoxString>(IoxString(TruncateToCapacity, v));
}

template <>
inline iox::optional<bool> convert::from_string<bool>(const char* v) noexcept
{
    // we should clean errno first
    errno = 0;
    char* end_ptr = nullptr;

    if (start_with_neg_sign(v))
    {
        return iox::nullopt;
    }

    auto call = IOX_POSIX_CALL(strtoul)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(ULONG_MAX)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    // we assume that in the IOX_POSIX_CALL procedure, no other POSIX call will change errno,
    // except for the target function 'f'.
    return evaluate_return_value<bool>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<float> convert::from_string<float>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    auto call = IOX_POSIX_CALL(strtof)(v, &end_ptr)
                    .failureReturnValue(HUGE_VALF, -HUGE_VALF)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<float>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<double> convert::from_string<double>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    auto call = IOX_POSIX_CALL(strtod)(v, &end_ptr)
                    .failureReturnValue(HUGE_VAL, -HUGE_VAL)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<double>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<long double> convert::from_string<long double>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    auto call = IOX_POSIX_CALL(strtold)(v, &end_ptr)
                    .failureReturnValue(HUGE_VALL, -HUGE_VALL)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<long double>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<uint64_t> convert::from_string<uint64_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    if (start_with_neg_sign(v))
    {
        return iox::nullopt;
    }

    auto call = IOX_POSIX_CALL(strtoull)(v, &end_ptr, STRTOULL_BASE)
                    .alwaysSuccess()
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<uint64_t>(call, errno, end_ptr, v);
}

#ifdef __APPLE__
/// introduced for mac os since unsigned long is not uint64_t despite it has the same size
/// who knows why ¯\_(ツ)_/¯
template <>
inline iox::optional<unsigned long> convert::from_string<unsigned long>(const char* v, unsigned long& dest) noexcept
{
    uint64_t temp{0};
    auto ret = from_string(v, temp);
    if (!ret.has_value())
    {
        return iox::optional<unsigned long>{};
    }
    return iox::optional<unsigned long>{static_cast<unsigned long>(ret.value())};
}
#endif

#if defined(__GNUC__) && (INTPTR_MAX == INT32_MAX)
/// introduced for 32-bit arm-none-eabi-gcc since uintptr_t is not uint32_t despite it has the same size
/// who knows why ¯\_(ツ)_/¯
template <>
inline iox::optional<uintptr_t> convert::from_string<uintptr_t>(const char* v, uintptr_t& dest) noexcept
{
    uint64_t temp{0};
    auto ret = from_string(v, temp);
    if (!ret.has_value())
    {
        return iox::optional<uintptr_t>{};
    }
    return iox::optional<uintptr_t>{static_cast<uintptr_t>(ret.value())};
}
#endif

template <>
inline iox::optional<uint32_t> convert::from_string<uint32_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    if (start_with_neg_sign(v))
    {
        return iox::nullopt;
    }

    auto call = IOX_POSIX_CALL(strtoull)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(ULLONG_MAX)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<uint32_t>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<uint16_t> convert::from_string<uint16_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    if (start_with_neg_sign(v))
    {
        return iox::nullopt;
    }

    auto call = IOX_POSIX_CALL(strtoul)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(ULONG_MAX)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<uint16_t>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<uint8_t> convert::from_string<uint8_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    if (start_with_neg_sign(v))
    {
        return iox::nullopt;
    }

    auto call = IOX_POSIX_CALL(strtoul)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(ULONG_MAX)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<uint8_t>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<int64_t> convert::from_string<int64_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    auto call = IOX_POSIX_CALL(strtoll)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(LLONG_MAX, LLONG_MIN)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<int64_t>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<int32_t> convert::from_string<int32_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    auto call = IOX_POSIX_CALL(strtoll)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(LLONG_MAX, LLONG_MIN)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<int32_t>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<int16_t> convert::from_string<int16_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    auto call = IOX_POSIX_CALL(strtol)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(LONG_MAX, LONG_MIN)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<int16_t>(call, errno, end_ptr, v);
}

template <>
inline iox::optional<int8_t> convert::from_string<int8_t>(const char* v) noexcept
{
    errno = 0;
    char* end_ptr = nullptr;

    auto call = IOX_POSIX_CALL(strtol)(v, &end_ptr, STRTOULL_BASE)
                    .failureReturnValue(LONG_MAX, LONG_MIN)
                    .suppressErrorMessagesForErrnos(EINVAL, ERANGE)
                    .evaluate();

    return evaluate_return_value<int8_t>(call, errno, end_ptr, v);
}

inline bool convert::start_with_neg_sign(const char* v) noexcept
{
    if (v == nullptr)
    {
        return false;
    }

    // remove space
    while (*v != '\0' && (isspace((unsigned char)*v) != 0))
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        v++;
    }

    return (*v == '-');
}

template <typename TargetType, typename RequireCheckValType>
inline bool convert::check_edge_case(decltype(errno) errno_cache,
                                     const char* end_ptr,
                                     const char* v,
                                     const RequireCheckValType& require_check_val) noexcept
{
    return is_valid_input(end_ptr, v, require_check_val) && is_valid_errno(errno_cache)
           && is_within_range<TargetType>(require_check_val);
}

template <typename TargetType, typename CallType>
inline iox::optional<TargetType>
convert::evaluate_return_value(CallType& call, decltype(errno) errno_cache, const char* end_ptr, const char* v) noexcept
{
    if (call.has_error())
    {
        return iox::nullopt;
    }

    if (!check_edge_case<TargetType>(errno_cache, end_ptr, v, call->value))
    {
        return iox::nullopt;
    }

    return iox::optional<TargetType>(static_cast<TargetType>(call->value));
}

template <typename RequireCheckValType>
inline bool
convert::is_valid_input(const char* end_ptr, const char* v, const RequireCheckValType& require_check_val) noexcept
{
    // invalid string
    if (v == end_ptr && require_check_val == 0)
    {
        IOX_LOG(DEBUG, "invalid input");
        return false;
    }

    // end_ptr is not '\0' which means conversion failure at end_ptr
    if (end_ptr != nullptr && v != end_ptr && *end_ptr != '\0')
    {
        IOX_LOG(DEBUG, "conversion failed at " << end_ptr - v << " : " << *end_ptr);
        return false;
    }

    return true;
}

inline bool convert::is_valid_errno(decltype(errno) errno_cache) noexcept
{
    // check errno
    if (errno_cache == ERANGE)
    {
        IOX_LOG(DEBUG, "ERANGE triggered during conversion");
        return false;
    }

    if (errno_cache == EINVAL)
    {
        IOX_LOG(DEBUG, "EINVAL triggered during conversion");
        return false;
    }

    return true;
}

template <typename TargetType, typename RequireCheckValType>
inline bool convert::is_within_range(const RequireCheckValType& require_check_val) noexcept
{
    if constexpr (std::is_arithmetic_v<TargetType>)
    {
        // out of range (upper bound)
        if (require_check_val > std::numeric_limits<TargetType>::max())
        {
            IOX_LOG(DEBUG,
                    require_check_val << " is out of range (upper bound), should be less than "
                                      << std::numeric_limits<TargetType>::max());
            return false;
        }

        // out of range (lower bound)
        if (require_check_val < std::numeric_limits<TargetType>::lowest())
        {
            IOX_LOG(DEBUG,
                    require_check_val << " is out of range (lower bound), should be larger than "
                                      << std::numeric_limits<TargetType>::lowest());
            return false;
        }
    }

    return true;
}

} // namespace iox

#endif // IOX_HOOFS_UTILITY_CONVERT_INL
