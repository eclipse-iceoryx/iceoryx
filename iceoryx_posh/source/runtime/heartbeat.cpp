// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/heartbeat.hpp"

#include "iceoryx_platform/time.hpp"
#include "iox/duration.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
namespace runtime
{
Heartbeat::Heartbeat() noexcept
{
    beat();
}

/// @brief Get the elapsed milliseconds since the last heartbeat
uint64_t Heartbeat::elapsed_milliseconds_since_last_beat() const noexcept
{
    auto now = milliseconds_since_epoch();
    auto timestamp_last_beat = m_timestamp_last_beat.load(std::memory_order_relaxed);
    if (timestamp_last_beat >= now)
    {
        return 0;
    }
    return now - timestamp_last_beat;
}

/// @brief Update the heartbeat timestamp
void Heartbeat::beat() noexcept
{
    m_timestamp_last_beat.store(milliseconds_since_epoch(), std::memory_order_relaxed);
}

uint64_t Heartbeat::milliseconds_since_epoch() noexcept
{
    struct timespec timepoint
    {
    };

    IOX_ENFORCE(
        !IOX_POSIX_CALL(iox_clock_gettime)(CLOCK_MONOTONIC, &timepoint).failureReturnValue(-1).evaluate().has_error(),
        "An error which should never happen occured during 'iox_clock_gettime'!");

    return units::Duration(timepoint).toMilliseconds();
}
} // namespace runtime
} // namespace iox
