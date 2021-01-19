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
#ifndef IOX_UTILS_UNITS_DURATION_HPP
#define IOX_UTILS_UNITS_DURATION_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/platform/time.hpp" // required for QNX

#include <chrono>
#include <cmath>
#include <iostream>

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
/// @brief Constructs a new Duration object from nanoseconds
constexpr Duration operator"" _ns(unsigned long long int); // PRQA S 48

/// @brief Constructs a new Duration object from microseconds
constexpr Duration operator"" _us(unsigned long long int); // PRQA S 48

/// @brief Constructs a new Duration object from milliseconds
constexpr Duration operator"" _ms(unsigned long long int); // PRQA S 48

/// @brief Constructs a new Duration object from seconds
constexpr Duration operator"" _s(unsigned long long int); // PRQA S 48

/// @brief Constructs a new Duration object from minutes
constexpr Duration operator"" _m(unsigned long long int); // PRQA S 48

/// @brief Constructs a new Duration object from hours
constexpr Duration operator"" _h(unsigned long long int); // PRQA S 48

/// @brief Constructs a new Duration object from days
constexpr Duration operator"" _d(unsigned long long int); // PRQA S 48
} // namespace duration_literals

/// @code
///   #include <iostream>
///   // ...
///   using namespace units;
///   using namespace units::duration_literals;
///   auto someDays = 2 * 7_d + 5_ns;
///   auto someSeconds = 42_s + 500_ms;
///   std::cout << someDays << std::endl;
///   std::cout << someDays.nanoSeconds<uint64_t>() << " ns" << std::endl;
///   std::cout << someSeconds.milliSeconds<int64_t>() << " ms" << std::endl;
/// @endcode
class Duration
{
  public:
    // BEGIN CREATION FROM STATIC FUNCTIONS

    /// @brief Constructs a new Duration object from nanoseconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as nanoseconds
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    static constexpr Duration nanoseconds(const T value);

    /// @brief Constructs a new Duration object from microseconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as microseconds
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    static constexpr Duration microseconds(const T value);

    /// @brief Constructs a new Duration object from milliseconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as milliseconds
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    static constexpr Duration milliseconds(const T value);

    /// @brief Constructs a new Duration object from seconds
    /// @tparam T is an integer type for the value
    /// @param[in] value as seconds
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    static constexpr Duration seconds(const T value);

    /// @brief Constructs a new Duration object from minutes
    /// @tparam T is an integer type for the value
    /// @param[in] value as minutes
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    static constexpr Duration minutes(const T value);

    /// @brief Constructs a new Duration object from hours
    /// @tparam T is an integer type for the value
    /// @param[in] value as hours
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    static constexpr Duration hours(const T value);

    /// @brief Constructs a new Duration object from days
    /// @tparam T is an integer type for the value
    /// @param[in] value as days
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    static constexpr Duration days(const T value);

    // END CREATION FROM STATIC FUNCTIONS

    // BEGIN CONSTRUCTORS AND ASSIGNMENT

    /// @brief Constructs a Duration from seconds and nanoseconds
    /// @param[in] seconds portion of the duration
    /// @param[in] nanoseconds portion of the duration
    constexpr Duration(const uint64_t seconds, const uint32_t nanoseconds);

    /// @brief Construct a Duration object from timeval
    /// @param[in] value as timeval
    constexpr explicit Duration(const struct timeval& value);

    /// @brief Construct a Duration object from timespec
    /// @param[in] value as timespec
    constexpr explicit Duration(const struct timespec& value);

    /// @brief Construct a Duration object from itimerspec
    /// @param[in] value as itimerspec
    /// @note only it_interval from the itimerspec is used
    constexpr explicit Duration(const struct itimerspec& value);

    /// @brief Construct a Duration object from std::chrono::milliseconds
    /// @param[in] value as milliseconds
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    constexpr explicit Duration(const std::chrono::milliseconds& value);

    /// @brief Construct a Duration object from std::chrono::nanoseconds
    /// @param[in] value as nanoseconds
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    constexpr explicit Duration(const std::chrono::nanoseconds& value);

    /// @brief Assigns a std::chrono::milliseconds to an duration object
    /// @param[in] right hand side of the assignment
    /// @return a reference to the Duration object with the assigned millisecond value
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    Duration& operator=(const std::chrono::milliseconds& right);

    // END CONSTRUCTORS AND ASSIGNMENT

    // BEGIN COMPARISON

    /// @brief Equal to operator
    /// @param[in] right hand side of the comparison
    /// @return true if duration equal to right
    constexpr bool operator==(const Duration& right) const;

    /// @brief Not equal to operator
    /// @param[in] right hand side of the comparison
    /// @return true if duration not equal to right
    constexpr bool operator!=(const Duration& right) const;

    /// @brief Less than operator
    /// @param[in] right hand side of the comparison
    /// @return true if duration is less than right
    constexpr bool operator<(const Duration& right) const;

    /// @brief Less than or equal to operator
    /// @param[in] right hand side of the comparison
    /// @return true if duration is less than or equal to right
    constexpr bool operator<=(const Duration& right) const;

    /// @brief Greater than operator
    /// @param[in] right hand side of the comparison
    /// @return true if duration is greater than right
    constexpr bool operator>(const Duration& right) const;

    /// @brief Greater than or equal to operator
    /// @param[in] right hand side of the comparison
    /// @return true if duration is greater than or equal to right
    constexpr bool operator>=(const Duration& right) const;

    // END COMPARISON

    // BEGIN ARITHMETIC

    /// @brief creates Duration object by adding right
    /// @param[in] right is the second summand
    /// @return a new Duration object
    constexpr Duration operator+(const Duration& right) const;

    /// @brief creates Duration object by subtracting right
    /// @param[in] right is the subtrahend
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    constexpr Duration operator-(const Duration& right) const;

    /// @brief creates Duration object by multiplication
    /// @tparam T is an arithmetic type for the multiplicator
    /// @param[in] right is the multiplicator
    /// @return a new Duration object
    /// @attention since negative durations are not allowed, the duration will be capped to 0
    template <typename T>
    constexpr Duration operator*(const T& right) const;

    /// @brief creates Duration object by division
    /// @tparam T is an arithmetic type for the divisor
    /// @param[in] right is the divisor
    /// @return a new Duration object
    template <typename T>
    constexpr Duration operator/(const T& right) const;

    // END ARITHMETIC

    // BEGIN CONVERSION

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

    /// @brief converts duration in a timespec c struct
    struct timespec timespec(const TimeSpecReference& reference = TimeSpecReference::None) const;

    /// @brief converts duration in a timeval c struct
    ///     timeval::tv_sec = seconds since the Epoch (01.01.1970)
    ///     timeval::tv_usec = microseconds
    constexpr operator struct timeval() const;

    // END CONVERSION

    friend constexpr Duration duration_literals::operator"" _ns(unsigned long long int); // PRQA S 48
    friend constexpr Duration duration_literals::operator"" _us(unsigned long long int); // PRQA S 48
    friend constexpr Duration duration_literals::operator"" _ms(unsigned long long int); // PRQA S 48
    friend constexpr Duration duration_literals::operator"" _s(unsigned long long int);  // PRQA S 48
    friend constexpr Duration duration_literals::operator"" _m(unsigned long long int);  // PRQA S 48
    friend constexpr Duration duration_literals::operator"" _h(unsigned long long int);  // PRQA S 48
    friend constexpr Duration duration_literals::operator"" _d(unsigned long long int);  // PRQA S 48

    template <typename T>
    friend constexpr Duration operator*(const T& left, const Duration& right);

    friend std::ostream& operator<<(std::ostream& stream, const Duration& t);

  private:
    template <typename T>
    inline constexpr Duration
    multiplySeconds(const uint64_t seconds, const std::enable_if_t<!std::is_floating_point<T>::value, T>& right) const;
    template <typename T>
    inline constexpr Duration multiplySeconds(const uint64_t seconds,
                                              const std::enable_if_t<std::is_floating_point<T>::value, T>& right) const;
    template <typename T>
    inline constexpr Duration
    multiplyNanoseconds(const uint32_t nanoseconds,
                        const std::enable_if_t<!std::is_floating_point<T>::value, T>& right) const;
    template <typename T>
    inline constexpr Duration
    multiplyNanoseconds(const uint32_t nanoseconds,
                        const std::enable_if_t<std::is_floating_point<T>::value, T>& right) const;

    static constexpr uint32_t SECS_PER_MINUTE{60U};
    static constexpr uint32_t SECS_PER_HOUR{3600U};
    static constexpr uint32_t HOURS_PER_DAY{24U};

    static constexpr uint32_t MILLISECS_PER_SEC{1000U};
    static constexpr uint32_t MICROSECS_PER_SEC{MILLISECS_PER_SEC * 1000U};

    static constexpr uint32_t NANOSECS_PER_MICROSEC{1000U};
    static constexpr uint32_t NANOSECS_PER_MILLISEC{NANOSECS_PER_MICROSEC * 1000U};
    static constexpr uint32_t NANOSECS_PER_SEC{NANOSECS_PER_MILLISEC * 1000U};

    static_assert(NANOSECS_PER_SEC == 1000U * MICROSECS_PER_SEC, "Mismatch in calculation for conversion constants!");

  private:
    uint64_t m_seconds{0U};
    uint32_t m_nanoseconds{0U};
};

/// @brief creates Duration object by multiplying object T with a duration
/// @tparam T is an arithmetic type for the multiplicator
/// @param[in] left is the multiplicator
/// @param[in] right is the multiplicant
/// @return a new Duration object
/// @attention since negative durations are not allowed, the duration will be capped to 0
template <typename T>
constexpr Duration operator*(const T& left, const Duration& right);

/// @brief stream operator for the Duration class
std::ostream& operator<<(std::ostream& stream, const Duration& t);

} // namespace units
} // namespace iox

#include "iceoryx_utils/internal/units/duration.inl"

#endif // IOX_UTILS_UNITS_DURATION_HPP
