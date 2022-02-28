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

#ifndef IOX_POSH_POPO_PORTS_CLIENT_PORT_USER_INL
#define IOX_POSH_POPO_PORTS_CLIENT_PORT_USER_INL

#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"

namespace iox
{
namespace popo
{
inline constexpr const char* asStringLiteral(const ClientSendError value) noexcept
{
    switch (value)
    {
    case ClientSendError::NO_CONNECT_REQUESTED:
        return "ClientSendError::NO_CONNECT_REQUESTED";
    case ClientSendError::SERVER_NOT_AVAILABLE:
        return "ClientSendError::SERVER_NOT_AVAILABLE";
    case ClientSendError::INVALID_REQUEST:
        return "ClientSendError::INVALID_REQUEST";
    }

    return "[Undefined ClientSendError]";
}

inline std::ostream& operator<<(std::ostream& stream, ClientSendError value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

inline log::LogStream& operator<<(log::LogStream& stream, ClientSendError value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_CLIENT_PORT_USER_INL
