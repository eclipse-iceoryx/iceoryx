// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGCOMMON_HPP
#define IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGCOMMON_HPP

#include "iceoryx_hoofs/iceoryx_hoofs_types.hpp"

#include <cstdint>

namespace iox
{
namespace pbb
{
/// @todo remove
using log::LogLevel;

/// @brief converts LogLevel into a string literal color code
/// @param[in] value the LogLevel to convert
/// @return string literal of the corresponding color code
constexpr const char* logLevelDisplayColor(const LogLevel value) noexcept;

/// @brief converts LogLevel into a string literal display text
/// @param[in] value the LogLevel to convert
/// @return string literal of the display text
constexpr const char* logLevelDisplayText(const LogLevel value) noexcept;

/// @brief Provides access to the log buffer if direct access is required
struct LogBuffer
{
    const char* buffer{nullptr};
    uint64_t writeIndex{0};
};

namespace internal
{
template <typename T>
constexpr const char* logFormatDec();

template <typename T>
constexpr const char* logFormatHex();

template <typename T>
constexpr const char* logFormatOct();
} // namespace internal

/// @brief printf-like format string for decimal formatting of numbers
template <typename T>
static constexpr const char* LOG_FORMAT_DEC = internal::logFormatDec<T>();

/// @brief printf-like format string for hexadecimal formatting of numbers
template <typename T>
static constexpr const char* LOG_FORMAT_HEX = internal::logFormatHex<T>();

/// @brief printf-like format string for octal formatting of numbers
template <typename T>
static constexpr const char* LOG_FORMAT_OCT = internal::logFormatOct<T>();

} // namespace pbb
} // namespace iox

#include "iceoryx_hoofs/internal/log/platform_building_blocks/logcommon.inl"

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_HPP
