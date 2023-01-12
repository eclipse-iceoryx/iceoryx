// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_UNITS_DURATION_HPP
#define IOX_HOOFS_UNITS_DURATION_HPP

#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logstream.hpp"
#include "iceoryx_platform/time.hpp" // required for QNX
#include "iox/expected.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>

namespace iox
{
namespace units
{
enum class TimeSpecReference : uint8_t
{
    None,
    Epoch,
    Monotonic
};

class Duration;

namespace duration_literals
{
/// @brief Constructs a new Duration object from nanoseconds
constexpr Duration operator"" _ns(unsigned long long int value) noexcept;

/// @brief Constructs a new Duration object from microseconds
constexpr Duration operator"" _us(unsigned long long int value) noexcept;

/// @brief Constructs a new Duration object from milliseconds
constexpr Duration operator"" _ms(unsigned long long int value) noexcept;

/// @brief Constructs a new Duration object from seconds
constexpr Duration operator"" _s(unsigned long long int value) noexcept;

/// @brief Constructs a new Duration object from minutes
constexpr Duration operator"" _m(unsigned long long int value) noexcept;

/// @brief Constructs a new Duration object from hours
constexpr Duration operator"" _h(unsigned long long int value) noexcept;

/// @brief Constructs a new Duration object from days
constexpr Duration operator"" _d(unsigned long long int value) noexcept;
} // namespace duration_literals

/// @code
///   #include <iostream>
///   // ...
///   using namespace units;
///   using namespace units::duration_literals;
///   auto someDays = 2 * 7_d + 5_ns;
///   auto someSeconds = 42_s + 500_ms;
///   IOX_LOG(INFO) << someDays;
///   IOX_LOG(INFO) << someDays.nanoSeconds<uint64_t>() << " ns";
///   IOX_LOG(INFO) << someSeconds.milliSeconds<int64_t>() << " ms";
/// @endcode
class Duration
{
  public:
    // BEGIN CREATION FROM STATIC FUNCTIONS

    /// @brief Constructs a new Duration object from nanoseconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as nanoseconds
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    template <typename T>
    static constexpr Duration fromNanoseconds(const T value) noexcept;

    /// @brief Constructs a new Duration object from microseconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as microseconds
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    template <typename T>
    static constexpr Duration fromMicroseconds(const T value) noexcept;

    /// @brief Constructs a new Duration object from milliseconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as milliseconds
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    template <typename T>
    static constexpr Duration fromMilliseconds(const T value) noexcept;

    /// @brief Constructs a new Duration object from seconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as seconds
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    template <typename T>
    static constexpr Duration fromSeconds(const T value) noexcept;

    /// @brief Constructs a new Duration object from minutes
    /// @tparam T is an integer type for the value
    /// @param[in] value as minutes
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    template <typename T>
    static constexpr Duration fromMinutes(const T value) noexcept;

    /// @brief Constructs a new Duration object from hours
    /// @tparam T is an integer type for the value
    /// @param[in] value as hours
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    template <typename T>
    static constexpr Duration fromHours(const T value) noexcept;

    /// @brief Constructs a new Duration object from days
    /// @tparam T is an integer type for the value
    /// @param[in] value as days
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    template <typename T>
    static constexpr Duration fromDays(const T value) noexcept;

    /// @brief Constructs a new Duration object of maximum allowed length. Useful for functions which should have an
    /// "infinite" timeout.
    static constexpr Duration max() noexcept;

    /// @brief Constructs a new Duration object with a duration of zero
    static constexpr Duration zero() noexcept;
    // END CREATION FROM STATIC FUNCTIONS

    // BEGIN CONSTRUCTORS AND ASSIGNMENT

    /// @brief Construct a Duration object from timeval
    /// @param[in] value as timeval
    constexpr explicit Duration(const struct timeval& value) noexcept;

    /// @brief Construct a Duration object from timespec
    /// @param[in] value as timespec
    constexpr explicit Duration(const struct timespec& value) noexcept;

    /// @brief Construct a Duration object from itimerspec
    /// @param[in] value as itimerspec
    /// @note only it_interval from the itimerspec is used
    constexpr explicit Duration(const struct itimerspec& value) noexcept;

    /// @brief Construct a Duration object from std::chrono::milliseconds
    /// @param[in] value as milliseconds
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    constexpr explicit Duration(const std::chrono::milliseconds& value) noexcept;

    /// @brief Construct a Duration object from std::chrono::nanoseconds
    /// @param[in] value as nanoseconds
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    constexpr explicit Duration(const std::chrono::nanoseconds& value) noexcept;

    /// @brief Assigns a std::chrono::milliseconds to an duration object
    /// @param[in] rhs is the right hand side of the assignment
    /// @return a reference to the Duration object with the assigned millisecond value
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    Duration& operator=(const std::chrono::milliseconds& rhs) noexcept;

    // END CONSTRUCTORS AND ASSIGNMENT

    // BEGIN COMPARISON

    /// @brief Equal to operator
    /// @param[in] rhs is the right hand side of the comparison
    /// @return true if duration equal to rhs
    constexpr bool operator==(const Duration& rhs) const noexcept;

    /// @brief Not equal to operator
    /// @param[in] rhs is the right hand side of the comparison
    /// @return true if duration not equal to rhs
    constexpr bool operator!=(const Duration& rhs) const noexcept;

    /// @brief Less than operator
    /// @param[in] rhs is the right hand side of the comparison
    /// @return true if duration is less than rhs
    constexpr bool operator<(const Duration& rhs) const noexcept;

    /// @brief Less than or equal to operator
    /// @param[in] rhs is the right hand side of the comparison
    /// @return true if duration is less than or equal to rhs
    constexpr bool operator<=(const Duration& rhs) const noexcept;

    /// @brief Greater than operator
    /// @param[in] rhs is the right hand side of the comparison
    /// @return true if duration is greater than rhs
    constexpr bool operator>(const Duration& rhs) const noexcept;

    /// @brief Greater than or equal to operator
    /// @param[in] rhs is the right hand side of the comparison
    /// @return true if duration is greater than or equal to rhs
    constexpr bool operator>=(const Duration& rhs) const noexcept;

    // END COMPARISON

    // BEGIN ARITHMETIC

    /// @brief Creates Duration object by addition. On overflow duration
    ///        saturates to Duration::max().
    /// @param[in] rhs is the second summand
    /// @return a new Duration object
    constexpr Duration operator+(const Duration& rhs) const noexcept;

    /// @brief Creates Duration object by subtraction. On underflow duration
    ///        saturates to Duration::zero().
    /// @param[in] rhs is the subtrahend
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    constexpr Duration operator-(const Duration& rhs) const noexcept;

    /// @brief Creates Duration object by multiplication.
    /// @tparam T is an arithmetic type for the multiplicator
    /// @param[in] rhs is the multiplicator
    /// @return a new Duration object
    /// @attention Since negative durations are not allowed, the duration will be clamped to 0
    /// @note A duration of 0 will always result in 0, no matter if multiplied with NaN or +Inf
    /// @note There is no explicit division operator! This can be achieved by multiplication with the inverse of the
    /// divisor.
    /// @note Multiplication of a non-zero duration with NaN and +Inf results in a saturated max duration
    template <typename T>
    constexpr Duration operator*(const T& rhs) const noexcept;

    // END ARITHMETIC

    // BEGIN CONVERSION

    /// @brief returns the duration in nanoseconds
    /// @note If the duration in nanoseconds is larger than an uint64_t can represent, it will be clamped to the
    /// uint64_t max value.
    constexpr uint64_t toNanoseconds() const noexcept;

    /// @brief returns the duration in microseconds
    /// @note If the duration in microseconds is larger than an uint64_t can represent, it will be clamped to the
    /// uint64_t max value.
    /// @note The remaining nanoseconds are truncated, similar to the casting behavior of a float to an int.
    constexpr uint64_t toMicroseconds() const noexcept;

    /// @brief returns the duration in milliseconds
    /// @note If the duration in milliseconds is larger than an uint64_t can represent, it will be clamped to the
    /// uint64_t max value.
    /// @note The remaining microseconds are truncated, similar to the casting behavior of a float to an int.
    constexpr uint64_t toMilliseconds() const noexcept;

    /// @brief returns the duration in seconds
    /// @note The remaining milliseconds are truncated, similar to the casting behavior of a float to an int.
    constexpr uint64_t toSeconds() const noexcept;

    /// @brief returns the duration in minutes
    /// @note The remaining seconds are truncated, similar to the casting behavior of a float to an int.
    constexpr uint64_t toMinutes() const noexcept;

    /// @brief returns the duration in hours
    /// @note The remaining minutes are truncated, similar to the casting behavior of a float to an int.
    constexpr uint64_t toHours() const noexcept;

    /// @brief returns the duration in days
    /// @note The remaining hours are truncated, similar to the casting behavior of a float to an int.
    constexpr uint64_t toDays() const noexcept;

    /// @brief converts duration in a timespec c struct
    struct timespec timespec(const TimeSpecReference& reference = TimeSpecReference::None) const noexcept;

    /// @brief converts duration in a timeval c struct
    ///     timeval::tv_sec = seconds since the Epoch (01.01.1970)
    ///     timeval::tv_usec = microseconds
    constexpr struct timeval timeval() const noexcept;

    // END CONVERSION

    friend constexpr Duration duration_literals::operator"" _ns(unsigned long long int value) noexcept;
    friend constexpr Duration duration_literals::operator"" _us(unsigned long long int value) noexcept;
    friend constexpr Duration duration_literals::operator"" _ms(unsigned long long int value) noexcept;
    friend constexpr Duration duration_literals::operator"" _s(unsigned long long int value) noexcept;
    friend constexpr Duration duration_literals::operator"" _m(unsigned long long int value) noexcept;
    friend constexpr Duration duration_literals::operator"" _h(unsigned long long int value) noexcept;
    friend constexpr Duration duration_literals::operator"" _d(unsigned long long int value) noexcept;

    template <typename T>
    friend constexpr Duration operator*(const T& lhs, const Duration& rhs) noexcept;

    friend std::ostream& operator<<(std::ostream& stream, const Duration& t) noexcept;
    friend iox::log::LogStream& operator<<(iox::log::LogStream& stream, const Duration t) noexcept;

    static constexpr uint32_t SECS_PER_MINUTE{60U};
    static constexpr uint32_t SECS_PER_HOUR{3600U};
    static constexpr uint32_t HOURS_PER_DAY{24U};

    static constexpr uint32_t MILLISECS_PER_SEC{1000U};
    static constexpr uint32_t MICROSECS_PER_SEC{MILLISECS_PER_SEC * 1000U};

    static constexpr uint32_t NANOSECS_PER_MICROSEC{1000U};
    static constexpr uint32_t NANOSECS_PER_MILLISEC{NANOSECS_PER_MICROSEC * 1000U};
    static constexpr uint32_t NANOSECS_PER_SEC{NANOSECS_PER_MILLISEC * 1000U};

  protected:
    using Seconds_t = uint64_t;
    using Nanoseconds_t = uint32_t;

    /// @brief Constructs a Duration from seconds and nanoseconds
    /// @param[in] seconds portion of the duration
    /// @param[in] nanoseconds portion of the duration
    /// @note this is protected to be able to use it in unit tests
    constexpr Duration(const Seconds_t seconds, const Nanoseconds_t nanoseconds) noexcept;

    /// @note this is factory method is necessary to build with msvc due to issues calling a protected constexpr ctor
    /// from public methods
    static constexpr Duration createDuration(const Seconds_t seconds, const Nanoseconds_t nanoseconds) noexcept;

  private:
    template <typename T>
    static constexpr unsigned long long int positiveValueOrClampToZero(const T value) noexcept;

    template <typename T>
    constexpr Duration fromFloatingPointSeconds(const T floatingPointSeconds) const noexcept;
    template <typename From, typename To>
    constexpr bool wouldCastFromFloatingPointProbablyOverflow(const From floatingPoint) const noexcept;

    template <typename T>
    constexpr Duration multiplyWith(const std::enable_if_t<!std::is_floating_point<T>::value, T>& rhs) const noexcept;

    template <typename T>
    constexpr Duration multiplyWith(const std::enable_if_t<std::is_floating_point<T>::value, T>& rhs) const noexcept;

  private:
    Seconds_t m_seconds{0U};
    Nanoseconds_t m_nanoseconds{0U};
};

/// @brief creates Duration object by multiplying object T with a duration. On overflow
///        duration will saturate to Duration::max()
/// @tparam T is an arithmetic type for the multiplicator
/// @param[in] lhs is the multiplicator
/// @param[in] rhs is the multiplicant
/// @return a new Duration object
/// @attention Since negative durations are not allowed, the duration will be clamped to 0
template <typename T>
constexpr Duration operator*(const T& lhs, const Duration& rhs) noexcept;

/// @brief stream operator for the Duration class
std::ostream& operator<<(std::ostream& stream, const Duration& t) noexcept;

} // namespace units
} // namespace iox

#include "iceoryx_hoofs/internal/units/duration.inl"

#endif // IOX_HOOFS_UNITS_DURATION_HPP
