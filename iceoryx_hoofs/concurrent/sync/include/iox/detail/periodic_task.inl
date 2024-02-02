// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CONCURRENT_SYNC_PERIODIC_TASK_INL
#define IOX_HOOFS_CONCURRENT_SYNC_PERIODIC_TASK_INL

#include "iox/detail/periodic_task.hpp"

namespace iox
{
namespace concurrent
{
namespace detail
{
template <typename T>
template <typename... Args>
// NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter) justification in header
inline PeriodicTask<T>::PeriodicTask(const PeriodicTaskManualStart_t,
                                     const ThreadName_t& taskName,
                                     Args&&... args) noexcept
    : m_callable(std::forward<Args>(args)...)
    , m_taskName(taskName)
{
    UnnamedSemaphoreBuilder().initialValue(0U).isInterProcessCapable(false).create(m_stop).expect(
        "Unable to create semaphore for periodic task");
}

template <typename T>
template <typename... Args>
// NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter) justification in header
inline PeriodicTask<T>::PeriodicTask(const PeriodicTaskAutoStart_t,
                                     const units::Duration interval,
                                     const ThreadName_t& taskName,
                                     Args&&... args) noexcept
    : PeriodicTask(PeriodicTaskManualStart, taskName, std::forward<Args>(args)...)
{
    start(interval);
}

template <typename T>
inline PeriodicTask<T>::~PeriodicTask() noexcept
{
    stop();
}

template <typename T>
inline void PeriodicTask<T>::start(const units::Duration interval) noexcept
{
    stop();
    m_interval = interval;
    m_taskExecutor = std::thread(&PeriodicTask::run, this);
}

template <typename T>
inline void PeriodicTask<T>::stop() noexcept
{
    if (m_taskExecutor.joinable())
    {
        m_stop->post().expect("'post' on a semaphore should always be successful");
        m_taskExecutor.join();
    }
}

template <typename T>
inline bool PeriodicTask<T>::is_active() const noexcept
{
    return m_taskExecutor.joinable();
}

template <typename T>
inline bool PeriodicTask<T>::isActive() const noexcept
{
    return is_active();
}

template <typename T>
inline void PeriodicTask<T>::run() noexcept
{
    setThreadName(m_taskName);

    auto waitState = SemaphoreWaitState::NO_TIMEOUT;
    do
    {
        m_callable();

        /// @todo iox-#337 use a refactored posix::Timer::wait method returning TIMER_TICK and TIMER_STOPPED once
        /// available
        waitState = m_stop->timedWait(m_interval).expect("'timedWait' on a semaphore should always be successful");
    } while (waitState == SemaphoreWaitState::TIMEOUT);
}

} // namespace detail
} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_PERIODIC_TASK_INL
