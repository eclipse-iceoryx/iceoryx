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

#include "iox/spin_semaphore.hpp"
#include "iox/detail/adaptive_wait.hpp"

namespace iox
{
namespace concurrent
{
expected<void, SemaphoreError>
SpinSemaphoreBuilder::create(optional<SpinSemaphore>& uninitializedSemaphore) const noexcept
{
    if (m_initialValue > IOX_SEM_VALUE_MAX)
    {
        IOX_LOG(Error,
                "The spin semaphore initial value of " << m_initialValue << " exceeds the maximum semaphore value "
                                                       << IOX_SEM_VALUE_MAX);
        return err(SemaphoreError::SEMAPHORE_OVERFLOW);
    }

    uninitializedSemaphore.emplace(static_cast<int32_t>(m_initialValue));
    return ok();
}

SpinSemaphore::SpinSemaphore(int32_t initial_value) noexcept
    : m_count(initial_value)
{
    SpinLockBuilder()
        .is_inter_process_capable(true)
        .lock_behavior(LockBehavior::NORMAL)
        .create(m_spinlock)
        .expect("Failed to create Lock");
}

SpinSemaphore::~SpinSemaphore() noexcept
{
    m_to_be_destroyed = true;
}

expected<void, SemaphoreError> SpinSemaphore::post_impl() noexcept
{
    std::lock_guard<concurrent::SpinLock> lock(*m_spinlock);

    if (m_count.load(std::memory_order_relaxed) == IOX_SEM_VALUE_MAX)
    {
        return err(SemaphoreError::SEMAPHORE_OVERFLOW);
    }

    ++m_count;
    return ok();
}

expected<void, SemaphoreError> SpinSemaphore::wait_impl() noexcept
{
    detail::adaptive_wait spinner;
    spinner.wait_loop([this] {
        auto wait_result = this->tryWait();
        return wait_result.has_value() && !wait_result.value();
    });
    return ok();
}

expected<bool, SemaphoreError> SpinSemaphore::try_wait_impl() noexcept
{
    std::lock_guard<concurrent::SpinLock> lock(*m_spinlock);
    if (m_to_be_destroyed.load(std::memory_order_relaxed))
    {
        return ok(true);
    }
    if (m_count.load(std::memory_order_relaxed) > 0)
    {
        --m_count;
        return ok(true);
    }
    return ok(false);
}

expected<SemaphoreWaitState, SemaphoreError> SpinSemaphore::timed_wait_impl(const units::Duration& timeout) noexcept
{
    iox::deadline_timer deadline_timer(timeout);
    detail::adaptive_wait spinner;

    auto ret_val = SemaphoreWaitState::TIMEOUT;
    spinner.wait_loop([this, &deadline_timer, &ret_val] {
        auto wait_result = this->tryWait();

        if (wait_result.has_value() && wait_result.value())
        {
            ret_val = SemaphoreWaitState::NO_TIMEOUT;
            return false;
        }
        return !deadline_timer.hasExpired();
    });

    return ok(ret_val);
}
} // namespace concurrent
} // namespace iox
