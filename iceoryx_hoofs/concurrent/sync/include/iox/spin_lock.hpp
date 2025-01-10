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

#ifndef IOX_HOOFS_CONCURRENT_SYNC_SPIN_LOCK_HPP
#define IOX_HOOFS_CONCURRENT_SYNC_SPIN_LOCK_HPP

#include "iceoryx_platform/unistd.hpp"
#include "iox/atomic.hpp"
#include "iox/lock_interface.hpp"

#include <thread>

namespace iox
{
namespace concurrent
{
class SpinLockBuilder;

/// @brief A spin lock implementation as drop-in replacement for a mutex
class SpinLock : public LockInterface<SpinLock>
{
  public:
    using Builder = SpinLockBuilder;

    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;

    ~SpinLock() noexcept = default;

  private:
    friend class optional<SpinLock>;
    friend class LockInterface<SpinLock>;

    explicit SpinLock(const LockBehavior lock_behavior) noexcept;

    expected<void, LockError> lock_impl() noexcept;

    expected<void, UnlockError> unlock_impl() noexcept;

    expected<TryLock, TryLockError> try_lock_impl() noexcept;

    struct LockInfo
    {
        pid_t tid;
        uint32_t recursive_count;
    };

  private:
    concurrent::AtomicFlag m_lock_flag =
        ATOMIC_FLAG_INIT; // NOTE: only initialization via assignment is guaranteed to work
    const concurrent::Atomic<bool> m_recursive{false};
    concurrent::Atomic<pid_t> m_pid{0};
    concurrent::Atomic<uint64_t> m_recursive_count{0};
    concurrent::Atomic<std::thread::id> m_tid{std::thread::id()};
};

class SpinLockBuilder
{
  public:
    enum class Error : uint8_t
    {
        LOCK_ALREADY_INITIALIZED,
        INTER_PROCESS_LOCK_UNSUPPORTED_BY_PLATFORM,
        UNKNOWN_ERROR
    };

    /// @brief Defines if the SpinLock should be usable in an inter process context. Default: true
    IOX_BUILDER_PARAMETER(bool, is_inter_process_capable, true)

    /// @brief Sets the LockBehavior, default: LockBehavior::RECURSIVE
    IOX_BUILDER_PARAMETER(LockBehavior, lock_behavior, LockBehavior::RECURSIVE)

  public:
    /// @brief Initializes a provided uninitialized SpinLock
    /// @param[in] uninitializedLock the uninitialized SpinLock which should be initialized
    /// @return On failure LockCreationError which explains the error
    expected<void, Error> create(optional<SpinLock>& uninitializedLock) noexcept;
};

} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_SYNC_SPIN_LOCK_HPP
