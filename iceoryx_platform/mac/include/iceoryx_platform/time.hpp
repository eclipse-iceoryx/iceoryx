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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>
#include <thread>

struct itimerspec
{
    timespec it_interval;
    timespec it_value;
};

struct appleTimer_t
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

using timer_t = appleTimer_t*;

int timer_create(clockid_t clockid, struct sigevent* sevp, timer_t* timerid);
int timer_delete(timer_t timerid);
int timer_settime(timer_t timerid, int flags, const struct itimerspec* new_value, struct itimerspec* old_value);
int timer_gettime(timer_t timerid, struct itimerspec* curr_value);
int timer_getoverrun(timer_t timerid);

#endif // IOX_HOOFS_MAC_PLATFORM_TIME_HPP
