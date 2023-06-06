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

#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"

namespace iox
{
namespace roudi
{
PortPool::PortPool(PortPoolData& portPoolData) noexcept
    : m_portPoolData(&portPoolData)
{
}

vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> PortPool::getInterfacePortDataList() noexcept
{
    return m_portPoolData->m_interfacePortMembers.content();
}

vector<runtime::NodeData*, MAX_NODE_NUMBER> PortPool::getNodeDataList() noexcept
{
    return m_portPoolData->m_nodeMembers.content();
}

vector<popo::ConditionVariableData*, MAX_NUMBER_OF_CONDITION_VARIABLES>
PortPool::getConditionVariableDataList() noexcept
{
    return m_portPoolData->m_conditionVariableMembers.content();
}

expected<popo::InterfacePortData*, PortPoolError> PortPool::addInterfacePort(const RuntimeName_t& runtimeName,
                                                                             const capro::Interfaces interface) noexcept
{
    if (m_portPoolData->m_interfacePortMembers.hasFreeSpace())
    {
        auto interfacePortData = m_portPoolData->m_interfacePortMembers.insert(runtimeName, interface);
        return ok(interfacePortData);
    }
    else
    {
        IOX_LOG(WARN) << "Out of interface ports! Requested by runtime '" << runtimeName << "'";
        errorHandler(PoshError::PORT_POOL__INTERFACELIST_OVERFLOW, ErrorLevel::MODERATE);
        return err(PortPoolError::INTERFACE_PORT_LIST_FULL);
    }
}

expected<runtime::NodeData*, PortPoolError> PortPool::addNodeData(const RuntimeName_t& runtimeName,
                                                                  const NodeName_t& nodeName,
                                                                  const uint64_t nodeDeviceIdentifier) noexcept
{
    if (m_portPoolData->m_nodeMembers.hasFreeSpace())
    {
        auto nodeData = m_portPoolData->m_nodeMembers.insert(runtimeName, nodeName, nodeDeviceIdentifier);
        return ok(nodeData);
    }
    else
    {
        IOX_LOG(WARN) << "Out of node data! Requested by runtime '" << runtimeName << "' and node name '" << nodeName
                      << "'";
        errorHandler(PoshError::PORT_POOL__NODELIST_OVERFLOW, ErrorLevel::MODERATE);
        return err(PortPoolError::NODE_DATA_LIST_FULL);
    }
}

expected<popo::ConditionVariableData*, PortPoolError>
PortPool::addConditionVariableData(const RuntimeName_t& runtimeName) noexcept
{
    if (m_portPoolData->m_conditionVariableMembers.hasFreeSpace())
    {
        auto conditionVariableData = m_portPoolData->m_conditionVariableMembers.insert(runtimeName);
        return ok(conditionVariableData);
    }
    else
    {
        IOX_LOG(WARN) << "Out of condition variables! Requested by runtime '" << runtimeName << "'";
        errorHandler(PoshError::PORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW, ErrorLevel::MODERATE);
        return err(PortPoolError::CONDITION_VARIABLE_LIST_FULL);
    }
}

void PortPool::removeInterfacePort(const popo::InterfacePortData* const portData) noexcept
{
    m_portPoolData->m_interfacePortMembers.erase(portData);
}

void PortPool::removeNodeData(const runtime::NodeData* const nodeData) noexcept
{
    m_portPoolData->m_nodeMembers.erase(nodeData);
}

void PortPool::removeConditionVariableData(const popo::ConditionVariableData* const conditionVariableData) noexcept
{
    m_portPoolData->m_conditionVariableMembers.erase(conditionVariableData);
}

vector<PublisherPortRouDiType::MemberType_t*, MAX_PUBLISHERS> PortPool::getPublisherPortDataList() noexcept
{
    return m_portPoolData->m_publisherPortMembers.content();
}

vector<SubscriberPortType::MemberType_t*, MAX_SUBSCRIBERS> PortPool::getSubscriberPortDataList() noexcept
{
    return m_portPoolData->m_subscriberPortMembers.content();
}

expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
PortPool::addPublisherPort(const capro::ServiceDescription& serviceDescription,
                           mepoo::MemoryManager* const memoryManager,
                           const RuntimeName_t& runtimeName,
                           const popo::PublisherOptions& publisherOptions,
                           const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_publisherPortMembers.hasFreeSpace())
    {
        auto publisherPortData = m_portPoolData->m_publisherPortMembers.insert(
            serviceDescription, runtimeName, memoryManager, publisherOptions, memoryInfo);
        return ok(publisherPortData);
    }
    else
    {
        IOX_LOG(WARN) << "Out of publisher ports! Requested by runtime '" << runtimeName
                      << "' and with service description '" << serviceDescription << "'";
        errorHandler(PoshError::PORT_POOL__PUBLISHERLIST_OVERFLOW, ErrorLevel::MODERATE);
        return err(PortPoolError::PUBLISHER_PORT_LIST_FULL);
    }
}

expected<SubscriberPortType::MemberType_t*, PortPoolError>
PortPool::addSubscriberPort(const capro::ServiceDescription& serviceDescription,
                            const RuntimeName_t& runtimeName,
                            const popo::SubscriberOptions& subscriberOptions,
                            const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_subscriberPortMembers.hasFreeSpace())
    {
        auto subscriberPortData = constructSubscriber<iox::build::CommunicationPolicy>(
            serviceDescription, runtimeName, subscriberOptions, memoryInfo);

        return ok(subscriberPortData);
    }
    else
    {
        IOX_LOG(WARN) << "Out of subscriber ports! Requested by runtime '" << runtimeName
                      << "' and with service description '" << serviceDescription << "'";
        errorHandler(PoshError::PORT_POOL__SUBSCRIBERLIST_OVERFLOW, ErrorLevel::MODERATE);
        return err(PortPoolError::SUBSCRIBER_PORT_LIST_FULL);
    }
}

vector<popo::ClientPortData*, MAX_CLIENTS> PortPool::getClientPortDataList() noexcept
{
    return m_portPoolData->m_clientPortMembers.content();
}

vector<popo::ServerPortData*, MAX_SERVERS> PortPool::getServerPortDataList() noexcept
{
    return m_portPoolData->m_serverPortMembers.content();
}

expected<popo::ClientPortData*, PortPoolError>
PortPool::addClientPort(const capro::ServiceDescription& serviceDescription,
                        mepoo::MemoryManager* const memoryManager,
                        const RuntimeName_t& runtimeName,
                        const popo::ClientOptions& clientOptions,
                        const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (!m_portPoolData->m_clientPortMembers.hasFreeSpace())
    {
        IOX_LOG(WARN) << "Out of client ports! Requested by runtime '" << runtimeName
                      << "' and with service description '" << serviceDescription << "'";
        errorHandler(PoshError::PORT_POOL__CLIENTLIST_OVERFLOW, ErrorLevel::MODERATE);
        return err(PortPoolError::CLIENT_PORT_LIST_FULL);
    }

    auto clientPortData = m_portPoolData->m_clientPortMembers.insert(
        serviceDescription, runtimeName, clientOptions, memoryManager, memoryInfo);
    return ok(clientPortData);
}

expected<popo::ServerPortData*, PortPoolError>
PortPool::addServerPort(const capro::ServiceDescription& serviceDescription,
                        mepoo::MemoryManager* const memoryManager,
                        const RuntimeName_t& runtimeName,
                        const popo::ServerOptions& serverOptions,
                        const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (!m_portPoolData->m_serverPortMembers.hasFreeSpace())
    {
        IOX_LOG(WARN) << "Out of server ports! Requested by runtime '" << runtimeName
                      << "' and with service description '" << serviceDescription << "'";
        errorHandler(PoshError::PORT_POOL__SERVERLIST_OVERFLOW, ErrorLevel::MODERATE);
        return err(PortPoolError::SERVER_PORT_LIST_FULL);
    }

    auto serverPortData = m_portPoolData->m_serverPortMembers.insert(
        serviceDescription, runtimeName, serverOptions, memoryManager, memoryInfo);
    return ok(serverPortData);
}

void PortPool::removePublisherPort(const PublisherPortRouDiType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_publisherPortMembers.erase(portData);
}

void PortPool::removeSubscriberPort(const SubscriberPortType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_subscriberPortMembers.erase(portData);
}

void PortPool::removeClientPort(const popo::ClientPortData* const portData) noexcept
{
    m_portPoolData->m_clientPortMembers.erase(portData);
}
void PortPool::removeServerPort(const popo::ServerPortData* const portData) noexcept
{
    m_portPoolData->m_serverPortMembers.erase(portData);
}

} // namespace roudi
} // namespace iox
