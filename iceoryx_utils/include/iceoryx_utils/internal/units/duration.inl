// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_UTILS_UNITS_DURATION_INL
#define IOX_UTILS_UNITS_DURATION_INL

#include "iceoryx_utils/internal/units/duration.hpp"

namespace iox
{
namespace units
{
template <typename T>
constexpr Duration Duration::nanoseconds(const T value)
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        return Duration{0U, 0U};
    }
    return operator"" _ns(value);
}
template <typename T>
constexpr Duration Duration::microseconds(const T value)
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        return Duration{0U, 0U};
    }
    return operator"" _us(value);
}
template <typename T>
constexpr Duration Duration::milliseconds(const T value)
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        return Duration{0U, 0U};
    }
    return operator"" _ms(value);
}
template <typename T>
constexpr Duration Duration::seconds(const T value)
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        return Duration{0U, 0U};
    }
    return operator"" _s(value);
}
template <typename T>
constexpr Duration Duration::minutes(const T value)
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        return Duration{0U, 0U};
    }
    return operator"" _m(value);
}
template <typename T>
constexpr Duration Duration::hours(const T value)
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        return Duration{0U, 0U};
    }
    return operator"" _h(value);
}
template <typename T>
constexpr Duration Duration::days(const T value)
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        return Duration{0U, 0U};
    }
    return operator"" _d(value);
}


inline constexpr Duration::Duration(const uint64_t seconds, const uint32_t nanoseconds)
    : m_seconds(seconds)
    , m_nanoseconds(nanoseconds)
{
    if (nanoseconds >= 1000000000U)
    {
        m_seconds += nanoseconds / 1000000000U;
        m_nanoseconds = m_nanoseconds % 1000000000U;
    }
}

inline constexpr Duration::Duration(const struct timeval& value)
    : Duration(static_cast<uint64_t>(value.tv_sec), static_cast<uint32_t>(value.tv_usec) * 1000U)
{
}

inline constexpr Duration::Duration(const struct timespec& value)
    : Duration(static_cast<uint64_t>(value.tv_sec), static_cast<uint32_t>(value.tv_nsec))
{
}

inline constexpr Duration::Duration(const struct itimerspec& value)
    : Duration(value.it_interval)
{
}

inline constexpr Duration::Duration(const std::chrono::milliseconds& value)
{
    auto milliseconds = value.count();
    if (milliseconds > 0)
    {
        *this = operator"" _ms(static_cast<unsigned long long int>(milliseconds));
    }
}

inline constexpr Duration::Duration(const std::chrono::nanoseconds& value)
{
    auto nanoseconds = value.count();
    if (nanoseconds > 0)
    {
        *this = operator"" _ns(static_cast<unsigned long long int>(nanoseconds));
    }
}

inline Duration& Duration::operator=(const std::chrono::milliseconds& right)
{
    *this = Duration(right);
    return *this;
}

template <typename T>
inline constexpr T Duration::nanoSeconds() const
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    return static_cast<T>(m_seconds * 1000000000U + m_nanoseconds);
}

template <typename T>
inline constexpr T Duration::microSeconds() const
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    return static_cast<T>(m_seconds * 1000000U + m_nanoseconds / 1000U);
}

template <typename T>
inline constexpr T Duration::milliSeconds() const
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    return static_cast<T>(m_seconds * 1000U + m_nanoseconds / 1000000U);
}

template <typename T>
inline constexpr T Duration::seconds() const
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds);
}

template <typename T>
inline constexpr T Duration::minutes() const
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds / 60U);
}

template <typename T>
inline constexpr T Duration::hours() const
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds / 3600U);
}

template <typename T>
inline constexpr T Duration::days() const
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds / (24U * 3600U));
}

inline constexpr Duration::operator timeval() const
{
    using SEC_TYPE = decltype(timeval::tv_sec);
    using USEC_TYPE = decltype(timeval::tv_usec);
    return {static_cast<SEC_TYPE>(m_seconds), static_cast<USEC_TYPE>(m_nanoseconds / 1000U)};
}

inline constexpr bool Duration::operator==(const Duration& right) const
{
    return (m_seconds == right.m_seconds) && (m_nanoseconds == right.m_nanoseconds);
}

inline constexpr bool Duration::operator!=(const Duration& right) const
{
    return !(*this == right);
}

inline constexpr bool Duration::operator<(const Duration& right) const
{
    return (m_seconds < right.m_seconds) || ((m_seconds == right.m_seconds) && (m_nanoseconds < right.m_nanoseconds));
}

inline constexpr bool Duration::operator<=(const Duration& right) const
{
    return (m_seconds < right.m_seconds) || ((m_seconds == right.m_seconds) && (m_nanoseconds <= right.m_nanoseconds));
}

inline constexpr bool Duration::operator>(const Duration& right) const
{
    return (m_seconds > right.m_seconds) || ((m_seconds == right.m_seconds) && (m_nanoseconds > right.m_nanoseconds));
}

inline constexpr bool Duration::operator>=(const Duration& right) const
{
    return (m_seconds > right.m_seconds) || ((m_seconds == right.m_seconds) && (m_nanoseconds >= right.m_nanoseconds));
}

inline constexpr Duration Duration::operator+(const Duration& right) const
{
    /// @todo decide if we want an overflow or saturation

    auto seconds = m_seconds + right.m_seconds;
    auto nanoseconds = m_nanoseconds + right.m_nanoseconds;
    if (nanoseconds >= 1000000000U)
    {
        ++seconds;
        nanoseconds -= 1000000000U;
    }
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration Duration::operator-(const Duration& right) const
{
    if (*this <= right)
    {
        return Duration(0U, 0U);
    }
    auto seconds = m_seconds - right.m_seconds;
    uint32_t nanoseconds{0};
    if (m_nanoseconds >= right.m_nanoseconds)
    {
        nanoseconds = m_nanoseconds - right.m_nanoseconds;
    }
    else
    {
        nanoseconds = 1000000000U - right.m_nanoseconds - m_nanoseconds;
        --seconds;
    }
    return Duration{seconds, nanoseconds};
}

template <typename T>
inline constexpr Duration
Duration::multiplySeconds(const uint64_t seconds,
                          const std::enable_if_t<!std::is_floating_point<T>::value, T>& right) const
{
    // specialization is needed to prevent a clang warning if `right` is a signed integer and not casted to unsigned
    // operator*(...) takes care of negative values for right
    return Duration(seconds * static_cast<uint64_t>(right), 0U);
}

template <typename T>
inline constexpr Duration
Duration::multiplySeconds(const uint64_t seconds,
                          const std::enable_if_t<std::is_floating_point<T>::value, T>& right) const
{
    // operator*(...) takes care of negative values for right
    auto result = seconds * right;
    double resultSeconds{0.0};
    double secondsFraction = modf(result, &resultSeconds);
    return Duration(static_cast<uint64_t>(resultSeconds), 0U)
           + Duration::nanoseconds(static_cast<uint64_t>(secondsFraction * 1000000000U));
}

template <typename T>
inline constexpr Duration
Duration::multiplyNanoseconds(const uint32_t nanoseconds,
                              const std::enable_if_t<!std::is_floating_point<T>::value, T>& right) const
{
    // specialization is needed to prevent a clang warning if `right` is a signed integer and not casted to unsigned
    // operator*(...) takes care of negative values for right
    return Duration::nanoseconds(nanoseconds * static_cast<uint64_t>(right));
}

template <typename T>
inline constexpr Duration
Duration::multiplyNanoseconds(const uint32_t nanoseconds,
                              const std::enable_if_t<std::is_floating_point<T>::value, T>& right) const
{
    // operator*(...) takes care of negative values for right
    return Duration::nanoseconds(static_cast<uint64_t>(nanoseconds * right));
}

template <typename T>
inline constexpr Duration Duration::operator*(const T& right) const
{
    static_assert(std::is_arithmetic<T>::value, "non arithmetic types are not supported for multiplication");

    if (right < static_cast<T>(0))
    {
        return Duration{0U, 0U};
    }

    /// @todo decide if we want an overflow or saturation

    return multiplySeconds<T>(m_seconds, right) + multiplyNanoseconds<T>(m_nanoseconds, right);
}

template <typename T>
inline constexpr Duration Duration::operator/(const T& right) const
{
    static_assert(std::is_arithmetic<T>::value, "non arithmetic types are not supported for multiplication");

    if (right < static_cast<T>(0))
    {
        return Duration{0U, 0U};
    }

    auto result = m_seconds / right;
    double seconds{0.0};
    double secondsFraction = modf(result, &seconds);
    auto nanoseconds =
        static_cast<uint64_t>(secondsFraction * 1000000000U) + static_cast<uint64_t>(m_nanoseconds / right);
    return Duration(static_cast<uint64_t>(seconds), 0U) + Duration::nanoseconds(nanoseconds);
}

inline namespace duration_literals
{
inline constexpr Duration operator"" _ns(unsigned long long int value) // PRQA S 48
{
    auto seconds = static_cast<uint64_t>(value / 1000000000U);
    auto nanoseconds = static_cast<uint32_t>(value % 1000000000U);
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration operator"" _us(unsigned long long int value) // PRQA S 48
{
    auto seconds = static_cast<uint64_t>(value / 1000000U);
    auto nanoseconds = static_cast<uint32_t>(value % 1000000U) * 1000U;
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration operator"" _ms(unsigned long long int value) // PRQA S 48
{
    auto seconds = static_cast<uint64_t>(value / 1000U);
    auto nanoseconds = static_cast<uint32_t>(value % 1000U) * 1000000U;
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration operator"" _s(unsigned long long int value) // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value), 0U};
}

inline constexpr Duration operator"" _m(unsigned long long int value) // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value * 60U), 0U};
}

inline constexpr Duration operator"" _h(unsigned long long int value) // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value * 3600U), 0U};
}

inline constexpr Duration operator"" _d(unsigned long long int value) // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value * 24U * 3600U), 0U};
}

} // namespace duration_literals

template <typename T>
inline constexpr Duration operator*(const T& left, const Duration& right)
{
    static_assert(std::is_arithmetic<T>::value, "non arithmetic types are not supported for multiplication");

    if (left < static_cast<T>(0))
    {
        return Duration{0U, 0U};
    }

    return right * left;
}

} // namespace units
} // namespace iox

#endif // IOX_UTILS_UNITS_DURATION_INL
