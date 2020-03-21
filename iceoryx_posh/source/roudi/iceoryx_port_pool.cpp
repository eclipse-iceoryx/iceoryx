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

#include "iceoryx_posh/internal/roudi/iceoryx_port_pool.hpp"
#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"

namespace iox
{
namespace roudi
{
IceOryxPortPool::IceOryxPortPool(PortPoolData& portPoolData) noexcept
    : PortPool(portPoolData)
    , m_portPoolData(&portPoolData)
{
}

cxx::vector<SenderPortType::MemberType_t*, MAX_PORT_NUMBER> IceOryxPortPool::senderPortDataList() noexcept
{
    return m_portPoolData->m_senderPortMembers.content();
}

cxx::vector<ReceiverPortType::MemberType_t*, MAX_PORT_NUMBER> IceOryxPortPool::receiverPortDataList() noexcept
{
    return m_portPoolData->m_receiverPortMembers.content();
}

cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
IceOryxPortPool::addSenderPort(const capro::ServiceDescription& serviceDescription,
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

cxx::expected<ReceiverPortType::MemberType_t*, PortPoolError>
IceOryxPortPool::addReceiverPort(const capro::ServiceDescription& serviceDescription,
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

void IceOryxPortPool::removeSenderPort(SenderPortType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_senderPortMembers.erase(portData);
}

void IceOryxPortPool::removeReceiverPort(ReceiverPortType::MemberType_t* const portData) noexcept
{
    m_portPoolData->m_receiverPortMembers.erase(portData);
}

} // namespace roudi
} // namespace iox
