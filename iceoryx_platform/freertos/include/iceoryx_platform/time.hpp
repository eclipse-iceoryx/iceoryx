// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
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

#ifndef IOX_HOOFS_FREERTOS_PLATFORM_TIME_HPP
#define IOX_HOOFS_FREERTOS_PLATFORM_TIME_HPP

#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/time.h"
#include <sys/time.h>
#include <time.h>

using iox_clockid_t = clockid_t;

inline int iox_clock_gettime(iox_clockid_t clk_id, struct timespec* tp)
{
    return clock_gettime(clk_id, tp);
}
inline int iox_gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    return gettimeofday(tp, tzp);
}

#endif // IOX_HOOFS_FREERTOS_PLATFORM_TIME_HPP
