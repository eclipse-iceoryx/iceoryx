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

#ifndef IOX_HOOFS_FREERTOS_PLATFORM_PTHREAD_HPP
#define IOX_HOOFS_FREERTOS_PLATFORM_PTHREAD_HPP

#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/types.hpp"

#include <sched.h>

#include "FreeRTOS.h"
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/pthread.h"

#include <thread>

#define PTHREAD_MUTEX_STALLED 1
#define PTHREAD_MUTEX_ROBUST 2

#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_FAST_NP PTHREAD_MUTEX_DEFAULT

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

using iox_pthread_t = pthread_t;
using iox_pthread_attr_t = pthread_attr_t;

inline int iox_pthread_getname_np(std::thread::native_handle_type, char*, size_t)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int iox_pthread_setname_np(std::thread::native_handle_type, const char*)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int pthread_mutexattr_getpshared(const pthread_mutexattr_t*, int*)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int pthread_mutexattr_setpshared(pthread_mutexattr_t*, int)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int pthread_mutexattr_getprotocol(const pthread_mutexattr_t*, int*)
{
    // Not needed on FreeRTOS
    return 0;
}

/* Values for blocking protocol. */
#define PTHREAD_PRIO_NONE 0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2

inline int pthread_mutexattr_setprotocol(pthread_mutexattr_t*, int)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int pthread_mutexattr_setrobust(pthread_mutexattr_t*, int)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int pthread_mutexattr_setprioceiling(pthread_mutexattr_t*, int)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int pthread_mutex_consistent(pthread_mutex_t*)
{
    // Not needed on FreeRTOS
    return 0;
}

inline int
iox_pthread_create(iox_pthread_t* thread, const iox_pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
{
    return pthread_create(thread, attr, start_routine, arg);
}

inline int iox_pthread_join(iox_pthread_t thread, void** retval)
{
    return pthread_join(thread, retval);
}

inline std::thread::native_handle_type iox_pthread_self()
{
    return {};
}


#endif // IOX_HOOFS_FREERTOS_PLATFORM_PTHREAD_HPP
