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

constexpr Duration MAX_DURATION{std::numeric_limits<uint64_t>::max(), GIGA - 1U};

// BEGIN CONSTRUCTOR TESTS

TEST(Duration_test, ConstructDurationWithZeroTime)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{0U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{0U};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithResultOfLessNanosecondsThanOneSecond)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{7337U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{NANOSECONDS};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsLessThanOneSecond)
{
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t NANOSECONDS{73U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{SECONDS * GIGA + NANOSECONDS};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsEqualToOneSecond)
{
    constexpr uint64_t SECONDS{13U};
    constexpr uint64_t NANOSECONDS{GIGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(SECONDS + 1U) * GIGA};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsMoreThanOneSecond)
{
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t NANOSECONDS{42U};
    constexpr uint64_t MORE_THAN_ONE_SECOND_NANOSECONDS{GIGA + NANOSECONDS};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(SECONDS + 1U) * GIGA + NANOSECONDS};

    auto sut = Duration{SECONDS, MORE_THAN_ONE_SECOND_NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsMaxValue)
{
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t MAX_NANOSECONDS_FOR_CTOR{std::numeric_limits<uint32_t>::max()};
    constexpr uint64_t EXPECTED_SECONDS = SECONDS + MAX_NANOSECONDS_FOR_CTOR / GIGA;
    constexpr uint64_t REMAINING_NANOSECONDS = MAX_NANOSECONDS_FOR_CTOR % GIGA;
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(EXPECTED_SECONDS)*GIGA + REMAINING_NANOSECONDS};

    auto sut = Duration{SECONDS, MAX_NANOSECONDS_FOR_CTOR};

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithSecondsAndNanosecondsMaxValues)
{
    constexpr uint64_t MAX_SECONDS_FOR_CTOR{std::numeric_limits<uint64_t>::max()};
    constexpr uint64_t MAX_NANOSECONDS_FOR_CTOR{std::numeric_limits<uint32_t>::max()};
    constexpr Duration EXPECTED_DURATION{MAX_SECONDS_FOR_CTOR, GIGA - 1U};

    auto sut = Duration{MAX_SECONDS_FOR_CTOR, MAX_NANOSECONDS_FOR_CTOR};

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructDurationWithOneNanosecondResultsNotInZeroNanoseconds)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{1U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{NANOSECONDS};

    auto sut = Duration{SECONDS, NANOSECONDS};

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructFromTimespecWithZeroValue)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{0U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct timespec ts;
    ts.tv_sec = SECONDS;
    ts.tv_nsec = NANOSECONDS;

    Duration sut{ts};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimespecWithValueLessThanOneSecond)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{456U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct timespec value;
    value.tv_sec = SECONDS;
    value.tv_nsec = NANOSECONDS;

    Duration sut{value};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimespecWithValueMoreThanOneSecond)
{
    constexpr uint64_t SECONDS{73U};
    constexpr uint64_t NANOSECONDS{456U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct timespec value;
    value.tv_sec = SECONDS;
    value.tv_nsec = NANOSECONDS;

    Duration sut{value};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimespecWithMaxValue)
{
    constexpr uint64_t SECONDS{std::numeric_limits<uint64_t>::max()};
    constexpr uint64_t NANOSECONDS{GIGA - 1U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct timespec ts;
    ts.tv_sec = SECONDS;
    ts.tv_nsec = NANOSECONDS;

    Duration sut{ts};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromITimerspecWithZeroValue)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{0U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromITimerspecWithValueLessThanOneSecond)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{642U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromITimerspecWithValueMoreThanOneSecond)
{
    constexpr uint64_t SECONDS{13U};
    constexpr uint64_t NANOSECONDS{42U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromITimerspecWithMaxValue)
{
    constexpr uint64_t SECONDS{std::numeric_limits<uint64_t>::max()};
    constexpr uint64_t NANOSECONDS{GIGA - 1U};
    constexpr Duration EXPECTED_DURATION{SECONDS, NANOSECONDS};

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimevalWithZeroValue)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t MICROSECONDS{0U};
    constexpr Duration EXPECTED_DURATION{SECONDS, MICROSECONDS * KILO};

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimevalWithValueLessThanOneSecond)
{
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t MICROSECONDS{13U};
    constexpr Duration EXPECTED_DURATION{SECONDS, MICROSECONDS * KILO};

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimevalWithValueMoreThanOneSecond)
{
    constexpr uint64_t SECONDS{1337U};
    constexpr uint64_t MICROSECONDS{42U};
    constexpr Duration EXPECTED_DURATION{SECONDS, MICROSECONDS * KILO};

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimevalWithMaxValue)
{
    constexpr uint64_t SECONDS{std::numeric_limits<uint64_t>::max()};
    constexpr uint64_t MICROSECONDS{MEGA - 1U};
    constexpr Duration EXPECTED_DURATION{SECONDS, MICROSECONDS * KILO};

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromChronoMillisecondsZero)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{0U};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, ConstructFromChronoMillisecondsLessThanOneSecond)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{44U};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_MILLISECONDS * MEGA));
}

TEST(Duration_test, ConstructFromChronoMillisecondsMoreThanOneSecond)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{1001};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_MILLISECONDS * MEGA));
}

TEST(Duration_test, ConstructFromChronoMillisecondsMax)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{std::numeric_limits<int64_t>::max()};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.milliSeconds(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, ConstructFromNegativeChronoMillisecondsIsZero)
{
    Duration sut(std::chrono::milliseconds(-1));
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, ConstructFromChronoNanosecondsZero)
{
    constexpr uint64_t EXPECTED_NANOSECONDS{0U};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromChronoNanosecondsLessThanOneSecond)
{
    constexpr uint64_t EXPECTED_NANOSECONDS{424242U};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromChronoNanosecondsMoreThanOneSecond)
{
    constexpr uint64_t EXPECTED_NANOSECONDS{42U + GIGA};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromChronoNanosecondsMax)
{
    constexpr uint64_t EXPECTED_NANOSECONDS{std::numeric_limits<int64_t>::max()};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromNegativeChronoNanosecondsIsZero)
{
    Duration sut(std::chrono::nanoseconds(-1));
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

// END CONSTRUCTOR TESTS

// BEGIN ASSIGNMENT TESTS

TEST(Duration_test, AssignFromChronoMillisecondsZero)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{0U};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, AssignFromChronoMillisecondsLessThanOneSecond)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{73U};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_MILLISECONDS * MEGA));
}

TEST(Duration_test, AssignFromChronoMillisecondsMoreThanOneSecond)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{1073U};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_MILLISECONDS * MEGA));
}

TEST(Duration_test, AssignFromChronoMillisecondsMax)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{std::numeric_limits<int64_t>::max()};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.milliSeconds(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, AssignFromNegativeChronoMillisecondsIsZero)
{
    Duration sut = 22_ns;
    sut = std::chrono::milliseconds(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

// END ASSIGNMENT TESTS

// BEGIN CREATION FROM LITERAL TESTS

TEST(Duration_test, CreateDurationFromDaysLiteral)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{2U * 24U * SECONDS_PER_HOUR * GIGA};
    auto sut = 2_d;

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromHoursLiteral)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{3U * SECONDS_PER_HOUR * GIGA};
    auto sut = 3_h;

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMinutesLiteral)
{
    constexpr uint64_t SECONDS_PER_MINUTE{60U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{4U * SECONDS_PER_MINUTE * GIGA};
    auto sut = 4_m;

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromSecondsLiteral)
{
    constexpr uint64_t NANOSECONDS_PER_SECOND{GIGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{5U * NANOSECONDS_PER_SECOND};
    auto sut = 5_s;

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMillisecondsLiteral)
{
    constexpr uint64_t NANOSECONDS_PER_MILLISECOND{MEGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{6U * NANOSECONDS_PER_MILLISECOND};
    auto sut = 6_ms;

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMicrosecondsLiteral)
{
    constexpr uint64_t NANOSECONDS_PER_MICROSECOND{KILO};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{7U * NANOSECONDS_PER_MICROSECOND};
    auto sut = 7_us;

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromNanosecondsLiteral)
{
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{8U};
    auto sut = 8_ns;

    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

// END CREATION FROM LITERAL TESTS

// BEGIN CREATION FROM STATIC FUNCTION TESTS

TEST(Duration_test, CreateDurationFromDaysFunctionWithZeroDays)
{
    auto sut1 = Duration::days(0);
    auto sut2 = Duration::days(0U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(0U));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithMultipleDays)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{2U * 24U * SECONDS_PER_HOUR * GIGA};
    auto sut1 = Duration::days(2);
    auto sut2 = Duration::days(2U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithDaysResultsNotYetInSaturation)
{
    constexpr uint64_t SECONDS_PER_DAY{60U * 60U * 24U};
    constexpr uint64_t MAX_DAYS_BEFORE_OVERFLOW{std::numeric_limits<uint64_t>::max() / SECONDS_PER_DAY};
    constexpr Duration EXPECTED_DURATION{MAX_DAYS_BEFORE_OVERFLOW * SECONDS_PER_DAY, 0U};
    static_assert(EXPECTED_DURATION < MAX_DURATION,
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    auto sut1 = Duration::days(static_cast<int64_t>(MAX_DAYS_BEFORE_OVERFLOW));
    auto sut2 = Duration::days(MAX_DAYS_BEFORE_OVERFLOW);

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithMaxDaysResultsInSaturation)
{
    auto sut1 = Duration::days(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::days(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(MAX_DURATION));
    EXPECT_THAT(sut2, Eq(MAX_DURATION));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithNegativeValuesIsZero)
{
    auto sut = Duration::days(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithZeroHours)
{
    auto sut1 = Duration::hours(0);
    auto sut2 = Duration::hours(0U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(0U));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithMultipleHours)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{3U * SECONDS_PER_HOUR * GIGA};
    auto sut1 = Duration::hours(3);
    auto sut2 = Duration::hours(3U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithHoursResultsNotYetInSaturation)
{
    constexpr uint64_t SECONDS_PER_HOUR{3600U};
    constexpr uint64_t MAX_HOURS_BEFORE_OVERFLOW{std::numeric_limits<uint64_t>::max() / SECONDS_PER_HOUR};
    constexpr Duration EXPECTED_DURATION{MAX_HOURS_BEFORE_OVERFLOW * SECONDS_PER_HOUR, 0U};
    static_assert(EXPECTED_DURATION < MAX_DURATION,
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    auto sut1 = Duration::hours(static_cast<int64_t>(MAX_HOURS_BEFORE_OVERFLOW));
    auto sut2 = Duration::hours(MAX_HOURS_BEFORE_OVERFLOW);

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithMaxHoursResultsInSaturation)
{
    auto sut1 = Duration::hours(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::hours(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(MAX_DURATION));
    EXPECT_THAT(sut2, Eq(MAX_DURATION));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::hours(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithZeroMinuts)
{
    auto sut1 = Duration::minutes(0);
    auto sut2 = Duration::minutes(0U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(0U));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithMultipleMinutes)
{
    constexpr uint64_t SECONDS_PER_MINUTE{60U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{4U * SECONDS_PER_MINUTE * GIGA};
    auto sut1 = Duration::minutes(4);
    auto sut2 = Duration::minutes(4U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithMinutesResultsNotYetInSaturation)
{
    constexpr uint64_t SECONDS_PER_MINUTE{60U};
    constexpr uint64_t MAX_MINUTES_BEFORE_OVERFLOW{std::numeric_limits<uint64_t>::max() / SECONDS_PER_MINUTE};
    constexpr Duration EXPECTED_DURATION{MAX_MINUTES_BEFORE_OVERFLOW * SECONDS_PER_MINUTE, 0U};
    static_assert(EXPECTED_DURATION < MAX_DURATION,
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    auto sut1 = Duration::minutes(static_cast<int64_t>(MAX_MINUTES_BEFORE_OVERFLOW));
    auto sut2 = Duration::minutes(MAX_MINUTES_BEFORE_OVERFLOW);

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithMaxMinutesResultsInSaturation)
{
    auto sut1 = Duration::minutes(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::minutes(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(MAX_DURATION));
    EXPECT_THAT(sut2, Eq(MAX_DURATION));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::minutes(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromSecondsFunctionWithZeroSeconds)
{
    auto sut1 = Duration::seconds(0);
    auto sut2 = Duration::seconds(0U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(0U));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromSecondsFunction)
{
    constexpr uint64_t NANOSECONDS_PER_SECOND{GIGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{5U * NANOSECONDS_PER_SECOND};
    auto sut1 = Duration::seconds(5);
    auto sut2 = Duration::seconds(5U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromSecondsFunctionWithMaxSeconds)
{
    constexpr uint64_t MAX_SECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED{MAX_SECONDS_FROM_SIGNED, 0U};
    constexpr uint64_t MAX_SECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED{MAX_SECONDS_FROM_USIGNED, 0U};

    auto sut1 = Duration::seconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::seconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromSecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::seconds(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithZeroMilliseconds)
{
    auto sut1 = Duration::milliseconds(0);
    auto sut2 = Duration::milliseconds(0U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(0U));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithMultipleMilliseconds)
{
    constexpr uint64_t NANOSECONDS_PER_MILLISECOND{MEGA};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{6U * NANOSECONDS_PER_MILLISECOND};
    auto sut1 = Duration::milliseconds(6);
    auto sut2 = Duration::milliseconds(6U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithMaxMilliseconds)
{
    constexpr uint64_t MAX_MILLISECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED{MAX_MILLISECONDS_FROM_SIGNED / KILO,
                                                         (MAX_MILLISECONDS_FROM_SIGNED % KILO) * MEGA};
    constexpr uint64_t MAX_MILLISECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED{MAX_MILLISECONDS_FROM_USIGNED / KILO,
                                                           (MAX_MILLISECONDS_FROM_USIGNED % KILO) * MEGA};

    auto sut1 = Duration::milliseconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::milliseconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::milliseconds(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithZeroMicroseconds)
{
    auto sut1 = Duration::microseconds(0);
    auto sut2 = Duration::microseconds(0U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(0U));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithMultipleMicroseconds)
{
    constexpr uint64_t NANOSECONDS_PER_MICROSECOND{KILO};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{7U * NANOSECONDS_PER_MICROSECOND};
    auto sut1 = Duration::microseconds(7);
    auto sut2 = Duration::microseconds(7U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithMaxMicroseconds)
{
    constexpr uint64_t MAX_MICROSECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED{MAX_MICROSECONDS_FROM_SIGNED / MEGA,
                                                         (MAX_MICROSECONDS_FROM_SIGNED % MEGA) * KILO};
    constexpr uint64_t MAX_MICROSECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED{MAX_MICROSECONDS_FROM_USIGNED / MEGA,
                                                           (MAX_MICROSECONDS_FROM_USIGNED % MEGA) * KILO};

    auto sut1 = Duration::microseconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::microseconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::microseconds(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunctionWithZeroNanoseconds)
{
    auto sut1 = Duration::nanoseconds(0);
    auto sut2 = Duration::nanoseconds(0U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(0U));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunctionWithMultipleNanoseconds)
{
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{8U};
    auto sut1 = Duration::nanoseconds(8);
    auto sut2 = Duration::nanoseconds(8U);

    EXPECT_THAT(sut1.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.nanoSeconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunction)
{
    constexpr uint64_t MAX_NANOSECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED{MAX_NANOSECONDS_FROM_SIGNED / GIGA,
                                                         MAX_NANOSECONDS_FROM_SIGNED % GIGA};
    constexpr uint64_t MAX_NANOSECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED{MAX_NANOSECONDS_FROM_USIGNED / GIGA,
                                                           MAX_NANOSECONDS_FROM_USIGNED % GIGA};

    auto sut1 = Duration::nanoseconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::nanoseconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunctionWithNegativeValueIsZero)
{
    auto sut = Duration::nanoseconds(-1);
    EXPECT_THAT(sut.nanoSeconds(), Eq(0U));
}

// END CREATION FROM STATIC FUNCTION TESTS

// BEGIN CONVERSION FUNCTION TESTS

TEST(Duration_test, ConvertDaysFromZeroDuration)
{
    auto sut = 0_s;
    EXPECT_THAT(sut.days(), Eq(0U));
}

TEST(Duration_test, ConvertDaysFromDurationLessThanOneDay)
{
    auto sut = 3473_s;
    EXPECT_THAT(sut.days(), Eq(0U));
}

TEST(Duration_test, ConvertDaysFromDurationMoreThanOneDay)
{
    auto sut = 7_d + 3066_s;
    EXPECT_THAT(sut.days(), Eq(7U));
}

TEST(Duration_test, ConvertDaysFromMaxDuration)
{
    constexpr uint64_t SECONDS_PER_DAY{60U * 60U * 24U};
    constexpr uint64_t EXPECTED_DAYS{std::numeric_limits<uint64_t>::max() / SECONDS_PER_DAY};
    auto sut = MAX_DURATION;
    EXPECT_THAT(sut.days(), Eq(EXPECTED_DAYS));
}

TEST(Duration_test, ConvertHoursFromZeroDuration)
{
    auto sut = 0_s;
    EXPECT_THAT(sut.hours(), Eq(0U));
}

TEST(Duration_test, ConvertHoursFromDurationLessThanOneHour)
{
    auto sut = 37_m;
    EXPECT_THAT(sut.hours(), Eq(0U));
}

TEST(Duration_test, ConvertHoursFromDurationMoreThanOneHour)
{
    auto sut = 73_h + 42_m;
    EXPECT_THAT(sut.hours(), Eq(73U));
}

TEST(Duration_test, ConvertHoursFromMaxDuration)
{
    constexpr uint64_t SECONDS_PER_HOUR{60U * 60U};
    constexpr uint64_t EXPECTED_HOURS{std::numeric_limits<uint64_t>::max() / SECONDS_PER_HOUR};
    auto sut = MAX_DURATION;
    EXPECT_THAT(sut.hours(), Eq(EXPECTED_HOURS));
}

TEST(Duration_test, ConvertMinutesFromZeroDuration)
{
    auto sut = 0_s;
    EXPECT_THAT(sut.minutes(), Eq(0U));
}

TEST(Duration_test, ConvertMinutesFromDurationLessThanOneMinute)
{
    auto sut = 34_s;
    EXPECT_THAT(sut.minutes(), Eq(0U));
}

TEST(Duration_test, ConvertMinutesFromDurationMoreThanOneMinute)
{
    auto sut = 13_m + 42_s;
    EXPECT_THAT(sut.minutes(), Eq(13U));
}

TEST(Duration_test, ConvertMinutesFromMaxDuration)
{
    constexpr uint64_t SECONDS_PER_MINUTE{60U};
    constexpr uint64_t EXPECTED_MINUTES{std::numeric_limits<uint64_t>::max() / SECONDS_PER_MINUTE};
    auto sut = MAX_DURATION;
    EXPECT_THAT(sut.minutes(), Eq(EXPECTED_MINUTES));
}

TEST(Duration_test, ConvertSecondsFromZeroDuration)
{
    auto sut = 0_s;
    EXPECT_THAT(sut.seconds(), Eq(0U));
}

TEST(Duration_test, ConvertSecondsFromDurationLessThanOneSecond)
{
    auto sut = 737_ms;
    EXPECT_THAT(sut.seconds(), Eq(0U));
}

TEST(Duration_test, ConvertSecondsFromDurationMoreThanOneSecond)
{
    auto sut = 7_s + 833_ms;
    EXPECT_THAT(sut.seconds(), Eq(7U));
}

TEST(Duration_test, ConvertSecondsFromMaxSecondsMinusOne)
{
    constexpr uint64_t EXPECTED_SECONDS{std::numeric_limits<uint64_t>::max() - 1U};
    auto sut = MAX_DURATION - 1_s;
    EXPECT_THAT(sut.seconds(), Eq(EXPECTED_SECONDS));
}

TEST(Duration_test, ConvertSecondsFromMaxDuration)
{
    constexpr uint64_t EXPECTED_SECONDS{std::numeric_limits<uint64_t>::max()};
    auto sut = MAX_DURATION;
    EXPECT_THAT(sut.seconds(), Eq(EXPECTED_SECONDS));
}

TEST(Duration_test, ConvertMillisecondsFromZeroDuration)
{
    auto sut = 0_s;
    EXPECT_THAT(sut.milliSeconds(), Eq(0U));
}

TEST(Duration_test, ConvertMillisecondsFromDurationLessThanOneMillisecond)
{
    auto sut = 637_us;
    EXPECT_THAT(sut.milliSeconds(), Eq(0U));
}

TEST(Duration_test, ConvertMilliecondsFromDurationMoreThanOneMillisecond)
{
    auto sut = 55_ms + 633_us;
    EXPECT_THAT(sut.milliSeconds(), Eq(55U));
}

TEST(Duration_test, ConvertMillisecondsFromDurationResultsNotYetInSaturation)
{
    constexpr uint64_t EXPECTED_MILLISECONDS{std::numeric_limits<uint64_t>::max() - 1U};
    auto sut = Duration::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.milliSeconds(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, ConvertMillisecondsFromMaxDurationResultsInSaturation)
{
    auto sut = MAX_DURATION;
    EXPECT_THAT(sut.milliSeconds(), Eq(std::numeric_limits<uint64_t>::max()));
}

TEST(Duration_test, ConvertMicrosecondsFromZeroDuration)
{
    auto sut = 0_s;
    EXPECT_THAT(sut.microSeconds(), Eq(0U));
}

TEST(Duration_test, ConvertMicrosecondsFromDurationLessThanOneMicrosecond)
{
    auto sut = 733_ns;
    EXPECT_THAT(sut.microSeconds(), Eq(0U));
}

TEST(Duration_test, ConvertMicrosecondsFromDurationMoreThanOneMicrosecond)
{
    auto sut = 555_us + 733_ns;
    EXPECT_THAT(sut.microSeconds(), Eq(555U));
}

TEST(Duration_test, ConvertMicrosecondsFromDurationResultsNotYetInSaturation)
{
    constexpr uint64_t EXPECTED_MICROSECONDS{std::numeric_limits<uint64_t>::max() - 1U};
    auto sut = Duration::microseconds(EXPECTED_MICROSECONDS);
    EXPECT_THAT(sut.microSeconds(), Eq(EXPECTED_MICROSECONDS));
}

TEST(Duration_test, ConvertMicroecondsFromMaxDurationResultsInSaturation)
{
    auto sut = MAX_DURATION;
    EXPECT_THAT(sut.microSeconds(), Eq(std::numeric_limits<uint64_t>::max()));
}

TEST(Duration_test, ConvertNanosecondsFromZeroDuration)
{
    auto sut = 0_s;
    EXPECT_THAT(sut.microSeconds(), Eq(0U));
}

TEST(Duration_test, ConvertNanosecondsFromDurationOfOneNanosecond)
{
    auto sut = 1_ns;
    EXPECT_THAT(sut.nanoSeconds(), Eq(1U));
}

TEST(Duration_test, ConvertNanosecondsFromDurationMultipleNanoseconds)
{
    auto sut = 42_ns;
    EXPECT_THAT(sut.nanoSeconds(), Eq(42U));
}

TEST(Duration_test, ConvertNanosecondsFromDurationResultsNotYetInSaturation)
{
    constexpr uint64_t EXPECTED_NANOSECONDS{std::numeric_limits<uint64_t>::max() - 1U};
    auto sut = Duration::nanoseconds(EXPECTED_NANOSECONDS);
    EXPECT_THAT(sut.nanoSeconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConvertNanoecondsFromMaxDurationResultsInSaturation)
{
    auto sut = MAX_DURATION;
    EXPECT_THAT(sut.nanoSeconds(), Eq(std::numeric_limits<uint64_t>::max()));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromZeroDuration)
{
    constexpr int64_t SECONDS{0};
    constexpr int64_t NANOSECONDS{0};

    Duration duration{SECONDS, NANOSECONDS};

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromDurationLessThanOneSecond)
{
    constexpr int64_t SECONDS{0};
    constexpr int64_t NANOSECONDS{55};

    Duration duration{SECONDS, NANOSECONDS};

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromDurationMoreThanOneSecond)
{
    constexpr int64_t SECONDS{44};
    constexpr int64_t NANOSECONDS{55};

    Duration duration{SECONDS, NANOSECONDS};

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromDurationResultsNotYetInSaturation)
{
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{GIGA - 1U};

    Duration duration{SECONDS, NANOSECONDS};

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromMaxDurationResultsInSaturation)
{
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{GIGA - 1U};

    struct timespec sut = MAX_DURATION.timespec(iox::units::TimeSpecReference::None);

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

TEST(Duration_test, ConvertTimespecWithMonotonicReferenceFromMaxDurationResultsInSaturation)
{
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{GIGA - 1U};

    struct timespec sut = MAX_DURATION.timespec(iox::units::TimeSpecReference::Monotonic);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
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

TEST(Duration_test, ConvertTimespecWithEpochReferenceFromMaxDurationResultsInSaturation)
{
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{GIGA - 1U};

    struct timespec sut = MAX_DURATION.timespec(iox::units::TimeSpecReference::Epoch);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
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

    EXPECT_THAT(time1.milliSeconds(), Eq(5200U));

    auto result = time1 + time2;
    EXPECT_EQ(result.milliSeconds(), 5400U);

    result = result + time3;
    EXPECT_EQ(result.microSeconds(), 6100000U);
}

TEST(Duration_test, SubtractDuration)
{
    auto time1 = 6_s - 800_ms;
    auto time2 = 200_ms;
    auto time3 = 300000_us;

    EXPECT_THAT(time1.milliSeconds(), Eq(5200U));

    auto result = time1 - time2;
    EXPECT_EQ(result.milliSeconds(), 5000U);

    result = result - time3;
    EXPECT_EQ(result.microSeconds(), 4700000U);

    auto time4 = 10_s;

    result = result - time4;
    EXPECT_EQ(result.milliSeconds(), 0U);
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

TEST(Duration_test, MultiplyDurationLessThanOneSecondResultsInMoreNanosecondsThan64BitCanRepresent)
{
    constexpr uint64_t MULTIPLICATOR{(1ULL << 32U) * 42U + 73U};
    constexpr Duration DURATION = 473_ms + 578_us + 511_ns;
    auto EXPECTED_RESULT = Duration{85428177141U, 573034055U};

    auto result = MULTIPLICATOR * DURATION;
    EXPECT_THAT(result, Eq(EXPECTED_RESULT));
    EXPECT_THAT(result.nanoSeconds(), Eq(std::numeric_limits<uint64_t>::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(EXPECTED_RESULT));
}

TEST(Duration_test, MultiplyDurationResultsNotYetInSaturation)
{
    constexpr uint64_t MULTIPLICATOR{1343535617188545796U};
    constexpr Duration DURATION = 13_s + 730_ms + 37_ns;
    constexpr Duration EXPECTED_DURATION{std::numeric_limits<uint64_t>::max(), 56194452U};
    static_assert(EXPECTED_DURATION < MAX_DURATION,
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(EXPECTED_DURATION));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, MultiplyDurationResultsInSaturation)
{
    constexpr uint64_t MULTIPLICATOR{1343535617188545797U};
    constexpr Duration DURATION = 13_s + 730_ms + 37_ns;

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(MAX_DURATION));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(MAX_DURATION));
}

TEST(Duration_test, MultiplyDurationWithMinimalDoubleResultsInZero)
{
    constexpr double MULTIPLICATOR{std::numeric_limits<double>::min()};
    constexpr Duration DURATION = 13_s + 730_ms + 37_ns;
    auto EXPECTED_DURATION = Duration{0U, 0U};

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(EXPECTED_DURATION));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(EXPECTED_DURATION));
}

// END ARITHMETIC TESTS
