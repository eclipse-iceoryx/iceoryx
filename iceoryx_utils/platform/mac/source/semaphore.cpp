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

#include "iceoryx_utils/platform/semaphore.hpp"

#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <thread>

int iox_sem_getvalue(iox_sem_t* sem, int* sval)
{
    std::cerr << "sem_getvalue is not available on mac OS\n";
    return -1;
}

int iox_sem_post(iox_sem_t* sem)
{
    if (sem->hasPosixHandle)
    {
        return sem_post(sem->handle.posix);
    }
    else
    {
        dispatch_semaphore_signal(nullptr);
    }
}

int iox_sem_wait(iox_sem_t* sem)
{
    if (sem->hasPosixHandle)
    {
        return sem_wait(sem->handle.posix);
    }
    else
    {
        dispatch_time_t timeout;
        dispatch_semaphore_wait(nullptr, timeout);
    }
}

int iox_sem_trywait(iox_sem_t* sem)
{
    if (sem->hasPosixHandle)
    {
        return sem_trywait(sem->handle.posix);
    }
    else
    {
    }
}

int iox_sem_timedwait(iox_sem_t* sem, const struct timespec* abs_timeout)
{
    if (sem->hasPosixHandle)
    {
        int tryWaitCall = sem_trywait(sem->handle.posix);
        if (tryWaitCall == 0)
        {
            return 0;
        }
        else if (tryWaitCall == -1 && errno != EAGAIN)
        {
            return -1;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        tryWaitCall = sem_trywait(sem->handle.posix);
        if (tryWaitCall == 0)
        {
            return 0;
        }
        else if (tryWaitCall == -1 && errno == EAGAIN)
        {
            errno = ETIMEDOUT;
        }
        return -1;
    }
}

int iox_sem_close(iox_sem_t* sem)
{
    return sem_close(sem->handle.posix);
}

int iox_sem_destroy(iox_sem_t* sem)
{
    // dispatch_release(nullptr);
}

int iox_sem_init(iox_sem_t* sem, int pshared, unsigned int value)
{
    // dispatch_semaphore_t handle = dispatch_semaphore_create(value);
}

int iox_sem_unlink(const char* name)
{
    return sem_unlink(name);
}

iox_sem_t* iox_sem_open_impl(const char* name, int oflag, ...)
{
    iox_sem_t* sem = static_cast<iox_sem_t*>(malloc(sizeof(iox_sem_t)));
    sem->hasPosixHandle = true;

    if (oflag & (O_CREAT | O_EXCL))
    {
        va_list va;
        va_start(va, oflag);
        mode_t mode = va_arg(va, mode_t);
        unsigned int value = va_arg(va, unsigned int);
        va_end(va);

        sem->handle.posix = sem_open(name, oflag, mode, value);
    }
    else
    {
        sem->handle.posix = sem_open(name, oflag);
    }

    if (sem->handle.posix == SEM_FAILED)
    {
        free(sem);
        return reinterpret_cast<iox_sem_t*>(SEM_FAILED);
    }

    return sem;
}
