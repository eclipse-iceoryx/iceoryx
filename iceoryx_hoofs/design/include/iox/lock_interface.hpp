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

#ifndef IOX_HOOFS_DESIGN_LOCK_INTERFACE_HPP
#define IOX_HOOFS_DESIGN_LOCK_INTERFACE_HPP

#include "iceoryx_platform/pthread.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"

#include <cstdint>

namespace iox
{
enum class LockError : uint8_t
{
    PRIORITY_MISMATCH,
    MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED,
    DEADLOCK_CONDITION,
    LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED,
    UNKNOWN_ERROR
};

enum class UnlockError : uint8_t
{
    NOT_OWNED_BY_THREAD,
    NOT_LOCKED,
    UNKNOWN_ERROR
};

enum class TryLockError : uint8_t
{
    PRIORITY_MISMATCH,
    MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED,
    LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED,
    UNKNOWN_ERROR
};

enum class TryLock : uint8_t
{
    LOCK_SUCCEEDED,
    FAILED_TO_ACQUIRE_LOCK
};

template <typename Lock>
class LockInterface
{
  public:
    /// @brief Engages the lock.
    /// @return When it fails it returns an enum describing the error.
    expected<void, LockError> lock() noexcept
    {
        return static_cast<Lock*>(this)->lock_impl();
    }

    /// @brief  Releases the lock.
    /// @return When it fails it returns an enum describing the error.
    expected<void, UnlockError> unlock() noexcept
    {
        return static_cast<Lock*>(this)->unlock_impl();
    }

    /// @brief Tries to engage the lock.
    /// @return If the lock was acquired LockInterfaceTryLock::LOCK_SUCCEEDED will be returned otherwise
    ///         LockInterfaceTryLock::FAILED_TO_ACQUIRE_LOCK.
    ///         If the lock is a recursive lock, this call will also succeed.
    ///         On failure it returns an enum describing the failure.
    expected<TryLock, TryLockError> try_lock() noexcept
    {
        return static_cast<Lock*>(this)->try_lock_impl();
    }

  protected:
    LockInterface() noexcept = default;
};

/// @brief Describes the behavior of the lock.
// NOLINTNEXTLINE(performance-enum-size) int32_t required for POSIX API
enum class LockBehavior : int32_t
{
    /// @brief Behavior without error detection and multiple locks from within
    ///        the same thread lead to deadlock
    NORMAL = IOX_PTHREAD_MUTEX_NORMAL,

    /// @brief Multiple locks from within the same thread do not lead to deadlock
    ///        but one requires the same amount of unlocks to make the thread lockable
    ///        from other threads
    RECURSIVE = IOX_PTHREAD_MUTEX_RECURSIVE,

    /// @brief Multiple locks from within the same thread will be detected and
    ///        reported. It detects also when unlock is called from a different
    ///        thread.
    WITH_DEADLOCK_DETECTION = IOX_PTHREAD_MUTEX_ERRORCHECK,
};

} // namespace iox

#endif // IOX_HOOFS_DESIGN_LOCK_INTERFACE_HPP
