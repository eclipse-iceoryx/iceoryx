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

#ifndef IOX_POSH_POPO_BASE_SERVER_INL
#define IOX_POSH_POPO_BASE_SERVER_INL

#include "iceoryx_posh/internal/popo/base_server.hpp"

namespace iox
{
namespace popo
{
template <typename Port>
inline BaseServer<Port>::BaseServer(const capro::ServiceDescription& service,
                                    const ServerOptions& serverOptions) noexcept
    : m_port(*iox::runtime::PoshRuntime::getInstance().getMiddlewareServer(service, serverOptions))
{
}

template <typename Port>
inline BaseServer<Port>::~BaseServer() noexcept
{
    m_port.destroy();
}

template <typename Port>
inline uid_t BaseServer<Port>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename Port>
inline const capro::ServiceDescription& BaseServer<Port>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename Port>
inline void BaseServer<Port>::offer() noexcept
{
    m_port.offer();
}

template <typename Port>
inline void BaseServer<Port>::stopOffer() noexcept
{
    m_port.stopOffer();
}

template <typename Port>
inline bool BaseServer<Port>::isOffered() const noexcept
{
    return m_port.isOffered();
}

template <typename Port>
inline bool BaseServer<Port>::hasClients() const noexcept
{
    return m_port.hasClients();
}

template <typename Port>
inline bool BaseServer<Port>::hasRequests() const noexcept
{
    return m_port.hasNewRequests();
}

template <typename Port>
inline bool BaseServer<Port>::hasMissedRequests() noexcept
{
    return m_port.hasLostRequestsSinceLastCall();
}

template <typename Port>
inline void BaseServer<Port>::releaseQueuedRequests() noexcept
{
    m_port.releaseQueuedRequests();
}

template <typename Port>
inline void BaseServer<Port>::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        m_port.unsetConditionVariable();
        m_trigger.invalidate();
    }
}

template <typename Port>
inline void BaseServer<Port>::enableState(iox::popo::TriggerHandle&& triggerHandle,
                                          const ServerState serverState) noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_REQUEST:
        if (m_trigger)
        {
            LogWarn()
                << "The server is already attached with either the ServerState::HAS_REQUEST or "
                   "ServerEvent::REQUEST_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ServerState::HAS_REQUEST. Best practice is to call detach first.";

            errorHandler(
                Error::kPOPO__BASE_SERVER_OVERRIDING_WITH_STATE_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED,
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
BaseServer<Port>::getCallbackForIsStateConditionSatisfied(const ServerState serverState) const noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_REQUEST:
        return {*this, &SelfType::hasRequests};
    }
    return {};
}

template <typename Port>
inline void BaseServer<Port>::disableState(const ServerState serverState) noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_REQUEST:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename Port>
inline void BaseServer<Port>::enableEvent(iox::popo::TriggerHandle&& triggerHandle,
                                          const ServerEvent serverEvent) noexcept
{
    switch (serverEvent)
    {
    case ServerEvent::REQUEST_RECEIVED:
        if (m_trigger)
        {
            LogWarn()
                << "The server is already attached with either the ServerState::HAS_REQUEST or "
                   "ServerEvent::REQUEST_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ServerEvent::REQUEST_RECEIVED. Best practice is to call detach "
                   "first.";
            errorHandler(
                Error::kPOPO__BASE_SERVER_OVERRIDING_WITH_EVENT_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED,
                nullptr,
                ErrorLevel::MODERATE);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}

template <typename Port>
inline void BaseServer<Port>::disableEvent(const ServerEvent serverEvent) noexcept
{
    switch (serverEvent)
    {
    case ServerEvent::REQUEST_RECEIVED:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename Port>
const Port& BaseServer<Port>::port() const noexcept
{
    return m_port;
}

template <typename Port>
Port& BaseServer<Port>::port() noexcept
{
    return m_port;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_SERVER_INL
