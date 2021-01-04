// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_CONCURRENT_PERIODIC_TASK_INL
#define IOX_UTILS_CONCURRENT_PERIODIC_TASK_INL

namespace iox
{
namespace concurrent
{
template <typename T>
template <typename... Args>
PeriodicTask<T>::PeriodicTask(const posix::ThreadName_t taskName,
                              const units::Duration interval,
                              Args&&... args) noexcept
    : m_callable(std::forward<Args>(args)...)
    , m_interval(interval)
{
    posix::setThreadName(m_taskExecutor.native_handle(), taskName);
}

template <typename T>
PeriodicTask<T>::~PeriodicTask() noexcept
{
    m_stop.post();
    if (m_taskExecutor.joinable())
    {
        m_taskExecutor.join();
    }
}

template <typename T>
void PeriodicTask<T>::run() noexcept
{
    posix::SemaphoreWaitState waitState = posix::SemaphoreWaitState::NO_TIMEOUT;
    do
    {
        m_callable();

        /// @todo use a refactored posix::Timer::wait method returning TIMER_TICK and TIMER_STOPPED once available
        auto targetTime = m_interval.timespec(units::TimeSpecReference::Epoch);
        auto waitResult = m_stop.timedWait(&targetTime, true);
        cxx::Expects(!waitResult.has_error());

        waitState = waitResult.value();
    } while (waitState == posix::SemaphoreWaitState::TIMEOUT);
}

} // namespace concurrent
} // namespace iox

#endif // IOX_UTILS_CONCURRENT_PERIODIC_TASK_INL
