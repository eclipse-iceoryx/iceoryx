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
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_posh/internal/runtime/runnable_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"

#include <mutex>

namespace iox
{
namespace roudi
{
capro::Interfaces StringToCaProInterface(const capro::IdString& str);

class PortManager
{
  public:
    using PortConfigInfo = iox::runtime::PortConfigInfo;
    PortManager(RouDiMemoryInterface* roudiMemoryInterface);

    virtual ~PortManager() = default;

    /// @todo Remove this later
    void stopPortIntrospection();

    void doDiscovery();

    /// @deprecated #25
    virtual cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
    acquireSenderPortData(const capro::ServiceDescription& service,
                          const ProcessName_t& processName,
                          mepoo::MemoryManager* payloadMemoryManager,
                          const RunnableName_t& runnable = "",
                          const PortConfigInfo& portConfigInfo = PortConfigInfo());

    /// @deprecated #25
    virtual ReceiverPortType::MemberType_t*
    acquireReceiverPortData(const capro::ServiceDescription& service,
                            const ProcessName_t& processName,
                            const RunnableName_t& runnable = "",
                            const PortConfigInfo& portConfigInfo = PortConfigInfo());

    cxx::expected<PublisherPortRouDiType::MemberType_t*, PortPoolError>
    acquirePublisherPortData(const capro::ServiceDescription& service,
                             const uint64_t& historyCapacity,
                             const ProcessName_t& processName,
                             mepoo::MemoryManager* payloadMemoryManager,
                             const RunnableName_t& runnable,
                             const PortConfigInfo& portConfigInfo);

    SubscriberPortProducerType::MemberType_t* acquireSubscriberPortData(const capro::ServiceDescription& service,
                                                                        const uint64_t& historyRequest,
                                                                        const ProcessName_t& processName,
                                                                        const RunnableName_t& runnable,
                                                                        const PortConfigInfo& portConfigInfo);

    popo::InterfacePortData* acquireInterfacePortData(capro::Interfaces interface,
                                                      const ProcessName_t& processName,
                                                      const RunnableName_t& runnable = "");

    popo::ApplicationPortData* acquireApplicationPortData(const ProcessName_t& processName);

    runtime::RunnableData* acquireRunnableData(const ProcessName_t& process, const RunnableName_t& runnable);

    cxx::expected<popo::ConditionVariableData*, PortPoolError>
    acquireConditionVariableData(const ProcessName_t& processName);

    void deletePortsOfProcess(const ProcessName_t& processName);

    /// @deprecated #25
    void destroySenderPort(SenderPortType::MemberType_t* const senderPortData);

    /// @deprecated #25
    void destroyReceiverPort(ReceiverPortType::MemberType_t* const receiverPortData);

    void destroyPublisherPort(PublisherPortRouDiType::MemberType_t* const publisherPortData);

    void destroySubscriberPort(SubscriberPortProducerType::MemberType_t* const subscriberPortData);

    const std::atomic<uint64_t>* serviceRegistryChangeCounter();
    runtime::MqMessage findService(const capro::ServiceDescription& service);

  protected:
    /// @deprecated #25
    void handleSenderPorts();

    /// @deprecated #25
    void handleReceiverPorts();

    void handlePublisherPorts();

    void handleSubscriberPorts();

    void handleInterfaces();

    void handleApplications();

    void handleRunnables();

    /// @deprecated #25
    bool sendToAllMatchingSenderPorts(const capro::CaproMessage& message, ReceiverPortType& receiverSource);

    /// @deprecated #25
    void sendToAllMatchingReceiverPorts(const capro::CaproMessage& message, SenderPortType& senderSource);

    bool sendToAllMatchingPublisherPorts(const capro::CaproMessage& message,
                                         SubscriberPortProducerType& subscriberSource);

    void sendToAllMatchingSubscriberPorts(const capro::CaproMessage& message, PublisherPortRouDiType& publisherSource);

    void sendToAllMatchingInterfacePorts(const capro::CaproMessage& message);

    void addEntryToServiceRegistry(const capro::IdString& service, const capro::IdString& instance) noexcept;
    void removeEntryFromServiceRegistry(const capro::IdString& service, const capro::IdString& instance) noexcept;

  private:
    RouDiMemoryInterface* m_roudiMemoryInterface{nullptr};
    PortPool* m_portPool{nullptr};
    ServiceRegistry m_serviceRegistry;
    PortIntrospectionType m_portIntrospection;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_PORT_MANAGER_HPP
