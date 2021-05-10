// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/posix_wrapper/periodic_timer.hpp"
#include "iceoryx_utils/testing/timing_test.hpp"
#include "test.hpp"

#include <chrono>
#include <thread>

using namespace ::testing;

using namespace iox::units;
using namespace iox::units::duration_literals;

using Timer = iox::posix::PeriodicTimer;

class PeriodicTimer_test : public Test
{
  public:
    virtual void SetUp()
    {
        numberOfCalls = 0;
    }

    virtual void TearDown()
    {
    }

    Duration second{1_s};

    std::atomic<int> numberOfCalls{0};
    static const Duration INTERVAL;
};

const Duration PeriodicTimer_test::INTERVAL{50_ms};

TEST_F(PeriodicTimer_test, TimerAutoStartTest)
{
    Timer sut(0_s);

    auto timerState = sut.wait();

    ASSERT_FALSE(timerState.has_error());
}

TEST_F(PeriodicTimer_test, ZeroINTERVALTest)
{
    Timer sut(0_s);

    auto timerState = sut.wait();
    bool result = (timerState.value() == iox::posix::PeriodicTimerEvent::TICK_DELAY
                   || timerState.value() == iox::posix::PeriodicTimerEvent::TICK)
                      ? true
                      : false;

    ASSERT_TRUE(result);
}

TIMING_TEST_F(PeriodicTimer_test, DurationINTERVALTest, Repeat(5), [&] {
    Timer sut(INTERVAL);

    auto timeBeforeWait = sut.now().value();
    auto timerState = sut.wait();
    auto timeAfterWait = sut.now().value();
    auto duration = timeAfterWait - timeBeforeWait;
    bool result = (duration.toMilliseconds() == INTERVAL.toMilliseconds()) ? true : false;

    TIMING_TEST_EXPECT_FALSE(timerState.has_error());
    TIMING_TEST_EXPECT_TRUE(result);
});

TEST_F(PeriodicTimer_test, TimerStopTest)
{
    Timer sut(0_s);

    sut.stop();
    auto timerState = sut.wait();
    bool result = timerState.value() == iox::posix::PeriodicTimerEvent::STOP ? true : false;

    ASSERT_TRUE(result);
}


TEST_F(PeriodicTimer_test, TimerStopAfterWaitTest)
{
    Timer sut(INTERVAL);

    auto timerState = sut.wait();
    sut.stop();
    timerState = sut.wait();
    bool result = timerState.value() == iox::posix::PeriodicTimerEvent::STOP ? true : false;

    ASSERT_TRUE(result);
}


TIMING_TEST_F(PeriodicTimer_test, ResetWithNewDurationINTERVALTest, Repeat(5), [&] {
    Timer sut(INTERVAL);
    iox::units::Duration NEW_DURATION{100_ms};
    sut.start(NEW_DURATION);

    auto timeBeforeWait = sut.now().value();
    auto timerState = sut.wait();
    auto timeAfterWait = sut.now().value();
    auto duration = timeAfterWait - timeBeforeWait;
    bool result = (duration.toMilliseconds() == NEW_DURATION.toMilliseconds()) ? true : false;

    TIMING_TEST_EXPECT_FALSE(timerState.has_error());
    TIMING_TEST_EXPECT_TRUE(result);
});

TIMING_TEST_F(PeriodicTimer_test, currentTimeTest, Repeat(5), [&] {
    Timer sut(INTERVAL);
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    auto timeNow = sut.now().value();
    auto currentSystemTime = iox::units::Duration(ts);

    bool result = timeNow.toMilliseconds() == currentSystemTime.toMilliseconds() ? true : false;

    TIMING_TEST_EXPECT_TRUE(result);
});

TIMING_TEST_F(PeriodicTimer_test, periodicityWithoutExecutionTimeTest, Repeat(5), [&] {
    Timer sut(INTERVAL);
    auto timeUntilNextActivation = sut.now().value() + INTERVAL;

    auto timerState = sut.wait();
    auto currentTime = sut.now().value();

    TIMING_TEST_EXPECT_TRUE(timerState.value() == iox::posix::PeriodicTimerEvent::TICK ? true : false);
    TIMING_TEST_EXPECT_TRUE(currentTime.toMilliseconds() == timeUntilNextActivation.toMilliseconds());
});

TIMING_TEST_F(PeriodicTimer_test, periodicityExecutionTimeLessThanActivationTimeTest, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 30;
    Timer sut(INTERVAL);
    auto timeUntilNextActivation = sut.now().value() + INTERVAL;

    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait();
    auto currentTime = sut.now().value();

    TIMING_TEST_EXPECT_TRUE(timerState.value() == iox::posix::PeriodicTimerEvent::TICK ? true : false);
    TIMING_TEST_EXPECT_TRUE(currentTime.toMilliseconds() == timeUntilNextActivation.toMilliseconds());
});

TIMING_TEST_F(PeriodicTimer_test, periodicityExecutionTimeGreaterThanActivationTimeTest, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 70;
    Timer sut(INTERVAL);
    auto timeUntilNextActivation = sut.now().value() + INTERVAL;

    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait();
    auto currentTime = sut.now().value();
    auto EXPECTED_DELAY = EXECUTIONTIME - INTERVAL.toMilliseconds();

    TIMING_TEST_EXPECT_TRUE(timerState.value() == iox::posix::PeriodicTimerEvent::TICK_DELAY ? true : false);
    TIMING_TEST_EXPECT_TRUE((currentTime.toMilliseconds() - timeUntilNextActivation.toMilliseconds())
                            >= EXPECTED_DELAY);
});

TIMING_TEST_F(PeriodicTimer_test, periodicityExecutionTimeGreaterThanDelayThreshold, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 120;
    Timer sut(INTERVAL, INTERVAL);
    auto timeUntilNextActivation = sut.now().value() + INTERVAL;

    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait();
    auto currentTime = sut.now().value();
    auto EXPECTED_DELAY = EXECUTIONTIME - INTERVAL.toMilliseconds();

    TIMING_TEST_EXPECT_TRUE(timerState.value() == iox::posix::PeriodicTimerEvent::TICK_THRESHOLD_DELAY ? true : false);
    TIMING_TEST_EXPECT_TRUE((currentTime.toMilliseconds() - timeUntilNextActivation.toMilliseconds())
                            >= EXPECTED_DELAY);
});