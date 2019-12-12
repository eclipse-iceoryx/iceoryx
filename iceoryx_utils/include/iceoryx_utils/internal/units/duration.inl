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

#include "iceoryx_utils/internal/units/duration.hpp"

namespace iox
{
namespace units
{
template <typename T>
constexpr Duration Duration::nanoseconds(const T ns)
{
    return operator"" _ns(ns);
}
template <typename T>
constexpr Duration Duration::microseconds(const T us)
{
    return operator"" _us(us);
}
template <typename T>
constexpr Duration Duration::milliseconds(const T ms)
{
    return operator"" _ms(ms);
}
template <typename T>
constexpr Duration Duration::seconds(const T seconds)
{
    return operator"" _s(seconds);
}
template <typename T>
constexpr Duration Duration::minutes(const T min)
{
    return operator"" _m(min);
}
template <typename T>
constexpr Duration Duration::hours(const T hours)
{
    return operator"" _h(hours);
}
template <typename T>
constexpr Duration Duration::days(const T days)
{
    return operator"" _d(days);
}
inline constexpr Duration::Duration(const struct timeval& value)
    : Duration(static_cast<long double>(value.tv_sec) + static_cast<long double>(value.tv_usec) / 1000000.0)
{
}

inline constexpr Duration::Duration(const struct timespec& value)
    : Duration(static_cast<long double>(value.tv_sec) + static_cast<long double>(value.tv_nsec) / 1000000000.0)
{
}

inline constexpr Duration::Duration(const struct itimerspec& value)
    : Duration(value.it_interval)
{
}

inline constexpr Duration::Duration(const std::chrono::milliseconds& value)
    : durationInSeconds(static_cast<long double>(value.count()) / 1000.0)
{
}

inline constexpr Duration::Duration(const std::chrono::nanoseconds& value)
    : durationInSeconds(static_cast<long double>(value.count()) / 1000000000.0)
{
}

inline Duration& Duration::operator=(const std::chrono::milliseconds& right)
{
    durationInSeconds = static_cast<long double>(right.count()) / 1000.0;
    return *this;
}

inline constexpr Duration::Duration(const long double durationInSeconds)
    : durationInSeconds(durationInSeconds >= 0.0 ? durationInSeconds : 0.0)
{
}

template <typename T>
inline constexpr T Duration::nanoSeconds() const
{
    return static_cast<T>(durationInSeconds * 1000000000.0);
}

template <typename T>
inline constexpr T Duration::microSeconds() const
{
    return static_cast<T>(durationInSeconds * 1000000.0);
}

template <typename T>
inline constexpr T Duration::milliSeconds() const
{
    return static_cast<T>(durationInSeconds * 1000.0);
}

template <typename T>
inline constexpr T Duration::seconds() const
{
    return static_cast<T>(durationInSeconds);
}

template <typename T>
inline constexpr T Duration::minutes() const
{
    return static_cast<T>(durationInSeconds / 60.0);
}

template <typename T>
inline constexpr T Duration::hours() const
{
    return static_cast<T>(durationInSeconds / 3600.0);
}

template <typename T>
inline constexpr T Duration::days() const
{
    return static_cast<T>(durationInSeconds / (24.0 * 3600.0));
}

inline constexpr Duration::operator timeval() const
{
    using SEC_TYPE = decltype(timeval::tv_sec);
    using USEC_TYPE = decltype(timeval::tv_usec);
    return {this->seconds<SEC_TYPE>(), this->microSeconds<USEC_TYPE>() - this->seconds<USEC_TYPE>() * 1000000};
}

inline constexpr bool Duration::operator<(const Duration& right) const
{
    return durationInSeconds < right.durationInSeconds;
}

inline constexpr bool Duration::operator>(const Duration& right) const
{
    return durationInSeconds > right.durationInSeconds;
}

inline constexpr bool Duration::operator>=(const Duration& right) const
{
    return durationInSeconds >= right.durationInSeconds;
}

inline constexpr bool Duration::operator<=(const Duration& right) const
{
    return durationInSeconds <= right.durationInSeconds;
}

inline constexpr Duration Duration::operator+(const Duration& right) const
{
    return Duration{durationInSeconds + right.durationInSeconds};
}

inline constexpr Duration Duration::operator-(const Duration& right) const
{
    return Duration{durationInSeconds - right.durationInSeconds};
}

inline constexpr Duration Duration::operator*(const Duration& right) const
{
    return Duration{durationInSeconds * right.durationInSeconds};
}

inline constexpr Duration Duration::operator/(const Duration& right) const
{
    return Duration{durationInSeconds / right.durationInSeconds};
}

template <typename T>
inline constexpr Duration Duration::operator*(const T& right) const
{
    return Duration{durationInSeconds * static_cast<long double>(right)};
}

template <typename T>
inline constexpr Duration Duration::operator/(const T& right) const
{
    return Duration{durationInSeconds / static_cast<long double>(right)};
}

inline namespace duration_literals
{
inline constexpr Duration operator"" _ns(long double value)
{
    return Duration{value / 1000000000.0};
}

inline constexpr Duration operator"" _ns(unsigned long long int value)
{
    return Duration{static_cast<long double>(value) / 1000000000.0};
}

inline constexpr Duration operator"" _us(long double value)
{
    return Duration{value / 1000000.0};
}

inline constexpr Duration operator"" _us(unsigned long long int value)
{
    return Duration{static_cast<long double>(value) / 1000000.0};
}

inline constexpr Duration operator"" _ms(long double value)
{
    return Duration{value / 1000.0};
}

inline constexpr Duration operator"" _ms(unsigned long long int value)
{
    return Duration{static_cast<long double>(value) / 1000.0};
}

inline constexpr Duration operator"" _s(long double value)
{
    return Duration{value};
}

inline constexpr Duration operator"" _s(unsigned long long int value)
{
    return Duration{static_cast<long double>(value)};
}

inline constexpr Duration operator"" _m(long double value)
{
    return Duration{value * 60.0};
}

inline constexpr Duration operator"" _m(unsigned long long int value)
{
    return Duration{static_cast<long double>(value) * 60.0};
}

inline constexpr Duration operator"" _h(long double value)
{
    return Duration{value * 3600.0};
}

inline constexpr Duration operator"" _h(unsigned long long int value)
{
    return Duration{static_cast<long double>(value) * 3600.0};
}

inline constexpr Duration operator"" _d(long double value)
{
    return Duration{value * 24.0 * 3600.0};
}

inline constexpr Duration operator"" _d(unsigned long long int value)
{
    return Duration{static_cast<long double>(value) * 24.0 * 3600.0};
}

} // namespace duration_literals

template <typename T>
inline constexpr Duration operator*(const T& left, const Duration& right)
{
    return Duration::seconds(static_cast<long double>(left) * right.seconds<long double>());
}

template <typename T>
inline constexpr Duration operator/(const T& left, const Duration& right)
{
    return Duration::seconds(static_cast<long double>(left) / right.seconds<long double>());
}

} // namespace units
} // namespace iox
