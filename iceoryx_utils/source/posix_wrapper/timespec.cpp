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

#include "iceoryx_utils/internal/posix_wrapper/timespec.hpp"

namespace iox
{
namespace posix
{
struct timespec addTimeMs(struct timespec time, const unsigned int timeToAdd_ms)
{
    decltype(time.tv_nsec) sec_ns = time.tv_nsec + ((timeToAdd_ms % 1000) * TS_DIVIDER_msec);
    time.tv_sec += (timeToAdd_ms / 1000);
    if (sec_ns < TS_DIVIDER_sec)
    {
        time.tv_nsec = sec_ns;
    }
    else
    {
        time.tv_nsec = sec_ns % TS_DIVIDER_sec;
        time.tv_sec++;
    }

    return time;
}

double subtractTimespecMS(const struct timespec minuend, const struct timespec subtrahend)
{
    long long diff_s = static_cast<long long>(minuend.tv_sec) - static_cast<long long>(subtrahend.tv_sec);
    long long diff_ns = static_cast<long long>(minuend.tv_nsec) - static_cast<long long>(subtrahend.tv_nsec);

    return (static_cast<double>(diff_s) * 1000.)
           + (static_cast<double>(diff_ns) / static_cast<double>(TS_DIVIDER_msec));
}

} // namespace posix
} // namespace iox
