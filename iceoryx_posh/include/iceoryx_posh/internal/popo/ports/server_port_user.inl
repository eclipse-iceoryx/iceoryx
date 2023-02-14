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

#ifndef IOX_POSH_POPO_PORTS_SERVER_PORT_USER_INL
#define IOX_POSH_POPO_PORTS_SERVER_PORT_USER_INL

#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"

namespace iox
{
template <>
constexpr popo::ServerRequestResult
from<popo::ChunkReceiveResult, popo::ServerRequestResult>(const popo::ChunkReceiveResult value) noexcept
{
    switch (value)
    {
    case popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL:
        return popo::ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL;
    case popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE:
        return popo::ServerRequestResult::NO_PENDING_REQUESTS;
    }
    return popo::ServerRequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR;
}

namespace popo
{
inline constexpr const char* asStringLiteral(const ServerRequestResult value) noexcept
{
    switch (value)
    {
    case ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL:
        return "ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL";
    case ServerRequestResult::NO_PENDING_REQUESTS:
        return "ServerRequestResult::NO_PENDING_REQUESTS";
    case ServerRequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR:
        return "ServerRequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR";
    case ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER:
        return "ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER";
    }

    return "[Undefined ServerRequestResult]";
}

inline std::ostream& operator<<(std::ostream& stream, ServerRequestResult value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

inline log::LogStream& operator<<(log::LogStream& stream, ServerRequestResult value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

inline constexpr const char* asStringLiteral(const ServerSendError value) noexcept
{
    switch (value)
    {
    case ServerSendError::NOT_OFFERED:
        return "ServerSendError::NOT_OFFERED";
    case ServerSendError::CLIENT_NOT_AVAILABLE:
        return "ServerSendError::CLIENT_NOT_AVAILABLE";
    case ServerSendError::INVALID_RESPONSE:
        return "ServerSendError::INVALID_RESPONSE";
    }

    return "[Undefined ServerSendError]";
}

inline std::ostream& operator<<(std::ostream& stream, ServerSendError value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

inline log::LogStream& operator<<(log::LogStream& stream, ServerSendError value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_SERVER_PORT_USER_INL
