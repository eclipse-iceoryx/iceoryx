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
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/unnamed_semaphore.hpp"

namespace iox
{
namespace posix
{
namespace internal
{

template <typename SemaphoreChild>
iox_sem_t* SemaphoreInterface<SemaphoreChild>::getHandle() noexcept
{
    return static_cast<SemaphoreChild*>(this)->getHandle();
}

template <typename SemaphoreChild>
void SemaphoreInterface<SemaphoreChild>::post() noexcept
{
    postUnsafe().expect("Fatal semaphore failure occurred.");
}

template <typename SemaphoreChild>
cxx::expected<SemaphoreError> SemaphoreInterface<SemaphoreChild>::postUnsafe() noexcept
{
    auto result = posixCall(iox_sem_post)(getHandle()).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EINVAL:
            LogError() << "The semaphore handle is no longer valid. This can indicate a corrupted system.";
            return cxx::error<SemaphoreError>(SemaphoreError::INVALID_SEMAPHORE_HANDLE);
        case EOVERFLOW:
            LogError() << "Semaphore overflow.";
            return cxx::error<SemaphoreError>(SemaphoreError::SEMAPHORE_OVERFLOW);
        default:
            LogError() << "This should never happen. An unknown error occurred.";
            break;
        }
        return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
    }

    return cxx::success<>();
}

template <typename SemaphoreChild>
SemaphoreState SemaphoreInterface<SemaphoreChild>::getState() noexcept
{
    return getStateUnsafe().expect("Fatal semaphore failure occurred.");
}

template <typename SemaphoreChild>
cxx::expected<SemaphoreState, SemaphoreError> SemaphoreInterface<SemaphoreChild>::getStateUnsafe() noexcept
{
    int value = 0;
    auto result = posixCall(iox_sem_getvalue)(getHandle(), &value).failureReturnValue(-1).evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EINVAL:
            LogError() << "The semaphore handle is no longer valid. This can indicate a corrupted system.";
            return cxx::error<SemaphoreError>(SemaphoreError::INVALID_SEMAPHORE_HANDLE);
        default:
            LogError() << "This should never happen. An unknown error occurred.";
            break;
        }
        return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
    }

    SemaphoreState state;
    state.value = (value > 0) ? static_cast<uint32_t>(value) : 0U;
    state.numberOfBlockedWait = (value < 0) ? static_cast<uint32_t>(-value) : 0U;

    return cxx::success<SemaphoreState>(state);
}

template <typename SemaphoreChild>
SemaphoreWaitState SemaphoreInterface<SemaphoreChild>::timedWait(const units::Duration& timeout) noexcept
{
    return timedWaitUnsafe(timeout).expect("Fatal semaphore failure occurred.");
}

template <typename SemaphoreChild>
cxx::expected<SemaphoreWaitState, SemaphoreError>
SemaphoreInterface<SemaphoreChild>::timedWaitUnsafe(const units::Duration& timeout) noexcept
{
    const timespec timeoutAsTimespec = timeout.timespec(units::TimeSpecReference::Epoch);
    auto result = posixCall(iox_sem_timedwait)(getHandle(), &timeoutAsTimespec)
                      .failureReturnValue(-1)
                      .ignoreErrnos(ETIMEDOUT)
                      .evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case ETIMEDOUT:
            return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::TIMEOUT);
        case EINVAL:
            LogError() << "The semaphore handle is no longer valid. This can indicate a corrupted system.";
            return cxx::error<SemaphoreError>(SemaphoreError::INVALID_SEMAPHORE_HANDLE);
        case EINTR:
            LogError() << "The sem_timedwait was interrupted multiple times by the operating system. Abort operation!";
            return cxx::error<SemaphoreError>(SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER);
        default:
            LogError() << "This should never happen. An unknown error occurred.";
            break;
        }
        return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
    }

    return cxx::success<SemaphoreWaitState>(SemaphoreWaitState::NO_TIMEOUT);
}

template <typename SemaphoreChild>
bool SemaphoreInterface<SemaphoreChild>::tryWait() noexcept
{
    return tryWaitUnsafe().expect("Fatal semaphore failure occurred.");
}

template <typename SemaphoreChild>
cxx::expected<bool, SemaphoreError> SemaphoreInterface<SemaphoreChild>::tryWaitUnsafe() noexcept
{
    auto result = posixCall(iox_sem_trywait)(getHandle()).failureReturnValue(-1).ignoreErrnos(EAGAIN).evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EAGAIN:
            return cxx::success<bool>(false);
        case EINVAL:
            LogError() << "The semaphore handle is no longer valid. This can indicate a corrupted system.";
            return cxx::error<SemaphoreError>(SemaphoreError::INVALID_SEMAPHORE_HANDLE);
        case EINTR:
            LogError() << "The sem_trywait was interrupted multiple times by the operating system. Abort operation!";
            return cxx::error<SemaphoreError>(SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER);
        default:
            LogError() << "This should never happen. An unknown error occurred.";
            break;
        }
        return cxx::error<SemaphoreError>(SemaphoreError::UNDEFINED);
    }

    return cxx::success<bool>(true);
}

template class SemaphoreInterface<UnnamedSemaphore>;
} // namespace internal
} // namespace posix
} // namespace iox
