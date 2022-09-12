// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/platform/time.hpp"

static std::chrono::nanoseconds getNanoSeconds(const timespec& value)
{
    static constexpr uint64_t NANOSECONDS = 1000000000u;
    return std::chrono::nanoseconds(static_cast<uint64_t>(value.tv_sec) * NANOSECONDS
                                    + static_cast<uint64_t>(value.tv_nsec));
}

static void stopTimerThread(timer_t timerid)
{
    timerid->parameter.mutex.lock();
    timerid->parameter.keepRunning = false;
    timerid->parameter.mutex.unlock();

    timerid->parameter.wakeup.notify_one();
    if (timerid->thread.joinable())
    {
        timerid->thread.join();
    }
}

static bool waitForExecution(timer_t timerid)
{
    using timePoint_t = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
    std::unique_lock<std::mutex> ulock(timerid->parameter.mutex);

    if (timerid->parameter.isTimerRunning)
    {
        timespec waitUntil;
        clock_gettime(CLOCK_REALTIME, &waitUntil);
        waitUntil.tv_sec += timerid->parameter.timeParameters.it_value.tv_sec;
        waitUntil.tv_nsec += timerid->parameter.timeParameters.it_value.tv_nsec;
        timerid->parameter.wakeup.wait_until(ulock, timePoint_t(getNanoSeconds(waitUntil)), [timerid] {
            return !timerid->parameter.isTimerRunning || !timerid->parameter.keepRunning;
        });
    }
    else
    {
        timerid->parameter.wakeup.wait(
            ulock, [timerid] { return timerid->parameter.isTimerRunning || !timerid->parameter.keepRunning; });
    }

    return timerid->parameter.isTimerRunning;
}

static void
setTimeParameters(timer_t timerid, const itimerspec& timeParameters, const bool runOnce, const bool isTimerRunning)
{
    std::lock_guard<std::mutex> l(timerid->parameter.mutex);
    clock_gettime(CLOCK_REALTIME, &timerid->parameter.startTime);
    timerid->parameter.timeParameters = timeParameters;
    timerid->parameter.runOnce = runOnce;
    timerid->parameter.wasCallbackCalled = false;
    timerid->parameter.isTimerRunning = isTimerRunning;
}

int timer_create(clockid_t, struct sigevent* sevp, timer_t* timerid)
{
    timer_t timer = new appleTimer_t();
    timer->callback = sevp->sigev_notify_function;
    timer->callbackParameter = sevp->sigev_value;

    timer->thread = std::thread([timer] {
        while ([&]() -> bool {
            std::lock_guard<std::mutex> lock(timer->parameter.mutex);
            return timer->parameter.keepRunning;
        }())
        {
            if (waitForExecution(timer))
            {
                timer->parameter.mutex.lock();
                bool doCallCallback =
                    !timer->parameter.runOnce || (timer->parameter.runOnce && !timer->parameter.wasCallbackCalled);
                timer->parameter.mutex.unlock();
                if (doCallCallback)
                {
                    std::thread t(timer->callback, timer->callbackParameter);
                    t.detach();

                    std::lock_guard<std::mutex> l(timer->parameter.mutex);
                    timer->parameter.wasCallbackCalled = true;
                }
            }
        }
    });

    *timerid = timer;
    return 0;
}

int timer_delete(timer_t timerid)
{
    stopTimerThread(timerid);
    delete timerid;
    return 0;
}

int timer_settime(timer_t timerid, int, const struct itimerspec* new_value, struct itimerspec*)
{
    // disarm timer
    if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 0)
    {
        setTimeParameters(timerid, *new_value, false, false);
    }
    // run once
    else if (new_value->it_interval.tv_sec == 0 && new_value->it_interval.tv_nsec == 0)
    {
        setTimeParameters(timerid, *new_value, true, true);
    }
    // run periodically
    else
    {
        setTimeParameters(timerid, *new_value, false, true);
    }
    timerid->parameter.wakeup.notify_one();

    return 0;
}

int timer_gettime(timer_t timerid, struct itimerspec* curr_value)
{
    constexpr int64_t NANO_SECONDS{1000000000};
    timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    int64_t currentTimeNs = currentTime.tv_sec * NANO_SECONDS + currentTime.tv_nsec;
    int64_t intervalTimeNs{0}, startTimeNs{0};
    {
        std::lock_guard<std::mutex> l(timerid->parameter.mutex);
        curr_value->it_interval = timerid->parameter.timeParameters.it_interval;
        intervalTimeNs = timerid->parameter.timeParameters.it_interval.tv_sec * NANO_SECONDS
                         + timerid->parameter.timeParameters.it_interval.tv_nsec;
        startTimeNs = timerid->parameter.startTime.tv_sec * NANO_SECONDS + timerid->parameter.startTime.tv_nsec;
    }
    int64_t remainingTimeNs = intervalTimeNs - (currentTimeNs - startTimeNs);
    curr_value->it_value.tv_sec = remainingTimeNs / NANO_SECONDS;
    curr_value->it_value.tv_nsec = remainingTimeNs - curr_value->it_interval.tv_sec * NANO_SECONDS;
    return 0;
}

int timer_getoverrun(timer_t)
{
    return 0;
}
