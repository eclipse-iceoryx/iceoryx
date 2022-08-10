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

#ifndef IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_INL
#define IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_INL

#include "iceoryx_hoofs/log/platform_building_blocks/console_logger.hpp"

namespace iox
{
namespace pbb
{
template <uint32_t N>
// NOLINTJUSTIFICATION see at declaration in header
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
inline constexpr uint32_t ConsoleLogger::bufferSize(const char (&)[N]) noexcept
{
    return N;
}

template <typename T>
inline void ConsoleLogger::unused(T&&) const noexcept
{
}

template <typename T>
inline void ConsoleLogger::logArithmetik(const T value, const char* format) noexcept
{
    auto retVal =
        // NOLINTJUSTIFICATION it is ensured that the index cannot be out of bounds
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        snprintf(&m_buffer[m_bufferWriteIndex],
                 NULL_TERMINATED_BUFFER_SIZE - m_bufferWriteIndex,
                 format,
                 value); // TODO do we need to check whether message is null-terminated at a reasonable length?
    if (retVal >= 0)
    {
        m_bufferWriteIndex += static_cast<uint32_t>(retVal);
    }
    else
    {
        // TODO an error occurred; what to do next? call error handler?
    }
}

} // namespace pbb
} // namespace iox

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_CONSOLE_LOGGER_INL
