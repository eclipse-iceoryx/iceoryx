// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_PORT_MANAGER_HPP
#define IOX_POSH_ROUDI_PORT_MANAGER_HPP

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/ports/application_port.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/type_traits.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"

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

    virtual ~PortManager() = default;

    /// @todo Remove this later
    void stopPortIntrospection() noexcept;

    void doDiscovery() noexcept;

    cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
    acquirePublisherPortData(const capro::ServiceDescription& service,
                             const popo::PublisherOptions& publisherOptions,
                             const ProcessName_t& processName,
                             mepoo::MemoryManager* payloadMemoryManager,
                             const NodeName_t& node,
                             const PortConfigInfo& portConfigInfo) noexcept;

    cxx::expected<SubscriberPortType::MemberType_t*, PortPoolError>
    acquireSubscriberPortData(const capro::ServiceDescription& service,
                              const popo::SubscriberOptions& subscriberOptions,
                              const ProcessName_t& processName,
                              const NodeName_t& node,
                              const PortConfigInfo& portConfigInfo) noexcept;

    popo::InterfacePortData* acquireInterfacePortData(capro::Interfaces interface,
                                                      const ProcessName_t& processName,
                                                      const NodeName_t& node = "") noexcept;

    popo::ApplicationPortData* acquireApplicationPortData(const ProcessName_t& processName) noexcept;

    runtime::NodeData* acquireNodeData(const ProcessName_t& process, const NodeName_t& node) noexcept;

    cxx::expected<popo::ConditionVariableData*, PortPoolError>
    acquireConditionVariableData(const ProcessName_t& process) noexcept;

    void deletePortsOfProcess(const ProcessName_t& processName) noexcept;

    void destroyPublisherPort(PublisherPortRouDiType::MemberType_t* const publisherPortData) noexcept;

    void destroySubscriberPort(SubscriberPortType::MemberType_t* const subscriberPortData) noexcept;

    const std::atomic<uint64_t>* serviceRegistryChangeCounter() noexcept;
    runtime::MqMessage findService(const capro::ServiceDescription& service) noexcept;

  protected:
    void handlePublisherPorts() noexcept;

    void handleSubscriberPorts() noexcept;

    void handleInterfaces() noexcept;

    void handleApplications() noexcept;

    void handleNodes() noexcept;

    void handleConditionVariables() noexcept;

    bool sendToAllMatchingPublisherPorts(const capro::CaproMessage& message,
                                         SubscriberPortType& subscriberSource) noexcept;

    void sendToAllMatchingSubscriberPorts(const capro::CaproMessage& message,
                                          PublisherPortRouDiType& publisherSource) noexcept;

    void sendToAllMatchingInterfacePorts(const capro::CaproMessage& message) noexcept;

    void addEntryToServiceRegistry(const capro::IdString_t& service, const capro::IdString_t& instance) noexcept;
    void removeEntryFromServiceRegistry(const capro::IdString_t& service, const capro::IdString_t& instance) noexcept;

    template <typename T, std::enable_if_t<std::is_same<T, iox::build::OneToManyPolicy>::value>* = nullptr>
    cxx::optional<ProcessName_t>
    doesViolateCommunicationPolicy(const capro::ServiceDescription& service) const noexcept;

    template <typename T, std::enable_if_t<std::is_same<T, iox::build::ManyToManyPolicy>::value>* = nullptr>
    cxx::optional<ProcessName_t> doesViolateCommunicationPolicy(const capro::ServiceDescription& service
                                                                [[gnu::unused]]) const noexcept;

  private:
    RouDiMemoryInterface* m_roudiMemoryInterface{nullptr};
    PortPool* m_portPool{nullptr};
    ServiceRegistry m_serviceRegistry;
    PortIntrospectionType m_portIntrospection;
};
} // namespace roudi
} // namespace iox

#include "iceoryx_posh/internal/roudi/port_manager.inl"

#endif // IOX_POSH_ROUDI_PORT_MANAGER_HPP
