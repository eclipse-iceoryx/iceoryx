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

#include "iox/mutex.hpp"
#include "iox/detail/posix_scheduler.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

#include "iceoryx_platform/platform_correction.hpp"

namespace iox
{
/// @brief Internal struct used during mutex construction to handle all the mutex attribute settings
struct MutexAttributes
{
  public:
    MutexAttributes() noexcept = default;
    MutexAttributes(const MutexAttributes&) = delete;
    MutexAttributes(MutexAttributes&&) = delete;
    MutexAttributes& operator=(const MutexAttributes&) = delete;
    MutexAttributes& operator=(MutexAttributes&&) = delete;

    ~MutexAttributes() noexcept
    {
        if (m_attributes)
        {
            auto destroyResult =
                IOX_POSIX_CALL(iox_pthread_mutexattr_destroy)(&*m_attributes).returnValueMatchesErrno().evaluate();
            if (destroyResult.has_error())
            {
                IOX_LOG(ERROR,
                        "This should never happen. An unknown error occurred while cleaning up the mutex attributes.");
            }
        }
    }

    expected<void, MutexCreationError> init() noexcept
    {
        m_attributes.emplace();
        auto result = IOX_POSIX_CALL(iox_pthread_mutexattr_init)(&*m_attributes).returnValueMatchesErrno().evaluate();
        if (result.has_error())
        {
            switch (result.error().errnum)
            {
            case ENOMEM:
                IOX_LOG(ERROR, "Not enough memory to initialize required mutex attributes");
                return err(MutexCreationError::INSUFFICIENT_MEMORY);
            default:
                IOX_LOG(ERROR,
                        "This should never happen. An unknown error occurred while initializing the mutex attributes.");
                return err(MutexCreationError::UNKNOWN_ERROR);
            }
        }

        return ok();
    }

    expected<void, MutexCreationError> enableIpcSupport(const bool enableIpcSupport) noexcept
    {
        auto result =
            IOX_POSIX_CALL(iox_pthread_mutexattr_setpshared)(
                &*m_attributes,
                static_cast<int>((enableIpcSupport) ? IOX_PTHREAD_PROCESS_SHARED : IOX_PTHREAD_PROCESS_PRIVATE))
                .returnValueMatchesErrno()
                .evaluate();
        if (result.has_error())
        {
            switch (result.error().errnum)
            {
            case ENOTSUP:
                IOX_LOG(ERROR, "The platform does not support shared mutex (inter process mutex)");
                return err(MutexCreationError::INTER_PROCESS_MUTEX_UNSUPPORTED_BY_PLATFORM);
            default:
                IOX_LOG(ERROR,
                        "This should never happen. An unknown error occurred while setting up the inter process "
                        "configuration.");
                return err(MutexCreationError::UNKNOWN_ERROR);
            }
        }

        return ok();
    }

    expected<void, MutexCreationError> setType(const MutexType mutexType) noexcept
    {
        auto result = IOX_POSIX_CALL(iox_pthread_mutexattr_settype)(&*m_attributes, static_cast<int>(mutexType))
                          .returnValueMatchesErrno()
                          .evaluate();
        if (result.has_error())
        {
            IOX_LOG(ERROR, "This should never happen. An unknown error occurred while setting up the mutex type.");
            return err(MutexCreationError::UNKNOWN_ERROR);
        }

        return ok();
    }

    expected<void, MutexCreationError> setProtocol(const MutexPriorityInheritance priorityInheritance)
    {
        auto result =
            IOX_POSIX_CALL(iox_pthread_mutexattr_setprotocol)(&*m_attributes, static_cast<int>(priorityInheritance))
                .returnValueMatchesErrno()
                .evaluate();
        if (result.has_error())
        {
            switch (result.error().errnum)
            {
            case ENOSYS:
                IOX_LOG(ERROR, "The system does not support mutex priorities");
                return err(MutexCreationError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
            case ENOTSUP:
                IOX_LOG(ERROR, "The used mutex priority is not supported by the platform");
                return err(MutexCreationError::USED_PRIORITY_UNSUPPORTED_BY_PLATFORM);
            case EPERM:
                IOX_LOG(ERROR, "Insufficient permissions to set mutex priorities");
                return err(MutexCreationError::PERMISSION_DENIED);
            default:
                IOX_LOG(ERROR,
                        "This should never happen. An unknown error occurred while setting up the mutex priority.");
                return err(MutexCreationError::UNKNOWN_ERROR);
            }
        }

        return ok();
    }

    expected<void, MutexCreationError> setPrioCeiling(const int32_t priorityCeiling) noexcept
    {
        auto result =
            IOX_POSIX_CALL(iox_pthread_mutexattr_setprioceiling)(&*m_attributes, static_cast<int>(priorityCeiling))
                .returnValueMatchesErrno()
                .evaluate();
        if (result.has_error())
        {
            switch (result.error().errnum)
            {
            case EPERM:
                IOX_LOG(ERROR, "Insufficient permissions to set the mutex priority ceiling.");
                return err(MutexCreationError::PERMISSION_DENIED);
            case ENOSYS:
                IOX_LOG(ERROR, "The platform does not support mutex priority ceiling.");
                return err(MutexCreationError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
            case EINVAL:
            {
                auto minimumPriority = detail::getSchedulerPriorityMinimum(detail::Scheduler::FIFO);
                auto maximumPriority = detail::getSchedulerPriorityMaximum(detail::Scheduler::FIFO);

                IOX_LOG(ERROR,
                        "The priority ceiling \"" << priorityCeiling << "\" is not in the valid priority range [ "
                                                  << minimumPriority << ", " << maximumPriority
                                                  << "] of the Scheduler::FIFO.");
                return err(MutexCreationError::INVALID_PRIORITY_CEILING_VALUE);
            }
            default:
                IOX_LOG(
                    ERROR,
                    "This should never happen. An unknown error occurred while setting up the mutex priority ceiling.");
                return err(MutexCreationError::UNKNOWN_ERROR);
            }
        }

        return ok();
    }

    expected<void, MutexCreationError>
    setThreadTerminationBehavior(const MutexThreadTerminationBehavior behavior) noexcept
    {
        auto result = IOX_POSIX_CALL(iox_pthread_mutexattr_setrobust)(&*m_attributes, static_cast<int>(behavior))
                          .returnValueMatchesErrno()
                          .evaluate();
        if (result.has_error())
        {
            IOX_LOG(ERROR,
                    "This should never happen. An unknown error occurred while setting up the mutex thread "
                    "termination behavior.");
            return err(MutexCreationError::UNKNOWN_ERROR);
        }

        return ok();
    }

    optional<iox_pthread_mutexattr_t> m_attributes;
};

expected<void, MutexCreationError> initializeMutex(iox_pthread_mutex_t* const handle,
                                                   const iox_pthread_mutexattr_t* const attributes) noexcept
{
    auto initResult = IOX_POSIX_CALL(iox_pthread_mutex_init)(handle, attributes).returnValueMatchesErrno().evaluate();
    if (initResult.has_error())
    {
        switch (initResult.error().errnum)
        {
        case EAGAIN:
            IOX_LOG(ERROR, "Not enough resources to initialize another mutex.");
            return err(MutexCreationError::INSUFFICIENT_RESOURCES);
        case ENOMEM:
            IOX_LOG(ERROR, "Not enough memory to initialize mutex.");
            return err(MutexCreationError::INSUFFICIENT_MEMORY);
        case EPERM:
            IOX_LOG(ERROR, "Insufficient permissions to create mutex.");
            return err(MutexCreationError::PERMISSION_DENIED);
        default:
            IOX_LOG(ERROR,
                    "This should never happen. An unknown error occurred while initializing the mutex handle. "
                    "This is possible when the handle is an already initialized mutex handle.");
            return err(MutexCreationError::UNKNOWN_ERROR);
        }
    }

    return ok();
}

expected<void, MutexCreationError> MutexBuilder::create(optional<mutex>& uninitializedMutex) noexcept
{
    if (uninitializedMutex.has_value())
    {
        IOX_LOG(ERROR, "Unable to override an already initialized mutex with a new mutex");
        return err(MutexCreationError::MUTEX_ALREADY_INITIALIZED);
    }

    MutexAttributes mutexAttributes;

    auto result = mutexAttributes.init();
    if (result.has_error())
    {
        return result;
    }

    result = mutexAttributes.enableIpcSupport(m_isInterProcessCapable);
    if (result.has_error())
    {
        return result;
    }

    result = mutexAttributes.setType(m_mutexType);
    if (result.has_error())
    {
        return result;
    }

    result = mutexAttributes.setProtocol(m_priorityInheritance);
    if (result.has_error())
    {
        return result;
    }

    if (m_priorityInheritance == MutexPriorityInheritance::PROTECT && m_priorityCeiling.has_value())
    {
        result = mutexAttributes.setPrioCeiling(*m_priorityCeiling);
        if (result.has_error())
        {
            return result;
        }
    }

    result = mutexAttributes.setThreadTerminationBehavior(m_threadTerminationBehavior);
    if (result.has_error())
    {
        return result;
    }

    uninitializedMutex.emplace();
    uninitializedMutex->m_isDestructable = false;

    result = initializeMutex(&uninitializedMutex->m_handle, &*mutexAttributes.m_attributes);
    if (result.has_error())
    {
        uninitializedMutex.reset();
        return result;
    }

    uninitializedMutex->m_isDestructable = true;
    return ok();
}

mutex::~mutex() noexcept
{
    if (m_isDestructable)
    {
        auto destroyCall = IOX_POSIX_CALL(iox_pthread_mutex_destroy)(&m_handle).returnValueMatchesErrno().evaluate();

        if (destroyCall.has_error())
        {
            switch (destroyCall.error().errnum)
            {
            case EBUSY:
                IOX_LOG(ERROR,
                        "Tried to remove a locked mutex which failed. The mutex handle is now leaked and "
                        "cannot be removed anymore!");
                break;
            default:
                IOX_LOG(ERROR, "This should never happen. An unknown error occurred while cleaning up the mutex.");
                break;
            }
        }
    }
}

void mutex::make_consistent() noexcept
{
    if (this->m_hasInconsistentState)
    {
        IOX_POSIX_CALL(iox_pthread_mutex_consistent)
        (&m_handle)
            .returnValueMatchesErrno()
            .evaluate()
            .and_then([&](auto) { this->m_hasInconsistentState = false; })
            .or_else([](auto) {
                IOX_LOG(ERROR, "This should never happen. Unable to put robust mutex in a consistent state!");
            });
    }
}

expected<void, MutexLockError> mutex::lock() noexcept
{
    auto result = IOX_POSIX_CALL(iox_pthread_mutex_lock)(&m_handle).returnValueMatchesErrno().evaluate();
    if (result.has_error())
    {
        switch (result.error().errnum)
        {
        case EINVAL:
            IOX_LOG(ERROR,
                    "The mutex has the attribute MutexPriorityInheritance::PROTECT set and the calling threads "
                    "priority is greater than the mutex priority.");
            return err(MutexLockError::PRIORITY_MISMATCH);
        case EAGAIN:
            IOX_LOG(ERROR, "Maximum number of recursive locks exceeded.");
            return err(MutexLockError::MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED);
        case EDEADLK:
            IOX_LOG(ERROR, "Deadlock in mutex detected.");
            return err(MutexLockError::DEADLOCK_CONDITION);
        case EOWNERDEAD:
            IOX_LOG(ERROR,
                    "The thread/process which owned the mutex died. The mutex is now in an inconsistent state "
                    "and must be put into a consistent state again with Mutex::make_consistent()");
            this->m_hasInconsistentState = true;
            return err(MutexLockError::LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
        default:
            IOX_LOG(ERROR,
                    "This should never happen. An unknown error occurred while locking the mutex. "
                    "This can indicate a either corrupted or non-POSIX compliant system.");
            return err(MutexLockError::UNKNOWN_ERROR);
        }
    }
    return ok();
}

expected<void, MutexUnlockError> mutex::unlock() noexcept
{
    auto result = IOX_POSIX_CALL(iox_pthread_mutex_unlock)(&m_handle).returnValueMatchesErrno().evaluate();
    if (result.has_error())
    {
        switch (result.error().errnum)
        {
        case EPERM:
            IOX_LOG(ERROR,
                    "The mutex is not owned by the current thread. The mutex must be unlocked by the same "
                    "thread it was locked by.");
            return err(MutexUnlockError::NOT_OWNED_BY_THREAD);
        default:
            IOX_LOG(ERROR,
                    "This should never happen. An unknown error occurred while unlocking the mutex. "
                    "This can indicate a either corrupted or non-POSIX compliant system.");
            return err(MutexUnlockError::UNKNOWN_ERROR);
        }
    }

    return ok();
}

expected<MutexTryLock, MutexTryLockError> mutex::try_lock() noexcept
{
    auto result =
        IOX_POSIX_CALL(iox_pthread_mutex_trylock)(&m_handle).returnValueMatchesErrno().ignoreErrnos(EBUSY).evaluate();

    if (result.has_error())
    {
        switch (result.error().errnum)
        {
        case EAGAIN:
            IOX_LOG(ERROR, "Maximum number of recursive locks exceeded.");
            return err(MutexTryLockError::MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED);
        case EINVAL:
            IOX_LOG(ERROR,
                    "The mutex has the attribute MutexPriorityInheritance::PROTECT set and the calling threads "
                    "priority is greater than the mutex priority.");
            return err(MutexTryLockError::PRIORITY_MISMATCH);
        case EOWNERDEAD:
            IOX_LOG(ERROR,
                    "The thread/process which owned the mutex died. The mutex is now in an inconsistent state and must "
                    "be put into a consistent state again with Mutex::make_consistent()");
            this->m_hasInconsistentState = true;
            return err(MutexTryLockError::LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
        default:
            IOX_LOG(ERROR,
                    "This should never happen. An unknown error occurred while trying to lock the mutex. This can "
                    "indicate a either corrupted or non-POSIX compliant system.");
            return err(MutexTryLockError::UNKNOWN_ERROR);
        }
    }

    return (result->errnum == EBUSY) ? ok(MutexTryLock::FAILED_TO_ACQUIRE_LOCK) : ok(MutexTryLock::LOCK_SUCCEEDED);
}
} // namespace iox
