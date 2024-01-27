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
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace popo
{
template <typename PortT, typename TriggerHandleT>
inline BaseServer<PortT, TriggerHandleT>::BaseServer(const capro::ServiceDescription& service,
                                                     const ServerOptions& serverOptions) noexcept
    : m_port(*iox::runtime::PoshRuntime::getInstance().getMiddlewareServer(service, serverOptions))
{
}

template <typename PortT, typename TriggerHandleT>
inline BaseServer<PortT, TriggerHandleT>::~BaseServer() noexcept
{
    m_port.destroy();
}

template <typename PortT, typename TriggerHandleT>
inline uid_t BaseServer<PortT, TriggerHandleT>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename PortT, typename TriggerHandleT>
inline const capro::ServiceDescription& BaseServer<PortT, TriggerHandleT>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::offer() noexcept
{
    m_port.offer();
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::stopOffer() noexcept
{
    m_port.stopOffer();
}

template <typename PortT, typename TriggerHandleT>
inline bool BaseServer<PortT, TriggerHandleT>::isOffered() const noexcept
{
    return m_port.isOffered();
}

template <typename PortT, typename TriggerHandleT>
inline bool BaseServer<PortT, TriggerHandleT>::hasClients() const noexcept
{
    return m_port.hasClients();
}

template <typename PortT, typename TriggerHandleT>
inline bool BaseServer<PortT, TriggerHandleT>::hasRequests() const noexcept
{
    return m_port.hasNewRequests();
}

template <typename PortT, typename TriggerHandleT>
inline bool BaseServer<PortT, TriggerHandleT>::hasMissedRequests() noexcept
{
    return m_port.hasLostRequestsSinceLastCall();
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::releaseQueuedRequests() noexcept
{
    m_port.releaseQueuedRequests();
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        m_port.unsetConditionVariable();
        m_trigger.invalidate();
    }
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::enableState(TriggerHandleT&& triggerHandle,
                                                           const ServerState serverState) noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_REQUEST:
        if (m_trigger)
        {
            IOX_LOG(
                WARN,
                "The server is already attached with either the ServerState::HAS_REQUEST or "
                "ServerEvent::REQUEST_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                "attaching it to the new one with ServerState::HAS_REQUEST. Best practice is to call detach first.");

            IOX_REPORT(
                PoshError::
                    POPO__BASE_SERVER_OVERRIDING_WITH_STATE_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED,
                iox::er::RUNTIME_ERROR);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}

template <typename PortT, typename TriggerHandleT>
inline WaitSetIsConditionSatisfiedCallback
BaseServer<PortT, TriggerHandleT>::getCallbackForIsStateConditionSatisfied(const ServerState serverState) const noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_REQUEST:
        return WaitSetIsConditionSatisfiedCallback(in_place, *this, &SelfType::hasRequests);
    }
    return nullopt;
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::disableState(const ServerState serverState) noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_REQUEST:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::enableEvent(TriggerHandleT&& triggerHandle,
                                                           const ServerEvent serverEvent) noexcept
{
    switch (serverEvent)
    {
    case ServerEvent::REQUEST_RECEIVED:
        if (m_trigger)
        {
            IOX_LOG(WARN,
                    "The server is already attached with either the ServerState::HAS_REQUEST or "
                    "ServerEvent::REQUEST_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                    "attaching it to the new one with ServerEvent::REQUEST_RECEIVED. Best practice is to call detach "
                    "first.");
            IOX_REPORT(
                PoshError::
                    POPO__BASE_SERVER_OVERRIDING_WITH_EVENT_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED,
                iox::er::RUNTIME_ERROR);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}

template <typename PortT, typename TriggerHandleT>
inline void BaseServer<PortT, TriggerHandleT>::disableEvent(const ServerEvent serverEvent) noexcept
{
    switch (serverEvent)
    {
    case ServerEvent::REQUEST_RECEIVED:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename PortT, typename TriggerHandleT>
const PortT& BaseServer<PortT, TriggerHandleT>::port() const noexcept
{
    return m_port;
}

template <typename PortT, typename TriggerHandleT>
PortT& BaseServer<PortT, TriggerHandleT>::port() noexcept
{
    return m_port;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_SERVER_INL
