// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by Michael Bentley <mikebentley15@gmail.com>. All rights reserved.
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

#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/binding_c_error_reporting.hpp"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iox/logging.hpp"

namespace c2cpp
{
iox::popo::ConsumerTooSlowPolicy consumerTooSlowPolicy(const enum iox_ConsumerTooSlowPolicy policy) noexcept
{
    switch (policy)
    {
    case ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER:
        return iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    case ConsumerTooSlowPolicy_DISCARD_OLDEST_DATA:
        return iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;
    }

    IOX_REPORT(iox::CBindingError::BINDING_C__UNDEFINED_STATE_IN_IOX_CONSUMER_TOO_SLOW_POLICY, iox::er::RUNTIME_ERROR);
    return iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;
}

iox::popo::QueueFullPolicy queueFullPolicy(const enum iox_QueueFullPolicy policy) noexcept
{
    switch (policy)
    {
    case QueueFullPolicy_BLOCK_PRODUCER:
        return iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    case QueueFullPolicy_DISCARD_OLDEST_DATA:
        return iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA;
    }

    IOX_REPORT(iox::CBindingError::BINDING_C__UNDEFINED_STATE_IN_IOX_QUEUE_FULL_POLICY, iox::er::RUNTIME_ERROR);
    return iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA;
}

iox::popo::SubscriberEvent subscriberEvent(const iox_SubscriberEvent value) noexcept
{
    switch (value)
    {
    case SubscriberEvent_DATA_RECEIVED:
        return iox::popo::SubscriberEvent::DATA_RECEIVED;
    }

    IOX_LOG(FATAL, "invalid iox_SubscriberEvent value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_EVENT_VALUE);
    return iox::popo::SubscriberEvent::DATA_RECEIVED;
}

iox::popo::SubscriberState subscriberState(const iox_SubscriberState value) noexcept
{
    switch (value)
    {
    case SubscriberState_HAS_DATA:
        return iox::popo::SubscriberState::HAS_DATA;
    }

    IOX_LOG(FATAL, "invalid iox_SubscriberState value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_STATE_VALUE);
    return iox::popo::SubscriberState::HAS_DATA;
}

iox::popo::ClientEvent clientEvent(const iox_ClientEvent value) noexcept
{
    switch (value)
    {
    case ClientEvent_RESPONSE_RECEIVED:
        return iox::popo::ClientEvent::RESPONSE_RECEIVED;
    }

    IOX_LOG(FATAL, "invalid iox_ClientEvent value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_CLIENT_EVENT_VALUE);
    return iox::popo::ClientEvent::RESPONSE_RECEIVED;
}

iox::popo::ClientState clientState(const iox_ClientState value) noexcept
{
    switch (value)
    {
    case ClientState_HAS_RESPONSE:
        return iox::popo::ClientState::HAS_RESPONSE;
    }

    IOX_LOG(FATAL, "invalid iox_ClientState value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_CLIENT_STATE_VALUE);
    return iox::popo::ClientState::HAS_RESPONSE;
}

iox::popo::ServerEvent serverEvent(const iox_ServerEvent value) noexcept
{
    switch (value)
    {
    case ServerEvent_REQUEST_RECEIVED:
        return iox::popo::ServerEvent::REQUEST_RECEIVED;
    }

    IOX_LOG(FATAL, "invalid iox_ServerEvent value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVER_EVENT_VALUE);
    return iox::popo::ServerEvent::REQUEST_RECEIVED;
}

iox::popo::ServerState serverState(const iox_ServerState value) noexcept
{
    switch (value)
    {
    case ServerState_HAS_REQUEST:
        return iox::popo::ServerState::HAS_REQUEST;
    }

    IOX_LOG(FATAL, "invalid iox_ServerState value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVER_STATE_VALUE);
    return iox::popo::ServerState::HAS_REQUEST;
}

iox::runtime::ServiceDiscoveryEvent serviceDiscoveryEvent(const iox_ServiceDiscoveryEvent value) noexcept
{
    switch (value)
    {
    case ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED:
        return iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED;
    }

    IOX_LOG(FATAL, "invalid iox_ServiceDiscoveryEvent value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVICE_DISCOVERY_EVENT_VALUE);
    return iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED;
}

iox::popo::MessagingPattern messagingPattern(const iox_MessagingPattern value) noexcept
{
    switch (value)
    {
    case MessagingPattern_PUB_SUB:
        return iox::popo::MessagingPattern::PUB_SUB;
    case MessagingPattern_REQ_RES:
        return iox::popo::MessagingPattern::REQ_RES;
    }

    IOX_LOG(FATAL, "invalid iox_MessagingPattern value");
    IOX_REPORT_FATAL(iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_MESSAGING_PATTERN_VALUE);
    return iox::popo::MessagingPattern::PUB_SUB;
}

} // namespace c2cpp
