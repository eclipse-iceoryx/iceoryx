// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/assertions.hpp"

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

QueueFullPolicy ClientPortRouDi::getResponseQueueFullPolicy() const noexcept
{
    return getMembers()->m_chunkReceiverData.m_queueFullPolicy;
}

ConsumerTooSlowPolicy ClientPortRouDi::getServerTooSlowPolicy() const noexcept
{
    return getMembers()->m_chunkSenderData.m_consumerTooSlowPolicy;
}

optional<capro::CaproMessage> ClientPortRouDi::tryGetCaProMessage() noexcept
{
    // get connect request from user side
    const auto currentConnectRequest = getMembers()->m_connectRequested.load(std::memory_order_relaxed);

    const auto currentConnectionState = getMembers()->m_connectionState.load(std::memory_order_relaxed);

    switch (currentConnectionState)
    {
    case ConnectionState::NOT_CONNECTED:
        if (currentConnectRequest)
        {
            capro::CaproMessage caproMessage(capro::CaproMessageType::CONNECT,
                                             BasePort::getMembers()->m_serviceDescription);
            return dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        }
        break;
    case ConnectionState::WAIT_FOR_OFFER:
        [[fallthrough]];
    case ConnectionState::CONNECTED:
        if (!currentConnectRequest)
        {
            capro::CaproMessage caproMessage(capro::CaproMessageType::DISCONNECT,
                                             BasePort::getMembers()->m_serviceDescription);
            return dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        }
        break;
    default:
        // do nothing
        break;
    }

    // nothing to change
    return nullopt;
}

optional<capro::CaproMessage>
ClientPortRouDi::dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& caProMessage) noexcept
{
    const auto currentConnectionState = getMembers()->m_connectionState.load(std::memory_order_relaxed);

    switch (currentConnectionState)
    {
    case ConnectionState::NOT_CONNECTED:
        return handleCaProMessageForStateNotConnected(caProMessage);
    case ConnectionState::CONNECT_REQUESTED:
        return handleCaProMessageForStateConnectRequested(caProMessage);
    case ConnectionState::WAIT_FOR_OFFER:
        return handleCaProMessageForStateWaitForOffer(caProMessage);
    case ConnectionState::CONNECTED:
        return handleCaProMessageForStateConnected(caProMessage);
    case ConnectionState::DISCONNECT_REQUESTED:
        return handleCaProMessageForStateDisconnectRequested(caProMessage);
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

void ClientPortRouDi::handleCaProProtocolViolation(const iox::capro::CaproMessageType messageType) noexcept
{
    // this shouldn't be reached
    IOX_LOG(FATAL,
            "CaPro Protocol Violation! Got '"
                << messageType << "' in '" << getMembers()->m_connectionState.load(std::memory_order_relaxed) << "'");
    IOX_REPORT_FATAL(PoshError::POPO__CAPRO_PROTOCOL_ERROR);
}

optional<capro::CaproMessage>
ClientPortRouDi::handleCaProMessageForStateNotConnected(const capro::CaproMessage& caProMessage) noexcept
{
    switch (caProMessage.m_type)
    {
    case capro::CaproMessageType::CONNECT:
    {
        getMembers()->m_connectionState.store(ConnectionState::CONNECT_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::CONNECT,
                                         BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);
        caproMessage.m_historyCapacity = 0;

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    case capro::CaproMessageType::OFFER:
        return nullopt;
    default:
        // handle capro protocol violation outside of switch statement
        break;
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

optional<capro::CaproMessage>
ClientPortRouDi::handleCaProMessageForStateConnectRequested(const capro::CaproMessage& caProMessage) noexcept
{
    switch (caProMessage.m_type)
    {
    case capro::CaproMessageType::ACK:
        IOX_ENFORCE(caProMessage.m_chunkQueueData != nullptr, "Invalid request queue passed to client");
        m_chunkSender
            .tryAddQueue(static_cast<ServerChunkQueueData_t*>(caProMessage.m_chunkQueueData),
                         caProMessage.m_historyCapacity)
            .expect("Adding server request queue to client");

        getMembers()->m_connectionState.store(ConnectionState::CONNECTED, std::memory_order_relaxed);
        return nullopt;
    case capro::CaproMessageType::NACK:
        getMembers()->m_connectionState.store(ConnectionState::WAIT_FOR_OFFER, std::memory_order_relaxed);
        return nullopt;
    default:
        // handle capro protocol violation outside of switch statement
        break;
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

optional<capro::CaproMessage>
ClientPortRouDi::handleCaProMessageForStateWaitForOffer(const capro::CaproMessage& caProMessage) noexcept
{
    switch (caProMessage.m_type)
    {
    case capro::CaproMessageType::OFFER:
    {
        getMembers()->m_connectionState.store(ConnectionState::CONNECT_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::CONNECT,
                                         BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);
        caproMessage.m_historyCapacity = 0;

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    case capro::CaproMessageType::DISCONNECT:
        getMembers()->m_connectionState.store(ConnectionState::NOT_CONNECTED, std::memory_order_relaxed);
        return nullopt;
    default:
        // handle capro protocol violation outside of switch statement
        break;
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

optional<capro::CaproMessage>
ClientPortRouDi::handleCaProMessageForStateConnected(const capro::CaproMessage& caProMessage) noexcept
{
    switch (caProMessage.m_type)
    {
    case capro::CaproMessageType::STOP_OFFER:
        getMembers()->m_connectionState.store(ConnectionState::WAIT_FOR_OFFER, std::memory_order_relaxed);
        m_chunkSender.removeAllQueues();
        return nullopt;
    case capro::CaproMessageType::DISCONNECT:
    {
        getMembers()->m_connectionState.store(ConnectionState::DISCONNECT_REQUESTED, std::memory_order_relaxed);
        m_chunkSender.removeAllQueues();

        capro::CaproMessage caproMessage(capro::CaproMessageType::DISCONNECT,
                                         BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    default:
        // handle capro protocol violation outside of switch statement
        break;
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

optional<capro::CaproMessage>
ClientPortRouDi::handleCaProMessageForStateDisconnectRequested(const capro::CaproMessage& caProMessage) noexcept
{
    switch (caProMessage.m_type)
    {
    case capro::CaproMessageType::ACK:
        [[fallthrough]];
    case capro::CaproMessageType::NACK:
        getMembers()->m_connectionState.store(ConnectionState::NOT_CONNECTED, std::memory_order_relaxed);
        return nullopt;
    default:
        // handle capro protocol violation outside of switch statement
        break;
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

void ClientPortRouDi::releaseAllChunks() noexcept
{
    m_chunkSender.releaseAll();
    m_chunkReceiver.releaseAll();
}

} // namespace popo
} // namespace iox
