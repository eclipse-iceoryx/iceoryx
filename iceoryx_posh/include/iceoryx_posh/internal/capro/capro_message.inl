// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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


namespace iox
{
namespace capro
{
inline constexpr const char* caproMessageTypeString(CaproMessageType messageType) noexcept
{
    switch (messageType)
    {
    case CaproMessageType::NOTYPE:
        return "NOTYPE";
    case CaproMessageType::FIND:
        return "FIND";
    case CaproMessageType::OFFER:
        return "OFFER";
    case CaproMessageType::STOP_OFFER:
        return "STOP_OFFER";
    case CaproMessageType::SUB:
        return "SUB";
    case CaproMessageType::UNSUB:
        return "UNSUB";
    case CaproMessageType::CONNECT:
        return "CONNECT";
    case CaproMessageType::DISCONNECT:
        return "DISCONNECT";
    case CaproMessageType::HANDSHAKE:
        return "HANDSHAKE";
    case CaproMessageType::ACK:
        return "ACK";
    case CaproMessageType::NACK:
        return "NACK";
    case CaproMessageType::PUB:
        return "PUB";
    case CaproMessageType::REQ:
        return "REQ";
    case CaproMessageType::RES:
        return "RES";
    case CaproMessageType::PING:
        return "PING";
    case CaproMessageType::PONG:
        return "PONG";
    case CaproMessageType::MESSGAGE_TYPE_END:
        return "MESSGAGE_TYPE_END";
    }

    return "UNKNOWN_TYPE";
}

} // namespace capro
} // namespace iox
