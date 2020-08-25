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
capro::Interfaces StringToCaProInterface(const cxx::CString100& str);

class PortManager
{
  public:
    using PortConfigInfo = iox::runtime::PortConfigInfo;
    PortManager(RouDiMemoryInterface* roudiMemoryInterface);

    virtual ~PortManager() = default;

    /// @todo Remove this later
    void stopPortIntrospection();

    void doDiscovery();

    virtual cxx::expected<SenderPortType::MemberType_t*, PortPoolError>
    acquireSenderPortData(const capro::ServiceDescription& service,
                          const cxx::CString100& processName,
                          mepoo::MemoryManager* payloadMemoryManager,
                          const cxx::CString100& runnable = "",
                          const PortConfigInfo& portConfigInfo = PortConfigInfo());

    virtual ReceiverPortType::MemberType_t*
    acquireReceiverPortData(const capro::ServiceDescription& service,
                            const cxx::CString100& processName,
                            const cxx::CString100& runnable = "",
                            const PortConfigInfo& portConfigInfo = PortConfigInfo());

    popo::InterfacePortData* acquireInterfacePortData(capro::Interfaces interface,
                                                      const cxx::CString100& processName,
                                                      const cxx::CString100& runnable = "");

    popo::ApplicationPortData* acquireApplicationPortData(const cxx::CString100& processName);

    runtime::RunnableData* acquireRunnableData(const cxx::CString100& process, const cxx::CString100& runnable);

    cxx::expected<popo::ConditionVariableData*, PortPoolError>
    acquireConditionVariableData(const cxx::CString100& processName);

    bool areAllReceiverPortsSubscribed(const cxx::CString100& appName);

    void deletePortsOfProcess(const cxx::CString100& processName);

    void destroySenderPort(SenderPortType::MemberType_t* const senderPortData);

    void destroyReceiverPort(ReceiverPortType::MemberType_t* const receiverPortData);

    const std::atomic<uint64_t>* serviceRegistryChangeCounter();
    runtime::MqMessage findService(const capro::ServiceDescription& service);

  protected:
    void handleSenderPorts();

    void handleReceiverPorts();

    void handleInterfaces();

    void handleApplications();

    void handleRunnables();

    bool sendToAllMatchingSenderPorts(const capro::CaproMessage& message, ReceiverPortType& receiverSource);

    void sendToAllMatchingReceiverPorts(const capro::CaproMessage& message, SenderPortType& senderSource);

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
