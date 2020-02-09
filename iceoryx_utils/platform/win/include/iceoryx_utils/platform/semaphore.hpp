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

#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/time.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/win32-error.hpp"
#include "iceoryx_utils/platform/windows.hpp"


#include <cstdlib>
#include <sddl.h>
#include <stdio.h>
#include <time.h>
#include <type_traits>

#define SEM_FAILED 0

struct sem_t
{
    HANDLE handle;
};
static constexpr LONG MAX_SEMAPHORE_VALUE = LONG_MAX;
static constexpr int MAX_SEMAPHORE_NAME_LENGTH = 128;

inline int sem_getvalue(sem_t* sem, int* sval)
{
    LONG previousValue;
    switch (WaitForSingleObject(sem->handle, 0))
    {
    case WAIT_OBJECT_0:
        if (ReleaseSemaphore(sem->handle, 1, &previousValue))
        {
            PrintLastErrorToConsole();
            *sval = previousValue + 1;
            return 0;
        }
        return 0;
    case WAIT_TIMEOUT:
        *sval = 0;
        return 0;
    default:
        return -1;
    }
}

inline int sem_post(sem_t* sem)
{
    int retVal = (ReleaseSemaphore(sem->handle, 1, nullptr) == 0) ? -1 : 0;
    PrintLastErrorToConsole();
    return retVal;
}

inline int sem_wait(sem_t* sem)
{
    int retVal = (WaitForSingleObject(sem->handle, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
    PrintLastErrorToConsole();
    return retVal;
}

inline int sem_trywait(sem_t* sem)
{
    int retVal = (WaitForSingleObject(sem->handle, 0) == WAIT_OBJECT_0) ? 0 : -1;
    PrintLastErrorToConsole();
    return retVal;
}

inline int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t epochCurrentTimeDiffInSeconds = abs_timeout->tv_sec - tv.tv_sec;
    long milliseconds = epochCurrentTimeDiffInSeconds * 1000 + ((abs_timeout->tv_nsec / 1000) - tv.tv_usec) / 1000;

    int retVal = (WaitForSingleObject(sem->handle, milliseconds) == WAIT_FAILED) ? -1 : 0;
    PrintLastErrorToConsole();

    return retVal;
}

inline int sem_close(sem_t* sem)
{
    int retVal = CloseHandle(sem->handle) ? 0 : -1;
    PrintLastErrorToConsole();
    free(sem);
    return retVal;
}

inline int sem_destroy(sem_t* sem)
{
    // semaphores are closed in windows when the last process which is
    // holding a semaphore calls CloseHandle
    return 0;
}

inline HANDLE __sem_create_win32_semaphore(LONG value, LPCSTR name)
{
    SECURITY_ATTRIBUTES securityAttribute;
    SECURITY_DESCRIPTOR securityDescriptor;
    InitializeSecurityDescriptor(&securityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    PrintLastErrorToConsole();

    TCHAR* permissions = TEXT("D:") TEXT("(A;OICI;GA;;;BG)") // access to built-in guests
        TEXT("(A;OICI;GA;;;AN)")                             // access to anonymous logon
        TEXT("(A;OICI;GRGWGX;;;AU)")                         // access to authenticated users
        TEXT("(A;OICI;GA;;;BA)");                            // access to administrators

    ConvertStringSecurityDescriptorToSecurityDescriptor(
        permissions, SDDL_REVISION_1, &(securityAttribute.lpSecurityDescriptor), NULL);
    PrintLastErrorToConsole();
    securityAttribute.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttribute.lpSecurityDescriptor = &securityDescriptor;
    securityAttribute.bInheritHandle = FALSE;

    HANDLE returnValue = CreateSemaphore(&securityAttribute, value, MAX_SEMAPHORE_VALUE, name);
    PrintLastErrorToConsole();
    return returnValue;
}


inline int sem_init(sem_t* sem, int pshared, unsigned int value)
{
    sem->handle = __sem_create_win32_semaphore(value, nullptr);
    if (sem != nullptr)
        return 0;
    return -1;
}

inline sem_t* sem_open(const char* name, int oflag, mode_t mode, unsigned int value)
{
    sem_t* sem = static_cast<sem_t*>(malloc(sizeof(sem_t)));
    if (oflag & (O_CREAT | O_EXCL))
    {
        sem->handle = __sem_create_win32_semaphore(value, name);
        if (oflag & O_EXCL && GetLastError() == ERROR_ALREADY_EXISTS)
        {
            sem_close(sem);
            return SEM_FAILED;
        }
    }
    else
    {
        sem->handle = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, name);
        PrintLastErrorToConsole();
        if (sem->handle == NULL)
        {
            free(sem);
            return SEM_FAILED;
        }
    }
    return sem;
}

inline sem_t* sem_open(const char* name, int oflag)
{
    return sem_open(name, oflag, mode_t(), 0);
}

inline int sem_unlink(const char* name)
{
    // semaphores are closed in windows when the last process which is
    // holding a semaphore calls CloseHandle
    return 0;
}
