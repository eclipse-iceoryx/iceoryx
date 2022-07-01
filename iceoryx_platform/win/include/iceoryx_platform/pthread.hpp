// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_PTHREAD_HPP
#define IOX_HOOFS_WIN_PLATFORM_PTHREAD_HPP

#include "iceoryx_platform/unique_system_id.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"
#include "iceoryx_platform/windows.hpp"

#include <thread>
#include <type_traits>

#define PTHREAD_PROCESS_SHARED 0
#define PTHREAD_PROCESS_PRIVATE 1
#define PTHREAD_MUTEX_RECURSIVE 2
#define PTHREAD_MUTEX_NORMAL 3
#define PTHREAD_MUTEX_ERRORCHECK 4
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL

#define PTHREAD_PRIO_NONE 4
#define PTHREAD_PRIO_INHERIT 5
#define PTHREAD_PRIO_PROTECT 6

#define PTHREAD_MUTEX_STALLED 7
#define PTHREAD_MUTEX_ROBUST 8

struct pthread_mutex_t
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    bool isInterprocessMutex = false;
    UniqueSystemId uniqueId;
};

struct pthread_mutexattr_t
{
    bool isInterprocessMutex = false;
};

using pthread_t = std::thread::native_handle_type;

int pthread_mutexattr_destroy(pthread_mutexattr_t* attr);
int pthread_mutexattr_init(pthread_mutexattr_t* attr);
int pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared);
int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t* attr, int protocol);
int pthread_mutexattr_setrobust(pthread_mutexattr_t* attr, int robustness);
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t* attr, int prioceiling);

int pthread_mutex_destroy(pthread_mutex_t* mutex);
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
int pthread_mutex_lock(pthread_mutex_t* mutex);
int pthread_mutex_trylock(pthread_mutex_t* mutex);
int pthread_mutex_unlock(pthread_mutex_t* mutex);

using iox_pthread_t = HANDLE;
using iox_pthread_attr_t = void;

int iox_pthread_setname_np(iox_pthread_t thread, const char* name);
int iox_pthread_getname_np(iox_pthread_t thread, char* name, size_t len);
int iox_pthread_create(iox_pthread_t* thread, const iox_pthread_attr_t* attr, void* (*start_routine)(void*), void* arg);
int iox_pthread_join(iox_pthread_t thread, void** retval);
iox_pthread_t iox_pthread_self();

#endif // IOX_HOOFS_WIN_PLATFORM_PTHREAD_HPP
