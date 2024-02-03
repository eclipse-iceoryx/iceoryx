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

#include "iceoryx_posh/internal/popo/ports/server_port_roudi.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace popo
{
ServerPortRouDi::ServerPortRouDi(MemberType_t& serverPortData) noexcept
    : BasePort(&serverPortData)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
    , m_chunkReceiver(&getMembers()->m_chunkReceiverData)

{
}

const ServerPortRouDi::MemberType_t* ServerPortRouDi::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ServerPortRouDi::MemberType_t* ServerPortRouDi::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

QueueFullPolicy ServerPortRouDi::getRequestQueueFullPolicy() const noexcept
{
    return getMembers()->m_chunkReceiverData.m_queueFullPolicy;
}

ConsumerTooSlowPolicy ServerPortRouDi::getClientTooSlowPolicy() const noexcept
{
    return getMembers()->m_chunkSenderData.m_consumerTooSlowPolicy;
}

optional<capro::CaproMessage> ServerPortRouDi::tryGetCaProMessage() noexcept
{
    // get offer state request from user side
    const auto offeringRequested = getMembers()->m_offeringRequested.load(std::memory_order_relaxed);

    const auto isOffered = getMembers()->m_offered.load(std::memory_order_relaxed);

    if (isOffered)
    {
        if (!offeringRequested)
        {
            capro::CaproMessage caproMessage(capro::CaproMessageType::STOP_OFFER, this->getCaProServiceDescription());
            caproMessage.m_serviceType = capro::CaproServiceType::SERVER;
            return dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        }
    }
    else
    {
        if (offeringRequested)
        {
            capro::CaproMessage caproMessage(capro::CaproMessageType::OFFER, this->getCaProServiceDescription());
            caproMessage.m_serviceType = capro::CaproServiceType::SERVER;
            return dispatchCaProMessageAndGetPossibleResponse(caproMessage);
        }
    }

    // nothing to change
    return nullopt;
}

optional<capro::CaproMessage>
ServerPortRouDi::dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& caProMessage) noexcept
{
    const auto isOffered = getMembers()->m_offered.load(std::memory_order_relaxed);

    return isOffered ? handleCaProMessageForStateOffered(caProMessage)
                     : handleCaProMessageForStateNotOffered(caProMessage);
}

void ServerPortRouDi::handleCaProProtocolViolation(const capro::CaproMessageType messageType) const noexcept
{
    // this shouldn't be reached
    IOX_LOG(FATAL,
            "CaPro Protocol Violation! Got '" << messageType << "' with offer state '"
                                              << (getMembers()->m_offered ? "OFFERED" : "NOT OFFERED") << "'!");
    IOX_REPORT_FATAL(PoshError::POPO__CAPRO_PROTOCOL_ERROR);
}

optional<capro::CaproMessage>
ServerPortRouDi::handleCaProMessageForStateOffered(const capro::CaproMessage& caProMessage) noexcept
{
    capro::CaproMessage responseMessage{capro::CaproMessageType::NACK, this->getCaProServiceDescription()};

    switch (caProMessage.m_type)
    {
    case capro::CaproMessageType::STOP_OFFER:
        getMembers()->m_offered.store(false, std::memory_order_relaxed);
        m_chunkSender.removeAllQueues();
        return caProMessage;
    case capro::CaproMessageType::OFFER:
        return responseMessage;
    case capro::CaproMessageType::CONNECT:
        if (caProMessage.m_chunkQueueData == nullptr)
        {
            IOX_LOG(WARN, "No client response queue passed to server");
            IOX_REPORT(PoshError::POPO__SERVER_PORT_NO_CLIENT_RESPONSE_QUEUE_TO_CONNECT, iox::er::RUNTIME_ERROR);
        }
        else
        {
            m_chunkSender
                .tryAddQueue(static_cast<ClientChunkQueueData_t*>(caProMessage.m_chunkQueueData),
                             caProMessage.m_historyCapacity)
                .and_then([this, &responseMessage]() {
                    responseMessage.m_type = capro::CaproMessageType::ACK;
                    responseMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);
                    responseMessage.m_historyCapacity = 0;
                });
        }
        return responseMessage;
    case capro::CaproMessageType::DISCONNECT:
        m_chunkSender.tryRemoveQueue(static_cast<ClientChunkQueueData_t*>(caProMessage.m_chunkQueueData))
            .and_then([&responseMessage]() { responseMessage.m_type = capro::CaproMessageType::ACK; });
        return responseMessage;
    default:
        // leave switch statement and handle protocol violation
        break;
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

optional<capro::CaproMessage>
ServerPortRouDi::handleCaProMessageForStateNotOffered(const capro::CaproMessage& caProMessage) noexcept
{
    switch (caProMessage.m_type)
    {
    case capro::CaproMessageType::OFFER:
        getMembers()->m_offered.store(true, std::memory_order_relaxed);
        return caProMessage;
    case capro::CaproMessageType::STOP_OFFER:
        [[fallthrough]];
    case capro::CaproMessageType::CONNECT:
        [[fallthrough]];
    case capro::CaproMessageType::DISCONNECT:
        return capro::CaproMessage(capro::CaproMessageType::NACK, this->getCaProServiceDescription());
    default:
        // leave switch statement and handle protocol violation
        break;
    }

    handleCaProProtocolViolation(caProMessage.m_type);
    return nullopt;
}

void ServerPortRouDi::releaseAllChunks() noexcept
{
    m_chunkSender.releaseAll();
    m_chunkReceiver.releaseAll();
}

} // namespace popo
} // namespace iox
