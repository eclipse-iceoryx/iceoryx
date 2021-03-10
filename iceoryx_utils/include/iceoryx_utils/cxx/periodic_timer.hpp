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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_UTILS_PERIODIC_TIMER_HPP
#define IOX_UTILS_PERIODIC_TIMER_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"


namespace iox
{
namespace cxx
{
/// @breif This enum class offers the timer state events.
/// If the timer is active the TimerEvent has TICK as the state
/// If tht timer is inactive the TimerEvent has STOP as the state
enum class TimerEvent
{
    TICK,
    STOP
};


/// @brief This class offers periodic timer functionality. This timer is started immediately upon construction.
/// The periodic timer waits for the breif duration specified as interval before it comes to execuion again
/// @code
///     iox::cxx::PeriodicTimer periodicTimer(1000_ms);
///
///     // to do the execution periodically
///     While(...){
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
    PeriodicTimer(const iox::units::Duration interval) noexcept;

    /// @brief starts the timer. This also calculates the time until the timer goes for sleep. This also acquires the
    /// binary semaphore.
    void start() noexcept;

    /// @brief starts the timer. This also calculates the time until the timer goes for sleep. This also acquires the
    /// binary semaphore.
    /// @param[in] interval the new duration until the timer sleeps and wakes up for execution
    void start(const units::Duration interval) noexcept;

    /// @brief stops the timer. This also releases the acquired binary semaphore.
    void stop() noexcept;

    /// @brief calculates the remaining time before the timer goes off
    /// @return the time duration before the timer expires
    const iox::units::Duration now() const noexcept;

    /// @brief breifly waits for the timer interval. This is acheived by trying to do a timed wait for the interval
    /// duration on the already aquired binary semaphore
    /// @return TICK if the timer is active and STOP if the timer is stopped already
    cxx::expected<iox::cxx::TimerEvent, posix::SemaphoreError> wait() noexcept;

  private:
    iox::units::Duration m_interval;
    posix::Semaphore m_stop{posix::Semaphore::create(posix::CreateUnnamedSingleProcessSemaphore, 0U).value()};
};

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_PERIODIC_TIMER_HPP
