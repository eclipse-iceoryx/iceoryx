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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_UTILS_POSIX_WRAPPER_TIMESPEC_HPP
#define IOX_UTILS_POSIX_WRAPPER_TIMESPEC_HPP

#include <time.h>
// For std types
#include <cstdint>

namespace iox
{
namespace posix
{
constexpr unsigned int TS_DIVIDER_sec = 1000000000;
constexpr unsigned int TS_DIVIDER_msec = (TS_DIVIDER_sec / 1000);

/// @brief adds period in time [ms] to a given timestruct
/// @param[in] time period  time base
/// @param[in] timeToAdd_ms period in time [ms] to be added
/// @return sum of the two inputs [timespec]
struct timespec addTimeMs(struct timespec time, const uint32_t timeToAdd_ms);

/// @brief subtract subtrahend from minuend
/// @param[in] minuend [timespec]
/// @param[in] subtrahend [timespec]
/// @return result [ms]
double subtractTimespecMS(const struct timespec minuend, const struct timespec subtrahend);

} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_TIMESPEC_HPP
