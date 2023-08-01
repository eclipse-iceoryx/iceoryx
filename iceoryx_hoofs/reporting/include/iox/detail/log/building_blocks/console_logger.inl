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

#ifndef IOX_HOOFS_REPORTING_LOG_BUILDING_BLOCKS_CONSOLE_LOGGER_INL
#define IOX_HOOFS_REPORTING_LOG_BUILDING_BLOCKS_CONSOLE_LOGGER_INL

#include "iox/log/building_blocks/console_logger.hpp"

namespace iox
{
namespace log
{
// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See at declaration in header
// AXIVION Next Construct AutosarC++19_03-A18.1.1 : See at declaration in header
template <uint32_t N>
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
inline constexpr uint32_t ConsoleLogger::bufferSize(const char (&)[N]) noexcept
{
    return N;
}

template <typename T>
inline constexpr void ConsoleLogger::unused(T&&) noexcept
{
}

template <typename T, typename std::enable_if_t<std::is_arithmetic<T>::value, bool>>
inline void ConsoleLogger::logDec(const T value) noexcept
{
    logArithmetic(value, LOG_FORMAT_DEC<T>);
}

template <typename T,
          typename std::enable_if_t<(std::is_integral<T>::value && std::is_unsigned<T>::value)
                                        || std::is_floating_point<T>::value || std::is_pointer<T>::value,
                                    bool>>
inline void ConsoleLogger::logHex(const T value) noexcept
{
    logArithmetic(value, LOG_FORMAT_HEX<T>);
}

template <typename T, typename std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, bool>>
inline void ConsoleLogger::logOct(const T value) noexcept
{
    logArithmetic(value, LOG_FORMAT_OCT<T>);
}

// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See at declaration in header
template <typename T>
inline void ConsoleLogger::logArithmetic(const T value, const char* format) noexcept
{
    auto& data = getThreadLocalData();
    // NOLINTJUSTIFICATION it is ensured that the index cannot be out of bounds
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    const auto retVal = snprintf(&data.buffer[data.bufferWriteIndex],
                                 ThreadLocalData::NULL_TERMINATED_BUFFER_SIZE - data.bufferWriteIndex,
                                 format,
                                 value);
    if (retVal < 0)
    {
        /// @todo iox-#1755 this path should never be reached since we ensured the correct encoding of the character
        /// conversion specifier; nevertheless, we might want to call the error handler after the error handler
        /// refactoring was merged
        return;
    }

    const auto stringSizeToLog = static_cast<uint32_t>(retVal);
    const auto bufferWriteIndexNext = data.bufferWriteIndex + stringSizeToLog;
    if (bufferWriteIndexNext <= ThreadLocalData::BUFFER_SIZE)
    {
        data.bufferWriteIndex = bufferWriteIndexNext;
    }
    else
    {
        /// @todo iox-#1755 currently we don't support log messages larger than the log buffer and everything larger
        /// that the log buffer will be truncated;
        /// it is intended to flush the buffer and create a new log message later on
        data.bufferWriteIndex = ThreadLocalData::BUFFER_SIZE;
    }
}

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_REPORTING_LOG_BUILDING_BLOCKS_CONSOLE_LOGGER_INL
