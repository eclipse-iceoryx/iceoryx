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
#ifndef IOX_HOOFS_WIN_PLATFORM_SEMAPHORE_HPP
#define IOX_HOOFS_WIN_PLATFORM_SEMAPHORE_HPP

#include "iceoryx_hoofs/platform/fcntl.hpp"
#include "iceoryx_hoofs/platform/time.hpp"
#include "iceoryx_hoofs/platform/types.hpp"
#include "iceoryx_hoofs/platform/unique_system_id.hpp"
#include "iceoryx_hoofs/platform/win32_errorHandling.hpp"
#include "iceoryx_hoofs/platform/windows.hpp"

#include <cstdlib>
#include <map>
#include <sddl.h>
#include <stdio.h>
#include <time.h>
#include <type_traits>

#define SEM_FAILED 0

struct iox_sem_t
{
    HANDLE handle{nullptr};
    bool isInterprocessSemaphore{false};
    UniqueSystemId uniqueId;
};
static constexpr LONG MAX_SEMAPHORE_VALUE = LONG_MAX;
static constexpr int MAX_SEMAPHORE_NAME_LENGTH = 128;

int iox_sem_getvalue(iox_sem_t* sem, int* sval);
int iox_sem_post(iox_sem_t* sem);
int iox_sem_wait(iox_sem_t* sem);
int iox_sem_trywait(iox_sem_t* sem);
int iox_sem_timedwait(iox_sem_t* sem, const struct timespec* abs_timeout);
int iox_sem_close(iox_sem_t* sem);
int iox_sem_destroy(iox_sem_t* sem);
int iox_sem_init(iox_sem_t* sem, int pshared, unsigned int value);
iox_sem_t* iox_sem_open_impl(const char* name, int oflag, ...);
int iox_sem_unlink(const char* name);

template <typename... Targs>
inline iox_sem_t* iox_sem_open(const char* name, int oflag, Targs... args)
{
    return iox_sem_open_impl(name, oflag, args...);
}

#endif // IOX_HOOFS_WIN_PLATFORM_SEMAPHORE_HPP
