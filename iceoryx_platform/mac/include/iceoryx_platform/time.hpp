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
#ifndef IOX_HOOFS_MAC_PLATFORM_TIME_HPP
#define IOX_HOOFS_MAC_PLATFORM_TIME_HPP

#include <condition_variable>
#include <mutex>
#include <sys/time.h>
#include <thread>

using iox_clockid_t = clockid_t;

struct itimerspec
{
    timespec it_interval;
    timespec it_value;
};

struct IceoryxPlatformTimer_t
{
    std::thread thread;
    void (*callback)(union sigval);
    sigval callbackParameter;

    struct
    {
        std::mutex mutex;
        bool keepRunning{true};
        timespec startTime;
        bool wasCallbackCalled{false};
        bool runOnce{false};
        bool isTimerRunning{false};
        itimerspec timeParameters;
        std::condition_variable wakeup;
    } parameter;
};

using iox_timer_t = IceoryxPlatformTimer_t*;

int iox_timer_create(iox_clockid_t clockid, struct sigevent* sevp, iox_timer_t* timerid);
int iox_timer_delete(iox_timer_t timerid);
int iox_timer_settime(iox_timer_t timerid, int flags, const struct itimerspec* new_value, struct itimerspec* old_value);
int iox_timer_gettime(iox_timer_t timerid, struct itimerspec* curr_value);
int iox_timer_getoverrun(iox_timer_t timerid);


inline int iox_clock_gettime(iox_clockid_t clk_id, struct timespec* tp)
{
    return clock_gettime(clk_id, tp);
}
inline int iox_gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    return gettimeofday(tp, tzp);
}

#endif // IOX_HOOFS_MAC_PLATFORM_TIME_HPP
