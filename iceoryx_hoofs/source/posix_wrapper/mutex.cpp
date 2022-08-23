// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/scheduler.hpp"

#include "iceoryx_hoofs/platform/platform_correction.hpp"
namespace iox
{
namespace posix
{
cxx::expected<MutexError> MutexBuilder::create(cxx::optional<Mutex>& uninitializedMutex) noexcept
{
    pthread_mutexattr_t mutexAttributes;
    auto result = posixCall(pthread_mutexattr_init)(&mutexAttributes).returnValueMatchesErrno().evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case ENOMEM:
            LogError() << "Not enough memory to initialize required mutex attributes";
            return cxx::error<MutexError>(MutexError::INSUFFICIENT_MEMORY);
        default:
            LogError()
                << "This should never happen. An unknown error occurred while initializing the mutex attributes.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }


    cxx::GenericRAII mutexAttributesCleanup([&] {
        auto destroyResult =
            posixCall(pthread_mutexattr_destroy)(&mutexAttributes).returnValueMatchesErrno().evaluate();
        if (destroyResult.has_error())
        {
            LogError() << "This should never happen. An unknown error occurred while cleaning up the mutex attributes.";
        }
    });


    result = posixCall(pthread_mutexattr_setpshared)(
                 &mutexAttributes,
                 static_cast<int>((m_isInterProcessCapable) ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case ENOTSUP:
            LogError() << "The platform does not support shared mutex (inter process mutex)";
            return cxx::error<MutexError>(MutexError::INTER_PROCESS_MUTEX_UNSUPPORTED_BY_PLATFORM);
        default:
            LogError() << "This should never happen. An unknown error occurred while setting up the inter process "
                          "configuration.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }


    result = posixCall(pthread_mutexattr_settype)(&mutexAttributes, static_cast<int>(m_mutexType))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        LogError() << "This should never happen. An unknown error occurred while setting up the mutex type.";
        return cxx::error<MutexError>(MutexError::UNDEFINED);
    }


    result = posixCall(pthread_mutexattr_setprotocol)(&mutexAttributes, static_cast<int>(m_priorityInheritance))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case ENOSYS:
            LogError() << "The system does not support mutex priorities";
            return cxx::error<MutexError>(MutexError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
        case ENOTSUP:
            LogError() << "The used mutex priority is not supported by the platform";
            return cxx::error<MutexError>(MutexError::USED_PRIORITY_UNSUPPORTED_BY_PLATFORM);
        case EPERM:
            LogError() << "Unsufficient permissions to set mutex priorities";
            return cxx::error<MutexError>(MutexError::PERMISSION_DENIED);
        default:
            LogError() << "This should never happen. An unknown error occurred while setting up the mutex priority.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }

    if (m_priorityInheritance == MutexPriorityInheritance::PROTECT)
    {
        result = posixCall(pthread_mutexattr_setprioceiling)(&mutexAttributes, static_cast<int>(m_priorityCeiling))
                     .returnValueMatchesErrno()
                     .evaluate();
        if (result.has_error())
        {
            switch (result.get_error().errnum)
            {
            case EPERM:
                LogError() << "Unsufficient permissions to set the mutex priority ceiling.";
                return cxx::error<MutexError>(MutexError::PERMISSION_DENIED);
            case ENOSYS:
                LogError() << "The platform does not support mutex priority ceiling.";
                return cxx::error<MutexError>(MutexError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
            case EINVAL:
            {
                auto minimumPriority = getSchedulerPriorityMinimum(Scheduler::FIFO);
                auto maximumPriority = getSchedulerPriorityMaximum(Scheduler::FIFO);

                LogError() << "The priority ceiling \"" << m_priorityCeiling
                           << "\" is not in the valid priority range [ " << minimumPriority << ", " << maximumPriority
                           << "] of the Scheduler::FIFO.";
                return cxx::error<MutexError>(MutexError::INVALID_PRIORITY_CEILING_VALUE);
            }
            }
        }
    }


    result = posixCall(pthread_mutexattr_setrobust)(&mutexAttributes, static_cast<int>(m_threadTerminationBehavior))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        LogError() << "This should never happen. An unknown error occurred while setting up the mutex thread "
                      "termination behavior.";
        return cxx::error<MutexError>(MutexError::UNDEFINED);
    }


    uninitializedMutex.emplace();
    result = posixCall(pthread_mutex_init)(&uninitializedMutex->m_handle, &mutexAttributes)
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        uninitializedMutex->m_isDescructable = false;
        uninitializedMutex.reset();

        switch (result.get_error().errnum)
        {
        case EAGAIN:
            LogError() << "Not enough resources to initialize another mutex.";
            return cxx::error<MutexError>(MutexError::INSUFFICIENT_RESOURCES);
        case ENOMEM:
            LogError() << "Not enough memory to initialize mutex.";
            return cxx::error<MutexError>(MutexError::INSUFFICIENT_MEMORY);
        case EPERM:
            LogError() << "Unsufficient permissions to create mutex.";
            return cxx::error<MutexError>(MutexError::PERMISSION_DENIED);
        default:
            LogError() << "This should never happen. An unknown error occurred while initializing the mutex handle. "
                          "This is possible when the handle is an already initialized mutex handle.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }

    uninitializedMutex->m_isDescructable = true;
    return cxx::success<>();
}

/// @todo iox-#1036 remove this, introduced to keep current API temporarily
Mutex::Mutex(bool f_isRecursive) noexcept
{
    pthread_mutexattr_t attr;
    bool isInitialized{true};
    isInitialized &= !posixCall(pthread_mutexattr_init)(&attr).returnValueMatchesErrno().evaluate().has_error();
    isInitialized &= !posixCall(pthread_mutexattr_setpshared)(&attr, PTHREAD_PROCESS_SHARED)
                          .returnValueMatchesErrno()
                          .evaluate()
                          .has_error();
    isInitialized &=
        !posixCall(pthread_mutexattr_settype)(&attr, f_isRecursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL)
             .returnValueMatchesErrno()
             .evaluate()
             .has_error();
    isInitialized &= !posixCall(pthread_mutexattr_setprotocol)(&attr, PTHREAD_PRIO_NONE)
                          .returnValueMatchesErrno()
                          .evaluate()
                          .has_error();
    isInitialized &= !posixCall(pthread_mutex_init)(&m_handle, &attr).returnValueMatchesErrno().evaluate().has_error();
    isInitialized &= !posixCall(pthread_mutexattr_destroy)(&attr).returnValueMatchesErrno().evaluate().has_error();

    /// NOLINTJUSTIFICATION is fixed in the PR iox-#1443
    /// NOLINTNEXTLINE(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    cxx::Ensures(isInitialized && "Unable to create mutex");
}

Mutex::~Mutex() noexcept
{
    if (m_isDescructable)
    {
        auto destroyCall = posixCall(pthread_mutex_destroy)(&m_handle).returnValueMatchesErrno().evaluate();

        if (destroyCall.has_error())
        {
            switch (destroyCall.get_error().errnum)
            {
            case EBUSY:
                LogError() << "Tried to remove a locked mutex which failed. The mutex handle is now leaked and "
                              "cannot be removed anymore!";
                break;
            default:
                LogError() << "This should never happen. An unknown error occurred while cleaning up the mutex.";
                break;
            }
        }
    }
}

void Mutex::make_consistent() noexcept
{
    if (this->m_hasInconsistentState)
    {
        posixCall(pthread_mutex_consistent)(&m_handle).returnValueMatchesErrno().evaluate().or_else(
            [](auto) { LogError() << "This should never happen. Unable to put robust mutex in a consistent state!"; });
        this->m_hasInconsistentState = false;
    }
}

cxx::expected<MutexError> Mutex::lock() noexcept
{
    auto result =
        posixCall(pthread_mutex_lock)(&m_handle).returnValueMatchesErrno().ignoreErrnos(EOWNERDEAD).evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EINVAL:
            LogError() << "The mutex has the attribute MutexPriorityInheritance::PROTECT set and the calling threads "
                          "priority is greater than the mutex priority.";
            return cxx::error<MutexError>(MutexError::PRIORITY_MISMATCH);
        case EAGAIN:
            LogError() << "Maximum number of recursive locks exceeded.";
            return cxx::error<MutexError>(MutexError::MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED);
        case EDEADLK:
            LogError() << "Deadlock in mutex detected.";
            return cxx::error<MutexError>(MutexError::DEADLOCK_CONDITION);
        case EOWNERDEAD:
            LogError() << "The thread/process which owned the mutex died. The mutex is now in an inconsistent state "
                          "and must be put into a consistent state again with Mutex::make_consistent()";
            this->m_hasInconsistentState = true;
            return cxx::error<MutexError>(MutexError::HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
        default:
            LogError() << "This should never happen. An unknown error occurred while locking the mutex. "
                          "This can indicate a either corrupted or non-posix compliant system.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }
    return cxx::success<>();
}

cxx::expected<MutexError> Mutex::unlock() noexcept
{
    auto result = posixCall(pthread_mutex_unlock)(&m_handle).returnValueMatchesErrno().evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EPERM:
            LogError() << "The mutex is not owned by the current thread. The mutex must be unlocked by the same "
                          "thread it was locked by.";
            return cxx::error<MutexError>(MutexError::NOT_OWNED_BY_THREAD);
        default:
            LogError() << "This should never happen. An unknown error occurred while unlocking the mutex. "
                          "This can indicate a either corrupted or non-posix compliant system.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }

    return cxx::success<>();
}

cxx::expected<MutexTryLock, MutexError> Mutex::try_lock() noexcept
{
    auto result = posixCall(pthread_mutex_trylock)(&m_handle)
                      .returnValueMatchesErrno()
                      .ignoreErrnos(EBUSY, EOWNERDEAD)
                      .evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EINVAL:
            LogError() << "The mutex has the attribute MutexPriorityInheritance::PROTECT set and the calling threads "
                          "priority is greater than the mutex priority.";
            return cxx::error<MutexError>(MutexError::PRIORITY_MISMATCH);
        case EOWNERDEAD:
            LogError() << "The thread/process which owned the mutex died. The mutex is now in an inconsistent state "
                          "and must be put into a consistent state again with Mutex::make_consistent()";
            this->m_hasInconsistentState = true;
            return cxx::error<MutexError>(MutexError::HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
        default:
            LogError() << "This should never happen. An unknown error occurred while try locking the mutex. "
                          "This can indicate a either corrupted or non-posix compliant system.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }

    return (result->errnum == EBUSY) ? cxx::success<MutexTryLock>(MutexTryLock::FAILED_TO_ACQUIRE_LOCK)
                                     : cxx::success<MutexTryLock>(MutexTryLock::LOCK_SUCCEEDED);
}
} // namespace posix
} // namespace iox
