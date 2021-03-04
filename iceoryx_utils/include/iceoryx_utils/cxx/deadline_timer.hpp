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
#ifndef IOX_UTILS_CXX_DEADLINETIMER_HPP
#define IOX_UTILS_CXX_DEADLINETIMER_HPP

#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/platform/signal.hpp"

#include <chrono>
#include <cstdint>

namespace iox
{
namespace cxx
{
/// @brief This offers the deadline timer functionality. It has user convenient methods to reset the timer [by default
/// it uses the intialized duration], reset timer to a customized duration, check if the timer is active and user can
/// also get to know about the remaining time before the timer goes off
/// @code
///     iox::cxx::DeadlineTimer deadlineTimer(1000_ms);
///
///     // to check if the timer is active
///     if( deadlineTimer.hasExpired()){
///     ...
///     }
///     // to reset the timer and start again with the same duration
///     deadlineTimer.reset();
///
/// @endcode
class DeadlineTimer
{
  public:
    /// @brief Constructor
    /// @param[in] timeToWait duration until the timer expires
    DeadlineTimer(const iox::units::Duration timeToWait) noexcept;

    /// @brief Checks if the timer has expired compared to its absolute end time
    /// @return false if the timer is still active and true if it is expired
    bool hasExpired() const noexcept;

    /// @brief reinitializes the ending time for the timer. The absolute end time is calculated by adding time to wait
    /// to the current time.
    void reset() noexcept;

    /// @brief reinitializes the ending time for the timer to the given new time to wait. The absolute end time is
    /// calculated by adding new time to wait to the current time.
    /// @param[in] timeToWait duration until the timer expires. This value overwrites the earlier value which was set
    /// during the timer creation.
    void reset(const iox::units::Duration timeToWait) noexcept;

    /// @brief calculates the remaining time before the timer goes off
    /// @return the time duration before the timer expires
    const iox::units::Duration remainingTime() const noexcept;

  private:
    iox::units::Duration getCurrentMonotonicTime() const noexcept;

    iox::units::Duration m_timeToWait;
    iox::units::Duration m_endTime;
};

} // namespace cxx
} // namespace iox


#endif // IOX_UTILS_CXX_DEADLINETIMER_HPP
