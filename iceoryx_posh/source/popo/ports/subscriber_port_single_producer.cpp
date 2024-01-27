// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace popo
{
SubscriberPortSingleProducer::SubscriberPortSingleProducer(not_null<MemberType_t* const> subscriberPortDataPtr) noexcept
    : SubscriberPortRouDi(subscriberPortDataPtr)
{
}

optional<capro::CaproMessage> SubscriberPortSingleProducer::tryGetCaProMessage() noexcept
{
    // get subscribe request from user side
    const auto currentSubscribeRequest = getMembers()->m_subscribeRequested.load(std::memory_order_relaxed);

    const auto currentSubscriptionState = getMembers()->m_subscriptionState.load(std::memory_order_relaxed);

    if (currentSubscribeRequest && (SubscribeState::NOT_SUBSCRIBED == currentSubscriptionState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::SUBSCRIBE_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::SUB, BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);
        caproMessage.m_historyCapacity = getMembers()->m_options.historyRequest;

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    else if (!currentSubscribeRequest && (SubscribeState::SUBSCRIBED == currentSubscriptionState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::UNSUBSCRIBE_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::UNSUB, BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    else if (!currentSubscribeRequest && (SubscribeState::WAIT_FOR_OFFER == currentSubscriptionState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::NOT_SUBSCRIBED, std::memory_order_relaxed);
        return nullopt_t();
    }
    else
    {
        // nothing to change
        return nullopt_t();
    }
}

optional<capro::CaproMessage> SubscriberPortSingleProducer::dispatchCaProMessageAndGetPossibleResponse(
    const capro::CaproMessage& caProMessage) noexcept
{
    const auto currentSubscriptionState = getMembers()->m_subscriptionState.load(std::memory_order_relaxed);

    if ((capro::CaproMessageType::OFFER == caProMessage.m_type)
        && (SubscribeState::WAIT_FOR_OFFER == currentSubscriptionState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::SUBSCRIBE_REQUESTED, std::memory_order_relaxed);

        capro::CaproMessage caproMessage(capro::CaproMessageType::SUB, BasePort::getMembers()->m_serviceDescription);
        caproMessage.m_chunkQueueData = static_cast<void*>(&getMembers()->m_chunkReceiverData);
        caproMessage.m_historyCapacity = getMembers()->m_options.historyRequest;

        return make_optional<capro::CaproMessage>(caproMessage);
    }
    else if ((capro::CaproMessageType::STOP_OFFER == caProMessage.m_type)
             && (SubscribeState::SUBSCRIBED == currentSubscriptionState))
    {
        getMembers()->m_subscriptionState.store(SubscribeState::WAIT_FOR_OFFER, std::memory_order_relaxed);

        return nullopt_t();
    }
    else if (capro::CaproMessageType::ACK == caProMessage.m_type)
    {
        if (SubscribeState::SUBSCRIBE_REQUESTED == currentSubscriptionState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::SUBSCRIBED, std::memory_order_relaxed);
        }
        else if (SubscribeState::UNSUBSCRIBE_REQUESTED == currentSubscriptionState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::NOT_SUBSCRIBED, std::memory_order_relaxed);
        }
        else
        {
            IOX_REPORT_FATAL(PoshError::POPO__CAPRO_PROTOCOL_ERROR);
        }

        return nullopt_t();
    }
    else if (capro::CaproMessageType::NACK == caProMessage.m_type)
    {
        if (SubscribeState::SUBSCRIBE_REQUESTED == currentSubscriptionState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::WAIT_FOR_OFFER, std::memory_order_relaxed);
        }
        else if (SubscribeState::UNSUBSCRIBE_REQUESTED == currentSubscriptionState)
        {
            getMembers()->m_subscriptionState.store(SubscribeState::NOT_SUBSCRIBED, std::memory_order_relaxed);
        }
        else
        {
            IOX_REPORT_FATAL(PoshError::POPO__CAPRO_PROTOCOL_ERROR);
        }

        return nullopt_t();
    }
    else if (((capro::CaproMessageType::OFFER == caProMessage.m_type)
              || (capro::CaproMessageType::STOP_OFFER == caProMessage.m_type))
             && (SubscribeState::NOT_SUBSCRIBED == currentSubscriptionState))
    {
        // No state change
        return nullopt_t();
    }
    else
    {
        IOX_REPORT_FATAL(PoshError::POPO__CAPRO_PROTOCOL_ERROR);
    }
}

} // namespace popo
} // namespace iox
