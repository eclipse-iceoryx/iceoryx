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

#pragma once

#include "iceoryx_utils/log/logcommon.hpp"
#include "iceoryx_utils/log/logger.hpp"
#include "iceoryx_utils/log/logstream.hpp"

#include <chrono>
#include <string>

namespace iox
{
namespace log
{
Logger& CreateLogger(std::string ctxId, std::string ctxDescription, LogLevel appDefLogLevel = LogLevel::kWarn) noexcept;

inline constexpr LogHex8 HexFormat(uint8_t value)
{
    return LogHex8(value);
}
inline constexpr LogHex8 HexFormat(int8_t value)
{
    return LogHex8(static_cast<uint8_t>(value));
}
inline constexpr LogHex16 HexFormat(uint16_t value)
{
    return LogHex16(value);
}
inline constexpr LogHex16 HexFormat(int16_t value)
{
    return LogHex16(static_cast<uint16_t>(value));
}
inline constexpr LogHex32 HexFormat(uint32_t value)
{
    return LogHex32(value);
}
inline constexpr LogHex32 HexFormat(int32_t value)
{
    return LogHex32(static_cast<uint32_t>(value));
}
inline constexpr LogHex64 HexFormat(uint64_t value)
{
    return LogHex64(value);
}
inline constexpr LogHex64 HexFormat(int64_t value)
{
    return LogHex64(static_cast<uint64_t>(value));
}

inline constexpr LogBin8 BinFormat(uint8_t value)
{
    return LogBin8(value);
}
inline constexpr LogBin8 BinFormat(int8_t value)
{
    return LogBin8(static_cast<uint8_t>(value));
}
inline constexpr LogBin16 BinFormat(uint16_t value)
{
    return LogBin16(value);
}
inline constexpr LogBin16 BinFormat(int16_t value)
{
    return LogBin16(static_cast<uint16_t>(value));
}
inline constexpr LogBin32 BinFormat(uint32_t value)
{
    return LogBin32(value);
}
inline constexpr LogBin32 BinFormat(int32_t value)
{
    return LogBin32(static_cast<uint32_t>(value));
}
inline constexpr LogBin64 BinFormat(uint64_t value)
{
    return LogBin64(value);
}
inline constexpr LogBin64 BinFormat(int64_t value)
{
    return LogBin64(static_cast<uint64_t>(value));
}

template <typename T>
inline constexpr LogRawBuffer RawBuffer(const T& value) noexcept
{
    // LogRawBuffer is used with the streaming operator which converts the data into a string,
    // therefore we shouldn't get lifetime issues
    return LogRawBuffer{reinterpret_cast<const uint8_t*>(&value), sizeof(T)};
}

} // namespace log
} // namespace iox

