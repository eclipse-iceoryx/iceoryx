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

#include "iox/detail/semaphore_interface.hpp"
#include "iox/logging.hpp"
#include "iox/named_semaphore.hpp"
#include "iox/posix_call.hpp"
#include "iox/unnamed_semaphore.hpp"

namespace iox
{
namespace detail
{
SemaphoreError errnoToEnum(const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EINVAL:
        IOX_LOG(ERROR, "The semaphore handle is no longer valid. This can indicate a corrupted system.");
        return SemaphoreError::INVALID_SEMAPHORE_HANDLE;
    case EOVERFLOW:
        IOX_LOG(ERROR, "Semaphore overflow. The maximum value of " << IOX_SEM_VALUE_MAX << " would be exceeded.");
        return SemaphoreError::SEMAPHORE_OVERFLOW;
    case EINTR:
        IOX_LOG(ERROR, "The semaphore call was interrupted multiple times by the operating system. Abort operation!");
        return SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER;
    default:
        IOX_LOG(ERROR, "This should never happen. An unknown error occurred.");
        break;
    }
    return SemaphoreError::UNDEFINED;
}

template <typename SemaphoreChild>
iox_sem_t* SemaphoreInterface<SemaphoreChild>::getHandle() noexcept
{
    return static_cast<SemaphoreChild*>(this)->getHandle();
}

template <typename SemaphoreChild>
expected<void, SemaphoreError> SemaphoreInterface<SemaphoreChild>::post() noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_post)(getHandle()).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        return err(errnoToEnum(result.error().errnum));
    }

    return ok<void>();
}

template <typename SemaphoreChild>
expected<SemaphoreWaitState, SemaphoreError>
SemaphoreInterface<SemaphoreChild>::timedWait(const units::Duration& timeout) noexcept
{
    const timespec timeoutAsTimespec = timeout.timespec(units::TimeSpecReference::Epoch);
    auto result = IOX_POSIX_CALL(iox_sem_timedwait)(getHandle(), &timeoutAsTimespec)
                      .failureReturnValue(-1)
                      .ignoreErrnos(ETIMEDOUT)
                      .evaluate();

    if (result.has_error())
    {
        return err(errnoToEnum(result.error().errnum));
    }

    return ok((result.value().errnum == ETIMEDOUT) ? SemaphoreWaitState::TIMEOUT : SemaphoreWaitState::NO_TIMEOUT);
}

template <typename SemaphoreChild>
expected<bool, SemaphoreError> SemaphoreInterface<SemaphoreChild>::tryWait() noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_trywait)(getHandle()).failureReturnValue(-1).ignoreErrnos(EAGAIN).evaluate();

    if (result.has_error())
    {
        return err(errnoToEnum(result.error().errnum));
    }

    return ok(result.value().errnum != EAGAIN);
}

template <typename SemaphoreChild>
expected<void, SemaphoreError> SemaphoreInterface<SemaphoreChild>::wait() noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_wait)(getHandle()).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        return err(errnoToEnum(result.error().errnum));
    }

    return ok<void>();
}

template class SemaphoreInterface<UnnamedSemaphore>;
template class SemaphoreInterface<NamedSemaphore>;
} // namespace detail
} // namespace iox
