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

#include "test.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

using namespace ::testing;
using namespace iox::units;

constexpr int KILO = 1000;
constexpr int MEGA = 1000000;
constexpr int GIGA = 1000000000;
constexpr double doubleEpsilon = std::numeric_limits<double>::epsilon();

TEST(Duration_test, convertDays)
{
    auto time = 10.0_d;
    EXPECT_DOUBLE_EQ(time.days<double>(), 10.0);
    EXPECT_DOUBLE_EQ(time.hours<double>(), 240.0);
    EXPECT_DOUBLE_EQ(time.minutes<double>(), 14400.0);
    EXPECT_DOUBLE_EQ(time.seconds<double>(), 864000.0);
    EXPECT_DOUBLE_EQ(time.milliSeconds<double>(), 864000.0 * KILO);
    EXPECT_DOUBLE_EQ(time.microSeconds<double>(), 864000.0 * MEGA);
    EXPECT_DOUBLE_EQ(time.nanoSeconds<double>(), 864000.0 * GIGA);
}

TEST(Duration_test, convertHours)
{
    auto time = 0.5_h;
    EXPECT_DOUBLE_EQ(time.days<double>(), 1.0 / 48.0);
    EXPECT_DOUBLE_EQ(time.hours<double>(), 0.5);
    EXPECT_DOUBLE_EQ(time.minutes<double>(), 30.0);
    EXPECT_DOUBLE_EQ(time.seconds<double>(), 1800.0);
    EXPECT_DOUBLE_EQ(time.milliSeconds<double>(), 1800.0 * KILO);
    EXPECT_DOUBLE_EQ(time.microSeconds<double>(), 1800.0 * MEGA);
    EXPECT_DOUBLE_EQ(time.nanoSeconds<double>(), 1800.0 * GIGA);
}

TEST(Duration_test, convertMinutes)
{
    auto time = 720_m;
    EXPECT_DOUBLE_EQ(time.days<double>(), 0.5);
    EXPECT_DOUBLE_EQ(time.hours<double>(), 12.0);
    EXPECT_DOUBLE_EQ(time.minutes<double>(), 720.0);
    EXPECT_DOUBLE_EQ(time.seconds<double>(), 43200.0);
    EXPECT_DOUBLE_EQ(time.milliSeconds<double>(), 43200.0 * KILO);
    EXPECT_DOUBLE_EQ(time.microSeconds<double>(), 43200.0 * MEGA);
    EXPECT_DOUBLE_EQ(time.nanoSeconds<double>(), 43200.0 * GIGA);
}

TEST(Duration_test, convertSeconds)
{
    auto time = 28800_s;
    EXPECT_DOUBLE_EQ(time.days<double>(), 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(time.hours<double>(), 8.0);
    EXPECT_DOUBLE_EQ(time.minutes<double>(), 480.0);
    EXPECT_DOUBLE_EQ(time.seconds<double>(), 28800.0);
    EXPECT_DOUBLE_EQ(time.milliSeconds<double>(), 28800.0 * KILO);
    EXPECT_DOUBLE_EQ(time.microSeconds<double>(), 28800.0 * MEGA);
    EXPECT_DOUBLE_EQ(time.nanoSeconds<double>(), 28800.0 * GIGA);
}

TEST(Duration_test, convertMilliseconds)
{
    auto time = 28800000_ms;
    EXPECT_DOUBLE_EQ(time.days<double>(), 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(time.hours<double>(), 8.0);
    EXPECT_DOUBLE_EQ(time.minutes<double>(), 480.0);
    EXPECT_DOUBLE_EQ(time.seconds<double>(), 28800.0);
    EXPECT_DOUBLE_EQ(time.milliSeconds<double>(), 28800.0 * KILO);
    EXPECT_DOUBLE_EQ(time.microSeconds<double>(), 28800.0 * MEGA);
    EXPECT_DOUBLE_EQ(time.nanoSeconds<double>(), 28800.0 * GIGA);
}

TEST(Duration_test, convertMicroseconds)
{
    auto time = 6000000_us;
    EXPECT_DOUBLE_EQ(time.days<double>(), 0.1 / 1440.0);
    EXPECT_DOUBLE_EQ(time.hours<double>(), 0.1 / 60.0);
    EXPECT_DOUBLE_EQ(time.minutes<double>(), 0.1);
    EXPECT_DOUBLE_EQ(time.seconds<double>(), 6.0);
    EXPECT_DOUBLE_EQ(time.milliSeconds<double>(), 6.0 * KILO);
    EXPECT_DOUBLE_EQ(time.microSeconds<double>(), 6.0 * MEGA);
    EXPECT_DOUBLE_EQ(time.nanoSeconds<double>(), 6.0 * GIGA);
}

TEST(Duration_test, convertNanoseconds)
{
    auto time = 1_ns;
    EXPECT_DOUBLE_EQ(time.seconds<double>(), 1.0 / GIGA);
    EXPECT_DOUBLE_EQ(time.milliSeconds<double>(), 1.0 / MEGA);
    EXPECT_DOUBLE_EQ(time.microSeconds<double>(), 1.0 / KILO);
    EXPECT_DOUBLE_EQ(time.nanoSeconds<double>(), 1.0);
}

TEST(Duration_test, comparison)
{
    auto time1 = 200_us;
    auto time2 = 200.0_us;
    EXPECT_EQ(time1.seconds<double>(), time2.seconds<double>());

    auto time3 = 0.21_ms;
    EXPECT_LT(time1, time3);
    EXPECT_GT(time3, time2);
}

TEST(Duration_test, timespec)
{
    auto time = 2.5_s;
    struct timespec t3;
    t3.tv_sec = 2;
    t3.tv_nsec = 500000000;

    struct timespec t4 = timespec(time.timespec(iox::units::TimeSpecReference::None));

    EXPECT_EQ(t3.tv_sec + t3.tv_nsec * 0.000000001, t4.tv_sec + t4.tv_nsec * 0.000000001);
}

TEST(Duration_test, timeval)
{
    auto time = 2.5_s;
    struct timeval t1;
    t1.tv_sec = 2;
    t1.tv_usec = 500000;

    struct timeval t2 = timeval(time);

    EXPECT_EQ(t1.tv_sec + t1.tv_usec * 0.000001, t2.tv_sec + t2.tv_usec * 0.000001);
}

TEST(Duration_test, addDuration)
{
    auto time1 = 5.2_s;
    auto time2 = 200_ms;
    auto time3 = 300000_us;

    auto result = time1 + time2;
    EXPECT_DOUBLE_EQ(result.milliSeconds<double>(), 5400.0);
    result = result + time3;
    EXPECT_DOUBLE_EQ(result.microSeconds<double>(), 5700000.0);
}

TEST(Duration_test, subtractDuration)
{
    auto time1 = 5.2_s;
    auto time2 = 200_ms;
    auto time3 = 300000_us;

    auto result = time1 - time2;
    EXPECT_DOUBLE_EQ(result.milliSeconds<double>(), 5000.0);
    result = result - time3;
    EXPECT_DOUBLE_EQ(result.microSeconds<double>(), 4700000.0);

    auto time4 = 10_s;

    result = result - time4;
    EXPECT_EQ(result.milliSeconds<double>(), 0.0);
}

TEST(Duration_test, multiplyDuration)
{
    auto time1 = 5.2_s;
    auto time2 = 200_ms;
    auto time3 = 300000_us;

    auto result = time1 * time2;
    EXPECT_NEAR(result.seconds<double>(), 1.04, doubleEpsilon);
    result = result * time3;
    EXPECT_NEAR(result.seconds<double>(), 0.312, doubleEpsilon);

    auto time4 = 10_s;

    result = result * time4;
    EXPECT_NEAR(result.seconds<double>(), 3.12, doubleEpsilon);
}

TEST(Duration_test, divideDuration)
{
    auto time1 = 5.2_s;
    auto time2 = 200_ms;
    auto time3 = 300000_us;

    auto result = time1 / time2;
    EXPECT_NEAR(result.seconds<double>(), 26, doubleEpsilon);
    result = result / time3;
    EXPECT_NEAR(result.seconds<double>(), 86.666666666666666, doubleEpsilon);

    auto time4 = 10_s;

    result = result / time4;
    EXPECT_NEAR(result.seconds<double>(), 8.666666666666666, doubleEpsilon);
}

TEST(Duration_test, constructFromTimespec)
{
    struct timespec value;
    value.tv_sec = 123;
    value.tv_nsec = 456;

    Duration result(value);
    EXPECT_EQ(result.nanoSeconds<uint64_t>(), 456ull + 1000000000ull * 123ull);
}

TEST(Duration_test, constructFromTimeval)
{
    struct timeval value;
    value.tv_sec = 1337;
    value.tv_usec = 42;

    Duration result(value);
    EXPECT_EQ(result.microSeconds<uint64_t>(), 42 + 1000000 * 1337);
}
