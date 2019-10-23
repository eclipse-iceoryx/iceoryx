// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/timer.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace posix
{
static void callbackHelper(sigval data)
{
    // Convert the this pointer of the object
    Timer* timer = static_cast<Timer*>(data.sival_ptr);
    timer->executeCallback();
}

cxx::expected<units::Duration, TimerError> Timer::now() noexcept
{
    struct timespec value;
    auto result =
        cxx::makeSmartC(clock_gettime, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, CLOCK_REALTIME, &value);

    if (result.hasErrors())
    {
        return createErrorFromErrno(result.getErrNum());
    }

    return cxx::success<units::Duration>(value);
}

Timer::Timer(units::Duration timeToWait) noexcept
    : m_timeToWait(timeToWait)
    , m_creationTime(now().get_value())
    , m_timerId(INVALID_TIMER_ID)
    , m_callback(nullptr)
{
}

Timer::Timer(units::Duration timeToWait, std::function<void()> callback) noexcept
    : m_timeToWait(timeToWait)
    , m_creationTime(now().get_value())
    , m_callback(callback)
{
    // Is the callback valid?
    if (!callback)
    {
        m_isInitialized = false;
        m_errorValue = TimerError::NO_VALID_CALLBACK;
    }

    // Create the struct in order to configure the timer in the OS
    struct sigevent asyncCallNotification;
    // We want the timer to call a function
    asyncCallNotification.sigev_notify = SIGEV_THREAD;
    // Set the function pointer to our sigevent
    asyncCallNotification.sigev_notify_function = &callbackHelper;
    // Save the pointer to self in order to execute the callback
    asyncCallNotification.sigev_value.sival_ptr = this;
    // Do not set any thread attributes
    asyncCallNotification.sigev_notify_attributes = nullptr;

    auto result = cxx::makeSmartC(timer_create,
                                  cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                  {-1},
                                  {},
                                  CLOCK_REALTIME,
                                  &asyncCallNotification,
                                  &m_timerId);

    if (result.hasErrors())
    {
        m_isInitialized = false;
        m_errorValue = createErrorFromErrno(result.getErrNum()).value;
    }
    else
    {
        m_isInitialized = true;
    }
}

Timer::~Timer() noexcept
{
    if (m_timerId != INVALID_TIMER_ID)
    {
        if (deleteTimer().has_error())
        {
            std::cerr << "Unable to cleanup posix::Timer \"" << m_timerId << "\" in the destructor" << std::endl;
        }
    }
}


void Timer::executeCallback() noexcept
{
    if (m_isInitialized && m_callback)
    {
        m_callback();
    }
    else
    {
        // Thread couldn't reach callback or object is not correctly initalized, maybe the originial object was a
        // temporary?
        errorHandler(Error::kPOSIX_TIMER__FIRED_TIMER_BUT_STATE_IS_INVALID);
    }
}

cxx::expected<TimerError> Timer::deleteTimer() noexcept
{
    if (m_timerId == INVALID_TIMER_ID)
    {
        return cxx::error<TimerError>(TimerError::NO_TIMER_TO_DELETE);
    }

    auto result = cxx::makeSmartC(timer_delete, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_timerId);

    if (result.hasErrors())
    {
        return createErrorFromErrno(result.getErrNum());
    }

    m_timerId = INVALID_TIMER_ID;

    return cxx::success<void>();
}

cxx::expected<TimerError> Timer::start(bool periodic) noexcept
{
    if (hasError())
    {
        cxx::error<TimerError>(TimerError::TIMER_NOT_INITIALIZED);
    }
    // Convert units::Duration to itimerspec
    struct itimerspec interval;
    interval.it_value = m_timeToWait.timespec(units::TimeSpecReference::None);

    // Enable the timer to be periodic
    if (!periodic)
    {
        interval.it_interval.tv_sec = 0;
        interval.it_interval.tv_nsec = 0;
    }
    else
    {
        interval.it_interval = m_timeToWait.timespec(units::TimeSpecReference::None);
    }

    // Set the timer
    auto result = cxx::makeSmartC(
        timer_settime, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_timerId, 0, &interval, nullptr);

    if (result.hasErrors())
    {
        return createErrorFromErrno(result.getErrNum());
    }

    // Timer is now armed
    m_armed = true;

    return cxx::success<void>();
}

cxx::expected<TimerError> Timer::stop() noexcept
{
    if (hasError())
    {
        cxx::error<TimerError>(TimerError::TIMER_NOT_INITIALIZED);
    }
    if (!m_armed)
    {
        // Timer was not started yet
        return cxx::success<void>();
    }

    struct itimerspec interval;
    units::Duration zero = 0_s;
    interval.it_value = zero.timespec(units::TimeSpecReference::None);

    // Disarm the timer
    auto result = cxx::makeSmartC(
        timer_settime, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_timerId, 0, &interval, nullptr);

    if (result.hasErrors())
    {
        return createErrorFromErrno(result.getErrNum());
    }
    return cxx::success<void>();
}

cxx::expected<TimerError> Timer::restart(units::Duration timeToWait, bool periodic) noexcept
{
    if (hasError())
    {
        cxx::error<TimerError>(TimerError::TIMER_NOT_INITIALIZED);
    }

    // See if there is currently an armed timer in the operating system and update m_armed accordingly
    auto gettimeResult = timeUntilExpiration();

    if (gettimeResult.has_error())
    {
        return gettimeResult;
    }

    // Set new timeToWait value
    m_timeToWait = timeToWait;

    // Disarm running timer
    if (m_armed)
    {
        auto stopResult = stop();

        if (stopResult.has_error())
        {
            return stopResult;
        }
    }

    // Arm the timer with the new timeToWait value
    auto startResult = start(periodic);

    if (startResult.has_error())
    {
        return startResult;
    }
    return cxx::success<void>();
}

cxx::expected<units::Duration, TimerError> Timer::timeUntilExpiration() noexcept
{
    if (hasError())
    {
        cxx::error<TimerError>(TimerError::TIMER_NOT_INITIALIZED);
    }
    struct itimerspec currentInterval;

    auto result =
        cxx::makeSmartC(timer_gettime, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_timerId, &currentInterval);

    if (result.hasErrors())
    {
        return createErrorFromErrno(result.getErrNum());
    }

    if (currentInterval.it_value.tv_sec == 0 && currentInterval.it_value.tv_sec == 0)
    {
        // Timer is disarmed
        m_armed = false;
    }
    return cxx::success<units::Duration>(currentInterval.it_value);
}

cxx::expected<uint64_t, TimerError> Timer::getOverruns() noexcept
{
    if (hasError())
    {
        cxx::error<TimerError>(TimerError::TIMER_NOT_INITIALIZED);
    }

    auto result = cxx::makeSmartC(timer_getoverrun, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_timerId);

    if (result.hasErrors())
    {
        return createErrorFromErrno(result.getErrNum());
    }
    return cxx::success<uint64_t>(result.getReturnValue());
}

void Timer::resetCreationTime() noexcept
{
    // Get the current time
    auto now = this->now();

    m_creationTime = now.get_value();
}

bool Timer::hasExpiredComparedToCreationTime() noexcept
{
    // Get the current time
    auto now = this->now();

    // Calc the elapsed time, since Timer object was created
    auto elapsedTime = now.get_value() - m_creationTime;

    if (elapsedTime >= m_timeToWait)
    {
        return true;
    }
    return false; // not enabled, returns false
}

bool Timer::hasError() const noexcept
{
    return !m_isInitialized;
}


cxx::error<TimerError> Timer::getError() const noexcept
{
    return m_errorValue;
}

cxx::error<TimerError> Timer::createErrorFromErrno(const int errnum) noexcept
{
    switch (errnum)
    {
    case EAGAIN:
    {
        std::cerr << "Kernel failed to allocate timer structures" << std::endl;
        return cxx::error<TimerError>(TimerError::KERNEL_ALLOC_FAILED);
    }
    case EINVAL:
    {
        std::cerr << "Provided invalid arguments for posix::Timer" << std::endl;
        return cxx::error<TimerError>(TimerError::INVALID_ARGUMENTS);
    }
    case ENOMEM:
    {
        std::cerr << "Could not allocate memory for posix::Timer" << std::endl;
        return cxx::error<TimerError>(TimerError::ALLOC_MEM_FAILED);
    }
    case EPERM:
    {
        std::cerr << "No permissions to set the clock" << std::endl;
        return cxx::error<TimerError>(TimerError::NO_PERMISSION);
    }
    case EFAULT:
    {
        std::cerr << "An invalid pointer was provided" << std::endl;
        return cxx::error<TimerError>(TimerError::INVALID_POINTER);
    }
    default:
    {
        std::cerr << "Internal logic error in posix::Timer occurred" << std::endl;
        return cxx::error<TimerError>(TimerError::INTERNAL_LOGIC_ERROR);
    }
    }
}

} // namespace posix
} // namespace iox
