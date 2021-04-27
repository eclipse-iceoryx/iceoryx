// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/cxx/timer.hpp"

namespace iox
{
namespace cxx
{
Timer::Timer(const iox::units::Duration interval, const iox::units::Duration delayThreshold) noexcept
    : m_interval(interval)
    , m_delayThreshold(delayThreshold)
{
    start();
}

void Timer::start() noexcept
{
    stop();
    auto waitResult = m_waitSemaphore.timedWait(m_interval, true);
    cxx::Expects(!waitResult.has_error());
    m_timeForNextActivation = now() + m_interval;
}

void Timer::start(const iox::units::Duration interval) noexcept
{
    m_interval = interval;
    start();
}

void Timer::stop() noexcept
{
    if (*(m_waitSemaphore.getValue()) == static_cast<int>(posix::SemaphoreWaitState::TIMEOUT))
    {
        auto stopResult = m_waitSemaphore.post();
        cxx::Expects(!stopResult.has_error());
    }
}


const iox::units::Duration Timer::now() const noexcept
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return iox::units::Duration(ts);
}

cxx::expected<iox::cxx::TimerEvent, posix::SemaphoreError> Timer::wait() noexcept
{
    if (*(m_waitSemaphore.getValue())
        == static_cast<int>(
            posix::SemaphoreWaitState::TIMEOUT)) // To check if the TIMER is active (if the sempahore is acquired)
    {
        if (now() > m_timeForNextActivation)
        {
            auto timeDiff =
                now() - m_timeForNextActivation; // Calculate the time delay to check if it breaches the threshold
            m_timeForNextActivation = m_timeForNextActivation + m_interval; // Calculate the next time for activation
            if (m_delayThreshold > 0_ms)
            {
                if (timeDiff > m_delayThreshold)
                {
                    return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK_THRESHOLD_DELAY);
                }
            }
            return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK_DELAY);
        }
        else
        {
            auto actualWaitDuration = m_timeForNextActivation - now();
            auto waitResult = m_waitSemaphore.timedWait(actualWaitDuration, true);
            if (waitResult.has_error())
            {
                return cxx::error<posix::SemaphoreError>(waitResult.get_error());
            }
            else
            {
                m_timeForNextActivation = m_timeForNextActivation + m_interval;
                return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK);
            }
        }
    }
    return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::STOP);
}

} // namespace cxx
} // namespace iox
