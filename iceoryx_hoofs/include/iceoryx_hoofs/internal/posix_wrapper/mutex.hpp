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
#ifndef IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP
#define IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/design_pattern/builder.hpp"
#include "iceoryx_hoofs/platform/pthread.hpp"
#include <pthread.h>

namespace iox
{
namespace posix
{
enum class MutexError
{
    INSUFFICIENT_MEMORY,
    INSUFFICIENT_RESOURCES,
    PERMISSION_DENIED,
    INTER_PROCESS_MUTEX_UNSUPPORTED_BY_PLATFORM,
    PRIORITIES_UNSUPPORTED_BY_PLATFORM,
    USED_PRIORITY_UNSUPPORTED_BY_PLATFORM,
    PRIORITY_MISMATCH,
    MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED,
    DEADLOCK_CONDITION,
    NOT_OWNED_BY_THREAD,
    INVALID_PRIORITY_CEILING_VALUE,
    HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED,
    UNDEFINED
};

enum class MutexTryLock
{
    LOCK_SUCCEEDED,
    FAILED_TO_ACQUIRE_LOCK
};

/// @brief Wrapper for a interprocess pthread based mutex which does not use
///         exceptions!
/// @code
///     #include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
///
///     int main() {
///         cxx::optional<iox::posix::Mutex> myMutex;
///         iox::posix::MutexBuilder().isInterProcessCapable(true)
///                                   .mutexType(MutexType::RECURSIVE)
///                                   .priorityInheritance(MutexPriorityInheritance::NONE)
///                                   .threadTerminationBehavior(MutexThreadTerminationBehavior::RELEASE_WHEN_LOCKED)
///                                   .create(myMutex)
///                                   .expect("Failed to create mutex!");
///
///         myMutex->lock().expect("Mutex lock failed. Maybe the system is corrupted.");
///         // ... do stuff
///         myMutex->unlock().expect("Mutex unlock failed. Maybe the system is corrupted.");
///
///         {
///             std::lock_guard<posix::mutex> lock(*myMutex);
///             // ...
///         }
///
///     }
/// @endcode
class Mutex
{
  public:
    /// @attention the construction of the mutex can fail. This can lead to a program termination!
    /// @todo iox-#1036 remove this, introduced to keep current API temporarily
    explicit Mutex(const bool f_isRecursive) noexcept;

    /// @brief Destroys the mutex. When the mutex is still locked this will fail and the
    ///        mutex is leaked! If the MutexThreadTerminationBehavior is set to RELEASE_WHEN_LOCKED
    ///        a locked mutex is unlocked and the handle is cleaned up correctly.
    ~Mutex() noexcept;

    /// @brief all copy and move assignment methods need to be deleted otherwise
    ///         undefined behavior or race conditions will occure if you copy
    ///         or move mutexe when its possible that they are locked or will
    ///         be locked
    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex& operator=(Mutex&&) = delete;

    /// @brief Locks the mutex.
    /// @return When it fails it returns a MutexError describing the error.
    ///         If the mutex is already locked it:
    ///          * will be non blocking when the lock call comes from the same thread and the
    ///              mutex type is MutexType::RECURSIVE
    ///          * will be blocking when for all non MutexType::RECURSIVE
    ///          * will return a MutexError::DEADLOCK_CONDITION when it is a
    ///              MutexType::WITH_DEADLOCK_DETECTION

    cxx::expected<MutexError> lock() noexcept;

    /// @brief  Unlocks the mutex.
    /// @return When it fails it returns a MutexError describing the error.
    ///         If the mutex was locked by another thread and
    ///         * MutexType::WITH_DEADLOCK_DETECTION it will return MutexError::NOT_OWNED_BY_THREAD
    ///         * is not MutexType::WITH_DEADLOCK_DETECTION it may fail (depending on the platform)
    ///         If the mutex was not locked it returns MutexType::NOT_OWNED_BY_THREAD
    cxx::expected<MutexError> unlock() noexcept;

    /// @brief Tries to lock the mutex.
    /// @return If the lock was acquired MutexTryLock::LOCK_SUCCEEDED will be returned otherwise
    ///         MutexTryLock::FAILED_TO_ACQUIRE_LOCK.
    ///         If the lock is of MutexType::RECURSIVE the lock will also succeed.
    ///         On failure it returns MutexError describing the failure.
    cxx::expected<MutexTryLock, MutexError> try_lock() noexcept;

    /// @brief When a mutex owning thread/process with MutexThreadTerminationBehavior::RELEASE_WHEN_LOCKED dies then the
    ///        next instance which would like to acquire the lock will get an
    ///        MutexError::HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED error. This method puts the mutex again into a
    ///        consistent state. If the mutex is already in a consistent state it will do nothing.
    void make_consistent() noexcept;

  private:
    Mutex() noexcept = default;

  private:
    friend class MutexBuilder;
    friend class cxx::optional<Mutex>;

    pthread_mutex_t m_handle = PTHREAD_MUTEX_INITIALIZER;
    bool m_isDescructable = true;
    bool m_hasInconsistentState = false;
};

/// @todo iox-#1036 remove this, introduced to keep current API temporarily
using mutex = Mutex;

/// @brief Describes the type of mutex.
enum class MutexType : int32_t
{
    /// @brief Behavior without error detection and multiple locks from within
    ///        the same thread lead to deadlock
    NORMAL = PTHREAD_MUTEX_NORMAL,

    /// @brief Multiple locks from within the same thread do not lead to deadlock
    ///        but one requires the same amount of unlocks to make the thread lockable
    ///        from other threads
    RECURSIVE = PTHREAD_MUTEX_RECURSIVE,

    /// @brief Multiple locks from within the same thread will be detected and
    ///        reported. It detects also when unlock is called from a different
    ///        thread.
    WITH_DEADLOCK_DETECTION = PTHREAD_MUTEX_ERRORCHECK,
};

/// @brief Describes how the priority of a mutex owning thread changes when another thread
///        with an higher priority would like to acquire the mutex.
enum class MutexPriorityInheritance : int32_t
{
    /// @brief No priority setting.
    NONE = PTHREAD_PRIO_NONE,

    /// @brief When a thread owns a mutex and another thread with a higher priority
    ///        tries to acquire that mutex the owning thread executes with the
    ///        priority of the highest waiting thread
    INHERIT = PTHREAD_PRIO_INHERIT,

    /// @brief When a thread owns a mutex it is executed with the priority ceiling
    ///        of the mutex when it is greater than the threads priority.
    PROTECT = PTHREAD_PRIO_PROTECT
};

/// @brief Defines the behavior when a mutex owning thread is terminated
enum class MutexThreadTerminationBehavior : int32_t
{
    /// @brief The mutex stays locked, is unlockable and no longer usable.
    ///        This can also lead to a mutex leak in the destructor.
    STALL_WHEN_LOCKED = PTHREAD_MUTEX_STALLED,

    /// @brief It implies the same behavior as MutexType::WITH_DEADLOCK_DETECTION. Additionally, when a mutex owning
    ///        thread/process dies the mutex is put into an inconsistent state which can be recovered with
    ///        Mutex::make_consistent(). The inconsistent state is detected by the next instance which calls
    ///        Mutex::lock() or Mutex::try_lock() by the error value MutexError::HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED
    RELEASE_WHEN_LOCKED = PTHREAD_MUTEX_ROBUST,
};

/// @brief Builder which creates a posix mutex
class MutexBuilder
{
    /// @brief Defines if the mutex should be usable in an inter process context. Default: true
    IOX_BUILDER_PARAMETER(bool, isInterProcessCapable, true)

    /// @brief Sets the MutexType, default: MutexType::RECURSIVE
    IOX_BUILDER_PARAMETER(MutexType, mutexType, MutexType::RECURSIVE)

    /// @brief States how thread priority is adjusted when they own the mutex, default: MutexPriorityInheritance::NONE
    IOX_BUILDER_PARAMETER(MutexPriorityInheritance, priorityInheritance, MutexPriorityInheritance::NONE)

    /// @brief Defines the maximum priority to which a thread which owns the thread can be promoted, default: 1
    IOX_BUILDER_PARAMETER(int32_t, priorityCeiling, 1)

    /// @brief Defines how a locked mutex behaves when the mutex owning thread terminates,
    ///        default: MutexThreadTerminationBehavior::RELEASE_WHEN_LOCKED
    IOX_BUILDER_PARAMETER(MutexThreadTerminationBehavior,
                          threadTerminationBehavior,
                          MutexThreadTerminationBehavior::RELEASE_WHEN_LOCKED)

  public:
    /// @brief Initializes a provided uninitialized mutex
    /// @param[in] uninitializedMutex the uninitialized mutex which should be initialized
    /// @return On failure MutexError which explains the error
    cxx::expected<MutexError> create(cxx::optional<Mutex>& uninitializedMutex) noexcept;
};
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP
