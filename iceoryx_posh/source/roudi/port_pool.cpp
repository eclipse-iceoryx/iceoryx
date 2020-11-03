// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

cxx::vector<runtime::RunnableData*, MAX_RUNNABLE_NUMBER> PortPool::getRunnableDataList() noexcept
{
    return m_portPoolData->m_runnableMembers.content();
}

cxx::expected<popo::InterfacePortData*, PortPoolError>
PortPool::addInterfacePort(const std::string& applicationName, const capro::Interfaces interface) noexcept
{
    if (m_portPoolData->m_interfacePortMembers.hasFreeSpace())
    {
        auto interfacePortData = m_portPoolData->m_interfacePortMembers.insert(
            iox::cxx::string<100>(iox::cxx::TruncateToCapacity, applicationName), interface);
        return cxx::success<popo::InterfacePortData*>(interfacePortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__INTERFACELIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::INTERFACE_PORT_LIST_FULL);
    }
}

cxx::expected<popo::ApplicationPortData*, PortPoolError>
PortPool::addApplicationPort(const std::string& applicationName) noexcept
{
    if (m_portPoolData->m_applicationPortMembers.hasFreeSpace())
    {
        auto applicationPortData = m_portPoolData->m_applicationPortMembers.insert(
            iox::cxx::string<100>(iox::cxx::TruncateToCapacity, applicationName));
        return cxx::success<popo::ApplicationPortData*>(applicationPortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__APPLICATIONLIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::APPLICATION_PORT_LIST_FULL);
    }
}

cxx::expected<runtime::RunnableData*, PortPoolError> PortPool::addRunnableData(
    const ProcessName_t& process, const RunnableName_t& runnable, const uint64_t runnableDeviceIdentifier) noexcept
{
    if (m_portPoolData->m_runnableMembers.hasFreeSpace())
    {
        auto runnableData = m_portPoolData->m_runnableMembers.insert(process, runnable, runnableDeviceIdentifier);
        return cxx::success<runtime::RunnableData*>(runnableData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__RUNNABLELIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::RUNNABLE_DATA_LIST_FULL);
    }
}

cxx::expected<popo::ConditionVariableData*, PortPoolError> PortPool::addConditionVariableData() noexcept
{
    if (m_portPoolData->m_conditionVariableMembers.hasFreeSpace())
    {
        auto conditionVariableData = m_portPoolData->m_conditionVariableMembers.insert();
        return cxx::success<popo::ConditionVariableData*>(conditionVariableData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::CONDITION_VARIABLE_LIST_FULL);
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

void PortPool::removeRunnableData(runtime::RunnableData* const runnableData) noexcept
{
    m_portPoolData->m_runnableMembers.erase(runnableData);
}

std::atomic<uint64_t>* PortPool::serviceRegistryChangeCounter() noexcept
{
    return &m_portPoolData->m_serviceRegistryChangeCounter;
}

/// @deprecated #25
cxx::vector<SenderPortType::MemberType_t*, MAX_PUBLISHERS> PortPool::senderPortDataList() noexcept
{
    return m_portPoolData->m_senderPortMembers.content();
}

/// @deprecated #25
cxx::vector<ReceiverPortType::MemberType_t*, MAX_SUBSCRIBERS> PortPool::receiverPortDataList() noexcept
{
    return m_portPoolData->m_receiverPortMembers.content();
}

/// @deprecated #25
cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
PortPool::addSenderPort(const capro::ServiceDescription& serviceDescription,
                        mepoo::MemoryManager* const memoryManager,
                        const std::string& applicationName,
                        const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_senderPortMembers.hasFreeSpace())
    {
        auto senderPortData =
            m_portPoolData->m_senderPortMembers.insert(serviceDescription, memoryManager, applicationName, memoryInfo);
        return cxx::success<SenderPortType::MemberType_t*>(senderPortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__SENDERLIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::SENDER_PORT_LIST_FULL);
    }
}

/// @deprecated #25
cxx::expected<ReceiverPortType::MemberType_t*, PortPoolError>
PortPool::addReceiverPort(const capro::ServiceDescription& serviceDescription,
                          const std::string& applicationName,
                          const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_receiverPortMembers.hasFreeSpace())
    {
        auto receiverPortData =
            m_portPoolData->m_receiverPortMembers.insert(serviceDescription, applicationName, memoryInfo);
        return cxx::success<ReceiverPortType::MemberType_t*>(receiverPortData);
    }
    else
    {
        errorHandler(Error::kPORT_POOL__RECEIVERLIST_OVERFLOW, nullptr, ErrorLevel::MODERATE);
        return cxx::error<PortPoolError>(PortPoolError::RECEIVER_PORT_LIST_FULL);
    }
}

/// @deprecated #25
void PortPool::removeSenderPort(SenderPortType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_senderPortMembers.erase(portData);
}

/// @deprecated #25
void PortPool::removeReceiverPort(ReceiverPortType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_receiverPortMembers.erase(portData);
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
                           const uint64_t& historyCapacity,
                           mepoo::MemoryManager* const memoryManager,
                           const ProcessName_t& applicationName,
                           const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_publisherPortMembers.hasFreeSpace())
    {
        auto publisherPortData = m_portPoolData->m_publisherPortMembers.insert(
            serviceDescription, applicationName, memoryManager, historyCapacity, memoryInfo);
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
                            const uint64_t& historyRequest,
                            const ProcessName_t& applicationName,
                            const mepoo::MemoryInfo& memoryInfo) noexcept
{
    if (m_portPoolData->m_subscriberPortMembers.hasFreeSpace())
    {
        auto subscriberPortData = constructSubscriber<iox::build::CommunicationPolicy>(
            serviceDescription, historyRequest, applicationName, memoryInfo);

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
