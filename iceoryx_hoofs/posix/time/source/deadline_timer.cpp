// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/deadline_timer.hpp"
#include "iox/assertions.hpp"
#include "iox/posix_call.hpp"

#include "iceoryx_platform/time.hpp"

namespace iox
{
deadline_timer::deadline_timer(const iox::units::Duration timeToWait) noexcept
    : m_timeToWait(timeToWait)
    , m_endTime(getCurrentMonotonicTime() + timeToWait)
{
}

bool deadline_timer::hasExpired() const noexcept
{
    return getCurrentMonotonicTime() >= m_endTime;
}

void deadline_timer::reset() noexcept
{
    m_endTime = getCurrentMonotonicTime() + m_timeToWait;
}

void deadline_timer::reset(const iox::units::Duration timeToWait) noexcept
{
    m_timeToWait = timeToWait;
    reset();
}

iox::units::Duration deadline_timer::remainingTime() const noexcept
{
    const iox::units::Duration currentTime{getCurrentMonotonicTime()};
    if (m_endTime > currentTime)
    {
        return m_endTime - currentTime;
    }
    using namespace iox::units::duration_literals;
    return 0_s;
}

iox::units::Duration deadline_timer::getCurrentMonotonicTime() noexcept
{
    timespec time_since_epoch{0, 0};
    IOX_ENFORCE(!IOX_POSIX_CALL(iox_clock_gettime)(CLOCK_MONOTONIC, &time_since_epoch)
                     .failureReturnValue(-1)
                     .evaluate()
                     .has_error(),
                "An error which should never happen occured during 'iox_clock_gettime'!");
    return iox::units::Duration{time_since_epoch};
}
} // namespace iox
