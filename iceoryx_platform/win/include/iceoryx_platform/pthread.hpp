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

#define IOX_PTHREAD_PROCESS_SHARED 0
#define IOX_PTHREAD_PROCESS_PRIVATE 1
#define IOX_PTHREAD_MUTEX_RECURSIVE 2
#define IOX_PTHREAD_MUTEX_NORMAL 3
#define IOX_PTHREAD_MUTEX_ERRORCHECK 4
#define IOX_PTHREAD_MUTEX_DEFAULT IOX_PTHREAD_MUTEX_NORMAL

#define IOX_PTHREAD_MUTEX_STALLED 7
#define IOX_PTHREAD_MUTEX_ROBUST 8

#define IOX_PTHREAD_PRIO_NONE 4
#define IOX_PTHREAD_PRIO_INHERIT 5
#define IOX_PTHREAD_PRIO_PROTECT 6

struct iox_pthread_mutex_t
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    bool isInterprocessMutex = false;
    UniqueSystemId uniqueId;
};

const iox_pthread_mutex_t IOX_PTHREAD_MUTEX_INITIALIZER;

struct iox_pthread_mutexattr_t
{
    bool isInterprocessMutex = false;
};

int iox_pthread_mutexattr_init(iox_pthread_mutexattr_t* attr);
int iox_pthread_mutexattr_destroy(iox_pthread_mutexattr_t* attr);
int iox_pthread_mutexattr_setpshared(iox_pthread_mutexattr_t* attr, int pshared);
int iox_pthread_mutexattr_settype(iox_pthread_mutexattr_t* attr, int type);
int iox_pthread_mutexattr_setprotocol(iox_pthread_mutexattr_t* attr, int protocol);
int iox_pthread_mutexattr_setprioceiling(iox_pthread_mutexattr_t* attr, int prioceiling);
int iox_pthread_mutexattr_setrobust(iox_pthread_mutexattr_t* attr, int robustness);

int iox_pthread_mutex_init(iox_pthread_mutex_t* mutex, const iox_pthread_mutexattr_t* attr);
int iox_pthread_mutex_destroy(iox_pthread_mutex_t* mutex);
int iox_pthread_mutex_lock(iox_pthread_mutex_t* mutex);
int iox_pthread_mutex_trylock(iox_pthread_mutex_t* mutex);
int iox_pthread_mutex_unlock(iox_pthread_mutex_t* mutex);
int iox_pthread_mutex_consistent(iox_pthread_mutex_t* mutex);

using iox_pthread_t = HANDLE;
using iox_pthread_attr_t = void;

int iox_pthread_setname_np(iox_pthread_t thread, const char* name);
int iox_pthread_getname_np(iox_pthread_t thread, char* name, size_t len);
int iox_pthread_create(iox_pthread_t* thread, const iox_pthread_attr_t* attr, void* (*start_routine)(void*), void* arg);
int iox_pthread_join(iox_pthread_t thread, void** retval);
iox_pthread_t iox_pthread_self();

#endif // IOX_HOOFS_WIN_PLATFORM_PTHREAD_HPP
