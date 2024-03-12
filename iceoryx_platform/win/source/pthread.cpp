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

#include "iceoryx_platform/pthread.hpp"
#include "iceoryx_platform/ipc_handle_manager.hpp"
#include "iceoryx_platform/logging.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"
#include "iceoryx_platform/windows.hpp"

#include <cwchar>
#include <sstream>
#include <vector>

HRESULT GetThreadDescription(HANDLE hThread, PWSTR* ppszThreadDescription);
HRESULT SetThreadDescription(HANDLE hThread, PCWSTR lpThreadDescription);

int iox_pthread_setname_np(iox_pthread_t thread [[maybe_unused]], const char* name [[maybe_unused]])
{
#if defined(__GNUC__) || defined(__GNUG__)

    return 0;

#elif defined(_MSC_VER)

    std::mbstate_t state = std::mbstate_t();
    uint64_t length = std::mbsrtowcs(nullptr, &name, 0, &state) + 1U;
    std::vector<wchar_t> wName(length);
    std::mbsrtowcs(wName.data(), &name, length, &state);

    return Win32Call(SetThreadDescription, thread, wName.data()).error;

#endif
}

int iox_pthread_getname_np(iox_pthread_t thread, char* name, size_t len)
{
#if defined(__GNUC__) || defined(__GNUG__)

    return 0;

#elif defined(_MSC_VER)

    wchar_t* wName;
    auto result = Win32Call(GetThreadDescription, thread, &wName).error;
    if (result == 0)
    {
        wcstombs(name, wName, len);
        LocalFree(wName);
    }

    return result;

#endif
}

struct win_routine_args
{
    void* (*start_routine)(void*);
    void* arg;
};

DWORD WINAPI win_start_routine(LPVOID lpParam)
{
    win_routine_args* args = static_cast<win_routine_args*>(lpParam);
    args->start_routine(args->arg);
    delete args;
    return 0;
}

int iox_pthread_create(iox_pthread_t* thread, const iox_pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
{
    win_routine_args* args = new win_routine_args();
    args->start_routine = start_routine;
    args->arg = arg;
    auto result = Win32Call(CreateThread,
                            static_cast<LPSECURITY_ATTRIBUTES>(NULL),
                            static_cast<SIZE_T>(0),
                            static_cast<LPTHREAD_START_ROUTINE>(win_start_routine),
                            static_cast<LPVOID>(args),
                            static_cast<DWORD>(0),
                            static_cast<LPDWORD>(NULL));

    if (result.error != 0)
    {
        delete args;
    }

    *thread = result.value;

    return result.error;
}

int iox_pthread_join(iox_pthread_t thread, void**)
{
    return Win32Call(WaitForSingleObject, thread, INFINITE).error;
}

iox_pthread_t iox_pthread_self()
{
    return GetCurrentThread();
}

int iox_pthread_mutexattr_destroy(iox_pthread_mutexattr_t* attr)
{
    return 0;
}

int iox_pthread_mutexattr_init(iox_pthread_mutexattr_t* attr)
{
    new (attr) iox_pthread_mutexattr_t();
    return 0;
}

int iox_pthread_mutexattr_setpshared(iox_pthread_mutexattr_t* attr, int pshared)
{
    if (pshared == IOX_PTHREAD_PROCESS_SHARED)
    {
        attr->isInterprocessMutex = true;
    }
    return 0;
}

int iox_pthread_mutexattr_settype(iox_pthread_mutexattr_t* attr, int type)
{
    return 0;
}

int iox_pthread_mutexattr_setprotocol(iox_pthread_mutexattr_t* attr, int protocol)
{
    return 0;
}

int iox_pthread_mutexattr_setrobust(iox_pthread_mutexattr_t* attr, int robustness)
{
    return 0;
}

int iox_pthread_mutexattr_setprioceiling(iox_pthread_mutexattr_t* attr, int prioceiling)
{
    return 0;
}

int iox_pthread_mutex_consistent(iox_pthread_mutex_t* mutex)
{
    return 0;
}

int iox_pthread_mutex_destroy(iox_pthread_mutex_t* mutex)
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

static HANDLE acquireMutexHandle(iox_pthread_mutex_t* mutex)
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
        std::stringstream stream;
        stream << "interprocess mutex '" << generateMutexName(mutex->uniqueId)
               << "' is corrupted - segmentation fault immenent";
        IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, stream.str().c_str());
        return nullptr;
    }

    IpcHandleManager::getInstance().addHandle(mutex->uniqueId, OwnerShip::LOAN, newHandle);
    return newHandle;
}

int iox_pthread_mutex_init(iox_pthread_mutex_t* mutex, const iox_pthread_mutexattr_t* attr)
{
    *mutex = iox_pthread_mutex_t();
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

int iox_pthread_mutex_lock(iox_pthread_mutex_t* mutex)
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

int iox_pthread_mutex_trylock(iox_pthread_mutex_t* mutex)
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

int iox_pthread_mutex_unlock(iox_pthread_mutex_t* mutex)
{
    auto releaseResult = Win32Call(ReleaseMutex, acquireMutexHandle(mutex)).value;
    if (!releaseResult)
    {
        return EPERM;
    }
    return 0;
}
