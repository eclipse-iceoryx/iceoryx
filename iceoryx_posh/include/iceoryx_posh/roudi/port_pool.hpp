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
#ifndef IOX_POSH_ROUDI_PORT_POOL_HPP
#define IOX_POSH_ROUDI_PORT_POOL_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/publisher_options.hpp"
#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iox/type_traits.hpp"

namespace iox
{
namespace roudi
{
struct PortPoolDataBase;

enum class PortPoolError : uint8_t
{
    UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS,
    INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN,
    PUBLISHER_PORT_LIST_FULL,
    SUBSCRIBER_PORT_LIST_FULL,
    INTERFACE_PORT_LIST_FULL,
    CLIENT_PORT_LIST_FULL,
    UNIQUE_SERVER_PORT_ALREADY_EXISTS,
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

    vector<PublisherPortRouDiType::MemberType_t*, MAX_PUBLISHERS> getPublisherPortDataList() noexcept;
    vector<SubscriberPortType::MemberType_t*, MAX_SUBSCRIBERS> getSubscriberPortDataList() noexcept;
    vector<popo::ClientPortData*, MAX_CLIENTS> getClientPortDataList() noexcept;
    vector<popo::ServerPortData*, MAX_SERVERS> getServerPortDataList() noexcept;
    vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> getInterfacePortDataList() noexcept;
    vector<runtime::NodeData*, MAX_NODE_NUMBER> getNodeDataList() noexcept;
    vector<popo::ConditionVariableData*, MAX_NUMBER_OF_CONDITION_VARIABLES> getConditionVariableDataList() noexcept;

    expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
    addPublisherPort(const capro::ServiceDescription& serviceDescription,
                     mepoo::MemoryManager* const memoryManager,
                     const RuntimeName_t& runtimeName,
                     const popo::PublisherOptions& publisherOptions,
                     const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    expected<SubscriberPortType::MemberType_t*, PortPoolError>
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

    /// @brief Adds a ClientPortData to the internal pool and returns a pointer for further usage
    /// @param[in] serviceDescription for the new client port
    /// @param[in] memoryManager to acquire chunks for the requests
    /// @param[in] runtimeName of the runtime the new client port belongs to
    /// @param[in] clientOptions for the new client port
    /// @param[in] memoryInfo for the new client port
    /// @return on success a pointer to a ClientPortData; on error a PortPoolError
    expected<popo::ClientPortData*, PortPoolError>
    addClientPort(const capro::ServiceDescription& serviceDescription,
                  mepoo::MemoryManager* const memoryManager,
                  const RuntimeName_t& runtimeName,
                  const popo::ClientOptions& clientOptions,
                  const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    /// @brief Adds a ServerPortData to the internal pool and returns a pointer for further usage
    /// @param[in] serviceDescription for the new server port
    /// @param[in] memoryManager to acquire chunks for the responses
    /// @param[in] runtimeName of the runtime the new server port belongs to
    /// @param[in] serverOptions for the new server port
    /// @param[in] memoryInfo for the new server port
    /// @return on success a pointer to a ServerPortData; on error a PortPoolError
    expected<popo::ServerPortData*, PortPoolError>
    addServerPort(const capro::ServiceDescription& serviceDescription,
                  mepoo::MemoryManager* const memoryManager,
                  const RuntimeName_t& runtimeName,
                  const popo::ServerOptions& serverOptions,
                  const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    expected<popo::InterfacePortData*, PortPoolError> addInterfacePort(const RuntimeName_t& runtimeName,
                                                                       const capro::Interfaces interface) noexcept;

    expected<runtime::NodeData*, PortPoolError> addNodeData(const RuntimeName_t& runtimeName,
                                                            const NodeName_t& nodeName,
                                                            const uint64_t nodeDeviceIdentifier) noexcept;

    expected<popo::ConditionVariableData*, PortPoolError>
    addConditionVariableData(const RuntimeName_t& runtimeName) noexcept;

    /// @brief Removes a PublisherPortData from the internal pool
    /// @param[in] portData is a  pointer to the PublisherPortData to be removed
    /// @note after this call the provided PublisherPortData is no longer available for usage
    void removePublisherPort(const PublisherPortRouDiType::MemberType_t* const portData) noexcept;

    /// @brief Removes a SubscriberPortData from the internal pool
    /// @param[in] portData is a  pointer to the SubscriberPortData to be removed
    /// @note after this call the provided SubscriberPortData is no longer available for usage
    void removeSubscriberPort(const SubscriberPortType::MemberType_t* const portData) noexcept;

    /// @brief Removes a ClientPortData from the internal pool
    /// @param[in] portData is a  pointer to the ClientPortData to be removed
    /// @note after this call the provided ClientPortData is no longer available for usage
    void removeClientPort(const popo::ClientPortData* const portData) noexcept;

    /// @brief Removes a ServerPortData from the internal pool
    /// @param[in] portData is a  pointer to the ServerPortData to be removed
    /// @note after this call the provided ServerPortData is no longer available for usage
    void removeServerPort(const popo::ServerPortData* const portData) noexcept;

    /// @brief Removes a InterfacePortData from the internal pool
    /// @param[in] portData is a  pointer to the InterfacePortData to be removed
    /// @note after this call the provided InterfacePortData is no longer available for usage
    void removeInterfacePort(const popo::InterfacePortData* const portData) noexcept;

    /// @brief Removes a NodeData from the internal pool
    /// @param[in] nodeData is a pointer to the NodeData to be removed
    /// @note after this call the provided NodeData is no longer available for usage
    void removeNodeData(const runtime::NodeData* const nodeData) noexcept;

    /// @brief Removes a ConditionVariableData from the internal pool
    /// @param[in] conditionVariableData is a pointer to the ConditionVariableData to be removed
    /// @note after this call the provided ConditionVariableData is no longer available for usage
    void removeConditionVariableData(const popo::ConditionVariableData* const conditionVariableData) noexcept;

  private:
    PortPoolData* m_portPoolData;
};

} // namespace roudi
} // namespace iox

#include "iceoryx_posh/roudi/port_pool.inl"

#endif // IOX_POSH_ROUDI_PORT_POOL_HPP
