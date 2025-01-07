// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iox/detail/semaphore_helper.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
namespace detail
{
SemaphoreError sem_errno_to_enum(const int32_t errnum) noexcept
{
    switch (errnum)
    {
    case EINVAL:
        IOX_LOG(Error, "The semaphore handle is no longer valid. This can indicate a corrupted system.");
        return SemaphoreError::INVALID_SEMAPHORE_HANDLE;
    case EOVERFLOW:
        IOX_LOG(Error, "Semaphore overflow. The maximum value of " << IOX_SEM_VALUE_MAX << " would be exceeded.");
        return SemaphoreError::SEMAPHORE_OVERFLOW;
    case EINTR:
        IOX_LOG(Error, "The semaphore call was interrupted multiple times by the operating system. Abort operation!");
        return SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER;
    default:
        IOX_LOG(Error, "This should never happen. An unknown error occurred.");
        break;
    }
    return SemaphoreError::UNDEFINED;
}

expected<void, SemaphoreError> sem_post(iox_sem_t* handle) noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_post)(handle).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        return err(sem_errno_to_enum(result.error().errnum));
    }

    return ok<void>();
}

expected<SemaphoreWaitState, SemaphoreError> sem_timed_wait(iox_sem_t* handle, const units::Duration& timeout) noexcept
{
    const timespec timeoutAsTimespec = timeout.timespec(units::TimeSpecReference::Epoch);
    auto result = IOX_POSIX_CALL(iox_sem_timedwait)(handle, &timeoutAsTimespec)
                      .failureReturnValue(-1)
                      .ignoreErrnos(ETIMEDOUT)
                      .evaluate();

    if (result.has_error())
    {
        return err(sem_errno_to_enum(result.error().errnum));
    }

    return ok((result.value().errnum == ETIMEDOUT) ? SemaphoreWaitState::TIMEOUT : SemaphoreWaitState::NO_TIMEOUT);
}

expected<bool, SemaphoreError> sem_try_wait(iox_sem_t* handle) noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_trywait)(handle).failureReturnValue(-1).ignoreErrnos(EAGAIN).evaluate();

    if (result.has_error())
    {
        return err(sem_errno_to_enum(result.error().errnum));
    }

    return ok(result.value().errnum != EAGAIN);
}

expected<void, SemaphoreError> sem_wait(iox_sem_t* handle) noexcept
{
    auto result = IOX_POSIX_CALL(iox_sem_wait)(handle).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        return err(sem_errno_to_enum(result.error().errnum));
    }

    return ok<void>();
}

} // namespace detail
} // namespace iox
