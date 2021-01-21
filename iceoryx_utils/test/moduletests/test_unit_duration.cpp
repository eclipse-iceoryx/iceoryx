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

#include "iceoryx_utils/internal/units/duration.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::units;

constexpr uint64_t KILO = 1000U;
constexpr uint64_t MEGA = 1000000U;
constexpr uint64_t GIGA = 1000000000U;

// BEGIN CONSTRUCTOR TESTS

TEST(Duration_test, ConstructDurationWithZeroTime)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{0U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{0U};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithResultOfLessNanosecondsThanOneSecond)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{7337U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{NANOSECONDS};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsLessThanOneSecond)
{
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t NANOSECONDS{73U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{SECONDS * GIGA + NANOSECONDS};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsEqualToOneSecond)
{
    constexpr uint64_t SECONDS{13U};
    constexpr uint64_t NANOSECONDS{GIGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(SECONDS + 1U) * GIGA};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsMoreThanOneSecond)
{
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t NANOSECONDS{42U};
    constexpr uint64_t MORE_THAN_ONE_SECOND_NANOSECONDS{GIGA + NANOSECONDS};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(SECONDS + 1U) * GIGA + NANOSECONDS};

    auto sut = Duration{SECONDS, MORE_THAN_ONE_SECOND_NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsMaxValue)
{
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t MAX_NANOSECONDS_FOR_CTOR{std::numeric_limits<uint32_t>::max()};
    constexpr uint64_t EXPECTED_SECONDS = SECONDS + MAX_NANOSECONDS_FOR_CTOR / GIGA;
    constexpr uint64_t REMAINING_NANOSECONDS = MAX_NANOSECONDS_FOR_CTOR % GIGA;
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(EXPECTED_SECONDS)*GIGA + REMAINING_NANOSECONDS};

    auto sut = Duration{SECONDS, MAX_NANOSECONDS_FOR_CTOR};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithSecondsAndNanosecondsMaxValue)
{
    constexpr uint64_t MAX_SECONDS_FOR_CTOR{std::numeric_limits<uint64_t>::max()};
    constexpr uint64_t MAX_NANOSECONDS_FOR_CTOR{std::numeric_limits<uint32_t>::max()};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{std::numeric_limits<uint64_t>::max()};

    auto sut = Duration{MAX_SECONDS_FOR_CTOR, MAX_NANOSECONDS_FOR_CTOR};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithOneNanosecondResultsNotInZeroNanoseconds)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{1U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{NANOSECONDS};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructFromTimespec)
{
    constexpr uint64_t SECONDS{123U};
    constexpr uint64_t NANOSECONDS{456U};

    struct timespec value;
    value.tv_sec = SECONDS;
    value.tv_nsec = NANOSECONDS;

    Duration sut{value};
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(SECONDS * GIGA + NANOSECONDS));
}

TEST(Duration_test, ConstructFromItimerspec)
{
    constexpr uint64_t SECONDS{135U};
    constexpr uint64_t NANOSECONDS{246U};

    struct itimerspec value;
    value.it_interval.tv_sec = SECONDS;
    value.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{value};
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(SECONDS * GIGA + NANOSECONDS));
}

TEST(Duration_test, ConstructFromTimeval)
{
    constexpr uint64_t SECONDS{1337U};
    constexpr uint64_t MICROSECONDS{42U};

    struct timeval value;
    value.tv_sec = SECONDS;
    value.tv_usec = MICROSECONDS;

    Duration sut{value};
    EXPECT_THAT(sut.microSeconds<uint64_t>(), Eq(SECONDS * MEGA + MICROSECONDS));
}

TEST(Duration_test, ConstructFromChronoMilliseconds)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{1001};
    Duration result{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(result.milliSeconds<uint64_t>(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, ConstructFromNegativeChronoMillisecondsIsZero)
{
    Duration result(std::chrono::milliseconds(-1));
    EXPECT_THAT(result.milliSeconds<uint64_t>(), Eq(0U));
}

TEST(Duration_test, ConstructFromChronoNanoseconds)
{
    constexpr uint64_t EXPECTED_NANOSECONDS{42U + GIGA};
    Duration result{std::chrono::milliseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(result.milliSeconds<uint64_t>(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromNegativeChronoNanosecondsIsZero)
{
    Duration result(std::chrono::nanoseconds(-1));
    EXPECT_THAT(result.nanoSeconds<uint64_t>(), Eq(0U));
}

// END CONSTRUCTOR TESTS

// BEGIN ASSIGNMENT TESTS

TEST(Duration_test, AssignFromChronoMilliseconds)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{1001};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.milliSeconds<uint64_t>(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, AssignFromNegativeChronoMillisecondsIsZero)
{
    Duration sut = 22_ns;
    sut = std::chrono::milliseconds(-1);
    EXPECT_THAT(sut.milliSeconds<uint64_t>(), Eq(0U));
}

// END ASSIGNMENT TESTS

// BEGIN CREATION FROM LITERAL TESTS

TEST(Duration_test, CreateDurationFromDaysLiteral)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{2U * 24U * SECONDS_PER_HOUR * GIGA};
    auto sut = 2_d;

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromHoursLiteral)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{3U * SECONDS_PER_HOUR * GIGA};
    auto sut = 3_h;

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMinutesLiteral)
{
    constexpr uint64_t SECONDS_PER_MINUTE{60U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{4U * SECONDS_PER_MINUTE * GIGA};
    auto sut = 4_m;

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromSecondsLiteral)
{
    constexpr uint64_t NANOSECONDS_PER_SECOND{GIGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{5U * NANOSECONDS_PER_SECOND};
    auto sut = 5_s;

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMillisecondsLiteral)
{
    constexpr uint64_t NANOSECONDS_PER_MILLISECOND{MEGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{6U * NANOSECONDS_PER_MILLISECOND};
    auto sut = 6_ms;

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMicrosecondsLiteral)
{
    constexpr uint64_t NANOSECONDS_PER_MICROSECOND{KILO};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{7U * NANOSECONDS_PER_MICROSECOND};
    auto sut = 7_us;

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromNanosecondsLiteral)
{
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{8U};
    auto sut = 8_ns;

    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

// END CREATION FROM LITERAL TESTS

// BEGIN CREATION FROM STATIC FUNCTION TESTS

TEST(Duration_test, CreateDurationFromDaysFunction)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{2U * 24U * SECONDS_PER_HOUR * GIGA};
    auto sut1 = Duration::days(2);
    auto sut2 = Duration::days(2U);

    EXPECT_THAT(sut1.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithNegativeValuesIsZero)
{
    auto sut = Duration::days(-1);
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromHoursFunction)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{3U * SECONDS_PER_HOUR * GIGA};
    auto sut1 = Duration::hours(3);
    auto sut2 = Duration::hours(3U);

    EXPECT_THAT(sut1.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::hours(-1);
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMinutesFunction)
{
    constexpr uint64_t SECONDS_PER_MINUTE{60U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{4U * SECONDS_PER_MINUTE * GIGA};
    auto sut1 = Duration::minutes(4);
    auto sut2 = Duration::minutes(4U);

    EXPECT_THAT(sut1.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::minutes(-1);
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromSecondsFunction)
{
    constexpr uint64_t NANOSECONDS_PER_SECOND{GIGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{5U * NANOSECONDS_PER_SECOND};
    auto sut1 = Duration::seconds(5);
    auto sut2 = Duration::seconds(5U);

    EXPECT_THAT(sut1.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromSecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::seconds(-1);
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunction)
{
    constexpr uint64_t NANOSECONDS_PER_MILLISECOND{MEGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{6U * NANOSECONDS_PER_MILLISECOND};
    auto sut1 = Duration::milliseconds(6);
    auto sut2 = Duration::milliseconds(6U);

    EXPECT_THAT(sut1.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::milliseconds(-1);
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunction)
{
    constexpr uint64_t NANOSECONDS_PER_MICROSECOND{KILO};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{7U * NANOSECONDS_PER_MICROSECOND};
    auto sut1 = Duration::microseconds(7);
    auto sut2 = Duration::microseconds(7U);

    EXPECT_THAT(sut1.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::microseconds(-1);
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunction)
{
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{8U};
    auto sut1 = Duration::nanoseconds(8);
    auto sut2 = Duration::nanoseconds(8U);

    EXPECT_THAT(sut1.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds<uint64_t>(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::nanoseconds(-1);
    EXPECT_THAT(sut.nanoSeconds<uint64_t>(), Eq(0U));
}

// END CREATION FROM STATIC FUNCTION TESTS

// BEGIN CONVERSION FUNCTION TESTS

TEST(Duration_test, ConvertDays)
{
    auto time = 10_d;
    EXPECT_EQ(time.days<uint64_t>(), 10U);
    EXPECT_EQ(time.hours<uint64_t>(), 240U);
    EXPECT_EQ(time.minutes<uint64_t>(), 14400U);
    EXPECT_EQ(time.seconds<uint64_t>(), 864000U);
    EXPECT_EQ(time.milliSeconds<uint64_t>(), 864000U * KILO);
    EXPECT_EQ(time.microSeconds<uint64_t>(), 864000U * MEGA);
    EXPECT_EQ(time.nanoSeconds<uint64_t>(), 864000U * GIGA);
}

TEST(Duration_test, ConvertHours)
{
    auto time = 2_h;
    EXPECT_EQ(time.days<uint64_t>(), 0U);
    EXPECT_EQ(time.hours<uint64_t>(), 2U);
    EXPECT_EQ(time.minutes<uint64_t>(), 120U);
    EXPECT_EQ(time.seconds<uint64_t>(), 7200U);
    EXPECT_EQ(time.milliSeconds<uint64_t>(), 7200U * KILO);
    EXPECT_EQ(time.microSeconds<uint64_t>(), 7200U * MEGA);
    EXPECT_EQ(time.nanoSeconds<uint64_t>(), 7200U * GIGA);
}

TEST(Duration_test, ConvertMinutes)
{
    auto time = 720_m;
    EXPECT_EQ(time.days<uint64_t>(), 0U);
    EXPECT_EQ(time.hours<uint64_t>(), 12U);
    EXPECT_EQ(time.minutes<uint64_t>(), 720U);
    EXPECT_EQ(time.seconds<uint64_t>(), 43200U);
    EXPECT_EQ(time.milliSeconds<uint64_t>(), 43200U * KILO);
    EXPECT_EQ(time.microSeconds<uint64_t>(), 43200U * MEGA);
    EXPECT_EQ(time.nanoSeconds<uint64_t>(), 43200U * GIGA);
}

TEST(Duration_test, ConvertSeconds)
{
    auto time = 28800_s;
    EXPECT_EQ(time.days<uint64_t>(), 0U);
    EXPECT_EQ(time.hours<uint64_t>(), 8U);
    EXPECT_EQ(time.minutes<uint64_t>(), 480U);
    EXPECT_EQ(time.seconds<uint64_t>(), 28800U);
    EXPECT_EQ(time.milliSeconds<uint64_t>(), 28800U * KILO);
    EXPECT_EQ(time.microSeconds<uint64_t>(), 28800U * MEGA);
    EXPECT_EQ(time.nanoSeconds<uint64_t>(), 28800U * GIGA);
}

TEST(Duration_test, ConvertMilliseconds)
{
    auto time = 28800000_ms;
    EXPECT_EQ(time.days<uint64_t>(), 0U);
    EXPECT_EQ(time.hours<uint64_t>(), 8U);
    EXPECT_EQ(time.minutes<uint64_t>(), 480U);
    EXPECT_EQ(time.seconds<uint64_t>(), 28800U);
    EXPECT_EQ(time.milliSeconds<uint64_t>(), 28800U * KILO);
    EXPECT_EQ(time.microSeconds<uint64_t>(), 28800U * MEGA);
    EXPECT_EQ(time.nanoSeconds<uint64_t>(), 28800U * GIGA);
}

TEST(Duration_test, ConvertMicroseconds)
{
    auto time = 6000000_us;
    EXPECT_EQ(time.days<uint64_t>(), 0U);
    EXPECT_EQ(time.hours<uint64_t>(), 0U);
    EXPECT_EQ(time.minutes<uint64_t>(), 0U);
    EXPECT_EQ(time.seconds<uint64_t>(), 6U);
    EXPECT_EQ(time.milliSeconds<uint64_t>(), 6U * KILO);
    EXPECT_EQ(time.microSeconds<uint64_t>(), 6U * MEGA);
    EXPECT_EQ(time.nanoSeconds<uint64_t>(), 6U * GIGA);
}

TEST(Duration_test, ConvertNanoseconds)
{
    auto time = 1_ns;
    EXPECT_EQ(time.seconds<uint64_t>(), 0U);
    EXPECT_EQ(time.milliSeconds<uint64_t>(), 0U);
    EXPECT_EQ(time.microSeconds<uint64_t>(), 0U);
    EXPECT_EQ(time.nanoSeconds<uint64_t>(), 1U);
}

TEST(Duration_test, ConvertTimespecWithNoneReference)
{
    constexpr int64_t SECONDS{44};
    constexpr int64_t NANOSECONDS{55};

    Duration duration{SECONDS, NANOSECONDS};

    struct timespec sut = timespec(duration.timespec(iox::units::TimeSpecReference::None));

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithMonotonicReference)
{
    constexpr int64_t SECONDS{4};
    constexpr int64_t NANOSECONDS{66};

    auto timeSinceUnixEpoch = std::chrono::system_clock::now().time_since_epoch();
    auto timeSinceMonotonicEpoch = std::chrono::steady_clock::now().time_since_epoch();

    Duration duration{SECONDS, NANOSECONDS};
    struct timespec sut = timespec(duration.timespec(iox::units::TimeSpecReference::Monotonic));

    auto secondsSinceUnixEpoch = std::chrono::duration_cast<std::chrono::seconds>(timeSinceUnixEpoch).count();
    auto secondsSinceMonotonicEpoch = std::chrono::duration_cast<std::chrono::seconds>(timeSinceMonotonicEpoch).count();
    EXPECT_THAT(sut.tv_sec, Lt(secondsSinceUnixEpoch));
    EXPECT_THAT(sut.tv_sec, Gt(secondsSinceMonotonicEpoch));
}

TEST(Duration_test, ConvertTimespecWithEpochReference)
{
    constexpr int64_t SECONDS{5};
    constexpr int64_t NANOSECONDS{77};

    auto timeSinceUnixEpoch = std::chrono::system_clock::now().time_since_epoch();

    Duration duration{SECONDS, NANOSECONDS};
    struct timespec sut = timespec(duration.timespec(iox::units::TimeSpecReference::Epoch));

    auto secondsSinceUnixEpoch = std::chrono::duration_cast<std::chrono::seconds>(timeSinceUnixEpoch).count();
    EXPECT_THAT(10 * SECONDS, Lt(secondsSinceUnixEpoch));
    EXPECT_THAT(sut.tv_sec, Gt(secondsSinceUnixEpoch));
}

// END CONVERSION FUNCTION TESTS

// BEGIN CONVERSION OPERATOR TESTS

TEST(Duration_test, OperatorTimeval)
{
    constexpr int64_t SECONDS{111};
    constexpr int64_t MICROSECONDS{222};
    constexpr int64_t ROUND_OFF_NANOSECONDS{666};

    Duration duration{SECONDS, MICROSECONDS * KILO + ROUND_OFF_NANOSECONDS};

    struct timeval sut = timeval(duration);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_usec, Eq(MICROSECONDS));
}

// END CONVERSION OPERATOR TESTS

// BEGIN COMPARISON TESTS

TEST(Duration_test, CompareTwoEqualDurationsForEquality)
{
    auto time1 = 200_us;
    auto time2 = 200000_ns;
    EXPECT_TRUE(time1 == time2);
}

TEST(Duration_test, CompareTwoNonEqualDurationsForEquality)
{
    auto time1 = 1_s + 200_us;
    auto time2 = 1_ns;
    EXPECT_FALSE(time1 == time2);
    EXPECT_FALSE(time2 == time1);
}

TEST(Duration_test, CompareTwoNonEqualDurationsForInequality)
{
    auto time1 = 1_s + 200_us;
    auto time2 = 1_ns;
    EXPECT_TRUE(time1 != time2);
    EXPECT_TRUE(time2 != time1);
}

TEST(Duration_test, CompareTwoEqualDurationsForInequality)
{
    auto time1 = 200_us;
    auto time2 = 200000_ns;
    EXPECT_FALSE(time1 != time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreNotLessThan)
{
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_FALSE(time1 < time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreNotGreaterThan)
{
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_FALSE(time1 > time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreLessThanOrEqualTo)
{
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_TRUE(time1 <= time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreGreaterThanOrEqualTo)
{
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_TRUE(time1 >= time2);
}

TEST(Duration_test, CompareDurationIsLessThanOther)
{
    auto time1 = 100_us;
    auto time2 = 400_us;
    auto time3 = 1_s + 200_us;
    auto time4 = 1_s + 300_us;
    EXPECT_TRUE(time1 < time2);
    EXPECT_TRUE(time1 < time3);
    EXPECT_TRUE(time2 < time3);
    EXPECT_TRUE(time3 < time4);
}

TEST(Duration_test, CompareDurationIsNotLessThanOther)
{
    auto time1 = 100_us;
    auto time2 = 400_us;
    auto time3 = 1_s + 200_us;
    auto time4 = 1_s + 300_us;
    EXPECT_FALSE(time2 < time1);
    EXPECT_FALSE(time3 < time1);
    EXPECT_FALSE(time3 < time2);
    EXPECT_FALSE(time4 < time3);
}

TEST(Duration_test, CompareDurationIsLessThanOrEqualToOther)
{
    auto time1 = 100_us;
    auto time2 = 400_us;
    auto time3 = 1_s + 200_us;
    auto time4 = 1_s + 300_us;
    EXPECT_TRUE(time1 <= time2);
    EXPECT_TRUE(time1 <= time3);
    EXPECT_TRUE(time2 <= time3);
    EXPECT_TRUE(time3 <= time4);
}

TEST(Duration_test, CompareDurationIsNotLessThanOrEqualToOther)
{
    auto time1 = 100_us;
    auto time2 = 400_us;
    auto time3 = 1_s + 200_us;
    auto time4 = 1_s + 300_us;
    EXPECT_FALSE(time2 <= time1);
    EXPECT_FALSE(time3 <= time1);
    EXPECT_FALSE(time3 <= time2);
    EXPECT_FALSE(time4 <= time3);
}

TEST(Duration_test, CompareDurationIsGreaterThanOther)
{
    auto time1 = 1_s + 300_us;
    auto time2 = 1_s + 200_us;
    auto time3 = 400_us;
    auto time4 = 100_us;
    EXPECT_TRUE(time1 > time2);
    EXPECT_TRUE(time1 > time3);
    EXPECT_TRUE(time2 > time3);
    EXPECT_TRUE(time3 > time4);
}

TEST(Duration_test, CompareDurationIsNotGreaterThanOther)
{
    auto time1 = 1_s + 300_us;
    auto time2 = 1_s + 200_us;
    auto time3 = 400_us;
    auto time4 = 100_us;
    EXPECT_FALSE(time2 > time1);
    EXPECT_FALSE(time3 > time1);
    EXPECT_FALSE(time3 > time2);
    EXPECT_FALSE(time4 > time3);
}

TEST(Duration_test, CompareDurationIsGreaterThanOrEqualToOther)
{
    auto time1 = 1_s + 300_us;
    auto time2 = 1_s + 200_us;
    auto time3 = 400_us;
    auto time4 = 100_us;
    EXPECT_TRUE(time1 >= time2);
    EXPECT_TRUE(time1 >= time3);
    EXPECT_TRUE(time2 >= time3);
    EXPECT_TRUE(time3 >= time4);
}

TEST(Duration_test, CompareDurationIsNotGreaterThanOrEqualToOther)
{
    auto time1 = 1_s + 300_us;
    auto time2 = 1_s + 200_us;
    auto time3 = 400_us;
    auto time4 = 100_us;
    EXPECT_FALSE(time2 >= time1);
    EXPECT_FALSE(time3 >= time1);
    EXPECT_FALSE(time3 >= time2);
    EXPECT_FALSE(time4 >= time3);
}

// END COMPARISON TESTS

// BEGIN ARITHMETIC TESTS

TEST(Duration_test, AddDuration)
{
    auto time1 = 5_s + 200_ms;
    auto time2 = 200_ms;
    auto time3 = 700000_us;

    EXPECT_THAT(time1.milliSeconds<uint64_t>(), Eq(5200U));

    auto result = time1 + time2;
    EXPECT_EQ(result.milliSeconds<uint64_t>(), 5400U);

    result = result + time3;
    EXPECT_EQ(result.microSeconds<uint64_t>(), 6100000U);
}

TEST(Duration_test, SubtractDuration)
{
    auto time1 = 6_s - 800_ms;
    auto time2 = 200_ms;
    auto time3 = 300000_us;

    EXPECT_THAT(time1.milliSeconds<uint64_t>(), Eq(5200U));

    auto result = time1 - time2;
    EXPECT_EQ(result.milliSeconds<uint64_t>(), 5000U);

    result = result - time3;
    EXPECT_EQ(result.microSeconds<uint64_t>(), 4700000U);

    auto time4 = 10_s;

    result = result - time4;
    EXPECT_EQ(result.milliSeconds<uint64_t>(), 0U);
}

TEST(Duration_test, MultiplyDuration)
{
    auto time = 5_s + 800_ms;

    EXPECT_THAT(time * 2, Eq(11_s + 600_ms));
    EXPECT_THAT(time * 2U, Eq(11_s + 600_ms));
    EXPECT_THAT(time * 2.95, Eq(17_s + 110_ms));

    EXPECT_THAT(2 * time, Eq(11_s + 600_ms));
    EXPECT_THAT(2U * time, Eq(11_s + 600_ms));
    EXPECT_THAT(2.95 * time, Eq(17_s + 110_ms));

    EXPECT_THAT(time * -1, Eq(0_s));
    EXPECT_THAT(time * -1.0, Eq(0_s));
}

// END ARITHMETIC TESTS
