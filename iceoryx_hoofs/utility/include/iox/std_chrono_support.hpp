// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_UTILITY_STD_CHRONO_SUPPORT_HPP
#define IOX_HOOFS_UTILITY_STD_CHRONO_SUPPORT_HPP

#include "iox/duration.hpp"
#include "iox/into.hpp"

#include <chrono>

namespace iox
{
/// @brief Construct a Duration object from std::chrono::nanoseconds
/// @param[in] value as nanoseconds
/// @attention Since negative durations are not allowed, the duration will be clamped to 0
template <>
struct FromImpl<std::chrono::nanoseconds, units::Duration>
{
    static units::Duration fromImpl(const std::chrono::nanoseconds& value) noexcept;
};

/// @brief Construct a Duration object from std::chrono::microseconds
/// @param[in] value as microseconds
/// @attention Since negative durations are not allowed, the duration will be clamped to 0
template <>
struct FromImpl<std::chrono::microseconds, units::Duration>
{
    static units::Duration fromImpl(const std::chrono::microseconds& value) noexcept;
};

/// @brief Construct a Duration object from std::chrono::milliseconds
/// @param[in] value as milliseconds
/// @attention Since negative durations are not allowed, the duration will be clamped to 0
template <>
struct FromImpl<std::chrono::milliseconds, units::Duration>
{
    static units::Duration fromImpl(const std::chrono::milliseconds& value) noexcept;
};

/// @brief Construct a Duration object from std::chrono::seconds
/// @param[in] value as seconds
/// @attention Since negative durations are not allowed, the duration will be clamped to 0
template <>
struct FromImpl<std::chrono::seconds, units::Duration>
{
    static units::Duration fromImpl(const std::chrono::seconds& value) noexcept;
};
} // namespace iox

#include "iox/detail/std_chrono_support.inl"

#endif // IOX_HOOFS_UTILITY_STD_CHRONO_SUPPORT_HPP
