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

#include "iceoryx_utils/cxx/periodic_timer.hpp"

namespace iox
{
namespace cxx
{
PeriodicTimer::PeriodicTimer(const iox::units::Duration interval) noexcept
    : m_interval(interval)
{
    start();
}

PeriodicTimer::PeriodicTimer(const iox::units::Duration interval, const iox::units::Duration delayThreshold) noexcept
    : m_interval(interval)
    , m_delayThreshold(delayThreshold)
{
    start();
}

PeriodicTimer::~PeriodicTimer() noexcept
{
    m_waitSemaphore.~Semaphore();
}

void PeriodicTimer::start() noexcept
{
    stop();
    auto waitResult = m_waitSemaphore.timedWait(m_interval, true);
    cxx::Expects(!waitResult.has_error());
    m_timeForNextActivation = now() + m_interval;
}

void PeriodicTimer::start(const iox::units::Duration interval) noexcept
{
    m_interval = interval;
    start();
}

void PeriodicTimer::stop() noexcept
{
    auto stopResult = m_waitSemaphore.post();
    cxx::Expects(!stopResult.has_error());
}


const iox::units::Duration PeriodicTimer::now() const noexcept
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return iox::units::Duration(ts);
}

cxx::expected<iox::cxx::TimerEvent, posix::SemaphoreError> PeriodicTimer::wait() noexcept
{
    if (*(m_waitSemaphore.getValue()) == static_cast<int>(posix::SemaphoreWaitState::TIMEOUT))
    {
        auto actualWaitDuration = m_timeForNextActivation - now();

        if (actualWaitDuration.toMilliseconds() == 0)
        {
            return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK);
        }
        // else if (actualWaitDuration.toMilliseconds() < 0)
        // {
        //     if (actualWaitDuration > m_delayThreshold)
        //     {
        //         return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK_THRESHOLD_DELAY);
        //     }
        //     else
        //     {
        //         return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK_DELAY);
        //     }
        // }
        else
        {
            auto waitResult = m_waitSemaphore.timedWait(actualWaitDuration, true);
            if (waitResult.has_error())
            {
                return cxx::error<posix::SemaphoreError>(waitResult.get_error());
            }
            else
            {
                return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK);
            }
        }
        // auto periodicWaitDuration = m_interval;
        // auto timeBeforeSemBlocking = now();
        // auto waitResult = m_waitSemaphore.timedWait(periodicWaitDuration, true);
        // auto timeAfterSemBlocking = now();
        // if (timeAfterSemBlocking - timeBeforeSemBlocking >= periodicWaitDuration)
        // {
        //     if (waitResult.has_error())
        //     {
        //         return cxx::error<posix::SemaphoreError>(waitResult.get_error());
        //     }
        //     else
        //     {
        //         return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::TICK);
        //     }
        // }
        // else
        // {
        //     stop();
        // }
    }
    return cxx::success<iox::cxx::TimerEvent>(iox::cxx::TimerEvent::STOP);
}

} // namespace cxx
} // namespace iox
