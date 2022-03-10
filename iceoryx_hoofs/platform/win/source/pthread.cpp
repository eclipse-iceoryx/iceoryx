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

#include "iceoryx_hoofs/platform/pthread.hpp"
#include "iceoryx_hoofs/platform/ipc_handle_manager.hpp"
#include "iceoryx_hoofs/platform/win32_errorHandling.hpp"
#include "iceoryx_hoofs/platform/windows.hpp"

#include <cwchar>
#include <vector>

int iox_pthread_setname_np(pthread_t thread, const char* name)
{
    DWORD threadId = Win32Call(GetThreadId, static_cast<HANDLE>(thread)).value;

    std::mbstate_t state = std::mbstate_t();
    uint64_t length = std::mbsrtowcs(nullptr, &name, 0, &state) + 1U;
    std::vector<wchar_t> wName(length);
    std::mbsrtowcs(wName.data(), &name, length, &state);

    return Win32Call(SetThreadDescription, static_cast<HANDLE>(thread), wName.data()).error;
}

int pthread_getname_np(pthread_t thread, char* name, size_t len)
{
    wchar_t* wName;
    auto result = Win32Call(GetThreadDescription, static_cast<HANDLE>(thread), &wName).error;
    if (result == 0)
    {
        wcstombs(name, wName, len);
        LocalFree(wName);
    }

    return result;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t* attr)
{
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t* attr)
{
    new (attr) pthread_mutexattr_t();
    return 0;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared)
{
    if (pshared == PTHREAD_PROCESS_SHARED)
    {
        attr->isInterprocessMutex = true;
    }
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type)
{
    return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t* attr, int protocol)
{
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex)
{
    if (!mutex->isInterprocessMutex)
    {
        Win32Call(CloseHandle, mutex->handle);
    }

    return 0;
}

static HANDLE createWin32Mutex(LPSECURITY_ATTRIBUTES securityAttributes, BOOL initialOwner, LPCSTR name)
{
    return Win32Call(CreateMutexA, securityAttributes, initialOwner, name).value;
}

static std::string generateMutexName(const UniqueSystemId& id) noexcept
{
    return "iox_mutex_" + static_cast<std::string>(id);
}

static HANDLE acquireMutexHandle(pthread_mutex_t* mutex)
{
    if (!mutex->isInterprocessMutex)
    {
        return mutex->handle;
    }

    HANDLE newHandle;
    if (IpcHandleManager::getInstance().getHandle(mutex->uniqueId, newHandle))
    {
        return newHandle;
    }

    newHandle = Win32Call(OpenMutexA, MUTEX_ALL_ACCESS, false, generateMutexName(mutex->uniqueId).c_str()).value;
    if (newHandle == nullptr)
    {
        fprintf(stderr,
                "interprocess mutex %s is corrupted - segmentation fault immenent\n",
                generateMutexName(mutex->uniqueId).c_str());
        return nullptr;
    }

    IpcHandleManager::getInstance().addHandle(mutex->uniqueId, OwnerShip::LOAN, newHandle);
    return newHandle;
}

int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr)
{
    mutex->isInterprocessMutex = (attr != NULL && attr->isInterprocessMutex);

    if (!mutex->isInterprocessMutex)
    {
        mutex->handle = createWin32Mutex(NULL, FALSE, NULL);
    }
    else
    {
        mutex->handle = createWin32Mutex(NULL, FALSE, generateMutexName(mutex->uniqueId).c_str());
        if (mutex->handle != nullptr)
        {
            IpcHandleManager::getInstance().addHandle(mutex->uniqueId, OwnerShip::OWN, mutex->handle);
        }
    }

    return (mutex->handle == nullptr) ? EINVAL : 0;
}

int pthread_mutex_lock(pthread_mutex_t* mutex)
{
    DWORD waitResult = Win32Call(WaitForSingleObject, acquireMutexHandle(mutex), INFINITE).value;

    switch (waitResult)
    {
    case WAIT_OBJECT_0:
        return 0;
    default:
        return EINVAL;
    }
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t* mutex)
{
    DWORD waitResult = Win32Call(WaitForSingleObject, acquireMutexHandle(mutex), 0).value;

    switch (waitResult)
    {
    case WAIT_TIMEOUT:
        return EBUSY;
    case WAIT_OBJECT_0:
        return 0;
    default:
        return EINVAL;
    }
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t* mutex)
{
    auto releaseResult = Win32Call(ReleaseMutex, acquireMutexHandle(mutex)).value;
    if (!releaseResult)
    {
        return EPERM;
    }
    return 0;
}
