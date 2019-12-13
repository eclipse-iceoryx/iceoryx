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

#pragma once

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

#include <chrono>
#include <iostream>
#include <sys/time.h> // required for QNX
#include <time.h>

namespace iox
{
namespace units
{
enum class TimeSpecReference
{
    None,
    Epoch,
    Monotonic
};

class Duration;

inline namespace duration_literals
{
/// @brief constructs a new Duration object in nanoseconds
constexpr Duration operator"" _ns(long double);

/// @brief constructs a new Duration object in nanoseconds
constexpr Duration operator"" _ns(unsigned long long int);

/// @brief constructs a new Duration object in microseconds
constexpr Duration operator"" _us(long double);

/// @brief constructs a new Duration object in microseconds
constexpr Duration operator"" _us(unsigned long long int);

/// @brief constructs a new Duration object in milliseconds
constexpr Duration operator"" _ms(long double);

/// @brief constructs a new Duration object in milliseconds
constexpr Duration operator"" _ms(unsigned long long int);

/// @brief constructs a new Duration object in seconds
constexpr Duration operator"" _s(long double);

/// @brief constructs a new Duration object in seconds
constexpr Duration operator"" _s(unsigned long long int);

/// @brief constructs a new Duration object in minutes
constexpr Duration operator"" _m(long double);

/// @brief constructs a new Duration object in minutes
constexpr Duration operator"" _m(unsigned long long int);

/// @brief constructs a new Duration object in hours
constexpr Duration operator"" _h(long double);

/// @brief constructs a new Duration object in hours
constexpr Duration operator"" _h(unsigned long long int);

/// @brief constructs a new Duration object in days
constexpr Duration operator"" _d(long double);

/// @brief constructs a new Duration object in days
constexpr Duration operator"" _d(unsigned long long int);
} // namespace duration_literals

/// @code
///   #include <iostream>
///   // ...
///   using namespace units;
///   auto timeInDays = 12_d;
///   auto timeInSeconds = 1.5_s;
///   std::cout << timeInDays << std::endl;
///   std::cout << timeInDays.nanoSeconds<int>() << " ns" << std::endl;
///   std::cout << timeInSeconds.minutes<float>() << " min" << std::endl;
/// @endcode
class Duration
{
  public:
    template <typename T>
    static constexpr Duration nanoseconds(const T ns);
    template <typename T>
    static constexpr Duration microseconds(const T us);
    template <typename T>
    static constexpr Duration milliseconds(const T ms);
    template <typename T>
    static constexpr Duration seconds(const T seconds);
    template <typename T>
    static constexpr Duration minutes(const T min);
    template <typename T>
    static constexpr Duration hours(const T hours);
    template <typename T>
    static constexpr Duration days(const T days);

    /// @brief Construct a Duration object from timeval
    constexpr explicit Duration(const struct timeval& value);

    /// @brief Construct a Duration object from timespec
    constexpr explicit Duration(const struct timespec& value);

    /// @brief Construct a Duration object from itimerspec
    constexpr explicit Duration(const struct itimerspec& value);

    /// @brief Construct a Duration object from std::chrono::milliseconds
    constexpr explicit Duration(const std::chrono::milliseconds& value);

    /// @brief Construct a Duration object from std::chrono::nanoseconds
    constexpr explicit Duration(const std::chrono::nanoseconds& value);

    /// @brief Assigns a std::chrono::milliseconds to an duration object
    Duration& operator=(const std::chrono::milliseconds& right);

    /// @brief return true if durationInSeconds is larger or equal than right
    constexpr bool operator<(const Duration& right) const;

    /// @brief return true if durationInSeconds is larger or equal than right
    constexpr bool operator>(const Duration& right) const;

    /// @brief return true if durationInSeconds is larger or equal than right
    constexpr bool operator>=(const Duration& right) const;

    /// @brief returns true if right is larger or equal than durationInSeconds
    constexpr bool operator<=(const Duration& right) const;

    /// @brief creates Duration object by adding right and durationInSeconds
    constexpr Duration operator+(const Duration& right) const;

    /// @brief creates Duration object by subtracting right and durationInSeconds
    constexpr Duration operator-(const Duration& right) const;

    /// @brief creates Duration object by multiplying right and durationInSeconds
    constexpr Duration operator*(const Duration& right) const;

    /// @brief creates Duration object by dividing right and durationInSeconds
    constexpr Duration operator/(const Duration& right) const;

    /// @brief creates Duration object multplying durationInSeconds with T
    template <typename T>
    constexpr Duration operator*(const T& right) const;

    /// @brief creates Duration object dividing durationInSeconds through T
    template <typename T>
    constexpr Duration operator/(const T& right) const;

    /// @brief returns the duration in nanoseconds
    template <typename T>
    constexpr T nanoSeconds() const;

    /// @brief returns the duration in microseconds
    template <typename T>
    constexpr T microSeconds() const;

    /// @brief returns the duration in milliseconds
    template <typename T>
    constexpr T milliSeconds() const;

    /// @brief returns the duration in seconds
    template <typename T>
    constexpr T seconds() const;

    /// @brief returns the duration in minutes
    template <typename T>
    constexpr T minutes() const;

    /// @brief returns the duration in hours
    template <typename T>
    constexpr T hours() const;

    /// @brief returns the duration in days
    template <typename T>
    constexpr T days() const;

    /// @brief converts duration in a timeval c struct
    ///     timeval::tv_sec = seconds since the Epoch (01.01.1970)
    ///     timeval::tv_usec = microseconds
    constexpr operator struct timeval() const;

    /// @brief converts time in a timespec c struct
    struct timespec timespec(const TimeSpecReference& reference = TimeSpecReference::None) const;

    //@ brief Make operators accessible, that have to be defined outside the class
    // template <typename T>
    // friend constexpr Duration operator*(const T& left, const Duration& right);
    // template <typename T>
    // friend constexpr Duration operator/(const T& left, const Duration& right);
    friend std::ostream& operator<<(std::ostream& stream, const Duration& t);
    friend constexpr Duration duration_literals::operator"" _ns(long double);
    friend constexpr Duration duration_literals::operator"" _ns(unsigned long long int);
    friend constexpr Duration duration_literals::operator"" _us(long double);
    friend constexpr Duration duration_literals::operator"" _us(unsigned long long int);
    friend constexpr Duration duration_literals::operator"" _ms(long double);
    friend constexpr Duration duration_literals::operator"" _ms(unsigned long long int);
    friend constexpr Duration duration_literals::operator"" _s(long double);
    friend constexpr Duration duration_literals::operator"" _s(unsigned long long int);
    friend constexpr Duration duration_literals::operator"" _m(long double);
    friend constexpr Duration duration_literals::operator"" _m(unsigned long long int);
    friend constexpr Duration duration_literals::operator"" _h(long double);
    friend constexpr Duration duration_literals::operator"" _h(unsigned long long int);
    friend constexpr Duration duration_literals::operator"" _d(long double);
    friend constexpr Duration duration_literals::operator"" _d(unsigned long long int);

  private:
    /// @brief constructor needs to be private to ensure a unit safe usage of duration
    constexpr explicit Duration(const long double durationInSeconds);
    long double durationInSeconds;
};

/// @brief creates Duration object multplying T with durationInSeconds
template <typename T>
constexpr Duration operator*(const T& left, const Duration& right);

/// @brief creates Duration object dividing T through durationInSeconds
template <typename T>
constexpr Duration operator/(const T& left, const Duration& right);

/// @brief stream operator for the Duration class
std::ostream& operator<<(std::ostream& stream, const Duration& t);

} // namespace units
} // namespace iox

#include "iceoryx_utils/internal/units/duration.inl"
