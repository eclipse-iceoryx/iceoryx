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

#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/platform/platform_correction.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace posix
{
Semaphore::Semaphore() noexcept = default;

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

        rhs.m_handlePtr = nullptr;
        rhs.m_isInitialized = false;
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
    int value{0};
    auto call = posixCall(iox_sem_getvalue)(getHandle(), &value).failureReturnValue(-1).evaluate();
    if (call.has_error())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.get_error().errnum));
    }

    return cxx::success<int>(value);
}

cxx::expected<SemaphoreError> Semaphore::post() noexcept
{
    auto call = posixCall(iox_sem_post)(getHandle()).failureReturnValue(-1).evaluate();
    if (call.has_error())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.get_error().errnum));
    }

    return cxx::success<>();
}

cxx::expected<SemaphoreWaitState, SemaphoreError> Semaphore::timedWait(const units::Duration abs_timeout) noexcept
{
    const struct timespec timeout = abs_timeout.timespec(units::TimeSpecReference::Epoch);
    auto call =
        posixCall(iox_sem_timedwait)(getHandle(), &timeout).failureReturnValue(-1).ignoreErrnos(ETIMEDOUT).evaluate();

    if (call.has_error())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.get_error().errnum));
    }
    else if (call->errnum == ETIMEDOUT)
    {
        return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::TIMEOUT);
    }
    else
    {
        return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::NO_TIMEOUT);
    }
}

cxx::expected<bool, SemaphoreError> Semaphore::tryWait() noexcept
{
    auto call = posixCall(iox_sem_trywait)(getHandle()).failureReturnValue(-1).ignoreErrnos(EAGAIN).evaluate();

    if (call.has_error())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.get_error().errnum));
    }

    return cxx::success<bool>(call->errnum != EAGAIN);
}

cxx::expected<SemaphoreError> Semaphore::wait() noexcept
{
    auto call = posixCall(iox_sem_wait)(getHandle()).failureReturnValue(-1).evaluate();

    if (call.has_error())
    {
        return cxx::error<SemaphoreError>(errnoToEnum(call.get_error().errnum));
    }

    return cxx::success<>();
}

iox_sem_t* Semaphore::getHandle() const noexcept
{
    return (isNamedSemaphore()) ? m_handlePtr : &m_handle;
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

Semaphore::Semaphore(OpenNamedSemaphore_t, const char* name, const int oflag) noexcept
    : m_isCreated(false)
{
    if (!m_name.unsafe_assign(name))
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

// NOLINTNEXTLINE(readability-function-size) todo(iox-#832): make a struct out of arguments
Semaphore::Semaphore(CreateNamedSemaphore_t, const char* name, const mode_t mode, const unsigned int value) noexcept
    : m_isCreated(true)
{
    if (!m_name.unsafe_assign(name))
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
    return !posixCall(iox_sem_close)(getHandle()).failureReturnValue(-1).evaluate().has_error();
}

bool Semaphore::destroy() noexcept
{
    return !posixCall(iox_sem_destroy)(getHandle()).failureReturnValue(-1).evaluate().has_error();
}

bool Semaphore::init(iox_sem_t* handle, const int pshared, const unsigned int value) noexcept
{
    return !posixCall(iox_sem_init)(handle, pshared, value).failureReturnValue(-1).evaluate().has_error();
}

bool Semaphore::open(const int oflag) noexcept
{
    return !posixCall(iox_sem_open<>)(m_name.c_str(), oflag)
                .failureReturnValue(reinterpret_cast<iox_sem_t*>(SEM_FAILED))
                .evaluate()
                .and_then([this](auto& r) { this->m_handlePtr = r.value; })
                .or_else([this](auto&) { this->m_errorValue = SemaphoreError::CREATION_FAILED; })
                .has_error();
}

bool Semaphore::open(const int oflag, const mode_t mode, const unsigned int value) noexcept
{
    auto iox_sem_open_call = iox_sem_open<mode_t, unsigned int>;
    return !posixCall(iox_sem_open_call)(m_name.c_str(), oflag, mode, value)
                .failureReturnValue(reinterpret_cast<iox_sem_t*>(SEM_FAILED))
                .evaluate()
                .and_then([this](auto& r) { this->m_handlePtr = r.value; })
                .or_else([this](auto&) { this->m_errorValue = SemaphoreError::CREATION_FAILED; })
                .has_error();
}

bool Semaphore::unlink(const char* name) noexcept
{
    return !posixCall(iox_sem_unlink)(name).failureReturnValue(-1).evaluate().has_error();
}

bool Semaphore::isNamedSemaphore() const noexcept
{
    return m_isNamedSemaphore;
}

SemaphoreError Semaphore::errnoToEnum(const int errnoValue) noexcept
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
                  // NOLINTNEXTLINE(concurrency-mt-unsafe) impossible case
                  << strerror(errnoValue) << std::endl;
        return SemaphoreError::UNDEFINED;
    }
}

} // namespace posix
} // namespace iox
