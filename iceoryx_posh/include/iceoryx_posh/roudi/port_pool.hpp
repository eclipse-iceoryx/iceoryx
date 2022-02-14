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
#ifndef IOX_POSH_ROUDI_PORT_POOL_HPP
#define IOX_POSH_ROUDI_PORT_POOL_HPP

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/publisher_options.hpp"
#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"

namespace iox
{
namespace roudi
{
struct PortPoolDataBase;

enum class PortPoolError : uint8_t
{
    UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS,
    PUBLISHER_PORT_LIST_FULL,
    SUBSCRIBER_PORT_LIST_FULL,
    INTERFACE_PORT_LIST_FULL,
    CLIENT_PORT_LIST_FULL,
    SERVER_PORT_LIST_FULL,
    NODE_DATA_LIST_FULL,
    CONDITION_VARIABLE_LIST_FULL,
    EVENT_VARIABLE_LIST_FULL,
};

class PortPool
{
  public:
    PortPool(PortPoolData& portPoolData) noexcept;

    virtual ~PortPool() noexcept = default;

    /// @todo don't create the vector with each call but only when the data really change
    /// there could be a member "cxx::vector<popo::PublisherPortData* m_publisherPorts;" and publisherPorts() would just
    /// update this member if the publisher ports actually changed
    cxx::vector<PublisherPortRouDiType::MemberType_t*, MAX_PUBLISHERS> getPublisherPortDataList() noexcept;
    cxx::vector<SubscriberPortType::MemberType_t*, MAX_SUBSCRIBERS> getSubscriberPortDataList() noexcept;
    cxx::vector<popo::ClientPortData*, MAX_CLIENTS> getClientPortDataList() noexcept;
    cxx::vector<popo::ServerPortData*, MAX_SERVERS> getServerPortDataList() noexcept;
    cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> getInterfacePortDataList() noexcept;
    cxx::vector<runtime::NodeData*, MAX_NODE_NUMBER> getNodeDataList() noexcept;
    cxx::vector<popo::ConditionVariableData*, MAX_NUMBER_OF_CONDITION_VARIABLES>
    getConditionVariableDataList() noexcept;

    cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
    addPublisherPort(const capro::ServiceDescription& serviceDescription,
                     mepoo::MemoryManager* const memoryManager,
                     const RuntimeName_t& runtimeName,
                     const popo::PublisherOptions& publisherOptions,
                     const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    cxx::expected<SubscriberPortType::MemberType_t*, PortPoolError>
    addSubscriberPort(const capro::ServiceDescription& serviceDescription,
                      const RuntimeName_t& runtimeName,
                      const popo::SubscriberOptions& subscriberOptions,
                      const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    template <typename T, std::enable_if_t<std::is_same<T, iox::build::ManyToManyPolicy>::value>* = nullptr>
    iox::popo::SubscriberPortData* constructSubscriber(const capro::ServiceDescription& serviceDescription,
                                                       const RuntimeName_t& runtimeName,
                                                       const popo::SubscriberOptions& subscriberOptions,
                                                       const mepoo::MemoryInfo& memoryInfo) noexcept;

    template <typename T, std::enable_if_t<std::is_same<T, iox::build::OneToManyPolicy>::value>* = nullptr>
    iox::popo::SubscriberPortData* constructSubscriber(const capro::ServiceDescription& serviceDescription,
                                                       const RuntimeName_t& runtimeName,
                                                       const popo::SubscriberOptions& subscriberOptions,
                                                       const mepoo::MemoryInfo& memoryInfo) noexcept;

    cxx::expected<popo::ClientPortData*, PortPoolError>
    addClientPort(const capro::ServiceDescription& serviceDescription,
                  mepoo::MemoryManager* const memoryManager,
                  const RuntimeName_t& runtimeName,
                  const popo::ClientOptions& clientOptions,
                  const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    cxx::expected<popo::ServerPortData*, PortPoolError>
    addServerPort(const capro::ServiceDescription& serviceDescription,
                  mepoo::MemoryManager* const memoryManager,
                  const RuntimeName_t& runtimeName,
                  const popo::ServerOptions& serverOptions,
                  const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    cxx::expected<popo::InterfacePortData*, PortPoolError> addInterfacePort(const RuntimeName_t& runtimeName,
                                                                            const capro::Interfaces interface) noexcept;

    cxx::expected<runtime::NodeData*, PortPoolError> addNodeData(const RuntimeName_t& runtimeName,
                                                                 const NodeName_t& nodeName,
                                                                 const uint64_t nodeDeviceIdentifier) noexcept;

    cxx::expected<popo::ConditionVariableData*, PortPoolError>
    addConditionVariableData(const RuntimeName_t& runtimeName) noexcept;

    void removePublisherPort(PublisherPortRouDiType::MemberType_t* const portData) noexcept;
    void removeSubscriberPort(SubscriberPortType::MemberType_t* const portData) noexcept;
    void removeClientPort(popo::ClientPortData* const portData) noexcept;
    void removeServerPort(popo::ServerPortData* const portData) noexcept;
    void removeInterfacePort(popo::InterfacePortData* const portData) noexcept;
    void removeNodeData(runtime::NodeData* const nodeData) noexcept;
    void removeConditionVariableData(popo::ConditionVariableData* const conditionVariableData) noexcept;

    std::atomic<uint64_t>* serviceRegistryChangeCounter() noexcept;

  private:
    PortPoolData* m_portPoolData;
};

} // namespace roudi
} // namespace iox

#include "iceoryx_posh/roudi/port_pool.inl"

#endif // IOX_POSH_ROUDI_PORT_POOL_HPP
