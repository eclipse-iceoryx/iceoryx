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

#include "iceoryx_utils/cxx/periodic_timer.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "test.hpp"
#include "testutils/timing_test.hpp"

#include <chrono>
#include <thread>

using namespace ::testing;

using namespace iox::units;
using namespace iox::units::duration_literals;

using Timer = iox::cxx::PeriodicTimer;

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
    static const int SLEEPTIME;
};

const Duration PeriodicTimer_test::INTERVAL{5_s};
const int PeriodicTimer_test::SLEEPTIME = PeriodicTimer_test::INTERVAL.toMilliseconds();

TIMING_TEST_F(PeriodicTimer_test, TimerAutoStartTest, Repeat(5), [&] {
    Timer sut(0_s);

    auto timerState = sut.wait();

    TIMING_TEST_EXPECT_FALSE(timerState.has_error());
});

TIMING_TEST_F(PeriodicTimer_test, ZeroINTERVALTest, Repeat(5), [&] {
    Timer sut(0_s);

    auto timerState = sut.wait();

    bool result = timerState.value() == iox::cxx::TimerEvent::TICK ? true : false;

    TIMING_TEST_EXPECT_TRUE(result);
});

TIMING_TEST_F(PeriodicTimer_test, DurationINTERVALTest, Repeat(5), [&] {
    Timer sut(INTERVAL);
    struct timespec timeBeforeWait;
    struct timespec timeAfterWait;

    clock_gettime(CLOCK_REALTIME, &timeBeforeWait);
    auto timerState = sut.wait();
    clock_gettime(CLOCK_REALTIME, &timeAfterWait);
    auto duration = timeAfterWait.tv_sec - timeBeforeWait.tv_sec;
    bool result = (((unsigned)duration) == INTERVAL.toSeconds()) ? true : false;


    TIMING_TEST_EXPECT_TRUE(result);
});

TIMING_TEST_F(PeriodicTimer_test, TimerStopTest, Repeat(5), [&] {
    Timer sut(0_s);

    sut.stop();
    auto timerState = sut.wait();
    bool result = timerState.value() == iox::cxx::TimerEvent::STOP ? true : false;

    TIMING_TEST_EXPECT_TRUE(result);
});


TIMING_TEST_F(PeriodicTimer_test, TimerStopAfterWaitTest, Repeat(5), [&] {
    Timer sut(INTERVAL);

    auto timerState = sut.wait();
    sut.stop();
    timerState = sut.wait();
    bool result = timerState.value() == iox::cxx::TimerEvent::STOP ? true : false;

    TIMING_TEST_EXPECT_TRUE(result);
});


TIMING_TEST_F(PeriodicTimer_test, ResetWithNewDurationINTERVALTest, Repeat(5), [&] {
    Timer sut(INTERVAL);

    auto timerState = sut.wait();
    iox::units::Duration NEW_DURATION{7_s};
    sut.start(NEW_DURATION);

    struct timespec timeBeforeWait;
    struct timespec timeAfterWait;
    clock_gettime(CLOCK_REALTIME, &timeBeforeWait);
    timerState = sut.wait();
    clock_gettime(CLOCK_REALTIME, &timeAfterWait);
    auto duration = timeAfterWait.tv_sec - timeBeforeWait.tv_sec;
    bool result = (((unsigned)duration) == NEW_DURATION.toSeconds()) ? true : false;

    TIMING_TEST_EXPECT_TRUE(result);
});