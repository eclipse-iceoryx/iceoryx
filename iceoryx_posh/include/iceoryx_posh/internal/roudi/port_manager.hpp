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
capro::Interfaces StringToCaProInterface(const capro::IdString& str) noexcept;

class PortManager
{
  public:
    using PortConfigInfo = iox::runtime::PortConfigInfo;
    PortManager(RouDiMemoryInterface* roudiMemoryInterface) noexcept;

    virtual ~PortManager() = default;

    /// @todo Remove this later
    void stopPortIntrospection() noexcept;

    void doDiscovery() noexcept;

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
                             const PortConfigInfo& portConfigInfo) noexcept;

    cxx::expected<SubscriberPortProducerType::MemberType_t*, PortPoolError>
    acquireSubscriberPortData(const capro::ServiceDescription& service,
                              const uint64_t& historyRequest,
                              const ProcessName_t& processName,
                              const RunnableName_t& runnable,
                              const PortConfigInfo& portConfigInfo) noexcept;

    popo::InterfacePortData* acquireInterfacePortData(capro::Interfaces interface,
                                                      const ProcessName_t& processName,
                                                      const RunnableName_t& runnable = "") noexcept;

    popo::ApplicationPortData* acquireApplicationPortData(const ProcessName_t& processName) noexcept;

    runtime::RunnableData* acquireRunnableData(const ProcessName_t& process, const RunnableName_t& runnable) noexcept;

    cxx::expected<popo::ConditionVariableData*, PortPoolError> acquireConditionVariableData() noexcept;

    void deletePortsOfProcess(const ProcessName_t& processName) noexcept;

    /// @deprecated #25
    void destroySenderPort(SenderPortType::MemberType_t* const senderPortData);

    /// @deprecated #25
    void destroyReceiverPort(ReceiverPortType::MemberType_t* const receiverPortData);

    void destroyPublisherPort(PublisherPortRouDiType::MemberType_t* const publisherPortData) noexcept;

    void destroySubscriberPort(SubscriberPortProducerType::MemberType_t* const subscriberPortData) noexcept;

    const std::atomic<uint64_t>* serviceRegistryChangeCounter() noexcept;
    runtime::MqMessage findService(const capro::ServiceDescription& service) noexcept;

  protected:
    /// @deprecated #25
    void handleSenderPorts();

    /// @deprecated #25
    void handleReceiverPorts();

    void handlePublisherPorts() noexcept;

    void handleSubscriberPorts() noexcept;

    void handleInterfaces() noexcept;

    void handleApplications() noexcept;

    void handleRunnables() noexcept;

    /// @deprecated #25
    bool sendToAllMatchingSenderPorts(const capro::CaproMessage& message, ReceiverPortType& receiverSource);

    /// @deprecated #25
    void sendToAllMatchingReceiverPorts(const capro::CaproMessage& message, SenderPortType& senderSource);

    bool sendToAllMatchingPublisherPorts(const capro::CaproMessage& message,
                                         SubscriberPortProducerType& subscriberSource) noexcept;

    void sendToAllMatchingSubscriberPorts(const capro::CaproMessage& message,
                                          PublisherPortRouDiType& publisherSource) noexcept;

    void sendToAllMatchingInterfacePorts(const capro::CaproMessage& message) noexcept;

    void addEntryToServiceRegistry(const capro::IdString& service, const capro::IdString& instance) noexcept;
    void removeEntryFromServiceRegistry(const capro::IdString& service, const capro::IdString& instance) noexcept;

    // @todo move this to cxx type_traits and create inl
    template <typename T>
    using IsManyToManyPolicy = typename std::
        integral_constant<bool, bool(std::is_same<typename std::decay<T>::type, iox::build::ManyToManyPolicy>::value)>;

    template <typename T>
    using IsOneToManyPolicy = typename std::
        integral_constant<bool, bool(std::is_same<typename std::decay<T>::type, iox::build::OneToManyPolicy>::value)>;

    template <typename T>
    using EnableIf = typename std::enable_if<T::value>::type;

    template <typename T, EnableIf<IsOneToManyPolicy<T>>* = nullptr>
    bool hasDuplicatePublisher(const capro::ServiceDescription& service, const ProcessName_t& processName) noexcept
    {
        // check if the publisher is already in the list
        for (auto publisherPortData : m_portPool->getPublisherPortDataList())
        {
            popo::PublisherPortRouDi publisherPort(publisherPortData);
            if (service == publisherPort.getCaProServiceDescription())
            {
                LogWarn() << "Process '" << processName
                          << "' tried to register an unique PublisherPort which is already used by '"
                          << publisherPortData->m_processName << "' with service '"
                          << service.operator cxx::Serialization().toString() << "'.";
                errorHandler(Error::kPOSH__PORT_MANAGER_PUBLISHERPORT_NOT_UNIQUE, nullptr, ErrorLevel::MODERATE);
                return true;
            }
        }
        return false;
    }

    template <typename T, EnableIf<IsManyToManyPolicy<T>>* = nullptr>
    bool hasDuplicatePublisher(const capro::ServiceDescription& service [[gnu::unused]],
                               const ProcessName_t& processName [[gnu::unused]]) noexcept
    {
        // Duplicates are allowed when using n:m policy
        return false;
    }

  private:
    RouDiMemoryInterface* m_roudiMemoryInterface{nullptr};
    PortPool* m_portPool{nullptr};
    ServiceRegistry m_serviceRegistry;
    PortIntrospectionType m_portIntrospection;
};
} // namespace roudi
} // namespace iox
#endif // IOX_POSH_ROUDI_PORT_MANAGER_HPP
