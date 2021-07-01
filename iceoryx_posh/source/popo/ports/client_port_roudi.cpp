// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"

namespace iox
{
namespace popo
{
ClientPortRouDi::ClientPortRouDi(MemberType_t& clientPortData) noexcept
    : BasePort(&clientPortData)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
    , m_chunkReceiver(&getMembers()->m_chunkReceiverData)

{
}

const ClientPortRouDi::MemberType_t* ClientPortRouDi::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ClientPortRouDi::MemberType_t* ClientPortRouDi::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

QueueFullPolicy2 ClientPortRouDi::getResponseQueueFullPolicy() const noexcept
{
    return static_cast<QueueFullPolicy2>(getMembers()->m_chunkReceiverData.m_queueFullPolicy);
}

cxx::optional<capro::CaproMessage> ClientPortRouDi::tryGetCaProMessage() noexcept
{
    // get subscribe request from user side
    const auto currentConnectRequest = getMembers()->m_connectRequested.load(std::memory_order_relaxed);

    const auto currentConnectionState = getMembers()->m_connectionState.load(std::memory_order_relaxed);

    if (currentConnectRequest && (ConnectionState::NOT_CONNECTED == currentConnectionState))
    {
        getMembers()->m_connectionState.store(ConnectionState::CONNECT_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::CONNECT,
                                         BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);
        caproMessage.m_historyCapacity = 0;

        return cxx::make_optional<capro::CaproMessage>(caproMessage);
    }
    else if (!currentConnectRequest && (ConnectionState::CONNECT_REQUESTED == currentConnectionState))
    {
        getMembers()->m_connectionState.store(ConnectionState::NOT_CONNECTED, std::memory_order_relaxed);

        // nothing to change
        return cxx::nullopt_t();
    }
    else if (!currentConnectRequest && (ConnectionState::CONNECTED == currentConnectionState))
    {
        getMembers()->m_connectionState.store(ConnectionState::DISCONNECT_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::DISCONNECT,
                                         BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);

        return cxx::make_optional<capro::CaproMessage>(caproMessage);
    }
    else
    {
        // nothing to change
        return cxx::nullopt_t();
    }
}

cxx::optional<capro::CaproMessage>
ClientPortRouDi::dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& caProMessage) noexcept
{
    const auto currentConnectionState = getMembers()->m_connectionState.load(std::memory_order_relaxed);

    switch (currentConnectionState)
    {
    case ConnectionState::NOT_CONNECTED:
        switch (caProMessage.m_type)
        {
        case capro::CaproMessageType::CONNECT:
            /// @todo iox-#27 the stuff from tryGetCaProMessage should be done here
            break;
        case capro::CaproMessageType::OFFER:
            return cxx::nullopt_t();
        default:
            break;
        }
        break;
    case ConnectionState::CONNECT_REQUESTED:
        switch (caProMessage.m_type)
        {
        case capro::CaproMessageType::ACK:
            cxx::Expects(caProMessage.m_chunkQueueData != nullptr && "Invalid request queue passed to client");
            cxx::Expects(!m_chunkSender
                              .tryAddQueue(static_cast<ServerChunkQueueData_t*>(caProMessage.m_chunkQueueData),
                                           caProMessage.m_historyCapacity)
                              .has_error());

            getMembers()->m_connectionState.store(ConnectionState::CONNECTED, std::memory_order_relaxed);
            return cxx::nullopt_t();

        case capro::CaproMessageType::NACK:
            getMembers()->m_connectionState.store(ConnectionState::WAIT_FOR_OFFER, std::memory_order_relaxed);
            return cxx::nullopt_t();
        default:
            break;
        }
        break;
    case ConnectionState::WAIT_FOR_OFFER:
        switch (caProMessage.m_type)
        {
        case capro::CaproMessageType::OFFER:
        {
            getMembers()->m_connectionState.store(ConnectionState::CONNECT_REQUESTED, std::memory_order_relaxed);

            capro::CaproMessage caproMessage(capro::CaproMessageType::CONNECT,
                                             BasePort::getMembers()->m_serviceDescription);
            caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);
            caproMessage.m_historyCapacity = 0;
        }
        break;
        case capro::CaproMessageType::DISCONNECT:
            getMembers()->m_connectionState.store(ConnectionState::NOT_CONNECTED, std::memory_order_relaxed);
            return cxx::nullopt_t();
        default:
            break;
        }
        break;
    case ConnectionState::CONNECTED:
        switch (caProMessage.m_type)
        {
        case capro::CaproMessageType::STOP_OFFER:
            getMembers()->m_connectionState.store(ConnectionState::WAIT_FOR_OFFER, std::memory_order_relaxed);
            return cxx::nullopt_t();
        case capro::CaproMessageType::DISCONNECT:
            /// @todo iox-#27 the stuff from tryGetCaProMessage should be done here
            return cxx::nullopt_t();
        default:
            break;
        }
        break;
    case ConnectionState::DISCONNECT_REQUESTED:
        switch (caProMessage.m_type)
        {
        case capro::CaproMessageType::ACK:
            IOX_FALLTHROUGH;
        case capro::CaproMessageType::NACK:
            getMembers()->m_connectionState.store(ConnectionState::NOT_CONNECTED, std::memory_order_relaxed);
            return cxx::nullopt_t();
        default:
            break;
        }
        break;
    }

    // this shouldn't be reached
    LogFatal() << "CaPro Protocol Violation! ConnectionState: " << cxx::enumTypeAsUnderlyingType(currentConnectionState)
               << " CaproMessageType: " << capro::caproMessageTypeString(caProMessage.m_type);
    errorHandler(Error::kPOPO__CAPRO_PROTOCOL_ERROR, nullptr, ErrorLevel::SEVERE);
    return cxx::nullopt_t();
}

void ClientPortRouDi::releaseAllChunks() noexcept
{
    m_chunkSender.releaseAll();
    m_chunkReceiver.releaseAll();
}

} // namespace popo
} // namespace iox
