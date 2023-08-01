// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_PORT_MANAGER_HPP
#define IOX_POSH_ROUDI_PORT_MANAGER_HPP

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iox/optional.hpp"
#include "iox/type_traits.hpp"

#include <mutex>

namespace iox
{
namespace roudi
{
capro::Interfaces StringToCaProInterface(const capro::IdString_t& str) noexcept;

class PortManager
{
  public:
    using PortConfigInfo = iox::runtime::PortConfigInfo;
    PortManager(RouDiMemoryInterface* roudiMemoryInterface) noexcept;

    virtual ~PortManager() noexcept = default;

    /// @todo iox-#518 Remove this later
    void stopPortIntrospection() noexcept;

    void doDiscovery() noexcept;

    expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
    acquirePublisherPortData(const capro::ServiceDescription& service,
                             const popo::PublisherOptions& publisherOptions,
                             const RuntimeName_t& runtimeName,
                             mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                             const PortConfigInfo& portConfigInfo) noexcept;

    PublisherPortRouDiType::MemberType_t*
    acquireInternalPublisherPortData(const capro::ServiceDescription& service,
                                     const popo::PublisherOptions& publisherOptions,
                                     mepoo::MemoryManager* const payloadDataSegmentMemoryManager) noexcept;

    expected<SubscriberPortType::MemberType_t*, PortPoolError>
    acquireSubscriberPortData(const capro::ServiceDescription& service,
                              const popo::SubscriberOptions& subscriberOptions,
                              const RuntimeName_t& runtimeName,
                              const PortConfigInfo& portConfigInfo) noexcept;

    /// @brief Acquires a ClientPortData for further usage
    /// @param[in] service is the ServiceDescription for the new client port
    /// @param[in] clientOptions for the new client port
    /// @param[in] runtimeName of the runtime the new client port belongs to
    /// @param[in] payloadDataSegmentMemoryManager to acquire chunks for the requests
    /// @param[in] portConfigInfo for the new client port
    /// @return on success a pointer to a ClientPortData; on error a PortPoolError
    expected<popo::ClientPortData*, PortPoolError>
    acquireClientPortData(const capro::ServiceDescription& service,
                          const popo::ClientOptions& clientOptions,
                          const RuntimeName_t& runtimeName,
                          mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                          const PortConfigInfo& portConfigInfo) noexcept;

    /// @brief Acquires a ServerPortData for further usage
    /// @param[in] service is the ServiceDescription for the new server port
    /// @param[in] serverOptions for the new server port
    /// @param[in] runtimeName of the runtime the new server port belongs to
    /// @param[in] payloadDataSegmentMemoryManager to acquire chunks for the requests
    /// @param[in] portConfigInfo for the new server port
    /// @return on success a pointer to a ServerPortData; on error a PortPoolError
    expected<popo::ServerPortData*, PortPoolError>
    acquireServerPortData(const capro::ServiceDescription& service,
                          const popo::ServerOptions& serverOptions,
                          const RuntimeName_t& runtimeName,
                          mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                          const PortConfigInfo& portConfigInfo) noexcept;

    popo::InterfacePortData* acquireInterfacePortData(capro::Interfaces interface,
                                                      const RuntimeName_t& runtimeName,
                                                      const NodeName_t& nodeName = {""}) noexcept;

    expected<runtime::NodeData*, PortPoolError> acquireNodeData(const RuntimeName_t& runtimeName,
                                                                const NodeName_t& nodeName) noexcept;

    expected<popo::ConditionVariableData*, PortPoolError>
    acquireConditionVariableData(const RuntimeName_t& runtimeName) noexcept;

    /// @brief Used to unblock potential locks in the shutdown phase of a process
    /// @param [in] name of the process runtime which is about to shut down
    void unblockProcessShutdown(const RuntimeName_t& runtimeName) noexcept;

    /// @brief Used to unblock potential locks in the shutdown phase of RouDi
    void unblockRouDiShutdown() noexcept;

    void deletePortsOfProcess(const RuntimeName_t& runtimeName) noexcept;

  protected:
    void makeAllPublisherPortsToStopOffer() noexcept;

    void destroyPublisherPort(PublisherPortRouDiType::MemberType_t* const publisherPortData) noexcept;

    void destroySubscriberPort(SubscriberPortType::MemberType_t* const subscriberPortData) noexcept;

    void handlePublisherPorts() noexcept;

    void doDiscoveryForPublisherPort(PublisherPortRouDiType& publisherPort) noexcept;

    void handleSubscriberPorts() noexcept;

    void doDiscoveryForSubscriberPort(SubscriberPortType& subscriberPort) noexcept;

    void destroyClientPort(popo::ClientPortData* const clientPortData) noexcept;

    void handleClientPorts() noexcept;

    void doDiscoveryForClientPort(popo::ClientPortRouDi& clientPort) noexcept;

    void makeAllServerPortsToStopOffer() noexcept;

    void destroyServerPort(popo::ServerPortData* const clientPortData) noexcept;

    void handleServerPorts() noexcept;

    void doDiscoveryForServerPort(popo::ServerPortRouDi& serverPort) noexcept;

    void handleInterfaces() noexcept;

    void handleNodes() noexcept;

    void handleConditionVariables() noexcept;

    bool isCompatiblePubSub(const PublisherPortRouDiType& publisher,
                            const SubscriberPortType& subscriber) const noexcept;

    bool sendToAllMatchingPublisherPorts(const capro::CaproMessage& message,
                                         SubscriberPortType& subscriberSource) noexcept;

    void sendToAllMatchingSubscriberPorts(const capro::CaproMessage& message,
                                          PublisherPortRouDiType& publisherSource) noexcept;

    bool isCompatibleClientServer(const popo::ServerPortRouDi& server,
                                  const popo::ClientPortRouDi& client) const noexcept;

    void sendToAllMatchingClientPorts(const capro::CaproMessage& message, popo::ServerPortRouDi& serverSource) noexcept;

    bool sendToAllMatchingServerPorts(const capro::CaproMessage& message, popo::ClientPortRouDi& clientSource) noexcept;

    void sendToAllMatchingInterfacePorts(const capro::CaproMessage& message) noexcept;

    void addPublisherToServiceRegistry(const capro::ServiceDescription& service) noexcept;
    void removePublisherFromServiceRegistry(const capro::ServiceDescription& service) noexcept;

    void addServerToServiceRegistry(const capro::ServiceDescription& service) noexcept;
    void removeServerFromServiceRegistry(const capro::ServiceDescription& service) noexcept;

    template <typename T, std::enable_if_t<std::is_same<T, iox::build::OneToManyPolicy>::value>* = nullptr>
    optional<RuntimeName_t> doesViolateCommunicationPolicy(const capro::ServiceDescription& service) noexcept;

    template <typename T, std::enable_if_t<std::is_same<T, iox::build::ManyToManyPolicy>::value>* = nullptr>
    optional<RuntimeName_t>
    doesViolateCommunicationPolicy(const capro::ServiceDescription& service IOX_MAYBE_UNUSED) noexcept;

    bool isInternal(const capro::ServiceDescription& service) const noexcept;

    void publishServiceRegistry() const noexcept;

    const ServiceRegistry& serviceRegistry() const noexcept;

  private:
    RouDiMemoryInterface* m_roudiMemoryInterface{nullptr};
    PortPool* m_portPool{nullptr};
    ServiceRegistry m_serviceRegistry;
    PortIntrospectionType m_portIntrospection;
    vector<capro::ServiceDescription, NUMBER_OF_INTERNAL_PUBLISHERS> m_internalServices;
    optional<PublisherPortRouDiType::MemberType_t*> m_serviceRegistryPublisherPortData;

    // some ports for the service registry requires special handling
    // as we cannot send registry information if it was not created yet
    expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
    acquirePublisherPortDataWithoutDiscovery(const capro::ServiceDescription& service,
                                             const popo::PublisherOptions& publisherOptions,
                                             const RuntimeName_t& runtimeName,
                                             mepoo::MemoryManager* const payloadDataSegmentMemoryManager,
                                             const PortConfigInfo& portConfigInfo) noexcept;

    PublisherPortRouDiType::MemberType_t* acquireInternalPublisherPortDataWithoutDiscovery(
        const capro::ServiceDescription& service,
        const popo::PublisherOptions& publisherOptions,
        mepoo::MemoryManager* const payloadDataSegmentMemoryManager) noexcept;
};
} // namespace roudi
} // namespace iox

#include "iceoryx_posh/internal/roudi/port_manager.inl"

#endif // IOX_POSH_ROUDI_PORT_MANAGER_HPP
