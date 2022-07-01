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
#ifndef IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP
#define IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/design_pattern/builder.hpp"
#include "iceoryx_hoofs/platform/pthread.hpp"

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
/// @attention Errors in c'tor or d'tor can lead to a program termination!
///
class Mutex
{
  public:
    /// @attention the construction of the mutex can fail. This can lead to a program termination!
    /// @todo iox-#1036 remove this, introduced to keep current API temporarily
    explicit Mutex(const bool f_isRecursive) noexcept;

    ~Mutex() noexcept;

    /// @brief all copy and move assignment methods need to be deleted otherwise
    ///         undefined behavior or race conditions will occure if you copy
    ///         or move mutexe when its possible that they are locked or will
    ///         be locked
    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex& operator=(Mutex&&) = delete;

    /// @brief Locks the mutex object and returns true if the underlying c
    ///         function did not returned any error. If the mutex is already
    ///         locked the method is blocking till the mutex can be locked.
    cxx::expected<MutexError> lock() noexcept;
    /// @brief Unlocks the mutex object and returns true if the underlying c
    ///         function did not returned any error.
    ///        IMPORTANT! Unlocking and unlocked mutex is undefined behavior
    ///         and the underlying c function will report success in this case!
    cxx::expected<MutexError> unlock() noexcept;

    /// @brief  Tries to lock the mutex object. If it is not possible to lock
    ///         the mutex object try_lock will return an error. If the c
    ///         function fails it will return false, otherwise true.
    cxx::expected<MutexTryLock, MutexError> try_lock() noexcept;

    /// @brief  Returns the native handle which then can be used in
    ///         pthread_mutex_** calls. Required when a pthread_mutex_**
    ///         call is not abstracted with this wrapper.
    const pthread_mutex_t& get_native_handle() const noexcept;

    pthread_mutex_t& get_native_handle() noexcept;

  private:
    Mutex() noexcept = default;

  private:
    friend class MutexBuilder;
    friend class cxx::optional<Mutex>;

    pthread_mutex_t m_handle;
    bool m_isDescructable = true;
};

/// @todo iox-#1036 remove this, introduced to keep current API temporarily
using mutex = Mutex;

enum class MutexType : int32_t
{
    NORMAL = PTHREAD_MUTEX_NORMAL,
    RECURSIVE = PTHREAD_MUTEX_RECURSIVE,
    WITH_DEADLOCK_DETECTION = PTHREAD_MUTEX_ERRORCHECK,
    PLATFORM_DEFAULT = PTHREAD_MUTEX_DEFAULT
};

enum class MutexPriorityInheritance : int32_t
{
    NONE = PTHREAD_PRIO_NONE,
    INHERIT = PTHREAD_PRIO_INHERIT,
    PROTECT = PTHREAD_PRIO_PROTECT
};

enum class MutexThreadTerminationBehavior : int32_t
{
    STALL_WHEN_LOCKED = PTHREAD_MUTEX_STALLED,
    RELEASE_WHEN_LOCKED = PTHREAD_MUTEX_ROBUST,
};

class MutexBuilder
{
    IOX_BUILDER_PARAMETER(bool, isInterProcessCapable, true)
    IOX_BUILDER_PARAMETER(MutexType, mutexType, MutexType::RECURSIVE)
    IOX_BUILDER_PARAMETER(MutexPriorityInheritance, priorityInheritance, MutexPriorityInheritance::NONE)
    IOX_BUILDER_PARAMETER(MutexThreadTerminationBehavior,
                          threadTerminationBehavior,
                          MutexThreadTerminationBehavior::RELEASE_WHEN_LOCKED)

  public:
    cxx::expected<MutexError> create(cxx::optional<Mutex>& uninitializedMutex) noexcept;
};
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP
