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

#include "iceoryx_platform/semaphore.hpp"
#include "iceoryx_platform/ipc_handle_manager.hpp"
#include "iceoryx_platform/logging.hpp"

#include <cstdarg>
#include <sstream>

static std::string generateSemaphoreName(const UniqueSystemId& id)
{
    return "iox_semaphore_" + static_cast<std::string>(id);
}

static HANDLE acquireSemaphoreHandle(iox_sem_t* sem)
{
    if (!sem->isInterprocessSemaphore)
    {
        return sem->handle;
    }

    HANDLE newHandle;
    if (IpcHandleManager::getInstance().getHandle(sem->uniqueId, newHandle))
    {
        return newHandle;
    }

    newHandle =
        Win32Call(OpenSemaphoreA, SEMAPHORE_ALL_ACCESS, false, generateSemaphoreName(sem->uniqueId).c_str()).value;
    if (newHandle == nullptr)
    {
        std::stringstream stream;
        stream << "interprocess semaphore '" << generateSemaphoreName(sem->uniqueId)
               << "' is corrupted - segmentation fault immenent";
        IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, stream.str().c_str());


        return nullptr;
    }

    IpcHandleManager::getInstance().addHandle(sem->uniqueId, OwnerShip::LOAN, newHandle);
    return newHandle;
}

int iox_sem_getvalue(iox_sem_t* sem, int* sval)
{
    LONG previousValue;
    auto waitResult = Win32Call(WaitForSingleObject, acquireSemaphoreHandle(sem), 0).value;
    switch (waitResult)
    {
    case WAIT_OBJECT_0:
    {
        auto releaseResult = Win32Call(ReleaseSemaphore, acquireSemaphoreHandle(sem), 1, &previousValue).value;
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
    auto retVal = Win32Call(ReleaseSemaphore, acquireSemaphoreHandle(sem), 1, nullptr);
    if (retVal.error == ERROR_TOO_MANY_POSTS)
    {
        errno = EOVERFLOW;
    }
    return (retVal.value != 0) ? 0 : -1;
}

int iox_sem_wait(iox_sem_t* sem)
{
    auto retVal = Win32Call(WaitForSingleObject, acquireSemaphoreHandle(sem), INFINITE);
    return (retVal.value == WAIT_OBJECT_0) ? 0 : -1;
}

int iox_sem_trywait(iox_sem_t* sem)
{
    int retVal = Win32Call(WaitForSingleObject, acquireSemaphoreHandle(sem), 0).value;
    if (retVal != WAIT_OBJECT_0)
    {
        errno = EAGAIN;
    }
    return (retVal == WAIT_OBJECT_0) ? 0 : -1;
}

int iox_sem_timedwait(iox_sem_t* sem, const struct timespec* abs_timeout)
{
    struct timeval tv;
    iox_gettimeofday(&tv, nullptr);
    if (abs_timeout->tv_sec < tv.tv_sec
        || (abs_timeout->tv_sec == tv.tv_sec && abs_timeout->tv_nsec <= tv.tv_usec * 1000))
    {
        return iox_sem_trywait(sem);
    }

    constexpr uint64_t NANO_SECOND = 1000000000;
    constexpr uint64_t HALF_MILLI_SECOND_ROUNDING_CORRECTION_IN_NS = 500000;
    constexpr uint64_t NANO_SECONDS_PER_MICRO_SECOND = 1000;
    constexpr uint64_t NANO_SECONDS_PER_MILLI_SECOND = 1000000;

    uint64_t epochCurrentTimeDiffInNanoSeconds = (abs_timeout->tv_sec - tv.tv_sec) * NANO_SECOND;
    long milliseconds =
        (epochCurrentTimeDiffInNanoSeconds + (abs_timeout->tv_nsec - tv.tv_usec * NANO_SECONDS_PER_MICRO_SECOND)
         + HALF_MILLI_SECOND_ROUNDING_CORRECTION_IN_NS)
        / NANO_SECONDS_PER_MILLI_SECOND;

    auto state =
        Win32Call(WaitForSingleObject, acquireSemaphoreHandle(sem), (milliseconds == 0) ? 1 : milliseconds).value;
    if (state == WAIT_TIMEOUT)
    {
        errno = ETIMEDOUT;
    }

    return (state == WAIT_OBJECT_0) ? 0 : -1;
}

int iox_sem_close(iox_sem_t* sem)
{
    // we close a named semaphore, therefore we do not need to perform an ipc cleanup
    int retVal = Win32Call(CloseHandle, acquireSemaphoreHandle(sem)).value ? 0 : -1;
    delete sem;
    return (retVal) ? 0 : -1;
}

int iox_sem_destroy(iox_sem_t* sem)
{
    CloseHandle(acquireSemaphoreHandle(sem));
    if (sem->isInterprocessSemaphore)
    {
        IpcHandleManager::getInstance().removeHandle(sem->uniqueId);
    }
    return 0;
}

static Win32CallReturn<HANDLE> sem_create_win32_semaphore(LONG value, LPCSTR name)
{
    SECURITY_ATTRIBUTES securityAttribute;
    SECURITY_DESCRIPTOR securityDescriptor;
    Win32Call(InitializeSecurityDescriptor, &securityDescriptor, SECURITY_DESCRIPTOR_REVISION).value;

    auto permissions = TEXT("D:") TEXT("(A;OICI;GA;;;BG)") // access to built-in guests
        TEXT("(A;OICI;GA;;;AN)")                           // access to anonymous logon
        TEXT("(A;OICI;GRGWGX;;;AU)")                       // access to authenticated users
        TEXT("(A;OICI;GA;;;BA)");                          // access to administrators

    Win32Call(ConvertStringSecurityDescriptorToSecurityDescriptor,
              reinterpret_cast<LPCSTR>(permissions),
              static_cast<DWORD>(SDDL_REVISION_1),
              static_cast<PSECURITY_DESCRIPTOR*>(&(securityAttribute.lpSecurityDescriptor)),
              static_cast<PULONG>(NULL));
    securityAttribute.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttribute.lpSecurityDescriptor = &securityDescriptor;
    securityAttribute.bInheritHandle = FALSE;

    return Win32Call(CreateSemaphoreA, &securityAttribute, value, IOX_SEM_VALUE_MAX, name);
}

int iox_sem_init(iox_sem_t* sem, int pshared, unsigned int value)
{
    sem->isInterprocessSemaphore = (pshared == 1);
    if (sem->isInterprocessSemaphore)
    {
        sem->handle = sem_create_win32_semaphore(value, generateSemaphoreName(sem->uniqueId).c_str()).value;
        if (sem->handle != nullptr)
        {
            IpcHandleManager::getInstance().addHandle(sem->uniqueId, OwnerShip::OWN, sem->handle);
        }
    }
    else
    {
        sem->handle = sem_create_win32_semaphore(value, nullptr).value;
    }

    return (sem->handle != nullptr) ? 0 : -1;
}

int iox_sem_unlink(const char* name)
{
    // semaphores are unlinked in windows when the last process which is
    // holding a semaphore calls CloseHandle
    return 0;
}

iox_sem_t* iox_sem_open_impl(const char* name, int oflag, ...) // mode_t mode, unsigned int value
{
    if (strlen(name) == 0)
    {
        return IOX_SEM_FAILED;
    }

    iox_sem_t* sem = new iox_sem_t;
    if (oflag & (O_CREAT | O_EXCL))
    {
        va_list va;
        va_start(va, oflag);
        // mode_t is an alias for unsigned short but this causes undefined
        // behavior in va_arg since it is a promotable type - which will be
        // promoted to int
        mode_t mode = static_cast<mode_t>(va_arg(va, unsigned int));
        unsigned int value = va_arg(va, unsigned int);
        va_end(va);

        auto result = sem_create_win32_semaphore(value, name);
        sem->handle = result.value;
        if (oflag & O_EXCL && result.error == ERROR_ALREADY_EXISTS)
        {
            errno = EEXIST;
            iox_sem_close(sem);
            return IOX_SEM_FAILED;
        }
        else if (sem->handle == nullptr)
        {
            delete sem;
            return IOX_SEM_FAILED;
        }
    }
    else
    {
        sem->handle = Win32Call(OpenSemaphoreA, SEMAPHORE_ALL_ACCESS, false, name).value;
        if (sem->handle == nullptr)
        {
            errno = ENOENT;
            delete sem;
            return IOX_SEM_FAILED;
        }
    }
    return sem;
}
