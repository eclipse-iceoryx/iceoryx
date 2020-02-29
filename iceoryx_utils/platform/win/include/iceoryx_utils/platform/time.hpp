// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_utils/platform/win32_errorHandling.hpp"
#include "iceoryx_utils/platform/windows.hpp"


#include <cstdint>
#include <cstdio>
#include <ctime>

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

using suseconds_t = uint64_t;
using timer_t = void*;
using clockid_t = int;

struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
};

struct itimerspec
{
    timespec it_interval;
    timespec it_value;
};


inline int timer_create(clockid_t clockid, struct sigevent* sevp, timer_t* timerid)
{
    return 0;
}

inline int timer_delete(timer_t timerid)
{
    return 0;
}

inline int timer_settime(timer_t timerid, int flags, const struct itimerspec* new_value, struct itimerspec* old_value)
{
    return 0;
}

inline int timer_gettime(timer_t timerid, struct itimerspec* curr_value)
{
    return 0;
}

inline int timer_getoverrun(timer_t timerid)
{
    return 0;
}

inline int clock_gettime(clockid_t clk_id, struct timespec* tp)
{
    if (clk_id != CLOCK_REALTIME)
    {
        fprintf(stderr, "Windows Version of clock_gettime supports only CLOCK_REALTIME clockID\n");
    }
    int retVal = Win32Call(timespec_get(tp, TIME_UTC));
    return retVal;
}

inline int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    // difference in nano seconds between 01.01.1601 (UTC) and 01.01.1970 (EPOCH, unix time)
    static constexpr uint64_t UTC_EPOCH_DIFF{116444736000000000};

    SYSTEMTIME systemTime;
    FILETIME fileTime;

    Win32Call(GetSystemTime(&systemTime));
    Win32Call(SystemTimeToFileTime(&systemTime, &fileTime));
    uint64_t time =
        static_cast<uint64_t>(fileTime.dwLowDateTime) + (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32);

    constexpr uint64_t TEN_MILLISECONDS_IN_NANOSECONDS = 10000000;
    tp->tv_sec = static_cast<time_t>((time - UTC_EPOCH_DIFF) / TEN_MILLISECONDS_IN_NANOSECONDS);
    tp->tv_usec = static_cast<suseconds_t>(systemTime.wMilliseconds * 1000);
    return 0;
}
