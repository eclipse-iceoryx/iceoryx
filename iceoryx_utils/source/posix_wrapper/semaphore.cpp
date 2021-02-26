// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/platform/platform_correction.hpp"

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
        closeHandle();

        CreationPattern_t::operator=(std::move(rhs));

        m_name = std::move(rhs.m_name);
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

        rhs.m_handlePtr = &rhs.m_handle;
    }

    return *this;
}

Semaphore::~Semaphore() noexcept
{
    closeHandle();
}

void Semaphore::closeHandle() noexcept
{
    if (m_isInitialized)
    {
        if (isNamedSemaphore())
        {
            close();
            if (m_isCreated)
            {
                unlink(m_name.c_str());
            }
        }
        else
        {
            destroy();
        }
        m_isInitialized = false;
    }
}

cxx::expected<int, SemaphoreError> Semaphore::getValue() const noexcept
{
    int value;
    auto call =
        cxx::makeSmartC(iox_sem_getvalue, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr, &value);
    if (call.hasErrors())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.getErrNum()));
    }

    return cxx::success<int>(value);
}

cxx::expected<SemaphoreError> Semaphore::post() noexcept
{
    auto call = cxx::makeSmartC(iox_sem_post, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr);
    if (call.hasErrors())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.getErrNum()));
    }

    return cxx::success<>();
}

cxx::expected<SemaphoreWaitState, SemaphoreError> Semaphore::timedWait(const units::Duration abs_timeout,
                                                                       const bool doContinueOnInterrupt) const noexcept
{
    const struct timespec timeout = abs_timeout.timespec(units::TimeSpecReference::Epoch);
    if (doContinueOnInterrupt)
    {
        // we wait so long until iox_sem_timedwait returns without an
        // interruption error
        while (true)
        {
            auto cCall = cxx::makeSmartC(
                iox_sem_timedwait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {ETIMEDOUT}, m_handlePtr, &timeout);
            if (cCall.hasErrors())
            {
                return cxx::error<SemaphoreError>(errnoToEnum(cCall.getErrNum()));
            }
            else if (cCall.getErrNum() == ETIMEDOUT)
            {
                return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::TIMEOUT);
            }
            else
            {
                return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::NO_TIMEOUT);
            }
        }
    }
    else
    {
        auto cCall = cxx::makeSmartC(
            iox_sem_timedwait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {ETIMEDOUT}, m_handlePtr, &timeout);
        if (cCall.hasErrors() || cCall.getErrNum() == ETIMEDOUT)
        {
            return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::TIMEOUT);
        }
        else
        {
            return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::NO_TIMEOUT);
        }
    }
}

cxx::expected<bool, SemaphoreError> Semaphore::tryWait() const noexcept
{
    auto cCall = cxx::makeSmartC(iox_sem_trywait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {EAGAIN}, m_handlePtr);

    if (cCall.hasErrors() && cCall.getErrNum() != EAGAIN)
    {
        return cxx::error<SemaphoreError>(errnoToEnum(cCall.getErrNum()));
    }
    else if (cCall.getErrNum() == EAGAIN)
    {
        return cxx::success<bool>(false);
    }

    return cxx::success<bool>(true);
}

cxx::expected<SemaphoreError> Semaphore::wait() const noexcept
{
    auto call = cxx::makeSmartC(iox_sem_wait, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr);
    if (call.hasErrors())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.getErrNum()));
    }

    return cxx::success<>();
}

iox_sem_t* Semaphore::getHandle() noexcept
{
    return m_handlePtr;
}

Semaphore::Semaphore(CreateUnnamedSingleProcessSemaphore_t, const unsigned int value) noexcept
    : m_isNamedSemaphore(false)
{
    if (init(&m_handle, 0, value))
    {
        m_isInitialized = true;
    }
    else
    {
        m_isInitialized = false;
        m_errorValue = SemaphoreError::CREATION_FAILED;
    }
}

Semaphore::Semaphore(CreateUnnamedSharedMemorySemaphore_t, const unsigned int value) noexcept
    : m_isNamedSemaphore(false)
{
    if (init(&m_handle, 1, value))
    {
        m_isInitialized = true;
    }
    else
    {
        m_isInitialized = false;
        m_errorValue = SemaphoreError::CREATION_FAILED;
    }
}

Semaphore::Semaphore(CreateUnnamedSharedMemorySemaphore_t, iox_sem_t* handle, const unsigned int value) noexcept
    : m_isNamedSemaphore(false)
    , m_isShared(true)
    , m_handlePtr(handle)
{
    if (init(handle, 1, value))
    {
        m_isInitialized = true;
    }
    else
    {
        m_isInitialized = false;
        m_errorValue = SemaphoreError::CREATION_FAILED;
    }
}

Semaphore::Semaphore(OpenNamedSemaphore_t, const char* name, const int oflag) noexcept
    : m_isCreated(false)
{
    if (m_name.unsafe_assign(name) == false)
    {
        m_isInitialized = false;
        m_errorValue = SemaphoreError::NAME_TOO_LONG;
        return;
    }

    if (open(oflag))
    {
        m_isInitialized = true;
    }
    else
    {
        m_errorValue = SemaphoreError::UNABLE_TO_OPEN_HANDLE;
        m_isInitialized = false;
    }
}

Semaphore::Semaphore(CreateNamedSemaphore_t, const char* name, const mode_t mode, const unsigned int value) noexcept
{
    if (m_name.unsafe_assign(name) == false)
    {
        m_isInitialized = false;
        m_errorValue = SemaphoreError::NAME_TOO_LONG;
        return;
    }

    if (open(O_CREAT | O_EXCL, mode, value))
    {
        m_isInitialized = true;
    }
    else
    {
        m_errorValue = SemaphoreError::CREATION_FAILED;
        m_isInitialized = false;
    }
}

bool Semaphore::close() noexcept
{
    return !cxx::makeSmartC(iox_sem_close, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_handlePtr).hasErrors();
}

bool Semaphore::destroy() noexcept
{
    return !cxx::makeSmartC(iox_sem_destroy, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, &m_handle).hasErrors();
}

bool Semaphore::init(iox_sem_t* handle, const int pshared, const unsigned int value) noexcept
{
    return !cxx::makeSmartC(iox_sem_init, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, handle, pshared, value)
                .hasErrors();
}

bool Semaphore::open(const int oflag) noexcept
{
    bool success = setHandleFromCall(cxx::makeSmartC(iox_sem_open<>,
                                                     cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                                     {reinterpret_cast<iox_sem_t*>(SEM_FAILED)},
                                                     {},
                                                     m_name.c_str(),
                                                     oflag));

    if (!success)
    {
        m_errorValue = SemaphoreError::CREATION_FAILED;
    }
    return success;
}

bool Semaphore::open(const int oflag, const mode_t mode, const unsigned int value) noexcept
{
    bool success = setHandleFromCall(cxx::makeSmartC(iox_sem_open<mode_t, unsigned int>,
                                                     cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                                     {reinterpret_cast<iox_sem_t*>(SEM_FAILED)},
                                                     {},
                                                     m_name.c_str(),
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
    return !cxx::makeSmartC(iox_sem_unlink, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, name).hasErrors();
}

bool Semaphore::isNamedSemaphore() noexcept
{
    return m_isNamedSemaphore;
}

SemaphoreError Semaphore::errnoToEnum(const int errnoValue) const noexcept
{
    switch (errnoValue)
    {
    case EINVAL:
        std::cerr << "semaphore object is in an inconsistent state" << std::endl;
        return SemaphoreError::INVALID_SEMAPHORE_HANDLE;
    case EOVERFLOW:
        std::cerr << "semaphore is overflowing" << std::endl;
        return SemaphoreError::SEMAPHORE_OVERFLOW;
    case EINTR:
        std::cerr << "call was interrupted by signal handler" << std::endl;
        return SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER;
    default:
        std::cerr << "an unexpected error occurred in semaphore - this should never happen! errno: "
                  << strerror(errnoValue) << std::endl;
        return SemaphoreError::UNDEFINED;
    }
}

} // namespace posix
} // namespace iox
