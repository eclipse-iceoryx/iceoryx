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

#pragma once

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/application_port.hpp"
#include "iceoryx_posh/internal/popo/interface_port.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_posh/internal/runtime/runnable_data.hpp"
#include "iceoryx_posh/internal/runtime/shared_memory_creator.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"

#include "ac3log/simplelogger.hpp"

#include <mutex>

namespace iox
{
namespace roudi
{
Interfaces StringToEInterfaces(std::string str);

/// @ brief workaround container until we have a fixed list with the needed functionality
template <typename T, uint64_t Capacity>
class FixedPositionContainer
{
  public:
    static constexpr uint64_t FIRST_ELEMENT = std::numeric_limits<uint64_t>::max();

    bool hasFreeSpace()
    {
        if (m_data.capacity() > m_data.size())
        {
            return true;
        }

        for (auto& e : m_data)
        {
            if (!e.has_value())
            {
                return true;
            }
        }

        return false;
    }

    template <typename... Targs>
    T* insert(Targs&&... args)
    {
        for (auto& e : m_data)
        {
            if (!e.has_value())
            {
                e.emplace(std::forward<Targs>(args)...);
                return &e.value();
            }
        }

        m_data.emplace_back();
        m_data.back().emplace(std::forward<Targs>(args)...);
        return &m_data.back().value();
    }

    void erase(T* const element)
    {
        for (auto& e : m_data)
        {
            if (e.has_value() && &e.value() == element)
            {
                e.reset();
                return;
            }
        }
    }

    cxx::vector<T*, Capacity> content()
    {
        cxx::vector<T*, Capacity> returnValue;
        for (auto& e : m_data)
        {
            if (e.has_value())
            {
                returnValue.emplace_back(&e.value());
            }
        }
        return returnValue;
    }

  private:
    cxx::vector<cxx::optional<T>, Capacity> m_data;
};

// class residing in Shared memory segment
class MiddlewareShm
{
  public:
    MiddlewareShm(posix::Allocator* allocator,
                  const mepoo::SegmentConfig& segmentConfig,
                  const uintptr_t m_sharedMemoryBaseAddressOffset,
                  const bool m_verifySharedMemoryPlacement)
        : m_managementAllocator(allocator)
        , m_segmentManager(segmentConfig, allocator, m_sharedMemoryBaseAddressOffset, m_verifySharedMemoryPlacement)
    {
    }

    posix::Allocator* m_managementAllocator;

    // segment manager
    mepoo::SegmentManager<> m_segmentManager;
    mepoo::MemoryManager m_roudiMemoryManager; /// for roudi services, e.g. introspection

    FixedPositionContainer<SenderPortType::MemberType_t, MAX_PORT_NUMBER> m_senderPortMembers;
    FixedPositionContainer<ReceiverPortType::MemberType_t, MAX_PORT_NUMBER> m_receiverPortMembers;
    FixedPositionContainer<popo::InterfacePortData, MAX_INTERFACE_NUMBER> m_interfacePortMembers;
    FixedPositionContainer<popo::ApplicationPortData, MAX_PROCESS_NUMBER> m_applicationPortMembers;
    FixedPositionContainer<runtime::RunnableData, MAX_RUNNABLE_NUMBER> m_runnableMembers;

    uint64_t m_segmentId;

    // required to be atomic since a service can be offered or stopOffered while reading
    // this variable in a user application
    std::atomic<uint64_t> m_serviceRegistryChangeCounter{0};

    static uint64_t getRequiredSharedMemory()
    {
        return sizeof(MiddlewareShm);
    }
};

class SharedMemoryManager
{
  public:
    SharedMemoryManager(const RouDiConfig_t& config);

    void stopPortIntrospection();

    void doDiscovery();

    SenderPortType::MemberType_t* acquireSenderPortData(const capro::ServiceDescription& service,
                                                        Interfaces interface,
                                                        const std::string& processName,
                                                        mepoo::MemoryManager* payloadMemoryManager,
                                                        const std::string& runnable = "");

    ReceiverPortType::MemberType_t* acquireReceiverPortData(const capro::ServiceDescription& service,
                                                            Interfaces interface,
                                                            const std::string& processName,
                                                            const std::string& runnable = "");
    popo::InterfacePortData*
    acquireInterfacePortData(Interfaces interface, const std::string& processName, const std::string& runnable = "");
    popo::ApplicationPortData* acquireApplicationPortData(Interfaces interface, const std::string& processName);
    runtime::RunnableData* acquireRunnableData(const cxx::CString100& process, const cxx::CString100& runnable);

    bool areAllReceiverPortsSubscribed(std::string appName);

    void deletePortsOfProcess(std::string processName);
    void deleteRunnableAndItsPorts(std::string runnableName);

    void printmempool();
    std::string GetShmAddrString();
    uint64_t getShmSizeInBytes() const;
    const runtime::SharedMemoryCreator<MiddlewareShm>& getShmInterface();

    runtime::MqMessage findService(const capro::ServiceDescription& service);

  protected:
    template <typename PortContainer_t>
    void portDiscoveryHandling(PortContainer_t& portContainer)
    {
        for (auto& port : portContainer)
        {
            port.cyclicServiceUpdate();
        }
    }

    void handleSenderPorts();

    void handleReceiverPorts();

    void handleInterfaces();

    void handleApplications();

    bool sendToAllMatchingSenderPorts(const capro::CaproMessage& message, ReceiverPortType& receiverSource);

    void sendToAllMatchingReceiverPorts(const capro::CaproMessage& message, SenderPortType& senderSource);

    void sendToAllMatchingInterfacePorts(const capro::CaproMessage& message, const iox::Interfaces& interfaceSource);

    void addEntryToServiceRegistry(const capro::ServiceDescription::IdString& service,
                                   const capro::ServiceDescription::IdString& instance) noexcept;
    void removeEntryFromServiceRegistry(const capro::ServiceDescription::IdString& service,
                                        const capro::ServiceDescription::IdString& instance) noexcept;

    /** Shared memory interface for POSIX IPC **/
    runtime::SharedMemoryCreator<MiddlewareShm> m_ShmInterface;

    ServiceRegistry m_serviceRegistry;

    PortIntrospectionType m_portIntrospection;
};

} // namespace roudi
} // namespace iox
