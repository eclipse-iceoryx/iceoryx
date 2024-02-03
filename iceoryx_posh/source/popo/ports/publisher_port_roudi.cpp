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

#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace popo
{
PublisherPortRouDi::PublisherPortRouDi(not_null<MemberType_t* const> publisherPortDataPtr) noexcept
    : BasePort(publisherPortDataPtr)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
{
}

const PublisherOptions& PublisherPortRouDi::getOptions() const noexcept
{
    return getMembers()->m_options;
}

const PublisherPortRouDi::MemberType_t* PublisherPortRouDi::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

PublisherPortRouDi::MemberType_t* PublisherPortRouDi::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

optional<capro::CaproMessage> PublisherPortRouDi::tryGetCaProMessage() noexcept
{
    // get offer state request from user side
    const auto offeringRequested = getMembers()->m_offeringRequested.load(std::memory_order_relaxed);

    const auto isOffered = getMembers()->m_offered.load(std::memory_order_relaxed);

    if (offeringRequested && !isOffered)
    {
        getMembers()->m_offered.store(true, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::OFFER, this->getCaProServiceDescription());

        const auto historyCapacity = m_chunkSender.getHistoryCapacity();
        caproMessage.m_historyCapacity = historyCapacity;
        caproMessage.m_serviceType = capro::CaproServiceType::PUBLISHER;

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    else if ((!offeringRequested) && isOffered)
    {
        getMembers()->m_offered.store(false, std::memory_order_relaxed);

        // remove all the subscribers (represented by their chunk queues)
        m_chunkSender.removeAllQueues();

        capro::CaproMessage caproMessage(capro::CaproMessageType::STOP_OFFER, this->getCaProServiceDescription());
        caproMessage.m_serviceType = capro::CaproServiceType::PUBLISHER;

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    else
    {
        // nothing to change
        return nullopt_t();
    }
}

optional<capro::CaproMessage>
PublisherPortRouDi::dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& caProMessage) noexcept
{
    capro::CaproMessage responseMessage(
        capro::CaproMessageType::NACK, this->getCaProServiceDescription(), capro::CaproServiceType::NONE);

    if (getMembers()->m_offered.load(std::memory_order_relaxed))
    {
        if (capro::CaproMessageType::SUB == caProMessage.m_type)
        {
            const auto ret = m_chunkSender.tryAddQueue(
                static_cast<PublisherPortData::ChunkQueueData_t*>(caProMessage.m_chunkQueueData),
                caProMessage.m_historyCapacity);
            if (!ret.has_error())
            {
                responseMessage.m_type = capro::CaproMessageType::ACK;
            }
        }
        else if (capro::CaproMessageType::UNSUB == caProMessage.m_type)
        {
            const auto ret = m_chunkSender.tryRemoveQueue(
                static_cast<PublisherPortData::ChunkQueueData_t*>(caProMessage.m_chunkQueueData));
            if (!ret.has_error())
            {
                responseMessage.m_type = capro::CaproMessageType::ACK;
            }
        }
        else
        {
            IOX_REPORT_FATAL(PoshError::POPO__CAPRO_PROTOCOL_ERROR);
        }
    }

    return make_optional<capro::CaproMessage>(responseMessage);
}

void PublisherPortRouDi::releaseAllChunks() noexcept
{
    m_chunkSender.releaseAll();
}

} // namespace popo
} // namespace iox
