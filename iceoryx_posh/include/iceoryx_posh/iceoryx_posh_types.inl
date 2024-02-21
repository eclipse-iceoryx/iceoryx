// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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
#ifndef IOX_POSH_ICEORYX_POSH_TYPES_INL
#define IOX_POSH_ICEORYX_POSH_TYPES_INL

namespace iox
{
inline constexpr const char* asStringLiteral(ConnectionState value) noexcept
{
    switch (value)
    {
    case ConnectionState::NOT_CONNECTED:
        return "ConnectionState::NOT_CONNECTED";
    case ConnectionState::CONNECT_REQUESTED:
        return "ConnectionState::CONNECT_REQUESTED";
    case ConnectionState::WAIT_FOR_OFFER:
        return "ConnectionState::WAIT_FOR_OFFER";
    case ConnectionState::CONNECTED:
        return "ConnectionState::CONNECTED";
    case ConnectionState::DISCONNECT_REQUESTED:
        return "ConnectionState::DISCONNECT_REQUESTED";
    }

    return "[Undefined ConnectionState]";
}

std::ostream& operator<<(std::ostream& stream, ConnectionState value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

log::LogStream& operator<<(log::LogStream& stream, ConnectionState value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

ResourcePrefix_t iceoryxResourcePrefix(uint16_t uniqueRouDiID, ResourceType resourceType)
{
    static_assert(std::is_same_v<uint16_t, std::remove_const_t<decltype(uniqueRouDiID)>>,
                  "Please adjust 'MAX_UINT16_WIDTH' to the new fixed width type to have enough space for the "
                  "stringified unique RouDi ID");
    constexpr auto MAX_UINT16_WIDTH{5};
    iox::string<MAX_UINT16_WIDTH> uniqueRoudiIdString{TruncateToCapacity,
                                                      iox::convert::toString(uniqueRouDiID).c_str()};

    auto resourceTypeString{resourceType == ResourceType::ICEORYX_DEFINED ? iox::string<1>{"i"} : iox::string<1>{"u"}};
    return concatenate(ICEORYX_RESOURCE_PREFIX, "_", uniqueRoudiIdString, "_", resourceTypeString, "_");
}

namespace roudi
{
inline iox::log::LogStream& operator<<(iox::log::LogStream& logstream, const MonitoringMode& mode) noexcept
{
    switch (mode)
    {
    case MonitoringMode::OFF:
        logstream << "MonitoringMode::OFF";
        break;
    case MonitoringMode::ON:
        logstream << "MonitoringMode::ON";
        break;
    default:
        logstream << "MonitoringMode::UNDEFINED";
        break;
    }
    return logstream;
}
} // namespace roudi

} // namespace iox

#endif // IOX_POSH_ICEORYX_POSH_TYPES_INL
