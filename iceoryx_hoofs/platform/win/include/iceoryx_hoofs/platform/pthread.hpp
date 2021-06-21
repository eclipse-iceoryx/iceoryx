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
#ifndef IOX_HOOFS_WIN_PLATFORM_PTHREAD_HPP
#define IOX_HOOFS_WIN_PLATFORM_PTHREAD_HPP

#include "iceoryx_hoofs/platform/unique_system_id.hpp"
#include "iceoryx_hoofs/platform/win32_errorHandling.hpp"
#include "iceoryx_hoofs/platform/windows.hpp"

#include <thread>
#include <type_traits>

#define PTHREAD_PROCESS_SHARED 0
#define PTHREAD_MUTEX_RECURSIVE_NP 1
#define PTHREAD_MUTEX_FAST_NP 2
#define PTHREAD_PRIO_NONE 3

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

int pthread_mutex_destroy(pthread_mutex_t* mutex);
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
int pthread_mutex_lock(pthread_mutex_t* mutex);
int pthread_mutex_trylock(pthread_mutex_t* mutex);
int pthread_mutex_unlock(pthread_mutex_t* mutex);

int iox_pthread_setname_np(pthread_t thread, const char* name);
int pthread_getname_np(pthread_t thread, char* name, size_t len);

#endif // IOX_HOOFS_WIN_PLATFORM_PTHREAD_HPP
