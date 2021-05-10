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

#ifndef IOX_UTILS_PERIODIC_TIMER_HPP
#define IOX_UTILS_PERIODIC_TIMER_HPP

#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include <thread>

namespace iox
{
namespace posix
{
/// @brief This enum class offers the timer error codes.
enum class TimerError
{
    INVALID_ARGUMENTS,
    NO_PERMISSION,
    INVALID_POINTER,

    INTERNAL_LOGIC_ERROR,
    NO_ERROR,
    INVALID_STATE
};

/// @brief This enum class offers the timer state events.
enum class PeriodicTimerState : uint8_t
{
    DISABLED, /// state for the inactive timer
    ENABLED   /// state for the active timer
};

/// @brief This enum class offers the timer state events.
/// After the wait if the timer is disabled it returns STOP.
/// After the wait if the timer is enabled and to be activated without delay it returns TICK.
/// After the wait if the timer is enabled and to be activated with delay it returns TICK_DELAY.
/// After the wait if the timer is enabled and to be activated with delay breaching the delay threshold it returns
/// TICK_THRESHOLD_DELAY.
enum class PeriodicTimerEvent : uint8_t
{
    STOP,
    TICK,
    TICK_DELAY,
    TICK_THRESHOLD_DELAY
};


/// @brief This class offers periodic timer functionality. This timer is started immediately upon construction.
/// The periodic timer waits for the duration specified as interval before it comes to execution again. The periodicity
/// is ensured based on the delay calculation.
/// @code
///     iox::posix::PeriodicTimer periodicTimer(1000_ms);
///
///     // to do the execution periodically
///     while(...){
///     ...
///     periodicTimer.wait();
///     }
///     // to stop the timer
///     periodicTimer.stop();
///
/// @endcode
class PeriodicTimer
{
  public:
    /// @brief Constructor
    /// @param[in] interval duration until the timer sleeps and wakes up for execution
    /// @param[in] delayThreshold optional parameter. The delay duration that is allowed between each activation. If the
    /// execution time breaches this threshold the timer returns the state accordingly. The delayThreshold should be a
    /// natural number for delay consideration.
    PeriodicTimer(const iox::units::Duration interval,
                  const iox::units::Duration delayThreshold = units::Duration::fromSeconds(0)) noexcept;

    /// @brief Stops and joins the thread spawned by the constructor.
    virtual ~PeriodicTimer() noexcept = default;

    /// @brief (re-)Starts the timer. This also calculates the time until the timer goes to sleep and next time of
    /// activation. This also acquires the binary semaphore.
    void start() noexcept;

    /// @brief Starts the timer. This also calculates the time until the timer goes for sleep. This also acquires the
    /// binary semaphore.
    /// @param[in] interval The new duration until the timer sleeps and wakes up for execution
    void start(const units::Duration interval) noexcept;

    /// @brief Stops the timer. This also releases the acquired binary semaphore.
    void stop() noexcept;

    /// @brief This function returns the current time.
    /// @return The system clock real time is returned.
    static cxx::expected<units::Duration, TimerError> now() noexcept;

    /// @brief Briefly waits for the timer interval. This is achieved by trying to do a timed wait for the interval
    /// duration on the already acquired binary semaphore.
    /// @return TICK if the timer is active, STOP if the timer is stopped already, TICK_DELAY if there is delay and
    /// TICK_THRESHOLD_DELAY if the delay is more than the given delay threshold.
    cxx::expected<iox::posix::PeriodicTimerEvent, TimerError> wait() noexcept;

  private:
    iox::units::Duration m_interval{units::Duration::fromSeconds(0)};
    iox::units::Duration m_timeForNextActivation{units::Duration::fromSeconds(0)};
    iox::units::Duration m_delayThreshold{units::Duration::fromSeconds(0)};
    posix::Semaphore m_waitSemaphore;
    static cxx::error<TimerError> createErrorCodeFromErrNo(const int32_t errnum) noexcept;
};

} // namespace posix
} // namespace iox

#endif // IOX_UTILS_PERIODIC_TIMER_HPP
