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

#include "iceoryx_utils/platform/types.hpp"

#include <stdio.h>
#include <windows.h>

#define SEM_FAILED 0

using sem_t = HANDLE;
static constexpr LONG MAX_SEMAPHORE_VALUE = LONG_MAX;

inline int sem_getvalue(sem_t* sem, int* sval)
{
    static_assert(false, "Not available for Windows");
    return 0;
}

inline int sem_post(sem_t* sem)
{
    int retVal = (ReleaseSemaphore(sem, 1, nullptr) == 0) ? -1 : 0;
    return retVal;
}

inline int sem_wait(sem_t* sem)
{
    int retVal = (WaitForSingleObject(sem, INFINITE) == WAIT_FAILED) ? -1 : 0;
    return retVal;
}

inline int sem_trywait(sem_t* sem)
{
    if (WaitForSingleObject(sem, 0) == WAIT_FAILED)
    {
        return -1;
    }
    return 0;
}

inline int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout)
{
    DWORD timeoutInMilliseconds = static_cast<DWORD>(1);
    int retVal = (WaitForSingleObject(sem, timeoutInMilliseconds) == WAIT_FAILED) ? -1 : 0;

    return retVal;
}

inline int sem_close(sem_t* sem)
{
    int retVal = CloseHandle(sem) ? 0 : -1;
    return retVal;
}

inline int sem_destroy(sem_t* sem)
{
    // semaphores are closed in windows when the last process which is
    // holding a semaphore calls CloseHandle
    return 0;
}

inline int sem_init(sem_t* sem, int pshared, unsigned int value)
{
    *sem = CreateSemaphore(nullptr, static_cast<LONG>(value), MAX_SEMAPHORE_VALUE, nullptr);
    if (sem != nullptr)
        return 0;
    return -1;
}

// sem_t *
// sem_open( const char *name, int oflag )
//{
//}

inline sem_t* sem_open(const char* name, int oflag, mode_t mode, unsigned int value)
{
    return OpenSemaphoreW(0, false, name);
}

inline int sem_unlink(const char* name)
{
    // semaphores are closed in windows when the last process which is
    // holding a semaphore calls CloseHandle
    return 0;
}
