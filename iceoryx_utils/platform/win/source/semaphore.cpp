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
// limitations under the License

#include "iceoryx_utils/platform/semaphore.hpp"

#include <cstdarg>

int iox_sem_getvalue(iox_sem_t* sem, int* sval)
{
    LONG previousValue;
    auto waitResult = Win32Call(WaitForSingleObject(sem->handle, 0));
    switch (waitResult)
    {
    case WAIT_OBJECT_0:
    {
        auto releaseResult = Win32Call(ReleaseSemaphore(sem->handle, 1, &previousValue));
        if (releaseResult)
        {
            *sval = previousValue + 1;
            return 0;
        }
        return 0;
    }
    case WAIT_TIMEOUT:
    {
        *sval = 0;
        return 0;
    }
    default:
    {
        return -1;
    }
    }
}

int iox_sem_post(iox_sem_t* sem)
{
    int retVal = Win32Call(ReleaseSemaphore(sem->handle, 1, nullptr));
    return (retVal != 0) ? 0 : -1;
}

int iox_sem_wait(iox_sem_t* sem)
{
    int retVal = Win32Call(WaitForSingleObject(sem->handle, INFINITE));
    return (retVal == WAIT_OBJECT_0) ? 0 : -1;
}

int iox_sem_trywait(iox_sem_t* sem)
{
    int retVal = Win32Call(WaitForSingleObject(sem->handle, 0));
    return (retVal == WAIT_OBJECT_0) ? 0 : -1;
}

int iox_sem_timedwait(iox_sem_t* sem, const struct timespec* abs_timeout)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    if (abs_timeout->tv_sec < tv.tv_sec
        || (abs_timeout->tv_sec == tv.tv_sec && abs_timeout->tv_nsec < tv.tv_usec * 1000))
    {
        return iox_sem_trywait(sem);
    }

    time_t epochCurrentTimeDiffInSeconds = abs_timeout->tv_sec - tv.tv_sec;
    long milliseconds = epochCurrentTimeDiffInSeconds * 1000 + ((abs_timeout->tv_nsec / 1000) - tv.tv_usec) / 1000;

    auto state = Win32Call(WaitForSingleObject(sem->handle, milliseconds));
    if (state == WAIT_TIMEOUT)
    {
        errno = ETIMEDOUT;
    }

    return (state == WAIT_OBJECT_0) ? 0 : -1;
}

int iox_sem_close(iox_sem_t* sem)
{
    int retVal = Win32Call(CloseHandle(sem->handle)) ? 0 : -1;
    delete sem;
    return (retVal) ? 0 : -1;
}

int iox_sem_destroy(iox_sem_t* sem)
{
    // semaphores are destroyed in windows when the last process which is
    // holding a semaphore calls CloseHandle
    return 0;
}

HANDLE __sem_create_win32_semaphore(LONG value, LPCSTR name)
{
    SECURITY_ATTRIBUTES securityAttribute;
    SECURITY_DESCRIPTOR securityDescriptor;
    Win32Call(InitializeSecurityDescriptor(&securityDescriptor, SECURITY_DESCRIPTOR_REVISION));

    TCHAR* permissions = TEXT("D:") TEXT("(A;OICI;GA;;;BG)") // access to built-in guests
        TEXT("(A;OICI;GA;;;AN)")                             // access to anonymous logon
        TEXT("(A;OICI;GRGWGX;;;AU)")                         // access to authenticated users
        TEXT("(A;OICI;GA;;;BA)");                            // access to administrators

    Win32Call(ConvertStringSecurityDescriptorToSecurityDescriptor(
        permissions, SDDL_REVISION_1, &(securityAttribute.lpSecurityDescriptor), NULL));
    securityAttribute.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttribute.lpSecurityDescriptor = &securityDescriptor;
    securityAttribute.bInheritHandle = FALSE;

    HANDLE returnValue = Win32Call(CreateSemaphoreA(&securityAttribute, value, MAX_SEMAPHORE_VALUE, name));
    return returnValue;
}


int iox_sem_init(iox_sem_t* sem, int pshared, unsigned int value)
{
    sem->handle = __sem_create_win32_semaphore(value, nullptr);
    if (sem->handle != nullptr)
    {
        return 0;
    }
    return -1;
}

int iox_sem_unlink(const char* name)
{
    // semaphores are unlinked in windows when the last process which is
    // holding a semaphore calls CloseHandle
    return 0;
}

iox_sem_t* iox_sem_open_impl(const char* name, int oflag, ...) // mode_t mode, unsigned int value
{
    iox_sem_t* sem = new iox_sem_t;
    if (oflag & (O_CREAT | O_EXCL))
    {
        va_list va;
        va_start(va, oflag);
        mode_t mode = va_arg(va, mode_t);
        unsigned int value = va_arg(va, unsigned int);
        va_end(va);

        sem->handle = __sem_create_win32_semaphore(value, name);
        if (oflag & O_EXCL && GetLastError() == ERROR_ALREADY_EXISTS)
        {
            iox_sem_close(sem);
            return SEM_FAILED;
        }
        else if (sem->handle == nullptr)
        {
            delete sem;
            return SEM_FAILED;
        }
    }
    else
    {
        sem->handle = Win32Call(OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, name));
        if (sem->handle == nullptr)
        {
            delete sem;
            return SEM_FAILED;
        }
    }
    return sem;
}
