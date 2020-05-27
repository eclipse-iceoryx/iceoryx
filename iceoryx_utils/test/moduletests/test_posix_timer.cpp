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

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"
#include "test.hpp"
#include "testutils/timing_test.hpp"

#include <atomic>
#include <chrono>
#include <thread>

using namespace ::testing;

using namespace iox::units::duration_literals;

using Timer = iox::posix::Timer;
using TimerError = iox::posix::TimerError;

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

class TimerStopWatch_test : public Test
{
  public:
    static const iox::units::Duration TIMEOUT;
};
iox::units::Duration TIMEOUT{4_ms};

TEST_F(TimerStopWatch_test, DurationOfZeroCausesError)
{
    Timer sut(0_s);
    EXPECT_THAT(sut.hasError(), Eq(true));
    EXPECT_THAT(sut.getError(), Eq(TimerError::TIMEOUT_IS_ZERO));
}

TIMING_TEST_F(TimerStopWatch_test, DurationOfNonZeroIsExpiresAfterTimeout, Repeat(5), [&] {
    Timer sut(TIMEOUT);

    TIMING_TEST_EXPECT_FALSE(sut.hasExpiredComparedToCreationTime());
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * TIMEOUT.milliSeconds<int>() / 3));
    TIMING_TEST_EXPECT_FALSE(sut.hasExpiredComparedToCreationTime());
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * TIMEOUT.milliSeconds<int>() / 3));
    TIMING_TEST_EXPECT_TRUE(sut.hasExpiredComparedToCreationTime());
});

TEST_F(TimerStopWatch_test, ResetWithDurationIsExpired)
{
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * TIMEOUT.milliSeconds<int>()));
    EXPECT_THAT(sut.hasExpiredComparedToCreationTime(), Eq(true));
    sut.resetCreationTime();
    EXPECT_THAT(sut.hasExpiredComparedToCreationTime(), Eq(false));
}

TEST_F(TimerStopWatch_test, ResetWhenNotExpiredIsStillNotExpired)
{
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * TIMEOUT.milliSeconds<int>() / 3));
    sut.resetCreationTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * TIMEOUT.milliSeconds<int>() / 3));
    EXPECT_THAT(sut.hasExpiredComparedToCreationTime(), Eq(false));
}

TIMING_TEST_F(TimerStopWatch_test, ResetAfterBeingExpiredIsNotExpired, Repeat(5), [&] {
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * TIMEOUT.milliSeconds<int>()));

    TIMING_TEST_ASSERT_TRUE(sut.hasExpiredComparedToCreationTime());
    sut.resetCreationTime();
    TIMING_TEST_EXPECT_FALSE(sut.hasExpiredComparedToCreationTime());
});

TEST_F(Timer_test, EmptyCallbackInCtorLeadsToError)
{
    Timer sut(1_s, std::function<void()>());

    EXPECT_THAT(sut.hasError(), Eq(true));
    EXPECT_THAT(sut.getError(), Eq(iox::posix::TimerError::NO_VALID_CALLBACK));
}

TEST_F(Timer_test, ZeroTimeoutIsNotAllowed)
{
    Timer sut(0_s, [] {});

    EXPECT_THAT(sut.hasError(), Eq(true));
    EXPECT_THAT(sut.getError(), Eq(iox::posix::TimerError::TIMEOUT_IS_ZERO));
}

TIMING_TEST_F(Timer_test, CallbackNotExecutedWhenNotStarted, Repeat(5), [&] {
    bool callbackExecuted{false};
    Timer sut(1_ns, [&] { callbackExecuted = true; });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    TIMING_TEST_EXPECT_ALWAYS_FALSE(callbackExecuted);
});

TEST_F(Timer_test, CallbackExecutedOnceAfterStart)
{
    std::atomic_int counter{0};
    Timer sut(1_ns, [&] { counter++; });
    sut.start(Timer::RunMode::ONCE);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(counter.load() == 1);
}

TIMING_TEST_F(Timer_test, CallbackExecutedPeriodicallyAfterStart, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(1_ms, [&] { counter++; });
    sut.start(Timer::RunMode::PERIODIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto finalCount = counter.load();

    TIMING_TEST_EXPECT_TRUE(6 <= finalCount && finalCount <= 11);
});

TIMING_TEST_F(Timer_test, PeriodicCallbackNotExecutedPrematurely, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(10_ms, [&] { counter++; });
    sut.start(Timer::RunMode::PERIODIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    TIMING_TEST_EXPECT_TRUE(counter.load() == 0);
});

TIMING_TEST_F(Timer_test, OneTimeCallbackNotExecutedPrematurely, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(10_ms, [&] { counter++; });
    sut.start(Timer::RunMode::ONCE);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    TIMING_TEST_EXPECT_TRUE(counter.load() == 0);
});

TEST_F(Timer_test, StartFailsWhenNoCallbackIsSet)
{
    Timer sut(1_ms);
    auto call = sut.start(Timer::RunMode::ONCE);

    ASSERT_THAT(call.has_error(), Eq(true));
    EXPECT_THAT(call.get_error(), Eq(TimerError::TIMER_NOT_INITIALIZED));
}

TIMING_TEST_F(Timer_test, StartRunModeOnceIsStoppedAfterStop, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(1_ms, [&] { counter++; });
    sut.start(Timer::RunMode::ONCE);
    sut.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    TIMING_TEST_EXPECT_TRUE(counter.load() == 0);
});

TIMING_TEST_F(Timer_test, StartRunPeriodicOnceIsStoppedAfterStop, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(1_ms, [&] { counter++; });
    sut.start(Timer::RunMode::PERIODIC);
    sut.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    TIMING_TEST_EXPECT_TRUE(counter.load() == 0);
});

TIMING_TEST_F(Timer_test, StartRunPeriodicOnceIsStoppedInTheMiddleAfterStop, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(2_ms, [&] { counter++; });
    sut.start(Timer::RunMode::PERIODIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sut.stop();
    auto previousCount = counter.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    TIMING_TEST_EXPECT_TRUE(previousCount == counter.load());
});

TEST_F(Timer_test, StopFailsWhenNoCallbackIsSet)
{
    Timer sut(1_ms);
    auto call = sut.stop();

    ASSERT_THAT(call.has_error(), Eq(true));
    EXPECT_THAT(call.get_error(), Eq(TimerError::TIMER_NOT_INITIALIZED));
}

TIMING_TEST_F(Timer_test, RestartWithDifferentTiming, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(5_ms, [&] { counter++; });
    sut.start(Timer::RunMode::PERIODIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sut.restart(1_ms, Timer::RunMode::PERIODIC);
    counter = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto finalCount = counter.load();

    TIMING_TEST_EXPECT_TRUE(6 <= finalCount && finalCount <= 13);
});

TIMING_TEST_F(Timer_test, RestartWithDifferentRunMode, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(4_ms, [&] { counter++; });
    sut.start(Timer::RunMode::PERIODIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sut.restart(4_ms, Timer::RunMode::ONCE);
    counter = 0;

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    TIMING_TEST_EXPECT_TRUE(counter.load() == 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(6));
    TIMING_TEST_EXPECT_TRUE(counter.load() == 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(6));
    TIMING_TEST_EXPECT_TRUE(counter.load() == 1);
});

TIMING_TEST_F(Timer_test, RestartWithDifferentTimingAndRunMode, Repeat(5), [&] {
    std::atomic_int counter{0};
    Timer sut(5_ms, [&] { counter++; });
    sut.start(Timer::RunMode::ONCE);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    counter = 0;
    sut.restart(1_ms, Timer::RunMode::PERIODIC);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto finalCount = counter.load();
    TIMING_TEST_EXPECT_TRUE(6 <= finalCount && finalCount <= 13);
});

TEST_F(Timer_test, RestartWithEmptyCallbackFails)
{
    Timer sut(1_ms);
    auto call = sut.restart(1_s, Timer::RunMode::ONCE);

    ASSERT_THAT(call.has_error(), Eq(true));
    EXPECT_THAT(call.get_error(), Eq(TimerError::TIMER_NOT_INITIALIZED));
}

TEST_F(Timer_test, RestartWithTimeoutOfZeroFails)
{
    Timer sut(1_ms, [] {});
    auto call = sut.restart(0_s, Timer::RunMode::ONCE);

    ASSERT_THAT(call.has_error(), Eq(true));
    EXPECT_THAT(call.get_error(), Eq(TimerError::TIMEOUT_IS_ZERO));
}

TEST_F(Timer_test, TimeUntilExpirationFailsWithoutCallback)
{
    Timer sut(1_ms);
    auto call = sut.timeUntilExpiration();

    ASSERT_THAT(call.has_error(), Eq(true));
    EXPECT_THAT(call.get_error(), Eq(TimerError::TIMER_NOT_INITIALIZED));
}

TIMING_TEST_F(Timer_test, TimeUntilExpirationWithCallback, Repeat(5), [&] {
    Timer sut(20_ms, [] {});
    sut.start(Timer::RunMode::PERIODIC);
    int timeUntilExpiration = sut.timeUntilExpiration().get_value().milliSeconds<int>();
    TIMING_TEST_EXPECT_TRUE(10 <= timeUntilExpiration && timeUntilExpiration <= 20);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timeUntilExpiration = sut.timeUntilExpiration().get_value().milliSeconds<int>();
    TIMING_TEST_EXPECT_TRUE(1 <= timeUntilExpiration && timeUntilExpiration <= 10);
});

TIMING_TEST_F(Timer_test, TimeUntilExpirationZeroAfterCallbackOnceCalled, Repeat(5), [&] {
    Timer sut(1_ms, [] {});
    sut.start(Timer::RunMode::ONCE);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    int timeUntilExpiration = sut.timeUntilExpiration().get_value().milliSeconds<int>();
    TIMING_TEST_EXPECT_TRUE(timeUntilExpiration == 0);
});

TIMING_TEST_F(Timer_test, StoppingIsNonBlocking, Repeat(5), [&] {
    Timer sut(1_ns, [] { std::this_thread::sleep_for(std::chrono::milliseconds(20)); });
    sut.start(Timer::RunMode::ONCE);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    auto startTime = std::chrono::system_clock::now();
    sut.stop();
    auto endTime = std::chrono::system_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    TIMING_TEST_EXPECT_TRUE(elapsedTime < 10);
});

// This test has to be repeated more often since we are spawning a lot
// of threads in here
TIMING_TEST_F(Timer_test, MultipleTimersRunningContinuously, Repeat(5), [&] {
    struct TimeValPair
    {
        TimeValPair()
            : timer(4_ms, [&] { this->value++; })
        {
        }
        int value{0};
        Timer timer;
    };

    std::vector<TimeValPair> sutList(4);

    for (auto& sut : sutList)
    {
        sut.timer.start(Timer::RunMode::PERIODIC);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    for (auto& sut : sutList)
    {
        sut.timer.stop();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    for (auto& sut : sutList)
    {
        TIMING_TEST_EXPECT_TRUE(4 <= sut.value && sut.value <= 6);
    }
});

// This test has to be repeated more often since we are spawning a lot
// of threads in here
TIMING_TEST_F(Timer_test, MultipleTimersRunningOnce, Repeat(5), [&] {
    struct TimeValPair
    {
        TimeValPair()
            : timer(2_ms, [&] { this->value++; })
        {
        }
        int value{0};
        Timer timer;
    };

    std::vector<TimeValPair> sutList(4);

    for (auto& sut : sutList)
    {
        sut.timer.start(Timer::RunMode::ONCE);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    for (auto& sut : sutList)
    {
        TIMING_TEST_EXPECT_TRUE(sut.value == 1);
    }
});

TIMING_TEST_F(Timer_test, DestructorIsBlocking, Repeat(5), [&] {
    std::chrono::time_point<std::chrono::system_clock> startTime;
    {
        Timer sut(1_ns, [] { std::this_thread::sleep_for(std::chrono::milliseconds(20)); });
        sut.start(Timer::RunMode::ONCE);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        startTime = std::chrono::system_clock::now();
    }
    auto endTime = std::chrono::system_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    TIMING_TEST_EXPECT_TRUE(15 <= elapsedTime);
});

TIMING_TEST_F(Timer_test, StartStopAndStartAgainIsNonBlocking, Repeat(5), [&] {
    Timer sut(1_ns, [] { std::this_thread::sleep_for(std::chrono::milliseconds(20)); });
    sut.start(Timer::RunMode::ONCE);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    auto startTime = std::chrono::system_clock::now();
    sut.stop();
    sut.start(Timer::RunMode::PERIODIC);
    auto endTime = std::chrono::system_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    TIMING_TEST_EXPECT_TRUE(elapsedTime <= 1);
});

TEST_F(Timer_test, GetOverrunsFailsWithNoCallback)
{
    Timer sut(1_ms);
    auto call = sut.getOverruns();

    ASSERT_THAT(call.has_error(), Eq(true));
    EXPECT_THAT(call.get_error(), Eq(TimerError::TIMER_NOT_INITIALIZED));
}
