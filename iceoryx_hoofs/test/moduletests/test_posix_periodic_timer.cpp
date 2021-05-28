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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "iceoryx_hoofs/internal/units/duration.hpp"
#include "iceoryx_hoofs/posix_wrapper/periodic_timer.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"
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

    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);

    ASSERT_FALSE(timerState.has_error());
}

TEST_F(PeriodicTimer_test, ZeroINTERVALTest)
{
    Timer sut(0_s);

    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    bool result = timerState.value().state == iox::posix::TimerState::TICK ? true : false;

    ASSERT_TRUE(result);
}

TIMING_TEST_F(PeriodicTimer_test, DurationINTERVALTest, Repeat(5), [&] {
    Timer sut(INTERVAL);

    auto timeBeforeWait = sut.now().value();
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
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
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    bool result = timerState.value().state == iox::posix::TimerState::STOP ? true : false;

    ASSERT_TRUE(result);
}


TEST_F(PeriodicTimer_test, TimerStopAfterWaitTest)
{
    Timer sut(INTERVAL);

    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    sut.stop();
    timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    bool result = timerState.value().state == iox::posix::TimerState::STOP ? true : false;

    ASSERT_TRUE(result);
}


TIMING_TEST_F(PeriodicTimer_test, ResetWithNewDurationINTERVALTest, Repeat(5), [&] {
    Timer sut(INTERVAL);
    iox::units::Duration NEW_DURATION{100_ms};
    const int RANGE_APPROX = 2; // 2ms arppoximation. This may be lost in execution.
    sut.start(NEW_DURATION);

    auto timeBeforeWait = sut.now().value();
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    auto timeAfterWait = sut.now().value();
    auto duration = timeAfterWait - timeBeforeWait;

    TIMING_TEST_EXPECT_FALSE(timerState.has_error());
    TIMING_TEST_EXPECT_TRUE(duration.toMilliseconds() - NEW_DURATION.toMilliseconds() <= RANGE_APPROX);
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
    const int RANGE_APPROX = 2; // 2ms arppoximation. This may be lost in execution.
    auto timeUntilNextActivation = sut.now().value() + INTERVAL;

    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    auto currentTime = sut.now().value();

    TIMING_TEST_EXPECT_TRUE(timerState.value().state == iox::posix::TimerState::TICK ? true : false);
    TIMING_TEST_EXPECT_TRUE(currentTime.toMilliseconds() - timeUntilNextActivation.toMilliseconds() <= RANGE_APPROX);
});

TIMING_TEST_F(PeriodicTimer_test, periodicityExecutionTimeLessThanActivationTimeTest, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 30;
    const int RANGE_APPROX = 2; // 2ms arppoximation. This may be lost in execution.
    Timer sut(INTERVAL);
    auto timeUntilNextActivation = sut.now().value() + INTERVAL;

    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    auto currentTime = sut.now().value();

    TIMING_TEST_EXPECT_TRUE(timerState.value().state == iox::posix::TimerState::TICK ? true : false);
    TIMING_TEST_EXPECT_TRUE(currentTime.toMilliseconds() - timeUntilNextActivation.toMilliseconds() <= RANGE_APPROX);
});

TIMING_TEST_F(PeriodicTimer_test, immediateCatchupPolicyTest, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 70;
    Timer sut(INTERVAL);

    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    TIMING_TEST_EXPECT_TRUE(timerState.value().state == iox::posix::TimerState::TICK ? true : false);

    auto currentTimeAfterExecution = sut.now().value();
    timerState = sut.wait(iox::posix::TimerCatchupPolicy::IMMEDIATE_TICK);
    auto currentTimeAfterWait = sut.now().value();

    auto remainingTimeForNextActivation = currentTimeAfterWait - currentTimeAfterExecution;
    const int RANGE_APPROX = 2; // 2ms arppoximation. This may be lost in execution.
    TIMING_TEST_EXPECT_TRUE(timerState.value().state == iox::posix::TimerState::TICK ? true : false);
    TIMING_TEST_EXPECT_TRUE(remainingTimeForNextActivation.toMilliseconds() <= RANGE_APPROX);
});

TIMING_TEST_F(PeriodicTimer_test, skipToNextTickCatchupPolicyWithLessDelayTest, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 70;
    Timer sut(INTERVAL);
    const int TIME_SLOT_CONSUMED = 2;

    auto timeBeforeActivation = sut.now().value();
    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::SKIP_TO_NEXT_TICK);
    auto timeAfterExecution = sut.now().value();

    auto timeBetweenActivation = timeAfterExecution - timeBeforeActivation;
    TIMING_TEST_EXPECT_TRUE(timerState.value().state == iox::posix::TimerState::TICK ? true : false);
    TIMING_TEST_EXPECT_TRUE(timeBetweenActivation.toMilliseconds() >= (INTERVAL.toMilliseconds() * TIME_SLOT_CONSUMED));
});

TIMING_TEST_F(PeriodicTimer_test, skipToNextTickCatchupPolicyWithLargeDelayTest, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 150;
    Timer sut(INTERVAL);
    const int TIME_SLOT_CONSUMED = 3;

    auto timeBeforeActivation = sut.now().value();
    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::SKIP_TO_NEXT_TICK);
    auto timeAfterExecution = sut.now().value();

    auto timeBetweenActivation = timeAfterExecution - timeBeforeActivation;
    TIMING_TEST_EXPECT_TRUE(timerState.value().state == iox::posix::TimerState::TICK ? true : false);
    TIMING_TEST_EXPECT_TRUE(timeBetweenActivation.toMilliseconds() >= (INTERVAL.toMilliseconds() * TIME_SLOT_CONSUMED));
});

TIMING_TEST_F(PeriodicTimer_test, errorCatchupPolicyTest, Repeat(5), [&] {
    constexpr int EXECUTIONTIME = 70;
    Timer sut(INTERVAL);

    std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTIONTIME));
    auto timerState = sut.wait(iox::posix::TimerCatchupPolicy::HOLD_ON_DELAY);

    auto delayExpected = EXECUTIONTIME - INTERVAL.toMilliseconds();
    TIMING_TEST_EXPECT_TRUE(timerState.value().state == iox::posix::TimerState::DELAY ? true : false);
    TIMING_TEST_EXPECT_TRUE(delayExpected == timerState.value().timeDelay.toMilliseconds());
});