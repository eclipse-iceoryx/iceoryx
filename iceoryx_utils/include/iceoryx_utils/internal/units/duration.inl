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
constexpr Duration Duration::nanoseconds(const T value) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Clampling negative value '" << value << "' to zero!" << std::endl;
        return Duration{0U, 0U};
    }
    return operator"" _ns(static_cast<unsigned long long int>(value));
}
template <typename T>
constexpr Duration Duration::microseconds(const T value) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Clampling negative value '" << value << "' to zero!" << std::endl;
        return Duration{0U, 0U};
    }
    return operator"" _us(static_cast<unsigned long long int>(value));
}
template <typename T>
constexpr Duration Duration::milliseconds(const T value) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Clampling negative value '" << value << "' to zero!" << std::endl;
        return Duration{0U, 0U};
    }
    return operator"" _ms(static_cast<unsigned long long int>(value));
}
template <typename T>
constexpr Duration Duration::seconds(const T value) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Clampling negative value '" << value << "' to zero!" << std::endl;
        return Duration{0U, 0U};
    }
    return operator"" _s(static_cast<unsigned long long int>(value));
}
template <typename T>
constexpr Duration Duration::minutes(const T value) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Clampling negative value '" << value << "' to zero!" << std::endl;
        return Duration{0U, 0U};
    }
    return operator"" _m(static_cast<unsigned long long int>(value));
}
template <typename T>
constexpr Duration Duration::hours(const T value) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Clampling negative value '" << value << "' to zero!" << std::endl;
        return Duration{0U, 0U};
    }
    return operator"" _h(static_cast<unsigned long long int>(value));
}
template <typename T>
constexpr Duration Duration::days(const T value) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    if (value < 0)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Clampling negative value '" << value << "' to zero!" << std::endl;
        return Duration{0U, 0U};
    }
    return operator"" _d(static_cast<unsigned long long int>(value));
}


inline constexpr Duration::Duration(const uint64_t seconds, const uint32_t nanoseconds) noexcept
    : m_seconds(seconds)
    , m_nanoseconds(nanoseconds)
{
    if (nanoseconds >= NANOSECS_PER_SEC)
    {
        m_seconds += nanoseconds / NANOSECS_PER_SEC;
        m_nanoseconds = m_nanoseconds % NANOSECS_PER_SEC;
    }
}

inline constexpr Duration::Duration(const struct timeval& value) noexcept
    : Duration(static_cast<uint64_t>(value.tv_sec), static_cast<uint32_t>(value.tv_usec) * NANOSECS_PER_MICROSEC)
{
}

inline constexpr Duration::Duration(const struct timespec& value) noexcept
    : Duration(static_cast<uint64_t>(value.tv_sec), static_cast<uint32_t>(value.tv_nsec))
{
}

inline constexpr Duration::Duration(const struct itimerspec& value) noexcept
    : Duration(value.it_interval)
{
}

inline constexpr Duration::Duration(const std::chrono::milliseconds& value) noexcept
{
    *this = Duration::milliseconds(value.count());
}

inline constexpr Duration::Duration(const std::chrono::nanoseconds& value) noexcept
{
    *this = Duration::nanoseconds(value.count());
}

inline Duration& Duration::operator=(const std::chrono::milliseconds& rhs) noexcept
{
    *this = Duration(rhs);
    return *this;
}

template <typename T>
inline constexpr T Duration::nanoSeconds() const noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    return static_cast<T>(m_seconds * NANOSECS_PER_SEC + m_nanoseconds);
}

template <typename T>
inline constexpr T Duration::microSeconds() const noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    return static_cast<T>(m_seconds * MICROSECS_PER_SEC + m_nanoseconds / NANOSECS_PER_MICROSEC);
}

template <typename T>
inline constexpr T Duration::milliSeconds() const noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    return static_cast<T>(m_seconds * MILLISECS_PER_SEC + m_nanoseconds / NANOSECS_PER_MILLISEC);
}

template <typename T>
inline constexpr T Duration::seconds() const noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds);
}

template <typename T>
inline constexpr T Duration::minutes() const noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds / SECS_PER_MINUTE);
}

template <typename T>
inline constexpr T Duration::hours() const noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds / SECS_PER_HOUR);
}

template <typename T>
inline constexpr T Duration::days() const noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "only integer are supported");

    /// @todo decide if we want an overflow or saturation if the result is out of range for T
    // since currently only integers are supported, the nanoseconds would be rounded off and can be omitted
    return static_cast<T>(m_seconds / (HOURS_PER_DAY * SECS_PER_HOUR));
}

inline constexpr Duration::operator timeval() const noexcept
{
    using SEC_TYPE = decltype(timeval::tv_sec);
    using USEC_TYPE = decltype(timeval::tv_usec);
    return {static_cast<SEC_TYPE>(m_seconds), static_cast<USEC_TYPE>(m_nanoseconds / NANOSECS_PER_MICROSEC)};
}

inline constexpr bool Duration::operator==(const Duration& rhs) const noexcept
{
    return (m_seconds == rhs.m_seconds) && (m_nanoseconds == rhs.m_nanoseconds);
}

inline constexpr bool Duration::operator!=(const Duration& rhs) const noexcept
{
    return !(*this == rhs);
}

inline constexpr bool Duration::operator<(const Duration& rhs) const noexcept
{
    return (m_seconds < rhs.m_seconds) || ((m_seconds == rhs.m_seconds) && (m_nanoseconds < rhs.m_nanoseconds));
}

inline constexpr bool Duration::operator<=(const Duration& rhs) const noexcept
{
    return (m_seconds < rhs.m_seconds) || ((m_seconds == rhs.m_seconds) && (m_nanoseconds <= rhs.m_nanoseconds));
}

inline constexpr bool Duration::operator>(const Duration& rhs) const noexcept
{
    return (m_seconds > rhs.m_seconds) || ((m_seconds == rhs.m_seconds) && (m_nanoseconds > rhs.m_nanoseconds));
}

inline constexpr bool Duration::operator>=(const Duration& rhs) const noexcept
{
    return (m_seconds > rhs.m_seconds) || ((m_seconds == rhs.m_seconds) && (m_nanoseconds >= rhs.m_nanoseconds));
}

inline constexpr Duration Duration::operator+(const Duration& rhs) const noexcept
{
    /// @todo decide if we want an overflow or saturation

    auto seconds = m_seconds + rhs.m_seconds;
    auto nanoseconds = m_nanoseconds + rhs.m_nanoseconds;
    if (nanoseconds >= NANOSECS_PER_SEC)
    {
        ++seconds;
        nanoseconds -= NANOSECS_PER_SEC;
    }
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration Duration::operator-(const Duration& rhs) const noexcept
{
    if (*this <= rhs)
    {
        std::clog << __PRETTY_FUNCTION__ << ": Result of subtraction would be negative, clamping to zero!" << std::endl;
        return Duration(0U, 0U);
    }
    auto seconds = m_seconds - rhs.m_seconds;
    uint32_t nanoseconds{0U};
    if (m_nanoseconds >= rhs.m_nanoseconds)
    {
        nanoseconds = m_nanoseconds - rhs.m_nanoseconds;
    }
    else
    {
        nanoseconds = NANOSECS_PER_SEC - rhs.m_nanoseconds - m_nanoseconds;
        --seconds;
    }
    return Duration{seconds, nanoseconds};
}

template <typename T>
inline constexpr Duration
Duration::multiplySeconds(const uint64_t seconds,
                          const std::enable_if_t<!std::is_floating_point<T>::value, T>& rhs) const noexcept
{
    // specialization is needed to prevent a clang warning if `rhs` is a signed integer and not casted to unsigned
    // operator*(...) takes care of negative values for rhs
    return Duration(seconds * static_cast<uint64_t>(rhs), 0U);
}

template <typename T>
inline constexpr Duration
Duration::multiplySeconds(const uint64_t seconds,
                          const std::enable_if_t<std::is_floating_point<T>::value, T>& rhs) const noexcept
{
    // operator*(...) takes care of negative values for rhs
    auto result = seconds * rhs;
    double resultSeconds{0.0};
    double secondsFraction = modf(result, &resultSeconds);
    return Duration(static_cast<uint64_t>(resultSeconds), 0U)
           + Duration::nanoseconds(static_cast<uint64_t>(secondsFraction * NANOSECS_PER_SEC));
}

template <typename T>
inline constexpr Duration
Duration::multiplyNanoseconds(const uint32_t nanoseconds,
                              const std::enable_if_t<!std::is_floating_point<T>::value, T>& rhs) const noexcept
{
    // specialization is needed to prevent a clang warning if `rhs` is a signed integer and not casted to unsigned
    // operator*(...) takes care of negative values for rhs
    return Duration::nanoseconds(nanoseconds * static_cast<uint64_t>(rhs));
}

template <typename T>
inline constexpr Duration
Duration::multiplyNanoseconds(const uint32_t nanoseconds,
                              const std::enable_if_t<std::is_floating_point<T>::value, T>& rhs) const noexcept
{
    // operator*(...) takes care of negative values for rhs
    return Duration::nanoseconds(static_cast<uint64_t>(nanoseconds * rhs));
}

template <typename T>
inline constexpr Duration Duration::operator*(const T& rhs) const noexcept
{
    static_assert(std::is_arithmetic<T>::value, "non arithmetic types are not supported for multiplication");

    if (rhs < static_cast<T>(0))
    {
        std::clog << __PRETTY_FUNCTION__ << ": Result of multiplication would be negative, clamping to zero!"
                  << std::endl;
        return Duration{0U, 0U};
    }

    /// @todo decide if we want an overflow or saturation

    return multiplySeconds<T>(m_seconds, rhs) + multiplyNanoseconds<T>(m_nanoseconds, rhs);
}

inline namespace duration_literals
{
inline constexpr Duration operator"" _ns(unsigned long long int value) noexcept // PRQA S 48
{
    auto seconds = static_cast<uint64_t>(value / Duration::NANOSECS_PER_SEC);
    auto nanoseconds = static_cast<uint32_t>(value % Duration::NANOSECS_PER_SEC);
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration operator"" _us(unsigned long long int value) noexcept // PRQA S 48
{
    auto seconds = static_cast<uint64_t>(value / Duration::MICROSECS_PER_SEC);
    auto nanoseconds = static_cast<uint32_t>((value % Duration::MICROSECS_PER_SEC) * Duration::NANOSECS_PER_MICROSEC);
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration operator"" _ms(unsigned long long int value) noexcept // PRQA S 48
{
    auto seconds = static_cast<uint64_t>(value / Duration::MILLISECS_PER_SEC);
    auto nanoseconds = static_cast<uint32_t>((value % Duration::MILLISECS_PER_SEC) * Duration::NANOSECS_PER_MILLISEC);
    return Duration{seconds, nanoseconds};
}

inline constexpr Duration operator"" _s(unsigned long long int value) noexcept // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value), 0U};
}

inline constexpr Duration operator"" _m(unsigned long long int value) noexcept // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value * Duration::SECS_PER_MINUTE), 0U};
}

inline constexpr Duration operator"" _h(unsigned long long int value) noexcept // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value * Duration::SECS_PER_HOUR), 0U};
}

inline constexpr Duration operator"" _d(unsigned long long int value) noexcept // PRQA S 48
{
    return Duration{static_cast<uint64_t>(value * Duration::HOURS_PER_DAY * Duration::SECS_PER_HOUR), 0U};
}

} // namespace duration_literals

template <typename T>
inline constexpr Duration operator*(const T& lhs, const Duration& rhs) noexcept
{
    static_assert(std::is_arithmetic<T>::value, "non arithmetic types are not supported for multiplication");

    if (lhs < static_cast<T>(0))
    {
        std::clog << __PRETTY_FUNCTION__ << ": Result of multiplication would be negative, clamping to zero!"
                  << std::endl;
        return Duration{0U, 0U};
    }

    return rhs * lhs;
}

} // namespace units
} // namespace iox

#endif // IOX_UTILS_UNITS_DURATION_INL
