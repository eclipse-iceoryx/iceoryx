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

#ifndef IOX_HOOFS_PRIMITIVES_ICEORYX_HOOFS_TYPES_INL
#define IOX_HOOFS_PRIMITIVES_ICEORYX_HOOFS_TYPES_INL

#include "iox/iceoryx_hoofs_types.hpp"

/// @note since this file will be included by many other files, it should not include other header except
/// iceoryx_platform or STL header

namespace iox
{
// AXIVION Next Construct AutosarC++19_03-M2.10.1 : See declaration in header
namespace log
{
// AXIVION Next Construct AutosarC++19_03-A3.9.1 : See declaration in header
inline constexpr const char* asStringLiteral(const LogLevel value) noexcept
{
    switch (value)
    {
    case LogLevel::OFF:
        return "LogLevel::OFF";
    case LogLevel::FATAL:
        return "LogLevel::FATAL";
    case LogLevel::ERROR:
        return "LogLevel::ERROR";
    case LogLevel::WARN:
        return "LogLevel::WARN";
    case LogLevel::INFO:
        return "LogLevel::INFO";
    case LogLevel::DEBUG:
        return "LogLevel::DEBUG";
    case LogLevel::TRACE:
        return "LogLevel::TRACE";
    }

    return "[Undefined LogLevel]";
}
} // namespace log
} // namespace iox

#endif // IOX_HOOFS_PRIMITIVES_ICEORYX_HOOFS_TYPES_INL
