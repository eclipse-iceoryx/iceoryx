// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/semaphore_interface.hpp"
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/named_semaphore.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/unnamed_semaphore.hpp"

namespace iox
{
namespace posix
{
namespace internal
{
cxx::error<SemaphoreError> createErrorFromErrno(const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EINVAL:
        LogError() << "The semaphore handle is no longer valid. This can indicate a corrupted system.";
        return cxx::error<SemaphoreError>(SemaphoreError::INVALID_SEMAPHORE_HANDLE);
    case EOVERFLOW:
        LogError() << "Semaphore overflow. The maximum value of " << IOX_SEM_VALUE_MAX << " would be exceeded.";
        return cxx::error<SemaphoreError>(SemaphoreError::SEMAPHORE_OVERFLOW);
    case EINTR:
        LogError() << "The semaphore call was interrupted multiple times by the operating system. Abort operation!";
        return cxx::error<SemaphoreError>(SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER);
    default:
        LogError() << "This should never happen. An unknown error occurred.";
        break;
    }
    return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
}

template <typename SemaphoreChild>
iox_sem_t* SemaphoreInterface<SemaphoreChild>::getHandle() noexcept
{
    return static_cast<SemaphoreChild*>(this)->getHandle();
}

template <typename SemaphoreChild>
cxx::expected<SemaphoreError> SemaphoreInterface<SemaphoreChild>::post() noexcept
{
    auto result = posixCall(iox_sem_post)(getHandle()).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        return createErrorFromErrno(result.get_error().errnum);
    }

    return cxx::success<>();
}

template <typename SemaphoreChild>
cxx::expected<SemaphoreWaitState, SemaphoreError>
SemaphoreInterface<SemaphoreChild>::timedWait(const units::Duration& timeout) noexcept
{
    const timespec timeoutAsTimespec = timeout.timespec(units::TimeSpecReference::Epoch);
    auto result = posixCall(iox_sem_timedwait)(getHandle(), &timeoutAsTimespec)
                      .failureReturnValue(-1)
                      .ignoreErrnos(ETIMEDOUT)
                      .evaluate();

    if (result.has_error())
    {
        return createErrorFromErrno(result.get_error().errnum);
    }

    return cxx::success<SemaphoreWaitState>((result.value().errnum == ETIMEDOUT) ? SemaphoreWaitState::TIMEOUT
                                                                                 : SemaphoreWaitState::NO_TIMEOUT);
}

template <typename SemaphoreChild>
cxx::expected<bool, SemaphoreError> SemaphoreInterface<SemaphoreChild>::tryWait() noexcept
{
    auto result = posixCall(iox_sem_trywait)(getHandle()).failureReturnValue(-1).ignoreErrnos(EAGAIN).evaluate();

    if (result.has_error())
    {
        return createErrorFromErrno(result.get_error().errnum);
    }

    return cxx::success<bool>(result.value().errnum != EAGAIN);
}

template <typename SemaphoreChild>
cxx::expected<SemaphoreError> SemaphoreInterface<SemaphoreChild>::wait() noexcept
{
    auto result = posixCall(iox_sem_wait)(getHandle()).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        return createErrorFromErrno(result.get_error().errnum);
    }

    return cxx::success<>();
}

template class SemaphoreInterface<UnnamedSemaphore>;
template class SemaphoreInterface<NamedSemaphore>;
} // namespace internal
} // namespace posix
} // namespace iox
