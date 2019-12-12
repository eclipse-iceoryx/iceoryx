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

#include "iceoryx_utils/posix_wrapper/timer.hpp"
#include "test.hpp"

#include <atomic>
#include <thread>

using namespace ::testing;

using namespace iox::units::duration_literals;

using Timer = iox::posix::Timer;

class Timer_test : public Test
{
  public:
    virtual void SetUp()
    {
        numberOfCalls = 0;
    }

    virtual void TearDown()
    {
    }

    iox::units::Duration second{1_s};

    std::atomic<int> numberOfCalls{0};
};

TEST_F(Timer_test, DISABLED_CreateAndFireOnce_PERFORMANCETEST42)
{
    Timer osTimer(second, [&]() { numberOfCalls++; });

    EXPECT_THAT(osTimer.hasError(), Eq(false));

    // Save the time before start()
    auto start = Timer::now();
    osTimer.start(Timer::RunMode::ONCE);

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    auto end = osTimer.now();

    // Calc the difference
    auto timediff = end.get_value() - start.get_value();

    EXPECT_THAT(numberOfCalls, Eq(1));

    // Just check for magnitude, allow up to 10% jitter for Jenkins and VMs
    ASSERT_LE(timediff, 1.2_s);

    osTimer.stop();
}

TEST_F(Timer_test, DISABLED_CreateAndStop_PERFORMANCETEST42)
{
    Timer osTimer(second, [&]() { numberOfCalls++; });

    EXPECT_THAT(osTimer.hasError(), Eq(false));

    osTimer.start(Timer::RunMode::ONCE);

    // Stop the timer and wait to make sure it does not fire
    osTimer.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    EXPECT_THAT(numberOfCalls, Eq(0));
}
TEST_F(Timer_test, DISABLED_CreateAndFirePeriodically_PERFORMANCETEST42)
{
    Timer osTimer(second, [&]() { numberOfCalls++; });

    EXPECT_THAT(osTimer.hasError(), Eq(false));

    auto start = osTimer.now();
    osTimer.start(Timer::RunMode::PERIODIC);

    std::this_thread::sleep_for(std::chrono::milliseconds(3200));

    auto end = osTimer.now();

    // Calc the difference
    auto timediff = end.get_value() - start.get_value();

    osTimer.stop();

    // Just check for magnitude, allow up to 10% jitter for Jenkins and VMs
    ASSERT_LE(timediff, 3.3_s);

    EXPECT_THAT(numberOfCalls, Eq(3));
}
TEST_F(Timer_test, DISABLED_CreateAndGetTimeUntilExpiration_PERFORMANCETEST42)
{
    Timer osTimer(second, [&]() { numberOfCalls++; });

    EXPECT_THAT(osTimer.hasError(), Eq(false));

    osTimer.start(Timer::RunMode::PERIODIC);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // timeUntilExpiration shall now be 500_ms
    auto end = osTimer.timeUntilExpiration();

    // Just check for magnitude, allow up to 10% jitter for Jenkins and VMs
    ASSERT_LE(end.get_value(), 550_ms);

    osTimer.stop();
}

TEST_F(Timer_test, DISABLED_CreateFirePeriodicallyAndGetoverruns_PERFORMANCETEST42)
{
    Timer osTimer(50_ms, [&]() { numberOfCalls++; });

    EXPECT_THAT(osTimer.hasError(), Eq(false));

    osTimer.start(Timer::RunMode::ONCE);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto overruns = osTimer.getOverruns();

    // See if an overruns occurred between time and actual callback from the operating system
    EXPECT_THAT(overruns.get_value(), Eq(0u));
}
TEST_F(Timer_test, CreateAndCallExpired_PERFORMANCETEST42)
{
    Timer osTimer(second);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_THAT(osTimer.hasExpiredComparedToCreationTime(), Eq(false));

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    EXPECT_THAT(osTimer.hasExpiredComparedToCreationTime(), Eq(true));
}
TEST_F(Timer_test, DISABLED_CreateAndRestart_PERFORMANCETEST42)
{
    Timer osTimer(second, [&]() { numberOfCalls++; });

    EXPECT_THAT(osTimer.hasError(), Eq(false));

    auto start = osTimer.now();

    osTimer.start(Timer::RunMode::ONCE);

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    auto end = osTimer.now();

    auto timediff = end.get_value() - start.get_value();

    // Just check for magnitude, allow up to 10% jitter for Jenkins and VMs
    EXPECT_LE(timediff, 1.2_s);

    start = osTimer.now();

    EXPECT_THAT(numberOfCalls, Eq(1));

    osTimer.restart(2_s, Timer::RunMode::ONCE);

    std::this_thread::sleep_for(std::chrono::milliseconds(2100));

    end = osTimer.now();
    osTimer.stop();

    timediff = end.get_value() - start.get_value();

    // Just check for magnitude, allow up to 10% jitter for Jenkins and VMs
    EXPECT_LE(timediff, 2.2_s);

    EXPECT_THAT(numberOfCalls, Eq(2));
}
