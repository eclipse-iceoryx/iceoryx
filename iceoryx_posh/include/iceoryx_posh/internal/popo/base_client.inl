// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_BASE_CLIENT_INL
#define IOX_POSH_POPO_BASE_CLIENT_INL

#include "iceoryx_posh/internal/popo/base_client.hpp"

namespace iox
{
namespace popo
{
// ============================== BaseClient ============================== //

template <typename Port>
inline BaseClient<Port>::BaseClient(const capro::ServiceDescription& service,
                                    const ClientOptions& clientOptions) noexcept
    : m_port(*iox::runtime::PoshRuntime::getInstance().getMiddlewareClient(service, clientOptions))
{
}

template <typename Port>
inline BaseClient<Port>::~BaseClient() noexcept
{
    m_port.destroy();
}

template <typename Port>
inline uid_t BaseClient<Port>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename Port>
inline const capro::ServiceDescription& BaseClient<Port>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename Port>
inline void BaseClient<Port>::connect() noexcept
{
    m_port.connect();
}

template <typename Port>
inline ConnectionState BaseClient<Port>::getConnectionState() const noexcept
{
    return m_port.getConnectionState();
}

template <typename Port>
inline void BaseClient<Port>::disconnect() noexcept
{
    m_port.disconnect();
}

template <typename Port>
inline bool BaseClient<Port>::hasResponses() const noexcept
{
    return m_port.hasNewResponses();
}

template <typename Port>
inline bool BaseClient<Port>::hasMissedResponses() noexcept
{
    return m_port.hasLostResponsesSinceLastCall();
}

template <typename Port>
inline void BaseClient<Port>::releaseQueuedResponses() noexcept
{
    m_port.releaseQueuedResponses();
}

template <typename Port>
inline void BaseClient<Port>::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        m_port.unsetConditionVariable();
        m_trigger.invalidate();
    }
}

template <typename Port>
inline void BaseClient<Port>::enableState(iox::popo::TriggerHandle&& triggerHandle,
                                          const ClientState clientState) noexcept
{
    switch (clientState)
    {
    case ClientState::HAS_RESPONSE:
        if (m_trigger)
        {
            LogWarn()
                << "The client is already attached with either the ClientState::HAS_RESPONSE or "
                   "ClientEvent::RESPONSE_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ClientState::HAS_RESPONSE. Best practice is to call detach first.";

            errorHandler(
                Error::
                    kPOPO__BASE_CLIENT_OVERRIDING_WITH_STATE_SINCE_HAS_RESPONSE_OR_RESPONSE_RECEIVED_ALREADY_ATTACHED,
                nullptr,
                ErrorLevel::MODERATE);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}

template <typename Port>
inline WaitSetIsConditionSatisfiedCallback
BaseClient<Port>::getCallbackForIsStateConditionSatisfied(const ClientState clientState) const noexcept
{
    switch (clientState)
    {
    case ClientState::HAS_RESPONSE:
        return {*this, &SelfType::hasResponses};
    }
    return {};
}

template <typename Port>
inline void BaseClient<Port>::disableState(const ClientState clientState) noexcept
{
    switch (clientState)
    {
    case ClientState::HAS_RESPONSE:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename Port>
inline void BaseClient<Port>::enableEvent(iox::popo::TriggerHandle&& triggerHandle,
                                          const ClientEvent clientEvent) noexcept
{
    switch (clientEvent)
    {
    case ClientEvent::RESPONSE_RECEIVED:
        if (m_trigger)
        {
            LogWarn()
                << "The client is already attached with either the ClientState::HAS_RESPONSE or "
                   "ClientEvent::RESPONSE_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ClientEvent::RESPONSE_RECEIVED. Best practice is to call detach "
                   "first.";
            errorHandler(
                Error::
                    kPOPO__BASE_CLIENT_OVERRIDING_WITH_EVENT_SINCE_HAS_RESPONSE_OR_RESPONSE_RECEIVED_ALREADY_ATTACHED,
                nullptr,
                ErrorLevel::MODERATE);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}

template <typename Port>
inline void BaseClient<Port>::disableEvent(const ClientEvent clientEvent) noexcept
{
    switch (clientEvent)
    {
    case ClientEvent::RESPONSE_RECEIVED:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename Port>
inline const Port& BaseClient<Port>::port() const noexcept
{
    return m_port;
}

template <typename Port>
inline Port& BaseClient<Port>::port() noexcept
{
    return m_port;
}


} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_CLIENT_INL
