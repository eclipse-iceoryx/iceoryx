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

#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_utils/cxx/string.hpp"

namespace iox
{
namespace roudi
{
PortPool::PortPool(PortPoolData& portPoolData) noexcept
    : m_portPoolData(&portPoolData)
{
}

cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> PortPool::getInterfacePortDataList() noexcept
{
    return m_portPoolData->m_interfacePortMembers.content();
}

cxx::vector<popo::ApplicationPortData*, MAX_PROCESS_NUMBER> PortPool::getApplicationPortDataList() noexcept
{
    return m_portPoolData->m_applicationPortMembers.content();
}

cxx::vector<runtime::NodeData*, MAX_NODE_NUMBER> PortPool::getNodeDataList() noexcept
{
    return m_portPoolData->m_nodeMembers.content();
}

cxx::vector<popo::ConditionVariableData*, MAX_NUMBER_OF_CONDITION_VARIABLES>
PortPool::getConditionVariableDataList() noexcept
{
    return m_portPoolData->m_conditionVariableMembers.content();
}

cxx::vector<popo::EventVariableData*, MAX_NUMBER_OF_EVENT_VARIABLES> PortPool::getEventVariableDataList() noexcept
{
    return m_portPoolData->m_eventVariableMembers.content();
}

cxx::expected<popo::InterfacePortData*, PortPoolError>
PortPool::addInterfacePort(const ProcessName_t& applicationName, const capro::Interfaces interface) noexcept
{
    if (m_portPoolData->m_interfacePortMembers.hasFreeSpace())
    {
        auto interfacePortData = m_portPoolData->m_interfacePortMembers.insert(applicationName, interface);
        return cxx::success<popo::InterfacePortData*>(interfacePortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__INTERFACELIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::INTERFACE_PORT_LIST_FULL);
    }
}

cxx::expected<popo::ApplicationPortData*, PortPoolError>
PortPool::addApplicationPort(const ProcessName_t& applicationName) noexcept
{
    if (m_portPoolData->m_applicationPortMembers.hasFreeSpace())
    {
        auto applicationPortData = m_portPoolData->m_applicationPortMembers.insert(applicationName);
        return cxx::success<popo::ApplicationPortData*>(applicationPortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__APPLICATIONLIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::APPLICATION_PORT_LIST_FULL);
    }
}

cxx::expected<runtime::NodeData*, PortPoolError> PortPool::addNodeData(const ProcessName_t& process,
                                                                       const NodeName_t& node,
                                                                       const uint64_t nodeDeviceIdentifier) noexcept
{
    if (m_portPoolData->m_nodeMembers.hasFreeSpace())
    {
        auto nodeData = m_portPoolData->m_nodeMembers.insert(process, node, nodeDeviceIdentifier);
        return cxx::success<runtime::NodeData*>(nodeData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__NODELIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::NODE_DATA_LIST_FULL);
    }
}

cxx::expected<popo::ConditionVariableData*, PortPoolError>
PortPool::addConditionVariableData(const ProcessName_t& process) noexcept
{
    if (m_portPoolData->m_conditionVariableMembers.hasFreeSpace())
    {
        auto conditionVariableData = m_portPoolData->m_conditionVariableMembers.insert(process);
        return cxx::success<popo::ConditionVariableData*>(conditionVariableData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::CONDITION_VARIABLE_LIST_FULL);
    }
}

cxx::expected<popo::EventVariableData*, PortPoolError>
PortPool::addEventVariableData(const ProcessName_t& process) noexcept
{
    if (m_portPoolData->m_eventVariableMembers.hasFreeSpace())
    {
        auto eventVariableData = m_portPoolData->m_eventVariableMembers.insert(process);
        return cxx::success<popo::EventVariableData*>(eventVariableData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__EVENT_VARIABLE_LIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::EVENT_VARIABLE_LIST_FULL);
    }
}

void PortPool::removeInterfacePort(popo::InterfacePortData* const portData) noexcept
{
    m_portPoolData->m_interfacePortMembers.erase(portData);
}

void PortPool::removeApplicationPort(popo::ApplicationPortData* const portData) noexcept
{
    m_portPoolData->m_applicationPortMembers.erase(portData);
}

void PortPool::removeNodeData(runtime::NodeData* const nodeData) noexcept
{
    m_portPoolData->m_nodeMembers.erase(nodeData);
}

void PortPool::removeConditionVariableData(popo::ConditionVariableData* const conditionVariableData) noexcept
{
    m_portPoolData->m_conditionVariableMembers.erase(conditionVariableData);
}

void PortPool::removeEventVariableData(popo::EventVariableData* const eventVariableData) noexcept
{
    m_portPoolData->m_eventVariableMembers.erase(eventVariableData);
}

std::atomic<uint64_t>* PortPool::serviceRegistryChangeCounter() noexcept
{
    return &m_portPoolData->m_serviceRegistryChangeCounter;
}

cxx::vector<PublisherPortRouDiType::MemberType_t*, MAX_PUBLISHERS> PortPool::getPublisherPortDataList() noexcept
{
    return m_portPoolData->m_publisherPortMembers.content();
}

cxx::vector<SubscriberPortType::MemberType_t*, MAX_SUBSCRIBERS> PortPool::getSubscriberPortDataList() noexcept
{
    return m_portPoolData->m_subscriberPortMembers.content();
}

cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
PortPool::addPublisherPort(const capro::ServiceDescription& serviceDescription,
                           mepoo::MemoryManager* const memoryManager,
                           const ProcessName_t& applicationName,
                           const popo::PublisherOptions& publisherOptions,
                           const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_publisherPortMembers.hasFreeSpace())
    {
        auto publisherPortData = m_portPoolData->m_publisherPortMembers.insert(
            serviceDescription, applicationName, memoryManager, publisherOptions, memoryInfo);
        return cxx::success<PublisherPortRouDiType::MemberType_t*>(publisherPortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__PUBLISHERLIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::PUBLISHER_PORT_LIST_FULL);
    }
}

cxx::expected<SubscriberPortType::MemberType_t*, PortPoolError>
PortPool::addSubscriberPort(const capro::ServiceDescription& serviceDescription,
                            const ProcessName_t& applicationName,
                            const popo::SubscriberOptions& subscriberOptions,
                            const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_subscriberPortMembers.hasFreeSpace())
    {
        auto subscriberPortData = constructSubscriber<iox::build::CommunicationPolicy>(
            serviceDescription, applicationName, subscriberOptions, memoryInfo);

        return cxx::success<SubscriberPortType::MemberType_t*>(subscriberPortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__SUBSCRIBERLIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::SUBSCRIBER_PORT_LIST_FULL);
    }
}

void PortPool::removePublisherPort(PublisherPortRouDiType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_publisherPortMembers.erase(portData);
}

void PortPool::removeSubscriberPort(SubscriberPortType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_subscriberPortMembers.erase(portData);
}

} // namespace roudi
} // namespace iox
