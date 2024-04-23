// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_UTILITY_POSIX_SCHEDULER_HPP
#define IOX_HOOFS_POSIX_UTILITY_POSIX_SCHEDULER_HPP

#include "iceoryx_platform/sched.hpp"

#include <cstdint>

namespace iox
{
namespace detail
{
/// @brief Defines all supported scheduler
// NOLINTNEXTLINE(performance-enum-size) int32_t required for POSIX API
enum class Scheduler : int32_t
{
    FIFO = SCHED_FIFO
};

/// @brief Returns the minimum priority of the provided scheduler
/// @param[in] scheduler the scheduler which is queried
/// @return The minimum priority of the scheduler
int32_t getSchedulerPriorityMinimum(const Scheduler scheduler) noexcept;

/// @brief Returns the maximum priority of the provided scheduler
/// @param[in] scheduler the scheduler which is queried
/// @return The maximum priority of the scheduler
int32_t getSchedulerPriorityMaximum(const Scheduler scheduler) noexcept;
} // namespace detail
} // namespace iox

#endif // IOX_HOOFS_POSIX_UTILITY_POSIX_SCHEDULER_HPP
