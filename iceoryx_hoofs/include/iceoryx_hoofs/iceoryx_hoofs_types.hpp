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

#ifndef IOX_HOOFS_ICEORYX_HOOFS_TYPES_HPP
#define IOX_HOOFS_ICEORYX_HOOFS_TYPES_HPP

/// @note since this file will be included by many other files, it should not include other header except
/// iceoryx_platform or STL header

#include <cstdint>

namespace iox
{
namespace cxx
{
using byte_t = uint8_t;
}

namespace log
{
enum class LogLevel : uint8_t
{
    OFF = 0,
    FATAL,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

/// @brief converts LogLevel into a string literal
/// @param[in] value the LogLevel to convert
/// @return string literal of the LogLevel value
constexpr const char* asStringLiteral(const LogLevel value) noexcept;

} // namespace log
} // namespace iox

#include "iceoryx_hoofs/internal/iceoryx_hoofs_types.inl"

#endif // IOX_HOOFS_ICEORYX_HOOFS_TYPES_HPP
