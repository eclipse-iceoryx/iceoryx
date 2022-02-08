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

namespace iox
{
namespace cxx
{
template <>
constexpr popo::ServerPortUser::RequestResult
from<popo::ChunkReceiveResult, popo::ServerPortUser::RequestResult>(const popo::ChunkReceiveResult value) noexcept
{
    switch (value)
    {
    case popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL:
        return popo::ServerPortUser::RequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL;
    case popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE:
        return popo::ServerPortUser::RequestResult::NO_REQUESTS_PENDING;
    }
    return popo::ServerPortUser::RequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR;
}
} // namespace cxx

namespace popo
{
inline constexpr const char* asStringLiteral(const ServerPortUser::RequestResult value) noexcept
{
    switch (value)
    {
    case ServerPortUser::RequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL:
        return "ServerPortUser::RequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL";
    case ServerPortUser::RequestResult::NO_REQUESTS_PENDING:
        return "ServerPortUser::RequestResult::NO_REQUESTS_PENDING";
    case ServerPortUser::RequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR:
        return "ServerPortUser::RequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR";
    case ServerPortUser::RequestResult::NO_REQUESTS_PENDING_AND_SERVER_DOES_NOT_OFFER:
        return "ServerPortUser::RequestResult::NO_REQUESTS_PENDING_AND_SERVER_DOES_NOT_OFFER";
    }

    return "[Undefined ServerPortUser::RequestResult]";
}

inline std::ostream& operator<<(std::ostream& stream, ServerPortUser::RequestResult value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

inline log::LogStream& operator<<(log::LogStream& stream, ServerPortUser::RequestResult value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_SERVER_PORT_USER_INL
