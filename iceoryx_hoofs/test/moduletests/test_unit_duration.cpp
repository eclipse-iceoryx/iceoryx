// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/units/duration.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::units;
using namespace iox::units::duration_literals;

constexpr uint64_t SECONDS_PER_MINUTE = Duration::SECS_PER_MINUTE;
constexpr uint64_t SECONDS_PER_HOUR = Duration::SECS_PER_HOUR;
constexpr uint64_t HOURS_PER_DAY = Duration::HOURS_PER_DAY;

constexpr uint64_t MILLISECS_PER_SECOND = Duration::MILLISECS_PER_SEC;
constexpr uint64_t MICROSECS_PER_SECONDS = Duration::MICROSECS_PER_SEC;

constexpr uint64_t NANOSECS_PER_MICROSECOND = Duration::NANOSECS_PER_MICROSEC;
constexpr uint64_t NANOSECS_PER_MILLISECOND = Duration::NANOSECS_PER_MILLISEC;
constexpr uint64_t NANOSECS_PER_SECOND = Duration::NANOSECS_PER_SEC;

struct DurationAccessor : public Duration
{
    using Duration::createDuration;
    using Duration::max;

    using Duration::Nanoseconds_t;
    using Duration::Seconds_t;
};

constexpr Duration createDuration(DurationAccessor::Seconds_t seconds, DurationAccessor::Nanoseconds_t nanoseconds)
{
    return DurationAccessor::createDuration(seconds, nanoseconds);
}

TEST(Duration_test, ConversionConstants)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d9090fc-438c-41dc-9350-04910ef9b27d");
    static_assert(Duration::SECS_PER_MINUTE == 60U, "Mismatch for conversion constants!");
    static_assert(Duration::SECS_PER_HOUR == 3600U, "Mismatch for conversion constants!");
    static_assert(Duration::HOURS_PER_DAY == 24U, "Mismatch for conversion constants!");

    static_assert(Duration::MILLISECS_PER_SEC == 1000U, "Mismatch for conversion constants!");
    static_assert(Duration::MICROSECS_PER_SEC == 1000000U, "Mismatch for conversion constants!");

    static_assert(Duration::NANOSECS_PER_MICROSEC == 1000U, "Mismatch for conversion constants!");
    static_assert(Duration::NANOSECS_PER_MILLISEC == 1000000U, "Mismatch for conversion constants!");
    static_assert(Duration::NANOSECS_PER_SEC == 1000000000U, "Mismatch for conversion constants!");
}

// BEGIN CONSTRUCTOR TESTS

TEST(Duration_test, ConstructDurationWithZeroTime)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2daaadc-b756-4c40-8bdf-e648160d7168");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{0U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{0U};

    auto sut = createDuration(SECONDS, NANOSECONDS);

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithResultOfLessNanosecondsThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e4d5165-9a01-49fd-8dca-ca4914f3604b");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{7337U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{NANOSECONDS};

    auto sut = createDuration(SECONDS, NANOSECONDS);

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "7de47e88-c4b3-4655-bdeb-3c51f3b34d3e");
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t NANOSECONDS{73U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{SECONDS * NANOSECS_PER_SECOND + NANOSECONDS};

    auto sut = createDuration(SECONDS, NANOSECONDS);

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsEqualToOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "617e61f1-6322-4f9b-9814-3a645a9a11ea");
    constexpr uint64_t SECONDS{13U};
    constexpr uint64_t NANOSECONDS{NANOSECS_PER_SECOND};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(SECONDS + 1U) * NANOSECS_PER_SECOND};

    auto sut = createDuration(SECONDS, NANOSECONDS);

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1bc3127-279c-4cf2-abfb-03a71a5e7e48");
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t NANOSECONDS{42U};
    constexpr uint64_t MORE_THAN_ONE_SECOND_NANOSECONDS{NANOSECS_PER_SECOND + NANOSECONDS};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(SECONDS + 1U) * NANOSECS_PER_SECOND + NANOSECONDS};

    auto sut = createDuration(SECONDS, MORE_THAN_ONE_SECOND_NANOSECONDS);

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithNanosecondsMaxValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe8152ca-cd14-4d99-aa68-0f56ab3007de");
    constexpr uint64_t SECONDS{37U};
    constexpr uint64_t MAX_NANOSECONDS_FOR_CTOR{std::numeric_limits<DurationAccessor::Nanoseconds_t>::max()};
    constexpr uint64_t EXPECTED_SECONDS = SECONDS + MAX_NANOSECONDS_FOR_CTOR / NANOSECS_PER_SECOND;
    constexpr uint64_t REMAINING_NANOSECONDS = MAX_NANOSECONDS_FOR_CTOR % NANOSECS_PER_SECOND;
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{(EXPECTED_SECONDS)*NANOSECS_PER_SECOND + REMAINING_NANOSECONDS};

    auto sut = createDuration(SECONDS, MAX_NANOSECONDS_FOR_CTOR);

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructDurationWithSecondsAndNanosecondsMaxValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf672288-064f-4033-9b6f-0345e38a066f");
    constexpr uint64_t MAX_SECONDS_FOR_CTOR{std::numeric_limits<DurationAccessor::Seconds_t>::max()};
    constexpr uint64_t MAX_NANOSECONDS_FOR_CTOR{std::numeric_limits<DurationAccessor::Nanoseconds_t>::max()};

    auto sut = createDuration(MAX_SECONDS_FOR_CTOR, MAX_NANOSECONDS_FOR_CTOR);

    EXPECT_THAT(sut, Eq(DurationAccessor::max()));
}

TEST(Duration_test, ConstructDurationWithOneNanosecondResultsNotInZeroNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f1948aa-e008-4aa0-b574-b2a068c1aabf");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{1U};
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{NANOSECONDS};

    auto sut = createDuration(SECONDS, NANOSECONDS);

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, ConstructFromTimespecWithZeroValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "0360ae1d-b7e7-4b07-b515-fdc82703d16f");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{0U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, NANOSECONDS);

    struct timespec ts;
    ts.tv_sec = SECONDS;
    ts.tv_nsec = NANOSECONDS;

    Duration sut{ts};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimespecWithValueLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cc49b4b-2bdf-491a-9015-9e8d769ba113");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{456U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, NANOSECONDS);

    struct timespec value;
    value.tv_sec = SECONDS;
    value.tv_nsec = NANOSECONDS;

    Duration sut{value};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimespecWithValueMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8a165e4-3117-4897-bd16-3c5a0333c1b5");
    constexpr uint64_t SECONDS{73U};
    constexpr uint64_t NANOSECONDS{456U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, NANOSECONDS);

    struct timespec value;
    value.tv_sec = SECONDS;
    value.tv_nsec = NANOSECONDS;

    Duration sut{value};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimespecWithMaxValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d160d324-914f-4a66-ac96-17828442d4bd");
    constexpr uint64_t SECONDS{std::numeric_limits<DurationAccessor::Seconds_t>::max()};
    constexpr uint64_t NANOSECONDS{NANOSECS_PER_SECOND - 1U};

    struct timespec ts;
    ts.tv_sec = SECONDS;
    ts.tv_nsec = NANOSECONDS;

    Duration sut{ts};
    EXPECT_THAT(sut, Eq(DurationAccessor::max()));
}

TEST(Duration_test, ConstructFromITimerspecWithZeroValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9dad794-793a-43a5-a10d-ba7ba7bf795a");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{0U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, NANOSECONDS);

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromITimerspecWithValueLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "01efb8a8-7698-43c6-b654-c77753dfd5b1");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t NANOSECONDS{642U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, NANOSECONDS);

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromITimerspecWithValueMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c68dbe5-7a95-499f-873c-93312bb6e645");
    constexpr uint64_t SECONDS{13U};
    constexpr uint64_t NANOSECONDS{42U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, NANOSECONDS);

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromITimerspecWithMaxValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed3643a1-c28e-404c-b9de-0a72c1ea427d");
    constexpr uint64_t SECONDS{std::numeric_limits<DurationAccessor::Seconds_t>::max()};
    constexpr uint64_t NANOSECONDS{NANOSECS_PER_SECOND - 1U};

    struct itimerspec its;
    its.it_interval.tv_sec = SECONDS;
    its.it_interval.tv_nsec = NANOSECONDS;

    Duration sut{its};
    EXPECT_THAT(sut, Eq(DurationAccessor::max()));
}

TEST(Duration_test, ConstructFromTimevalWithZeroValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2127562-8d96-4c32-aa48-4b0b6af27012");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t MICROSECONDS{0U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, MICROSECONDS * NANOSECS_PER_MICROSECOND);

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimevalWithValueLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa83386f-6e78-4f5b-880b-a952b1b3759e");
    constexpr uint64_t SECONDS{0U};
    constexpr uint64_t MICROSECONDS{13U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, MICROSECONDS * NANOSECS_PER_MICROSECOND);

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimevalWithValueMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "2c59493d-83e2-4230-9465-93f744b97b4b");
    constexpr uint64_t SECONDS{1337U};
    constexpr uint64_t MICROSECONDS{42U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, MICROSECONDS * NANOSECS_PER_MICROSECOND);

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromTimevalWithMaxValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "30063044-3b0c-4ec6-85e7-e89cf6d748f5");
    constexpr uint64_t SECONDS{std::numeric_limits<uint64_t>::max()};
    constexpr uint64_t MICROSECONDS{Duration::MICROSECS_PER_SEC - 1U};
    constexpr Duration EXPECTED_DURATION = createDuration(SECONDS, MICROSECONDS * Duration::NANOSECS_PER_MICROSEC);

    struct timeval tv;
    tv.tv_sec = SECONDS;
    tv.tv_usec = MICROSECONDS;

    Duration sut{tv};
    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, ConstructFromChronoMillisecondsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "40b02547-8a9d-4ae6-90b2-72e76e5143f0");
    constexpr uint64_t EXPECTED_MILLISECONDS{0U};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, ConstructFromChronoMillisecondsLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "ccbcd8df-c146-48ea-a9a0-40abaff31e68");
    constexpr uint64_t EXPECTED_MILLISECONDS{44U};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_MILLISECONDS * NANOSECS_PER_MILLISECOND));
}

TEST(Duration_test, ConstructFromChronoMillisecondsMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "04313e7e-2954-4741-ad0e-6f5a9e5aebce");
    constexpr uint64_t EXPECTED_MILLISECONDS{1001};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_MILLISECONDS * NANOSECS_PER_MILLISECOND));
}

TEST(Duration_test, ConstructFromChronoMillisecondsMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "553d4ab1-ff8a-437e-8be8-8105e62850c6");
    constexpr uint64_t EXPECTED_MILLISECONDS{std::numeric_limits<int64_t>::max()};
    Duration sut{std::chrono::milliseconds(EXPECTED_MILLISECONDS)};
    EXPECT_THAT(sut.toMilliseconds(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, ConstructFromNegativeChronoMillisecondsIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "181bc67d-1674-44eb-8784-710306efde30");
    Duration sut(std::chrono::milliseconds(-1));
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, ConstructFromChronoNanosecondsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "3044b00f-a765-417b-a16b-da01c16f7ed0");
    constexpr uint64_t EXPECTED_NANOSECONDS{0U};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromChronoNanosecondsLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "a81475d5-5732-44f4-91fc-950792808d7a");
    constexpr uint64_t EXPECTED_NANOSECONDS{424242U};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromChronoNanosecondsMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "43c49a79-28a6-4df4-862d-fc3aaa363191");
    constexpr uint64_t EXPECTED_NANOSECONDS{NANOSECS_PER_SECOND + 42U};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromChronoNanosecondsMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2520ffb-ecc9-4bbb-9f7e-0fbe05383efc");
    constexpr uint64_t EXPECTED_NANOSECONDS{std::numeric_limits<int64_t>::max()};
    Duration sut{std::chrono::nanoseconds(EXPECTED_NANOSECONDS)};
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConstructFromNegativeChronoNanosecondsIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "11993efd-9f4f-41b0-a07b-84732d7e6f71");
    Duration sut(std::chrono::nanoseconds(-1));
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

// END CONSTRUCTOR TESTS

// BEGIN ASSIGNMENT TESTS

TEST(Duration_test, AssignFromChronoMillisecondsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "65fe35e6-4610-4af3-8809-96c53c07f254");
    constexpr uint64_t EXPECTED_MILLISECONDS{0U};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, AssignFromChronoMillisecondsLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5b55852-9a03-4b1d-8a05-69a048388768");
    constexpr uint64_t EXPECTED_MILLISECONDS{73U};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_MILLISECONDS * NANOSECS_PER_MILLISECOND));
}

TEST(Duration_test, AssignFromChronoMillisecondsMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "0cc0ea5b-4ca8-4274-bef0-40ef83b023a0");
    constexpr uint64_t EXPECTED_MILLISECONDS{1073U};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_MILLISECONDS * NANOSECS_PER_MILLISECOND));
}

TEST(Duration_test, AssignFromChronoMillisecondsMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb4d634a-e35c-4c30-a918-e865f92cc7b1");
    constexpr uint64_t EXPECTED_MILLISECONDS{std::numeric_limits<int64_t>::max()};
    Duration sut = 0_ns;
    sut = std::chrono::milliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.toMilliseconds(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, AssignFromNegativeChronoMillisecondsIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "aaaa05b6-3195-43f6-813b-7c4d058248b6");
    Duration sut = 22_ns;
    sut = std::chrono::milliseconds(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

// END ASSIGNMENT TESTS

// BEGIN CREATION FROM LITERAL TESTS

TEST(Duration_test, CreateDurationFromDaysLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e1ff08d-4969-46a8-b781-1ad92ef1d792");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{2U * HOURS_PER_DAY * SECONDS_PER_HOUR * NANOSECS_PER_SECOND};
    auto sut = 2_d;

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromHoursLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "834cbe33-711d-4ea1-b3d3-06246f588efe");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{3U * SECONDS_PER_HOUR * NANOSECS_PER_SECOND};
    auto sut = 3_h;

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMinutesLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "58dcec41-09e3-4995-a594-dcdf13d2a670");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{4U * SECONDS_PER_MINUTE * NANOSECS_PER_SECOND};
    auto sut = 4_m;

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromSecondsLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "56640d8b-dde0-4355-84af-ded9be418375");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{5U * NANOSECS_PER_SECOND};
    auto sut = 5_s;

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMillisecondsLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "e19dec13-b5d4-46b7-9d23-9797a2163b5a");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{6U * NANOSECS_PER_MILLISECOND};
    auto sut = 6_ms;

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMicrosecondsLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "6138cd81-034d-46cd-8063-1eb0989823cd");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{7U * NANOSECS_PER_MICROSECOND};
    auto sut = 7_us;

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromNanosecondsLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "c05c8562-84fe-430e-9e3a-04b047aff48c");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{8U};
    auto sut = 8_ns;

    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

// END CREATION FROM LITERAL TESTS

// BEGIN CREATION FROM STATIC FUNCTION TESTS

TEST(Duration_test, CreateDurationFromDaysFunctionWithZeroDays)
{
    ::testing::Test::RecordProperty("TEST_ID", "bdac8676-3d59-4dff-922a-5eea4bf850b9");
    auto sut1 = Duration::fromDays(0);
    auto sut2 = Duration::fromDays(0U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(0U));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithMultipleDays)
{
    ::testing::Test::RecordProperty("TEST_ID", "25eeee95-1b1c-405e-a066-d1c35a9ec40e");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{2U * 24U * SECONDS_PER_HOUR * NANOSECS_PER_SECOND};
    auto sut1 = Duration::fromDays(2);
    auto sut2 = Duration::fromDays(2U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithDaysResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f34a190-d9bf-4206-b635-9d170df87053");
    constexpr uint64_t SECONDS_PER_DAY{HOURS_PER_DAY * SECONDS_PER_HOUR};
    constexpr uint64_t MAX_DAYS_BEFORE_OVERFLOW{std::numeric_limits<DurationAccessor::Seconds_t>::max()
                                                / SECONDS_PER_DAY};
    constexpr Duration EXPECTED_DURATION = createDuration(MAX_DAYS_BEFORE_OVERFLOW * SECONDS_PER_DAY, 0U);
    static_assert(EXPECTED_DURATION < DurationAccessor::max(),
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    auto sut1 = Duration::fromDays(static_cast<int64_t>(MAX_DAYS_BEFORE_OVERFLOW));
    auto sut2 = Duration::fromDays(MAX_DAYS_BEFORE_OVERFLOW);

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithMaxDaysResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "b802d652-b04c-41b5-a703-36af3a0dd1af");
    auto sut1 = Duration::fromDays(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::fromDays(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(DurationAccessor::max()));
    EXPECT_THAT(sut2, Eq(DurationAccessor::max()));
}

TEST(Duration_test, CreateDurationFromDaysFunctionWithNegativeValuesIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "f22a4350-b647-4ab1-b2c2-49056503e8f7");
    auto sut = Duration::fromDays(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithZeroHours)
{
    ::testing::Test::RecordProperty("TEST_ID", "35e6fd8d-8afe-4f85-a557-abc5e783b055");
    auto sut1 = Duration::fromHours(0);
    auto sut2 = Duration::fromHours(0U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(0U));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithMultipleHours)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd12fcff-2d2f-4790-a867-a62399115145");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{3U * SECONDS_PER_HOUR * NANOSECS_PER_SECOND};
    auto sut1 = Duration::fromHours(3);
    auto sut2 = Duration::fromHours(3U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithHoursResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "90d340f5-09e2-4b8b-a916-f2da6a28377f");
    constexpr uint64_t MAX_HOURS_BEFORE_OVERFLOW{std::numeric_limits<DurationAccessor::Seconds_t>::max()
                                                 / SECONDS_PER_HOUR};
    constexpr Duration EXPECTED_DURATION = createDuration(MAX_HOURS_BEFORE_OVERFLOW * SECONDS_PER_HOUR, 0U);
    static_assert(EXPECTED_DURATION < DurationAccessor::max(),
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    auto sut1 = Duration::fromHours(static_cast<int64_t>(MAX_HOURS_BEFORE_OVERFLOW));
    auto sut2 = Duration::fromHours(MAX_HOURS_BEFORE_OVERFLOW);

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithMaxHoursResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc6c3f1e-0329-48aa-8f0a-d67655a8128f");
    auto sut1 = Duration::fromHours(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::fromHours(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(DurationAccessor::max()));
    EXPECT_THAT(sut2, Eq(DurationAccessor::max()));
}

TEST(Duration_test, CreateDurationFromHoursFunctionWithNegativeValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "8180c3b7-1a60-40de-a5b5-21f81da46fbf");
    auto sut = Duration::fromHours(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithZeroMinuts)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d5b6083-2774-45fa-80d2-e31a38016a38");
    auto sut1 = Duration::fromMinutes(0);
    auto sut2 = Duration::fromMinutes(0U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(0U));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithMultipleMinutes)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb8eb9da-7bbd-400d-9ba0-132fa9150a90");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{4U * SECONDS_PER_MINUTE * NANOSECS_PER_SECOND};
    auto sut1 = Duration::fromMinutes(4);
    auto sut2 = Duration::fromMinutes(4U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithMinutesResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "21825950-16ff-4ade-b8f6-41fa8d00596a");
    constexpr uint64_t MAX_MINUTES_BEFORE_OVERFLOW{std::numeric_limits<DurationAccessor::Seconds_t>::max()
                                                   / SECONDS_PER_MINUTE};
    constexpr Duration EXPECTED_DURATION = createDuration(MAX_MINUTES_BEFORE_OVERFLOW * SECONDS_PER_MINUTE, 0U);
    static_assert(EXPECTED_DURATION < DurationAccessor::max(),
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    auto sut1 = Duration::fromMinutes(static_cast<int64_t>(MAX_MINUTES_BEFORE_OVERFLOW));
    auto sut2 = Duration::fromMinutes(MAX_MINUTES_BEFORE_OVERFLOW);

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithMaxMinutesResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0ef1468-048b-4a9f-8e96-ef68efc2ae97");
    auto sut1 = Duration::fromMinutes(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::fromMinutes(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(DurationAccessor::max()));
    EXPECT_THAT(sut2, Eq(DurationAccessor::max()));
}

TEST(Duration_test, CreateDurationFromMinutesFunctionWithNegativeValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "06cf0c89-7fcb-4b36-8c2a-000753820b74");
    auto sut = Duration::fromMinutes(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromSecondsFunctionWithZeroSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f221bb9-de92-4e59-b807-c4388c6adf72");
    auto sut1 = Duration::fromSeconds(0);
    auto sut2 = Duration::fromSeconds(0U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(0U));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromSecondsFunction)
{
    ::testing::Test::RecordProperty("TEST_ID", "85905c6d-b07b-4bd0-8d3d-e6a8e5b79c38");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{5U * NANOSECS_PER_SECOND};
    auto sut1 = Duration::fromSeconds(5);
    auto sut2 = Duration::fromSeconds(5U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromSecondsFunctionWithMaxSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba61d3c6-29ae-4b78-9a19-4c87bc791dd6");
    constexpr uint64_t MAX_SECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED = createDuration(MAX_SECONDS_FROM_SIGNED, 0U);
    constexpr uint64_t MAX_SECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED = createDuration(MAX_SECONDS_FROM_USIGNED, 0U);

    auto sut1 = Duration::fromSeconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::fromSeconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromSecondsFunctionWithNegativeValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "9a9125b2-5ec5-4315-89f5-362e1fddb7c8");
    auto sut = Duration::fromSeconds(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithZeroMilliseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "710070e8-e83e-4977-9aa3-9335b24a0feb");
    auto sut1 = Duration::fromMilliseconds(0);
    auto sut2 = Duration::fromMilliseconds(0U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(0U));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithMultipleMilliseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "1bd8f1fd-ca51-4f44-93f4-68680573cc34");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{6U * NANOSECS_PER_MILLISECOND};
    auto sut1 = Duration::fromMilliseconds(6);
    auto sut2 = Duration::fromMilliseconds(6U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithMaxMilliseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "cef856b5-31dc-4de7-8421-d6f2968ffde1");
    constexpr uint64_t MAX_MILLISECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED =
        createDuration(MAX_MILLISECONDS_FROM_SIGNED / MILLISECS_PER_SECOND,
                       (MAX_MILLISECONDS_FROM_SIGNED % MILLISECS_PER_SECOND) * NANOSECS_PER_MILLISECOND);
    constexpr uint64_t MAX_MILLISECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED =
        createDuration(MAX_MILLISECONDS_FROM_USIGNED / MILLISECS_PER_SECOND,
                       (MAX_MILLISECONDS_FROM_USIGNED % MILLISECS_PER_SECOND) * NANOSECS_PER_MILLISECOND);

    auto sut1 = Duration::fromMilliseconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::fromMilliseconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromMillisecondsFunctionWithNegativeValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "8578176b-7b7d-493d-8781-5146d4b408ee");
    auto sut = Duration::fromMilliseconds(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithZeroMicroseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f33fa38-9ce9-4e65-a63a-2b4353e0f346");
    auto sut1 = Duration::fromMicroseconds(0);
    auto sut2 = Duration::fromMicroseconds(0U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(0U));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithMultipleMicroseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f504b85-b810-4d59-b033-366a9964ba10");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{7U * NANOSECS_PER_MICROSECOND};
    auto sut1 = Duration::fromMicroseconds(7);
    auto sut2 = Duration::fromMicroseconds(7U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithMaxMicroseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "70a9e513-bedb-4c84-ba54-cee5c97ca049");
    constexpr uint64_t MAX_MICROSECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED =
        createDuration(MAX_MICROSECONDS_FROM_SIGNED / MICROSECS_PER_SECONDS,
                       (MAX_MICROSECONDS_FROM_SIGNED % MICROSECS_PER_SECONDS) * NANOSECS_PER_MICROSECOND);
    constexpr uint64_t MAX_MICROSECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED =
        createDuration(MAX_MICROSECONDS_FROM_USIGNED / MICROSECS_PER_SECONDS,
                       (MAX_MICROSECONDS_FROM_USIGNED % MICROSECS_PER_SECONDS) * NANOSECS_PER_MICROSECOND);

    auto sut1 = Duration::fromMicroseconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::fromMicroseconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromMicrosecondsFunctionWithNegativeValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f5adb66-f70b-4cad-b7b2-2f174853a1f5");
    auto sut = Duration::fromMicroseconds(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunctionWithZeroNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "b92b6fc8-8aa9-451d-94fc-b228cc19fbbe");
    auto sut1 = Duration::fromNanoseconds(0);
    auto sut2 = Duration::fromNanoseconds(0U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(0U));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(0U));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunctionWithMultipleNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef647589-11f9-4dd5-8919-078a0906f36c");
    constexpr uint64_t EXPECTED_DURATION_IN_NANOSECONDS{8U};
    auto sut1 = Duration::fromNanoseconds(8);
    auto sut2 = Duration::fromNanoseconds(8U);

    EXPECT_THAT(sut1.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
    EXPECT_THAT(sut2.toNanoseconds(), Eq(EXPECTED_DURATION_IN_NANOSECONDS));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunction)
{
    ::testing::Test::RecordProperty("TEST_ID", "9217dfe1-3b73-4e61-8abe-14ecfc9fc196");
    constexpr uint64_t MAX_NANOSECONDS_FROM_SIGNED{static_cast<uint64_t>(std::numeric_limits<int64_t>::max())};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_SIGNED = createDuration(
        MAX_NANOSECONDS_FROM_SIGNED / NANOSECS_PER_SECOND, MAX_NANOSECONDS_FROM_SIGNED % NANOSECS_PER_SECOND);
    constexpr uint64_t MAX_NANOSECONDS_FROM_USIGNED{std::numeric_limits<uint64_t>::max()};
    constexpr Duration EXPECTED_DURATION_FROM_MAX_UNSIGNED = createDuration(
        MAX_NANOSECONDS_FROM_USIGNED / NANOSECS_PER_SECOND, MAX_NANOSECONDS_FROM_USIGNED % NANOSECS_PER_SECOND);

    auto sut1 = Duration::fromNanoseconds(std::numeric_limits<int64_t>::max());
    auto sut2 = Duration::fromNanoseconds(std::numeric_limits<uint64_t>::max());

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION_FROM_MAX_SIGNED));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION_FROM_MAX_UNSIGNED));
}

TEST(Duration_test, CreateDurationFromNanosecondsFunctionWithNegativeValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "bebf67a5-1cc5-4073-85f5-95fd62fd4c15");
    auto sut = Duration::fromNanoseconds(-1);
    EXPECT_THAT(sut.toNanoseconds(), Eq(0U));
}

// END CREATION FROM STATIC FUNCTION TESTS

// BEGIN CONVERSION FUNCTION TESTS

TEST(Duration_test, ConvertDaysFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b5080da-7d39-4d17-9ad5-27b99cd940c6");
    auto sut = 0_s;
    EXPECT_THAT(sut.toDays(), Eq(0U));
}

TEST(Duration_test, ConvertDaysFromDurationLessThanOneDay)
{
    ::testing::Test::RecordProperty("TEST_ID", "04b6a134-0dbb-4787-935a-0774055c3402");
    auto sut = 3473_s;
    EXPECT_THAT(sut.toDays(), Eq(0U));
}

TEST(Duration_test, ConvertDaysFromDurationMoreThanOneDay)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3159329-de5c-43b4-b515-3fff00dfbfda");
    auto sut = 7_d + 3066_s;
    EXPECT_THAT(sut.toDays(), Eq(7U));
}

TEST(Duration_test, ConvertDaysFromMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "32d1bc55-d2c5-4d83-8386-d199dfffb66d");
    constexpr uint64_t SECONDS_PER_DAY{60U * 60U * 24U};
    constexpr uint64_t EXPECTED_DAYS{std::numeric_limits<DurationAccessor::Seconds_t>::max() / SECONDS_PER_DAY};
    auto sut = DurationAccessor::max();
    EXPECT_THAT(sut.toDays(), Eq(EXPECTED_DAYS));
}

TEST(Duration_test, ConvertHoursFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "40c1c2dc-9700-45b8-a0d1-a7898d5a1588");
    auto sut = 0_s;
    EXPECT_THAT(sut.toHours(), Eq(0U));
}

TEST(Duration_test, ConvertHoursFromDurationLessThanOneHour)
{
    ::testing::Test::RecordProperty("TEST_ID", "bac778e6-03b0-4704-9bd2-40bd5f872de4");
    auto sut = 37_m;
    EXPECT_THAT(sut.toHours(), Eq(0U));
}

TEST(Duration_test, ConvertHoursFromDurationMoreThanOneHour)
{
    ::testing::Test::RecordProperty("TEST_ID", "f52e35ce-8d3b-46db-a5ee-084c2d56c809");
    auto sut = 73_h + 42_m;
    EXPECT_THAT(sut.toHours(), Eq(73U));
}

TEST(Duration_test, ConvertHoursFromMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "622e78aa-cdc6-4f8c-8de7-3f9433752893");
    constexpr uint64_t SECONDS_PER_HOUR{60U * 60U};
    constexpr uint64_t EXPECTED_HOURS{std::numeric_limits<DurationAccessor::Seconds_t>::max() / SECONDS_PER_HOUR};
    auto sut = DurationAccessor::max();
    EXPECT_THAT(sut.toHours(), Eq(EXPECTED_HOURS));
}

TEST(Duration_test, ConvertMinutesFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "bff59c1e-260d-4bc2-9bae-95311b5085fa");
    auto sut = 0_s;
    EXPECT_THAT(sut.toMinutes(), Eq(0U));
}

TEST(Duration_test, ConvertMinutesFromDurationLessThanOneMinute)
{
    ::testing::Test::RecordProperty("TEST_ID", "19f8b9a7-45cc-4a20-884d-ae52804cd8bd");
    auto sut = 34_s;
    EXPECT_THAT(sut.toMinutes(), Eq(0U));
}

TEST(Duration_test, ConvertMinutesFromDurationMoreThanOneMinute)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f606d01-e9f7-4ab3-a16f-ee4068a4879b");
    auto sut = 13_m + 42_s;
    EXPECT_THAT(sut.toMinutes(), Eq(13U));
}

TEST(Duration_test, ConvertMinutesFromMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "f64e6a77-e481-4de7-bad0-0571d9aef1ad");
    constexpr uint64_t SECONDS_PER_MINUTE{60U};
    constexpr uint64_t EXPECTED_MINUTES{std::numeric_limits<DurationAccessor::Seconds_t>::max() / SECONDS_PER_MINUTE};
    auto sut = DurationAccessor::max();
    EXPECT_THAT(sut.toMinutes(), Eq(EXPECTED_MINUTES));
}

TEST(Duration_test, ConvertSecondsFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e892aed-b156-45ff-ac72-9b9986d9e388");
    auto sut = 0_s;
    EXPECT_THAT(sut.toSeconds(), Eq(0U));
}

TEST(Duration_test, ConvertSecondsFromDurationLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "d38be257-c6c9-476d-9c98-aec46da7db56");
    auto sut = 737_ms;
    EXPECT_THAT(sut.toSeconds(), Eq(0U));
}

TEST(Duration_test, ConvertSecondsFromDurationMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "fd393be4-4339-4138-843d-f386f93a59a2");
    auto sut = 7_s + 833_ms;
    EXPECT_THAT(sut.toSeconds(), Eq(7U));
}

TEST(Duration_test, ConvertSecondsFromMaxSecondsMinusOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "eaa9153d-8503-4bea-8c41-6f702e49914a");
    constexpr uint64_t EXPECTED_SECONDS{std::numeric_limits<DurationAccessor::Seconds_t>::max() - 1U};
    auto sut = DurationAccessor::max() - 1_s;
    EXPECT_THAT(sut.toSeconds(), Eq(EXPECTED_SECONDS));
}

TEST(Duration_test, ConvertSecondsFromMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "3109dfa7-9d2c-4b61-bb6f-e3ad256abed6");
    constexpr uint64_t EXPECTED_SECONDS{std::numeric_limits<DurationAccessor::Seconds_t>::max()};
    auto sut = DurationAccessor::max();
    EXPECT_THAT(sut.toSeconds(), Eq(EXPECTED_SECONDS));
}

TEST(Duration_test, ConvertMillisecondsFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "e56f9f62-7276-4291-a27a-52a2f4225618");
    auto sut = 0_s;
    EXPECT_THAT(sut.toMilliseconds(), Eq(0U));
}

TEST(Duration_test, ConvertMillisecondsFromDurationLessThanOneMillisecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "91b3de3d-07f1-42c0-94b6-a40279fe6ee2");
    auto sut = 637_us;
    EXPECT_THAT(sut.toMilliseconds(), Eq(0U));
}

TEST(Duration_test, ConvertMilliecondsFromDurationMoreThanOneMillisecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b5339ca-6583-4d9a-9bcd-f23a6c77021d");
    auto sut = 55_ms + 633_us;
    EXPECT_THAT(sut.toMilliseconds(), Eq(55U));
}

TEST(Duration_test, ConvertMillisecondsFromDurationResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "cff7dee2-7b72-48e2-a1e6-3fb7cebe065a");
    constexpr uint64_t EXPECTED_MILLISECONDS{std::numeric_limits<uint64_t>::max() - 1U};
    auto sut = Duration::fromMilliseconds(EXPECTED_MILLISECONDS);
    EXPECT_THAT(sut.toMilliseconds(), Eq(EXPECTED_MILLISECONDS));
}

TEST(Duration_test, ConvertMillisecondsFromMaxDurationResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2ca439c-f27e-4bde-9c45-1c3203106690");
    auto sut = DurationAccessor::max();
    EXPECT_THAT(sut.toMilliseconds(), Eq(std::numeric_limits<uint64_t>::max()));
}

TEST(Duration_test, ConvertMicrosecondsFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "869ab03f-0078-457b-a783-9a5eeeedaddb");
    auto sut = 0_s;
    EXPECT_THAT(sut.toMicroseconds(), Eq(0U));
}

TEST(Duration_test, ConvertMicrosecondsFromDurationLessThanOneMicrosecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "810d52f6-3685-4ad3-a5d2-c303ca258ddc");
    auto sut = 733_ns;
    EXPECT_THAT(sut.toMicroseconds(), Eq(0U));
}

TEST(Duration_test, ConvertMicrosecondsFromDurationMoreThanOneMicrosecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "d87656f2-7f27-49b0-92ad-eb3ac00cb3bf");
    auto sut = 555_us + 733_ns;
    EXPECT_THAT(sut.toMicroseconds(), Eq(555U));
}

TEST(Duration_test, ConvertMicrosecondsFromDurationResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "c194616a-fce7-46ef-9a43-cff4118c37c4");
    constexpr uint64_t EXPECTED_MICROSECONDS{std::numeric_limits<uint64_t>::max() - 1U};
    auto sut = Duration::fromMicroseconds(EXPECTED_MICROSECONDS);
    EXPECT_THAT(sut.toMicroseconds(), Eq(EXPECTED_MICROSECONDS));
}

TEST(Duration_test, ConvertMicroecondsFromMaxDurationResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "d98850e7-bd92-444f-8d51-3f2513a26924");
    auto sut = DurationAccessor::max();
    EXPECT_THAT(sut.toMicroseconds(), Eq(std::numeric_limits<uint64_t>::max()));
}

TEST(Duration_test, ConvertNanosecondsFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "1dc6de1b-e66e-4e41-ae94-e79c3f7d0fa9");
    auto sut = 0_s;
    EXPECT_THAT(sut.toMicroseconds(), Eq(0U));
}

TEST(Duration_test, ConvertNanosecondsFromDurationOfOneNanosecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2c50d7b-a520-44ea-8fe8-2e63436175f8");
    auto sut = 1_ns;
    EXPECT_THAT(sut.toNanoseconds(), Eq(1U));
}

TEST(Duration_test, ConvertNanosecondsFromDurationMultipleNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "784cb756-3a95-489b-8d2c-b8f3459ebcba");
    auto sut = 42_ns;
    EXPECT_THAT(sut.toNanoseconds(), Eq(42U));
}

TEST(Duration_test, ConvertNanosecondsFromDurationResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "35e83bed-0625-4515-9627-b443dc4e7b98");
    constexpr uint64_t EXPECTED_NANOSECONDS{std::numeric_limits<uint64_t>::max() - 1U};
    auto sut = Duration::fromNanoseconds(EXPECTED_NANOSECONDS);
    EXPECT_THAT(sut.toNanoseconds(), Eq(EXPECTED_NANOSECONDS));
}

TEST(Duration_test, ConvertNanoecondsFromMaxDurationResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "c1376842-8eb7-4878-a60d-19460033f48c");
    auto sut = DurationAccessor::max();
    EXPECT_THAT(sut.toNanoseconds(), Eq(std::numeric_limits<uint64_t>::max()));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a049ba6-885e-48c0-a9e8-3de6fa9a79f2");
    constexpr int64_t SECONDS{0};
    constexpr int64_t NANOSECONDS{0};

    auto duration = createDuration(SECONDS, NANOSECONDS);

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromDurationLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "d20435da-4585-44d9-9be7-24eb22c3ca85");
    constexpr int64_t SECONDS{0};
    constexpr int64_t NANOSECONDS{55};

    auto duration = createDuration(SECONDS, NANOSECONDS);

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromDurationMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "1b298bc3-3a0b-48a2-8d9c-1ee03dea925c");
    constexpr int64_t SECONDS{44};
    constexpr int64_t NANOSECONDS{55};

    auto duration = createDuration(SECONDS, NANOSECONDS);

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromDurationResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "70f11b99-78ec-442a-aefe-4dd9152b7903");
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{NANOSECS_PER_SECOND - 1U};

    auto duration = createDuration(SECONDS, NANOSECONDS);

    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithNoneReferenceFromMaxDurationResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bf4bb34-46f3-4889-84f5-9220b32fff73");
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{NANOSECS_PER_SECOND - 1U};

    struct timespec sut = DurationAccessor::max().timespec(iox::units::TimeSpecReference::None);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithMonotonicReference)
{
    ::testing::Test::RecordProperty("TEST_ID", "14d400bd-8109-4eec-a342-272ec0acea04");
    constexpr int64_t SECONDS{4};
    constexpr int64_t NANOSECONDS{66};

    auto timeSinceUnixEpoch = std::chrono::system_clock::now().time_since_epoch();
    auto timeSinceMonotonicEpoch = std::chrono::steady_clock::now().time_since_epoch();

    auto duration = createDuration(SECONDS, NANOSECONDS);
    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::Monotonic);

    auto secondsSinceUnixEpoch = std::chrono::duration_cast<std::chrono::seconds>(timeSinceUnixEpoch).count();
    auto secondsSinceMonotonicEpoch = std::chrono::duration_cast<std::chrono::seconds>(timeSinceMonotonicEpoch).count();
    EXPECT_THAT(sut.tv_sec, Lt(secondsSinceUnixEpoch));
    EXPECT_THAT(sut.tv_sec, Gt(secondsSinceMonotonicEpoch));
}

TEST(Duration_test, ConvertTimespecWithMonotonicReferenceFromMaxDurationResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff5a1fe2-65c8-490a-b31d-4d8ea615c91a");
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{NANOSECS_PER_SECOND - 1U};

    struct timespec sut = DurationAccessor::max().timespec(iox::units::TimeSpecReference::Monotonic);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

TEST(Duration_test, ConvertTimespecWithEpochReference)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef86dda6-7b0c-4e25-9f20-2acfe0cd9845");
    constexpr int64_t SECONDS{5};
    constexpr int64_t NANOSECONDS{77};

    auto timeSinceUnixEpoch = std::chrono::system_clock::now().time_since_epoch();

    auto duration = createDuration(SECONDS, NANOSECONDS);
    struct timespec sut = duration.timespec(iox::units::TimeSpecReference::Epoch);

    auto secondsSinceUnixEpoch = std::chrono::duration_cast<std::chrono::seconds>(timeSinceUnixEpoch).count();
    EXPECT_THAT(10 * SECONDS, Lt(secondsSinceUnixEpoch));
    EXPECT_THAT(sut.tv_sec, Gt(secondsSinceUnixEpoch));
}

TEST(Duration_test, ConvertTimespecWithEpochReferenceFromMaxDurationResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "97ff4204-a7f7-43ef-b81e-0e359d1dce93");
    constexpr int64_t SECONDS{std::numeric_limits<int64_t>::max()};
    constexpr int64_t NANOSECONDS{NANOSECS_PER_SECOND - 1U};

    struct timespec sut = DurationAccessor::max().timespec(iox::units::TimeSpecReference::Epoch);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_nsec, Eq(NANOSECONDS));
}

// END CONVERSION FUNCTION TESTS

// BEGIN CONVERSION OPERATOR TESTS

TEST(Duration_test, OperatorTimevalFromZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "10d3b209-093c-42c2-b3ab-2f2ac7e53836");
    auto duration = createDuration(0U, 0U);

    struct timeval sut = timeval(duration);

    EXPECT_THAT(sut.tv_sec, Eq(0U));
    EXPECT_THAT(sut.tv_usec, Eq(0U));
}

TEST(Duration_test, OperatorTimevalFromDurationWithLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc53677c-34ce-475f-b4d1-586c1189618a");
    constexpr int64_t SECONDS{0};
    constexpr int64_t MICROSECONDS{222};
    constexpr int64_t ROUND_OFF_NANOSECONDS{666};

    auto duration = createDuration(SECONDS, MICROSECONDS * NANOSECS_PER_MICROSECOND + ROUND_OFF_NANOSECONDS);

    struct timeval sut = timeval(duration);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_usec, Eq(MICROSECONDS));
}

TEST(Duration_test, OperatorTimevalFromDurationWithMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e23080e-e6e6-4bed-b6d1-758c06317f60");
    constexpr int64_t SECONDS{111};
    constexpr int64_t MICROSECONDS{222};
    constexpr int64_t ROUND_OFF_NANOSECONDS{666};

    auto duration = createDuration(SECONDS, MICROSECONDS * NANOSECS_PER_MICROSECOND + ROUND_OFF_NANOSECONDS);

    struct timeval sut = timeval(duration);

    EXPECT_THAT(sut.tv_sec, Eq(SECONDS));
    EXPECT_THAT(sut.tv_usec, Eq(MICROSECONDS));
}

TEST(Duration_test, OperatorTimevalFromDurationResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4dff7c7-178b-4b7a-a2b1-f6edfb9c2a22");
    using SEC_TYPE = decltype(timeval::tv_sec);
    auto duration = Duration::fromSeconds(std::numeric_limits<SEC_TYPE>::max());

    struct timeval sut = timeval(duration);

    EXPECT_THAT(sut.tv_sec, Eq(std::numeric_limits<SEC_TYPE>::max()));
    EXPECT_THAT(sut.tv_usec, Eq(0));
}

TEST(Duration_test, OperatorTimevalFromMaxDurationResultsInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "e3016dbf-bea7-4925-ab96-1674ee141905");
    using SEC_TYPE = decltype(timeval::tv_sec);
    using USEC_TYPE = decltype(timeval::tv_usec);

    struct timeval sut = timeval(DurationAccessor::max());

    EXPECT_THAT(sut.tv_sec, Eq(std::numeric_limits<SEC_TYPE>::max()));
    EXPECT_THAT(sut.tv_usec, Eq(static_cast<USEC_TYPE>(MICROSECS_PER_SECONDS - 1U)));
}

// END CONVERSION OPERATOR TESTS

// BEGIN COMPARISON TESTS

TEST(Duration_test, CompareTwoEqualDurationsForEquality)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b233585-559d-4baf-a8ac-6a291641c4b1");
    auto time1 = 200_us;
    auto time2 = 200000_ns;
    EXPECT_TRUE(time1 == time2);
}

TEST(Duration_test, CompareTwoNonEqualDurationsForEquality)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb531d5b-bce9-4cf1-bfb9-8e250b2e0068");
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 1_ns;
    auto time3 = 1_ns;
    EXPECT_FALSE(time1 == time2);
    EXPECT_FALSE(time2 == time1);
    EXPECT_FALSE(time2 == time3);
    EXPECT_FALSE(time3 == time2);
}

TEST(Duration_test, CompareTwoNonEqualDurationsForInequality)
{
    ::testing::Test::RecordProperty("TEST_ID", "49364301-ec17-4f27-8f1b-99da5d8f269e");
    auto time1 = 1_s + 200_us;
    auto time2 = 1_ns;
    EXPECT_TRUE(time1 != time2);
    EXPECT_TRUE(time2 != time1);
}

TEST(Duration_test, CompareTwoEqualDurationsForInequality)
{
    ::testing::Test::RecordProperty("TEST_ID", "a434aaa6-7549-405b-90bf-b46526c37c2e");
    auto time1 = 200_us;
    auto time2 = 200000_ns;
    EXPECT_FALSE(time1 != time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreNotLessThan)
{
    ::testing::Test::RecordProperty("TEST_ID", "29257e38-cdcb-45ae-b5d1-b1d2cbf9cf46");
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_FALSE(time1 < time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreNotGreaterThan)
{
    ::testing::Test::RecordProperty("TEST_ID", "b541d47f-58f5-46b6-af22-dc393c8b366d");
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_FALSE(time1 > time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreLessThanOrEqualTo)
{
    ::testing::Test::RecordProperty("TEST_ID", "6090f9f9-9caf-4a12-9e25-ea8c71d6fe78");
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_TRUE(time1 <= time2);
}

TEST(Duration_test, CompareTwoEqualDurationsAreGreaterThanOrEqualTo)
{
    ::testing::Test::RecordProperty("TEST_ID", "653aabbc-06ae-4fd5-912d-b270d888fab1");
    auto time1 = 1_s + 200_us;
    auto time2 = 1_s + 200_us;
    EXPECT_TRUE(time1 >= time2);
}

TEST(Duration_test, CompareDurationIsLessThanOther)
{
    ::testing::Test::RecordProperty("TEST_ID", "41f62afb-da29-46dc-93fa-2ad62a8a9b4a");
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
    ::testing::Test::RecordProperty("TEST_ID", "6399a6e1-523b-4ef1-864f-3a01d06214e1");
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
    ::testing::Test::RecordProperty("TEST_ID", "1f958f6a-5092-4592-a07f-bd358180006e");
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
    ::testing::Test::RecordProperty("TEST_ID", "2a31439d-0fe8-4883-aa61-0b74c1706481");
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
    ::testing::Test::RecordProperty("TEST_ID", "24719355-ccee-4307-b9fd-932ab5ee4b62");
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
    ::testing::Test::RecordProperty("TEST_ID", "e38e9982-40ca-4ec6-8097-57e45bb618f8");
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
    ::testing::Test::RecordProperty("TEST_ID", "6d9d9976-e438-4c23-8056-7e6476dd0330");
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
    ::testing::Test::RecordProperty("TEST_ID", "cf28e711-5fec-4176-8209-7c23acf848f3");
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

TEST(Duration_test, AddDurationDoesNotChangeOriginalObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "d146983f-6d39-420c-9398-e80fa6880887");
    constexpr Duration EXPECTED_DURATION{13_s + 42_ns};

    auto sut1 = EXPECTED_DURATION;
    auto result1 IOX_MAYBE_UNUSED = sut1 + 15_s;

    auto sut2 = EXPECTED_DURATION;
    auto result2 IOX_MAYBE_UNUSED = 15_s + sut1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationWithTwoZeroDurationsResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "64aa64f9-0105-43a1-bdcb-7b62f12fd86b");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration1 = 0_s;
    auto duration2 = 0_s;

    auto sut = duration1 + duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationWithOneZeroDurationsResultsInNoneZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "32f4f923-057c-4481-b50e-78a8fd4bceed");
    constexpr Duration EXPECTED_DURATION{10_ns};
    auto duration1 = 0_s;
    auto duration2 = 10_ns;

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationWithSumOfDurationsLessThanOneSecondsResultsInLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3704e3c-588a-4c1f-a09b-9244c210d853");
    constexpr Duration EXPECTED_DURATION = createDuration(0U, 100 * NANOSECS_PER_MICROSECOND + 10U);
    auto duration1 = 100_us;
    auto duration2 = 10_ns;

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationWithSumOfDurationsMoreThanOneSecondsResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "62ad6686-990f-4c3e-b1ff-d61238a2d8c2");
    constexpr Duration EXPECTED_DURATION = createDuration(1U, 700 * NANOSECS_PER_MILLISECOND);
    auto duration1 = 800_ms;
    auto duration2 = 900_ms;

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationWithOneDurationMoreThanOneSecondsResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "435175e4-33d1-4829-a526-4b01268233fc");
    constexpr Duration EXPECTED_DURATION = createDuration(2U, 700 * NANOSECS_PER_MILLISECOND);
    auto duration1 = createDuration(1U, 800 * NANOSECS_PER_MILLISECOND);
    auto duration2 = 900_ms;

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationWithDurationsMoreThanOneSecondsResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "a509667d-507a-4521-abae-f24879367a0a");
    constexpr Duration EXPECTED_DURATION = createDuration(3U, 700 * NANOSECS_PER_MILLISECOND);
    auto duration1 = createDuration(1U, 800 * NANOSECS_PER_MILLISECOND);
    auto duration2 = createDuration(1U, 900 * NANOSECS_PER_MILLISECOND);

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "289d9f13-01e0-4635-875b-5b239dc6f102");
    constexpr Duration EXPECTED_DURATION =
        createDuration(std::numeric_limits<DurationAccessor::Seconds_t>::max(), NANOSECS_PER_SECOND - 2U);
    auto duration1 =
        createDuration(std::numeric_limits<DurationAccessor::Seconds_t>::max() - 1U, NANOSECS_PER_SECOND - 1U);
    auto duration2 = createDuration(0U, NANOSECS_PER_SECOND - 1U);

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, AddDurationResultsInSaturationFromNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "179b2630-1c16-4955-95e2-8dbd69429162");
    auto duration1 = createDuration(std::numeric_limits<DurationAccessor::Seconds_t>::max(), NANOSECS_PER_SECOND - 2U);
    auto duration2 = createDuration(0U, 2U);

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(DurationAccessor::max()));
    EXPECT_THAT(sut2, Eq(DurationAccessor::max()));
}

TEST(Duration_test, AddDurationResultsInSaturationFromSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "11766b56-cf0e-4fbe-a8a2-a167b6e4a36a");
    auto duration1 =
        createDuration(std::numeric_limits<DurationAccessor::Seconds_t>::max() - 1U, NANOSECS_PER_SECOND - 1U);
    auto duration2 = createDuration(2U, 0U);

    auto sut1 = duration1 + duration2;
    auto sut2 = duration2 + duration1;

    EXPECT_THAT(sut1, Eq(DurationAccessor::max()));
    EXPECT_THAT(sut2, Eq(DurationAccessor::max()));
}

TEST(Duration_test, SubtractDurationDoesNotChangeOriginalObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "c61d7d6e-4164-4c00-b264-7ed62ad22748");
    constexpr Duration EXPECTED_DURATION{13_s + 42_ns};

    auto sut1 = EXPECTED_DURATION;
    auto result1 IOX_MAYBE_UNUSED = sut1 - 5_s;

    auto sut2 = EXPECTED_DURATION;
    auto result2 IOX_MAYBE_UNUSED = 35_s + sut1;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationWithTwoZeroDurationsResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "e1bc9ccf-2702-498a-b069-750f49022e17");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration1 = 0_s;
    auto duration2 = 0_s;

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationWithDurationsWithSameValueResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "20aa9e6a-880d-4839-b31d-77efcc3daa9d");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration1 = createDuration(10U, 123U);
    auto duration2 = createDuration(10U, 123U);

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationFromZeroDurationsResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "17f83a4a-a1b7-4f34-9eb8-c1ffaf40ffde");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration0 = 0_s;
    auto duration1 = 10_ns;
    auto duration2 = 10_s;

    auto sut1 = duration0 - duration1;
    auto sut2 = duration0 - duration2;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationWithLargerDurationsResultsInZeroDurationFromNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c60f928-50d1-4309-b01c-ffda9ebd5594");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration1 = 10_ns;
    auto duration2 = 110_ns;

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationWithLargerDurationsResultsInZeroDurationFromSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b0eded3-616e-42b2-9143-0407b5687728");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration1 = createDuration(10U, 123U);
    auto duration2 = createDuration(100U, 123U);

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationWithZeroDurationsResultsInOriginaDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "c661946d-ad4f-4a64-bdc2-b7e5b98be1fd");
    constexpr Duration EXPECTED_DURATION = createDuration(10U, 42U);
    auto duration1 = EXPECTED_DURATION;
    auto duration2 = 0_s;

    auto sut = duration1 + duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationMoreThanOneSecondWithLessThanOneSecondResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "83838524-402c-46b5-a1ee-2dd09c950148");
    constexpr Duration EXPECTED_DURATION = createDuration(1U, 36U);
    auto duration1 = createDuration(1U, 73U);
    auto duration2 = createDuration(0U, 37U);

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationMoreThanOneSecondWithLessThanOneSecondResultsInLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "51df6a6c-7c22-4b9e-b406-229a8ddbf49d");
    constexpr Duration EXPECTED_DURATION = createDuration(0U, NANOSECS_PER_SECOND - 36U);
    auto duration1 = createDuration(1U, 37U);
    auto duration2 = createDuration(0U, 73U);

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationMoreThanOneSecondWithMoreThanOneSecondResultsInLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "3416550c-2cb9-4cbc-9746-6eedab32cca3");
    constexpr Duration EXPECTED_DURATION = createDuration(0U, 36U);
    auto duration1 = createDuration(1U, 73U);
    auto duration2 = createDuration(1U, 37U);

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, SubtractDurationWithSecondsAndNanosecondsCausingReductionOfSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb9ca012-f5cc-4ab1-a3be-a63d48f1319f");
    constexpr Duration EXPECTED_DURATION = createDuration(0U, NANOSECS_PER_SECOND - 36U);
    auto duration1 = createDuration(2U, 37U);
    auto duration2 = createDuration(1U, 73U);

    auto sut = duration1 - duration2;

    EXPECT_THAT(sut, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, MultiplyDurationDoesNotChangeOriginalObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "c243775f-7a9b-42bf-8bd7-78bc843f0953");
    constexpr Duration EXPECTED_DURATION{13_s + 42_ns};

    auto sut1 = EXPECTED_DURATION;
    auto result1 IOX_MAYBE_UNUSED = sut1 * 0;

    auto sut2 = EXPECTED_DURATION;
    auto result2 IOX_MAYBE_UNUSED = sut2 * 0;

    EXPECT_THAT(sut1, Eq(EXPECTED_DURATION));
    EXPECT_THAT(sut2, Eq(EXPECTED_DURATION));
}

template <typename T>
void multiply(Duration duration, T multiplicator, const Duration expectedDuration)
{
    auto sut1 = duration * multiplicator;
    auto sut2 = multiplicator * duration;

    EXPECT_THAT(sut1, Eq(expectedDuration));
    EXPECT_THAT(sut2, Eq(expectedDuration));
}

TEST(Duration_test, MultiplyZeroDurationWithZeroSignedMultiplicatorResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "95c85fb3-c8a8-4170-9c7a-28f7dc9b8523");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration = 0_s;

    multiply(duration, 0, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyZeroDurationWithZeroUnsignedMultiplicatorResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9568756-0c13-49aa-883a-94f4720fb945");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration = 0_s;

    multiply(duration, 0U, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyZeroDurationWithZeroFloatMultiplicatorResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "5090f23c-eb59-4e6d-9399-f62940af5e50");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration = 0_s;

    multiply(duration, 0.0, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationWithZeroSignedMultiplicatorResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "88fc2bf9-6748-4bbd-aa9e-0d7e73afbeff");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration = 1_s + 12_ns;

    multiply(duration, 0, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationWithZeroUnsignedMultiplicatorResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d224e46-3055-4b88-8f65-26850fb3ec53");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration = 1_s + 12_ns;

    multiply(duration, 0U, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationWithZeroFloatMultiplicatorResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "b867e055-64f2-44fb-a85d-69c11af75c69");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration = 1_s + 12_ns;

    multiply(duration, 0.0, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationLessThanOneSecondWithSignedResultsInLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "73ecd04c-1054-4aed-b4d6-bf782f7f9cb5");
    constexpr Duration EXPECTED_DURATION{36_ns};
    auto duration = 12_ns;

    multiply(duration, 3, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationLessThanOneSecondWithUnsignedResultsInLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "54c235b2-cc18-4007-be85-32847d94fe54");
    constexpr Duration EXPECTED_DURATION{36_ns};
    auto duration = 12_ns;

    multiply(duration, 3U, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationLessThanOneSecondWithFloatResultsInLessThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "6193219f-06cb-4f63-b223-d06157eaf559");
    constexpr Duration EXPECTED_DURATION{42_ns};
    auto duration = 12_ns;

    multiply(duration, 3.5, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationLessThanOneSecondWithSignedResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "29c9f01c-7a4f-462f-8293-fc08c7e27f64");
    constexpr Duration EXPECTED_DURATION{1_s + 800_ms};
    auto duration = 600_ms;

    multiply(duration, 3, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationLessThanOneSecondWithUnsignedResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "6fdb2c21-fc0c-43b2-8120-b459e6e9f571");
    constexpr Duration EXPECTED_DURATION{1_s + 800_ms};
    auto duration = 600_ms;

    multiply(duration, 3U, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationLessThanOneSecondWithFloatResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "39759894-77e0-4b0d-9279-d787367a76ab");
    constexpr Duration EXPECTED_DURATION{2_s + 100_ms};
    auto duration = 600_ms;

    multiply(duration, 3.5, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationMoreThanOneSecondWithSignedResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "6467c466-1720-4336-acec-06bd11a43efc");
    constexpr Duration EXPECTED_DURATION{13_s + 800_ms};
    auto duration = 4_s + 600_ms;

    multiply(duration, 3, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationMoreThanOneSecondWithUnsignedResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "9db6e3ec-bf24-4d31-a88a-18077d1b1674");
    constexpr Duration EXPECTED_DURATION{13_s + 800_ms};
    auto duration = 4_s + 600_ms;

    multiply(duration, 3U, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationMoreThanOneSecondWithFloatResultsInMoreThanOneSecond)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b42fbe9-7255-4669-bd4c-893765b83a4a");
    constexpr Duration EXPECTED_DURATION{16_s + 100_ms};
    auto duration = 4_s + 600_ms;

    multiply(duration, 3.5, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationWithFractionalFloat)
{
    ::testing::Test::RecordProperty("TEST_ID", "3adcaec4-06fb-4ae5-a05b-70764fc00d64");
    constexpr Duration EXPECTED_DURATION{2_s + 800_ms};
    auto duration = 5_s + 600_ms;

    multiply(duration, 0.5, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationWithNegativMultiplicatorResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bfc4c50-1673-4338-be38-9d9b6d058cc8");
    constexpr Duration EXPECTED_DURATION{0_s};
    auto duration = 4_s + 600_ms;

    multiply(duration, -1, EXPECTED_DURATION);
    multiply(duration, -1.0, EXPECTED_DURATION);
}

TEST(Duration_test, MultiplyDurationLessThanOneSecondResultsInMoreNanosecondsThan64BitCanRepresent)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f1bbcb3-3692-4895-a36f-38eed7775b6f");
    constexpr uint64_t MULTIPLICATOR{(1ULL << 32U) * 42U + 73U};
    constexpr Duration DURATION = 473_ms + 578_us + 511_ns;
    auto EXPECTED_RESULT = createDuration(85428177141U, 573034055U);

    auto result = MULTIPLICATOR * DURATION;
    EXPECT_THAT(result, Eq(EXPECTED_RESULT));
    EXPECT_THAT(result.toNanoseconds(), Eq(std::numeric_limits<uint64_t>::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(EXPECTED_RESULT));
}

TEST(Duration_test, MultiplyDurationResultsNotYetInSaturation)
{
    ::testing::Test::RecordProperty("TEST_ID", "b50e3ebf-fa29-4498-9318-491ba030a0cc");
    constexpr uint64_t MULTIPLICATOR{1343535617188545796U};
    constexpr Duration DURATION = 13_s + 730_ms + 37_ns;
    constexpr Duration EXPECTED_DURATION =
        createDuration(std::numeric_limits<DurationAccessor::Seconds_t>::max(), 56194452U);
    static_assert(EXPECTED_DURATION < DurationAccessor::max(),
                  "EXPECTED_DURATION too large to exclude saturation! Please decrease!");

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(EXPECTED_DURATION));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, MultiplyDurationResultsInSaturationDueToSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba59abd8-cdb8-462d-b0b5-c45f7dc80657");
    constexpr uint64_t MULTIPLICATOR{1343535617188545797U};
    constexpr Duration DURATION = 14_s;

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(DurationAccessor::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyDurationResultsInSaturationDueToNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0cd4e22-29c5-440f-bad5-fe4d86ebe8ed");
    constexpr uint64_t MULTIPLICATOR{1343535617188545797U};
    constexpr Duration DURATION = 13_s + 730_ms + 37_ns;

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(DurationAccessor::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyZeroDurationWithNaNDoubleResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "4639ac39-639f-4df9-b591-05714f1cbc30");
    EXPECT_THAT(0_s * NAN, Eq(0_s));
}

TEST(Duration_test, MultiplyMaxDurationWithNaNDoubleResultsInMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "e09e4248-284d-4ab7-993b-1474ac5e3b11");
    EXPECT_THAT(DurationAccessor::max() * NAN, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyZeroDurationWithPosInfDoubleResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "71dea32f-1200-4df2-8eff-ea607f1b6a01");
    EXPECT_THAT(0_s * INFINITY, Eq(0_ns));
}

TEST(Duration_test, MultiplyMaxDurationWithPosInfDoubleResultsInMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f66a93a-2df1-4f7d-abbf-03d5424a1534");
    EXPECT_THAT(DurationAccessor::max() * INFINITY, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyZeroDurationWithNegInfDoubleResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "3931a3df-e09f-414a-8a1e-64bcb1e010b7");
    EXPECT_THAT(0_s * (INFINITY * -1.0), Eq(0_ns));
}

TEST(Duration_test, MultiplyMaxDurationWithNegInfDoubleResultsInZeroDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f563db0-85fa-4558-8928-8fe730f3ff47");
    EXPECT_THAT(DurationAccessor::max() * (INFINITY * -1.0), Eq(0_ns));
}

TEST(Duration_test, MultiplyDurationWithMinimalFloatResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2e9ffc7-a33d-4c2e-ac93-57f621fd66f9");
    constexpr float MULTIPLICATOR{std::numeric_limits<float>::min()};
    constexpr Duration DURATION = 13_s + 730_ms + 37_ns;
    auto EXPECTED_DURATION = createDuration(0U, 0U);

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(EXPECTED_DURATION));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, MultiplyDurationWithMinimalDoubleResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "29be653f-8ec7-4ab6-b2ea-501bf7e78c1d");
    constexpr double MULTIPLICATOR{std::numeric_limits<double>::min()};
    constexpr Duration DURATION = 13_s + 730_ms + 37_ns;
    auto EXPECTED_DURATION = createDuration(0U, 0U);

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(EXPECTED_DURATION));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(EXPECTED_DURATION));
}

TEST(Duration_test, MultiplyMaxDurationWithFloatOneResultsInMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "c700d04c-8847-4234-ad94-a08bb57bac9a");
    EXPECT_THAT(DurationAccessor::max() * 1.0, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyMaxDurationWithDoubleOneResultsInMaxDuration)
{
    ::testing::Test::RecordProperty("TEST_ID", "c78a9d3a-2613-4b67-ba8b-a7a7b368a4ed");
    EXPECT_THAT(DurationAccessor::max() * 1.0, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyDurationWithFloatResultsInSaturationDueToSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "085de609-5f38-4a63-8719-f15263ca448b");
    constexpr float MULTIPLICATOR{1343535617188545797.0};
    constexpr Duration DURATION = 14_s;

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(DurationAccessor::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyDurationWithDoubleResultsInSaturationDueToSeconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cc27397-5e7f-45bd-b5a1-bbdb78920daf");
    constexpr double MULTIPLICATOR{1343535617188545797.0};
    constexpr Duration DURATION = 14_s;

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(DurationAccessor::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyDurationWithFloatResultsInSaturationDueToNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "38ad7f5d-480a-4c4b-8ff0-8a24f3d6f2a6");
    constexpr float MULTIPLICATOR{1343535617188545797.0};
    constexpr Duration DURATION = 13_s + 930_ms + 37_ns;

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(DurationAccessor::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(DurationAccessor::max()));
}

TEST(Duration_test, MultiplyDurationWithDoubleResultsInSaturationDueToNanoseconds)
{
    ::testing::Test::RecordProperty("TEST_ID", "2c9f05a8-8392-4924-8a76-fd1447aa675e");
    constexpr double MULTIPLICATOR{1343535617188545797.0};
    constexpr Duration DURATION = 13_s + 930_ms + 37_ns;

    EXPECT_THAT(MULTIPLICATOR * DURATION, Eq(DurationAccessor::max()));
    EXPECT_THAT(DURATION * MULTIPLICATOR, Eq(DurationAccessor::max()));
}

TEST(Duration_test, StreamingOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "526d6cf3-b3df-44ce-8668-a0a170c94919");
    std::stringstream capture;
    auto clogBuffer = std::clog.rdbuf();
    std::clog.rdbuf(capture.rdbuf());

    capture.str("");
    std::clog << 0_s;
    EXPECT_STREQ(capture.str().c_str(), "0s 0ns");

    capture.str("");
    std::clog << 42_ns;
    EXPECT_STREQ(capture.str().c_str(), "0s 42ns");

    capture.str("");
    std::clog << (13_s + 73_ms + 37_us + 42_ns);
    EXPECT_STREQ(capture.str().c_str(), "13s 73037042ns");

    std::clog.rdbuf(clogBuffer);
}

// END ARITHMETIC TESTS
} // namespace
