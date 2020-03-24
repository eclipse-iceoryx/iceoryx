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

#include <sys/time.h>

using timer_t = void*;

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