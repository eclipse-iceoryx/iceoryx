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

#include "iceoryx_utils/platform/time.hpp"

int timer_create(clockid_t clockid, struct sigevent* sevp, timer_t* timerid)
{
    timer_t timer = new appleTimer_t();
    timer->callback = sevp->sigev_notify_function;
    timer->callbackParameter = sevp->sigev_value;
    *timerid = timer;
    return 0;
}

int timer_delete(timer_t timerid)
{
    if (timerid->thread.joinable())
    {
        timerid->thread.join();
    }
    delete timerid;
    return 0;
}

#include <iostream>

int timer_settime(timer_t timerid, int flags, const struct itimerspec* new_value, struct itimerspec* old_value)
{
    timerid->timeParameters = *new_value;
    // disarm timer
    if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 0)
    {
        timerid->keepRunning.store(false);
        if (timerid->thread.joinable())
        {
            timerid->thread.join();
        }
    }
    // run once
    else if (new_value->it_interval.tv_sec == 0 && new_value->it_interval.tv_nsec == 0)
    {
        timerid->thread = std::thread([timerid] {
            static constexpr uint64_t NANOSECONDS = 1000000000;
            auto& timeout = timerid->timeParameters.it_value;
            std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<uint64_t>(timeout.tv_sec) * NANOSECONDS
                                                                 + static_cast<uint64_t>(timeout.tv_nsec)));
            timerid->callback(timerid->callbackParameter);
        });
    }
    // run periodically
    else
    {
        timerid->keepRunning.store(true);

        timerid->thread = std::thread([timerid] {
            static constexpr uint64_t NANOSECONDS = 1000000000;
            auto& timeout = timerid->timeParameters.it_value;
            while (timerid->keepRunning.load())
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<uint64_t>(timeout.tv_sec) * NANOSECONDS
                                                                     + static_cast<uint64_t>(timeout.tv_nsec)));
                timerid->callback(timerid->callbackParameter);
            }
        });
    }

    return 0;
}

int timer_gettime(timer_t timerid, struct itimerspec* curr_value)
{
    return 0;
}

int timer_getoverrun(timer_t timerid)
{
    return 0;
}
