// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

namespace iox
{
namespace posix
{
Semaphore::Semaphore() noexcept
{
}

Semaphore::Semaphore(Semaphore&& rhs) noexcept
{
    *this = std::move(rhs);
}

Semaphore& Semaphore::operator=(Semaphore&& rhs) noexcept
{
    if (this != &rhs)
    {
        strncpy(m_name, rhs.m_name, m_nameSize);
        m_isInitialized = std::move(rhs.m_isInitialized);
        m_isCreated = std::move(rhs.m_isCreated);
        m_isNamedSemaphore = std::move(rhs.m_isNamedSemaphore);
        m_handle = std::move(rhs.m_handle);
        m_isShared = std::move(rhs.m_isShared);
        if (m_isNamedSemaphore || m_isShared)
        {
            m_handlePtr = std::move(rhs.m_handlePtr);
        }
        else
        {
            m_handlePtr = &m_handle;
        }

        rhs.m_isInitialized = false;
    }

    return *this;
}

Semaphore::~Semaphore() noexcept
{
    if (m_isInitialized)
    {
        if (isNamedSemaphore())
        {
            close();
            if (m_isCreated)
            {
                unlink(m_name);
            }
        }
        else
        {
            destroy();
        }
    }
}

bool Semaphore::getValue(int& value) const noexcept
{
    return m_isInitialized
           && !cxx::makeSmartC(sem_getvalue, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr, &value)
                   .hasErrors();
}

bool Semaphore::post() noexcept
{
    return m_isInitialized
           && !cxx::makeSmartC(sem_post, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr).hasErrors();
}

bool Semaphore::timedWait(const struct timespec* abs_timeout, const bool doContinueOnInterrupt) const noexcept
{
    if (!m_isInitialized)
    {
        return false;
    }

    if (doContinueOnInterrupt)
    {
        // we wait so long until sem_timedwait returns without an
        // interruption error
        while (true)
        {
            auto cCall = cxx::makeSmartC(
                sem_timedwait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {ETIMEDOUT}, m_handlePtr, abs_timeout);
            if (cCall.hasErrors())
            {
                return false;
            }
            else if (cCall.getErrNum() == ETIMEDOUT)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
    else
    {
        auto cCall = cxx::makeSmartC(
            sem_timedwait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {ETIMEDOUT}, m_handlePtr, abs_timeout);
        if (cCall.hasErrors() || cCall.getErrNum() == ETIMEDOUT)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}

bool Semaphore::tryWait() const noexcept
{
    if (!m_isInitialized)
    {
        return false;
    }

    auto cCall = cxx::makeSmartC(sem_trywait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {EAGAIN}, m_handlePtr);

    if (cCall.hasErrors() || cCall.getErrNum() == EAGAIN)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Semaphore::wait() const noexcept
{
    return m_isInitialized
           && !cxx::makeSmartC(sem_wait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr).hasErrors();
}

sem_t* Semaphore::getHandle() noexcept
{
    return m_handlePtr;
}

bool Semaphore::hasSemaphoreNameOverflow(const char* name) noexcept
{
    if (strlen(name) >= m_nameSize)
    {
        std::cerr << "Semaphore name: '" << name << "' is too long. ";
        std::cerr << "Maximum allowed characters are " << m_nameSize << std::endl;
        return true;
    }
    return false;
}

Semaphore::Semaphore(const unsigned int value) noexcept
    : m_isNamedSemaphore(false)
{
    if (init(&m_handle, 0, value))
    {
        m_isInitialized = true;
    }
}

Semaphore::Semaphore(sem_t* handle, const unsigned int value) noexcept
    : m_isNamedSemaphore(false)
    , m_isShared(true)
    , m_handlePtr(handle)
{
    if (init(handle, 1, value))
    {
        m_isInitialized = true;
    }
}

Semaphore::Semaphore(const char* name, const int oflag) noexcept
    : m_isCreated(false)
{
    if (!hasSemaphoreNameOverflow(name))
    {
        strncpy(m_name, name, m_nameSize);
        if (open(oflag))
        {
            m_isInitialized = true;
        }
    }
}

Semaphore::Semaphore(const char* name, const mode_t mode, const unsigned int value) noexcept
{
    if (!hasSemaphoreNameOverflow(name))
    {
        strncpy(m_name, name, m_nameSize);
        if (open(O_CREAT | O_EXCL, mode, value))
        {
            m_isInitialized = true;
        }
    }
}

bool Semaphore::close() noexcept
{
    return !cxx::makeSmartC(sem_close, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr).hasErrors();
}

bool Semaphore::destroy() noexcept
{
    return !cxx::makeSmartC(sem_destroy, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, &m_handle).hasErrors();
}

bool Semaphore::init(sem_t* handle, const int pshared, const unsigned int value) noexcept
{
    return !cxx::makeSmartC(sem_init, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, handle, pshared, value)
                .hasErrors();
}

bool Semaphore::open(const int oflag) noexcept
{
    bool success = setHandleFromCall(cxx::makeSmartC(
        sem_open, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {static_cast<sem_t*>(SEM_FAILED)}, {}, m_name, oflag));

    if (!success)
    {
        m_errorValue = SemaphoreError::CREATION_FAILED;
    }
    return success;
}

bool Semaphore::open(const int oflag, const mode_t mode, const unsigned int value) noexcept
{
    bool success = setHandleFromCall(cxx::makeSmartC(sem_open,
                                                     cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                                     {static_cast<sem_t*>(SEM_FAILED)},
                                                     {},
                                                     m_name,
                                                     oflag,
                                                     mode,
                                                     value));
    if (!success)
    {
        m_errorValue = SemaphoreError::CREATION_FAILED;
    }
    return success;
}

bool Semaphore::unlink(const char* name) noexcept
{
    return !cxx::makeSmartC(sem_unlink, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, name).hasErrors();
}

bool Semaphore::isNamedSemaphore() noexcept
{
    return m_isNamedSemaphore;
}
} // namespace posix
} // namespace iox
