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

#include "iceoryx_utils/posix_wrapper/periodic_timer.hpp"

using namespace iox::units::duration_literals;

namespace iox
{
namespace posix
{
PeriodicTimer::PeriodicTimer(const iox::units::Duration interval, const iox::units::Duration delayThreshold) noexcept
    : m_interval(interval)
    , m_delayThreshold(delayThreshold)
{
    m_waitSemaphore =
        std::move(posix::Semaphore::create(posix::CreateUnnamedSharedMemorySemaphore, 0U)
                      .or_else([](posix::SemaphoreError&) {
                          errorHandler(Error::kROUDI_APP__FAILED_TO_CREATE_SEMAPHORE, nullptr, ErrorLevel::FATAL);
                      })
                      .value());
    start();
}

void PeriodicTimer::start() noexcept
{
    stop();
    auto waitResult = m_waitSemaphore.timedWait(m_interval, true);
    cxx::Ensures(!waitResult.has_error());
    m_timeForNextActivation = now().value() + m_interval;
}

void PeriodicTimer::start(const iox::units::Duration interval) noexcept
{
    m_interval = interval;
    start();
}

void PeriodicTimer::stop() noexcept
{
    if (*(m_waitSemaphore.getValue()) == static_cast<int>(posix::SemaphoreWaitState::TIMEOUT))
    {
        auto stopResult = m_waitSemaphore.post();
        cxx::Ensures(!stopResult.has_error());
    }
}

cxx::expected<units::Duration, TimerError> PeriodicTimer::now() noexcept
{
    struct timespec ts;
    auto result =
        cxx::makeSmartC(clock_gettime, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, CLOCK_REALTIME, &ts);

    if (result.hasErrors())
    {
        return createErrorCodeFromErrNo(result.getErrNum());
    }

    return cxx::success<units::Duration>(ts);
}

cxx::expected<iox::posix::PeriodicTimerEvent, TimerError> PeriodicTimer::wait() noexcept
{
    // To check if the TIMER is active (if the sempahore is acquired)
    if (*(m_waitSemaphore.getValue()) == static_cast<int>(posix::SemaphoreWaitState::TIMEOUT))
    {
        if (now().value() > m_timeForNextActivation)
        {
            auto timeDiff = now().value()
                            - m_timeForNextActivation; // Calculate the time delay to check if it breaches the threshold
            m_timeForNextActivation = m_timeForNextActivation + m_interval; // Calculate the next time for activation
            if (m_delayThreshold > 0_ms)
            {
                if (timeDiff > m_delayThreshold)
                {
                    return cxx::success<iox::posix::PeriodicTimerEvent>(
                        iox::posix::PeriodicTimerEvent::TICK_THRESHOLD_DELAY);
                }
            }
            return cxx::success<iox::posix::PeriodicTimerEvent>(iox::posix::PeriodicTimerEvent::TICK_DELAY);
        }
        else
        {
            auto actualWaitDuration = m_timeForNextActivation - now().value();
            auto waitResult = m_waitSemaphore.timedWait(actualWaitDuration, true);
            if (waitResult.has_error())
            {
                return cxx::error<TimerError>(TimerError::INTERNAL_LOGIC_ERROR);
            }
            else
            {
                m_timeForNextActivation = m_timeForNextActivation + m_interval;
                return cxx::success<iox::posix::PeriodicTimerEvent>(iox::posix::PeriodicTimerEvent::TICK);
            }
        }
    }
    return cxx::success<iox::posix::PeriodicTimerEvent>(iox::posix::PeriodicTimerEvent::STOP);
}

cxx::error<TimerError> PeriodicTimer::createErrorCodeFromErrNo(const int32_t errnum) noexcept
{
    TimerError timerError = TimerError::INTERNAL_LOGIC_ERROR;
    switch (errnum)
    {
    case EINVAL:
    {
        std::cerr << "The argument provided is invalid" << std::endl;
        timerError = TimerError::INVALID_ARGUMENTS;
        break;
    }
    case EPERM:
    {
        std::cerr << "No permissions to set the clock" << std::endl;
        timerError = TimerError::NO_PERMISSION;
        break;
    }
    case EFAULT:
    {
        std::cerr << "An invalid pointer was provided" << std::endl;
        timerError = TimerError::INVALID_POINTER;
        break;
    }
    default:
    {
        std::cerr << "Internal logic error in posix::Timer occurred" << std::endl;
        break;
    }
    }
    return cxx::error<TimerError>(timerError);
}

} // namespace posix
} // namespace iox
