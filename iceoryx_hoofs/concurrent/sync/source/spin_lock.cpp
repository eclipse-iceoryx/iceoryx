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

#include "iox/spin_lock.hpp"
#include "iox/detail/adaptive_wait.hpp"

namespace iox
{
namespace concurrent
{
expected<void, SpinLockBuilder::Error>
SpinLockBuilder::create(optional<concurrent::SpinLock>& uninitializedLock) noexcept
{
    if (uninitializedLock.has_value())
    {
        IOX_LOG(Error, "Unable to override an already initialized SpinLock with a new SpinLock");
        return err(Error::LOCK_ALREADY_INITIALIZED);
    }

    uninitializedLock.emplace(m_lock_behavior);
    return ok();
}

SpinLock::SpinLock(const LockBehavior lock_behavior) noexcept
    : m_recursive(lock_behavior == LockBehavior::RECURSIVE)
{
}

expected<void, LockError> SpinLock::lock_impl() noexcept
{
    auto pid = getpid();
    auto tid = std::this_thread::get_id();

    if (m_pid.load() == pid && m_tid.load() == tid)
    {
        if (m_recursive.load(std::memory_order_relaxed))
        {
            m_recursive_count.fetch_add(1);

            return ok();
        }

        return err(LockError::DEADLOCK_CONDITION);
    }

    detail::adaptive_wait spinner;
    spinner.wait_loop([this] { return this->m_lock_flag.test_and_set(std::memory_order_acquire); });

    m_pid.store(pid);
    m_tid.store(tid);
    m_recursive_count.store(1);

    return ok();
}

expected<void, UnlockError> SpinLock::unlock_impl() noexcept
{
    auto pid = getpid();
    auto tid = std::this_thread::get_id();

    if (m_pid.load() != pid || m_tid.load() != tid)
    {
        return err(UnlockError::NOT_OWNED_BY_THREAD);
    }

    if (m_recursive_count.load() == 0)
    {
        return err(UnlockError::NOT_LOCKED);
    }

    auto old_recursive_count = m_recursive_count.fetch_sub(1);
    if (old_recursive_count == 1)
    {
        m_pid.store(0);
        m_tid.store(std::thread::id());
        m_lock_flag.clear(std::memory_order_release);
    }

    return ok();
}

expected<TryLock, TryLockError> SpinLock::try_lock_impl() noexcept
{
    auto pid = getpid();
    auto tid = std::this_thread::get_id();

    if (m_pid.load() == pid && m_tid.load() == tid)
    {
        if (m_recursive.load(std::memory_order_relaxed))
        {
            m_recursive_count.fetch_add(1);
            return ok(TryLock::LOCK_SUCCEEDED);
        }

        return ok(TryLock::FAILED_TO_ACQUIRE_LOCK);
    }

    if (!m_lock_flag.test_and_set(std::memory_order_acquire))
    {
        m_pid.store(pid);
        m_tid.store(tid);
        m_recursive_count.store(1);

        return ok(TryLock::LOCK_SUCCEEDED);
    }
    return ok(TryLock::FAILED_TO_ACQUIRE_LOCK);
}

} // namespace concurrent
} // namespace iox
