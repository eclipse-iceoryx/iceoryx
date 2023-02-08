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
#ifndef IOX_HOOFS_UNIX_PLATFORM_SEMAPHORE_HPP
#define IOX_HOOFS_UNIX_PLATFORM_SEMAPHORE_HPP

#include <cstdint>
#include <semaphore.h>
#include <sys/types.h>

using iox_sem_t = sem_t;

#define IOX_SEM_FAILED SEM_FAILED
constexpr uint32_t IOX_SEM_VALUE_MAX = SEM_VALUE_MAX;

inline int iox_sem_getvalue(iox_sem_t* sem, int* sval)
{
    return sem_getvalue(sem, sval);
}

inline int iox_sem_post(iox_sem_t* sem)
{
    return sem_post(sem);
}

inline int iox_sem_wait(iox_sem_t* sem)
{
    return sem_wait(sem);
}

inline int iox_sem_trywait(iox_sem_t* sem)
{
    return sem_trywait(sem);
}

inline int iox_sem_timedwait(iox_sem_t* sem, const struct timespec* abs_timeout)
{
    return sem_timedwait(sem, abs_timeout);
}

inline int iox_sem_close(iox_sem_t* sem)
{
    return sem_close(sem);
}

inline int iox_sem_destroy(iox_sem_t* sem)
{
    return sem_destroy(sem);
}

inline int iox_sem_init(iox_sem_t* sem, int pshared, unsigned int value)
{
    return sem_init(sem, pshared, value);
}

inline iox_sem_t* iox_sem_open(const char* name, int oflag)
{
    return sem_open(name, oflag);
}

inline iox_sem_t* iox_sem_open_ext(const char* name, int oflag, mode_t mode, unsigned int value)
{
    return sem_open(name, oflag, mode, value);
}

inline int iox_sem_unlink(const char* name)
{
    return sem_unlink(name);
}

#endif // IOX_HOOFS_UNIX_PLATFORM_SEMAPHORE_HPP
