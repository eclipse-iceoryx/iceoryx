// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
template <typename port_t>
inline BaseClient<port_t>::BaseClient(const capro::ServiceDescription& service,
                                            const ClientOptions& clientOptions)
    // : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareClient(service, clientOptions))
{
}

template <typename port_t>
inline BaseClient<port_t>::~BaseClient()
{
    m_port.destroy();
}

template <typename port_t>
inline uid_t BaseClient<port_t>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename port_t>
inline capro::ServiceDescription BaseClient<port_t>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename port_t>
inline void BaseClient<port_t>::connect() noexcept
{
    m_port.connect();
}

template <typename port_t>
inline void BaseClient<port_t>::disconnect() noexcept
{
    m_port.disconnect();
}

template <typename port_t>
inline ConnectionState BaseClient<port_t>::getConnectionState() const noexcept
{
    return m_port.getConnectionState();
}

template <typename port_t>
inline cxx::expected<const ResponseHeader*, ChunkReceiveResult> BaseClient<port_t>::takeResponses() noexcept
{
    return m_port.getResponse();
}

template <typename port_t>
inline bool BaseClient<port_t>::hasResponses() const noexcept
{
    return m_port.hasResponse();
}

template <typename port_t>
inline bool BaseClient<port_t>::hasMissedResponses() noexcept
{
    return m_port.hasMissedResponses();
}

template <typename port_t>
const port_t& BaseClient<port_t>::port() const noexcept
{
    return m_port;
}

template <typename port_t>
port_t& BaseClient<port_t>::port() noexcept
{
    return m_port;
}

template <typename port_t>
inline void BaseClient<port_t>::releaseQueuedResponses() noexcept
{
    m_port.releaseQueuedResponses();
}


template <typename port_t>
inline void BaseClient<port_t>::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        m_port.unsetConditionVariable();
        m_trigger.invalidate();
    }
}

template <typename port_t>
inline void BaseClient<port_t>::enableState(iox::popo::TriggerHandle&& triggerHandle,
                                                IOX_MAYBE_UNUSED const ClientState clientState) noexcept

{
    switch (clientState)
    {
    case ClientState::HAS_DATA:
        if (m_trigger)
        {
            LogWarn()
                << "The client is already attached with either the ClientState::HAS_DATA or "
                   "ClientEvent::DATA_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ClientState::HAS_DATA. Best practice is to call detach first.";

            errorHandler(
                Error::kPOPO__BASE_CLIENT_OVERRIDING_WITH_STATE_SINCE_HAS_DATA_OR_DATA_RECEIVED_ALREADY_ATTACHED,
                nullptr,
                ErrorLevel::MODERATE);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}


template <typename port_t>
inline WaitSetIsConditionSatisfiedCallback
BaseClient<port_t>::getCallbackForIsStateConditionSatisfied(const ClientState clientState) const noexcept
{
    switch (clientState)
    {
    case ClientState::HAS_DATA:
        return {*this, &SelfType::hasResponse};
    }
    return {};
}

template <typename port_t>
inline void BaseClient<port_t>::disableState(const ClientState clientState) noexcept
{
    switch (clientState)
    {
    case ClientState::HAS_DATA:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename port_t>
inline void BaseClient<port_t>::enableEvent(iox::popo::TriggerHandle&& triggerHandle,
                                                const ClientEvent clientEvent) noexcept
{
    switch (clientEvent)
    {
    case ClientEvent::DATA_RECEIVED:
        if (m_trigger)
        {
            LogWarn()
                << "The client is already attached with either the ClientState::HAS_DATA or "
                   "ClientEvent::DATA_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ClientEvent::DATA_RECEIVED. Best practice is to call detach "
                   "first.";
            errorHandler(
                Error::kPOPO__BASE_CLIENT_OVERRIDING_WITH_EVENT_SINCE_HAS_DATA_OR_DATA_RECEIVED_ALREADY_ATTACHED,
                nullptr,
                ErrorLevel::MODERATE);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}

template <typename port_t>
inline void BaseClient<port_t>::disableEvent(const ClientEvent clientEvent) noexcept
{
    switch (clientEvent)
    {
    case ClientEvent::DATA_RECEIVED:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_CLIENT_INL
