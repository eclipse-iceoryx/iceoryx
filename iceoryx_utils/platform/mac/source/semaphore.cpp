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
#include "iceoryx_utils/platform/time.hpp"

#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <thread>

static void deleteValue(std::atomic<int>*& value) noexcept
{
    if (value != nullptr)
    {
        delete value;
        value = nullptr;
    }
}

iox_sem_t::iox_sem_t() noexcept
    : m_value{new std::atomic<int>}
{
}

iox_sem_t::iox_sem_t(iox_sem_t&& rhs) noexcept
{
    *this = std::move(rhs);
}

iox_sem_t::~iox_sem_t()
{
    deleteValue(m_value);
}

iox_sem_t& iox_sem_t::operator=(iox_sem_t&& rhs) noexcept
{
    if (this != &rhs)
    {
        deleteValue(m_value);
        m_handle = rhs.m_handle;
        m_hasPosixHandle = rhs.m_hasPosixHandle;
        m_value = rhs.m_value;
        rhs.m_value = nullptr;
    }
    return *this;
}

int iox_sem_getvalue(iox_sem_t* sem, int* sval)
{
    *sval = sem->m_value->load(std::memory_order_relaxed);
    return 0;
}

int iox_sem_post(iox_sem_t* sem)
{
    int retVal{0};
    if (sem->m_hasPosixHandle)
    {
        retVal = sem_post(sem->m_handle.posix);
        if (retVal == 0)
        {
            sem->m_value->fetch_add(1, std::memory_order_relaxed);
        }
    }
    else
    {
        // dispatch semaphore always succeed
        dispatch_semaphore_signal(sem->m_handle.dispatch);
        sem->m_value->fetch_add(1, std::memory_order_relaxed);
    }
    return retVal;
}

int iox_sem_wait(iox_sem_t* sem)
{
    int retVal{0};
    if (sem->m_hasPosixHandle)
    {
        retVal = sem_wait(sem->m_handle.posix);
        if (retVal == 0)
        {
            sem->m_value->fetch_sub(1, std::memory_order_relaxed);
        }
    }
    else
    {
        // dispatch semaphore always succeed
        dispatch_semaphore_wait(sem->m_handle.dispatch, DISPATCH_TIME_FOREVER);
        sem->m_value->fetch_sub(1, std::memory_order_relaxed);
    }

    return retVal;
}

int iox_sem_trywait(iox_sem_t* sem)
{
    int retVal{0};
    if (sem->m_hasPosixHandle)
    {
        retVal = sem_trywait(sem->m_handle.posix);
        if (retVal == 0)
        {
            sem->m_value->fetch_sub(1, std::memory_order_relaxed);
        }
    }
    else
    {
        if (dispatch_semaphore_wait(sem->m_handle.dispatch, 0) != 0)
        {
            errno = EAGAIN;
            retVal = -1;
        }
        else
        {
            sem->m_value->fetch_sub(1, std::memory_order_relaxed);
        }
    }

    return retVal;
}

int iox_sem_timedwait(iox_sem_t* sem, const struct timespec* abs_timeout)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    static constexpr int64_t NANOSECONDS_PER_SECOND = 1000000000;
    static constexpr int64_t NANOSECONDS_PER_MICROSECOND = 1000;

    int64_t timeoutInNanoSeconds = std::max(0ll,
                                            (abs_timeout->tv_sec - tv.tv_sec) * NANOSECONDS_PER_SECOND
                                                + abs_timeout->tv_nsec - tv.tv_usec * NANOSECONDS_PER_MICROSECOND);

    if (sem->m_hasPosixHandle)
    {
        int tryWaitCall = sem_trywait(sem->m_handle.posix);
        if (tryWaitCall == -1 && errno != EAGAIN)
        {
            return -1;
        }
        else if (tryWaitCall == -1 && errno == EAGAIN && timeoutInNanoSeconds == 0)
        {
            errno = ETIMEDOUT;
            return -1;
        }
        else if (tryWaitCall == 0)
        {
            sem->m_value->fetch_sub(1, std::memory_order_relaxed);
            return 0;
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(timeoutInNanoSeconds));

        tryWaitCall = sem_trywait(sem->m_handle.posix);
        if (tryWaitCall == -1 && errno == EAGAIN)
        {
            errno = ETIMEDOUT;
            return -1;
        }
        else if (tryWaitCall == -1 && errno != EAGAIN)
        {
            return -1;
        }
        else if (tryWaitCall == 0)
        {
            sem->m_value->fetch_sub(1, std::memory_order_relaxed);
            return 0;
        }
    }
    else
    {
        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, timeoutInNanoSeconds);
        if (dispatch_semaphore_wait(sem->m_handle.dispatch, timeout) != 0)
        {
            errno = ETIMEDOUT;
            return -1;
        }
        else
        {
            sem->m_value->fetch_sub(1, std::memory_order_relaxed);
            return 0;
        }
    }

    return -1;
}

int iox_sem_close(iox_sem_t* sem)
{
    // will only be called by named semaphores which are in our case
    // posix semaphores
    int retVal = sem_close(sem->m_handle.posix);
    delete sem;
    return retVal;
}

int iox_sem_destroy(iox_sem_t* sem)
{
    // will only be called by unnamed semaphores which are in our
    // case dispatch semaphores
    dispatch_release(sem->m_handle.dispatch);
    return 0;
}

int iox_sem_init(iox_sem_t* sem, int pshared, unsigned int value)
{
    sem->m_hasPosixHandle = false;
    sem->m_handle.dispatch = dispatch_semaphore_create(value);
    sem->m_value->store(value, std::memory_order_relaxed);

    if (sem->m_handle.dispatch == nullptr)
    {
        delete sem;
        return -1;
    }
    return 0;
}

int iox_sem_unlink(const char* name)
{
    return sem_unlink(name);
}

iox_sem_t* iox_sem_open_impl(const char* name, int oflag, ...)
{
    iox_sem_t* sem = new iox_sem_t;

    if (oflag & (O_CREAT | O_EXCL))
    {
        va_list va;
        va_start(va, oflag);
        mode_t mode = va_arg(va, mode_t);
        unsigned int value = va_arg(va, unsigned int);
        va_end(va);

        sem->m_handle.posix = sem_open(name, oflag, mode, value);
        sem->m_value->store(value, std::memory_order_relaxed);
    }
    else
    {
        sem->m_handle.posix = sem_open(name, oflag);
    }

    if (sem->m_handle.posix == SEM_FAILED)
    {
        delete sem;
        return reinterpret_cast<iox_sem_t*>(SEM_FAILED);
    }

    return sem;
}
