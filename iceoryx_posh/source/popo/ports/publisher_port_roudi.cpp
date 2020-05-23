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
// limitations under the License

#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"

namespace iox
{
namespace popo
{
PublisherPortRouDi::PublisherPortRouDi(cxx::not_null<MemberType_t* const> publisherPortDataPtr) noexcept
    : BasePort(publisherPortDataPtr)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
{
}

const PublisherPortRouDi::MemberType_t* PublisherPortRouDi::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

PublisherPortRouDi::MemberType_t* PublisherPortRouDi::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

cxx::optional<capro::CaproMessage> PublisherPortRouDi::getCaProMessage() noexcept
{
    // get offer state request from user side
    const auto offeringRequested = getMembers()->m_offeringRequested.load(std::memory_order_relaxed);

    const auto isOffered = getMembers()->m_offered.load(std::memory_order_relaxed);

    if (offeringRequested && !isOffered)
    {
        getMembers()->m_offered.store(true, std::memory_order_relaxed);

        capro::CaproMessage l_caproMessage(capro::CaproMessageType::OFFER, this->getCaProServiceDescription());
        // provide additional AUTOSAR Adaptive like information
        if (0 < m_chunkSender.getHistoryCapacity())
        {
            l_caproMessage.m_subType = capro::CaproMessageSubType::FIELD;
        }
        else
        {
            l_caproMessage.m_subType = capro::CaproMessageSubType::EVENT;
        }
        return cxx::make_optional<capro::CaproMessage>(l_caproMessage);
    }
    else if (!offeringRequested && isOffered)
    {
        getMembers()->m_offered.store(false, std::memory_order_relaxed);

        // remove all the subscribers (represented by their chunk queues)
        m_chunkSender.removeAllQueues();

        capro::CaproMessage l_caproMessage(capro::CaproMessageType::STOP_OFFER, this->getCaProServiceDescription());
        return cxx::make_optional<capro::CaproMessage>(l_caproMessage);
    }
    else
    {
        // nothing to change
        return cxx::nullopt_t();
    }
}

cxx::optional<capro::CaproMessage>
PublisherPortRouDi::dispatchCaProMessage(const capro::CaproMessage& caProMessage) noexcept
{
    capro::CaproMessage l_responseMessage(capro::CaproMessageType::NACK,
                                          this->getCaProServiceDescription(),
                                          capro::CaproMessageSubType::NOSUBTYPE,
                                          caProMessage.m_requestPort);

    /// TODO replacement m_requestPort, add history to CaPro message
    if (getMembers()->m_offered.load(std::memory_order_relaxed))
    {
        if (capro::CaproMessageType::SUB == caProMessage.m_type)
        {
            auto ret =
                m_chunkSender.addQueue(reinterpret_cast<PublisherPortData::ChunkDistributorData_t::ChunkQueueData_t*>(
                    caProMessage.m_requestPort));
            if (!ret.has_error())
            {
                l_responseMessage.m_type = capro::CaproMessageType::ACK;
            }
        }
        else if (capro::CaproMessageType::UNSUB == caProMessage.m_type)
        {
            m_chunkSender.removeQueue(reinterpret_cast<PublisherPortData::ChunkDistributorData_t::ChunkQueueData_t*>(
                caProMessage.m_requestPort));

            l_responseMessage.m_type = capro::CaproMessageType::ACK;
        }
    }

    return cxx::make_optional<capro::CaproMessage>(l_responseMessage);
}

void PublisherPortRouDi::cleanup() noexcept
{
    m_chunkSender.releaseAll();
}

} // namespace popo
} // namespace iox
