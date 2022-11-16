// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/roudi/port_manager.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"

#include "test.hpp"

#include <cstdint>
#include <limits>

namespace iox_test_roudi_portmanager
{
using namespace ::testing;
using namespace iox;
using namespace iox::capro;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::roudi;

using iox::runtime::PortConfigInfo;

class PortManagerTester : public PortManager
{
  public:
    PortManagerTester(IceOryxRouDiMemoryManager* roudiMemoryManager)
        : PortManager(roudiMemoryManager)
    {
    }

  private:
    FRIEND_TEST(PortManager_test, CheckDeleteOfPortsFromProcess1);
    FRIEND_TEST(PortManager_test, CheckDeleteOfPortsFromProcess2);
    FRIEND_TEST(PortManager_test, CreateServerWithNotOfferOnCreateDoesNotAddServerToServiceRegistry);
    FRIEND_TEST(PortManager_test, CreateServerWithOfferOnCreateAddsServerToServiceRegistry);
    FRIEND_TEST(PortManager_test, StopOfferRemovesServerFromServiceRegistry);
    FRIEND_TEST(PortManager_test, OfferAddsServerToServiceRegistry);
};

class PortManager_test : public Test
{
  public:
    iox::mepoo::MemoryManager* m_payloadDataSegmentMemoryManager{nullptr};
    IceOryxRouDiMemoryManager* m_roudiMemoryManager{nullptr};
    PortManagerTester* m_portManager{nullptr};

    uint16_t m_instIdCounter, m_eventIdCounter, m_sIdCounter;

    iox::RuntimeName_t m_runtimeName{"TestApp"};

    cxx::vector<iox::capro::ServiceDescription, NUMBER_OF_INTERNAL_PUBLISHERS> internalServices;
    const capro::ServiceDescription serviceRegistry{
        SERVICE_DISCOVERY_SERVICE_NAME, SERVICE_DISCOVERY_INSTANCE_NAME, SERVICE_DISCOVERY_EVENT_NAME};

    void SetUp() override
    {
        m_instIdCounter = m_sIdCounter = 1U;
        m_eventIdCounter = 0;
        // starting at {1,1,1}

        auto config = iox::RouDiConfig_t().setDefaults();
        m_roudiMemoryManager = new IceOryxRouDiMemoryManager(config);
        EXPECT_FALSE(m_roudiMemoryManager->createAndAnnounceMemory().has_error());
        m_portManager = new PortManagerTester(m_roudiMemoryManager);

        auto user = iox::posix::PosixUser::getUserOfCurrentProcess();
        auto segmentInfo =
            m_roudiMemoryManager->segmentManager().value()->getSegmentInformationWithWriteAccessForUser(user);
        ASSERT_TRUE(segmentInfo.m_memoryManager.has_value());

        m_payloadDataSegmentMemoryManager = &segmentInfo.m_memoryManager.value().get();

        // clearing the introspection, is not in d'tor -> SEGFAULT in delete sporadically
        m_portManager->stopPortIntrospection();
        m_portManager->deletePortsOfProcess(iox::roudi::IPC_CHANNEL_ROUDI_NAME);
    }

    void TearDown() override
    {
        delete m_portManager;
        delete m_roudiMemoryManager;
        iox::memory::UntypedRelativePointer::unregisterAll();
    }

    void addInternalPublisherOfPortManagerToVector()
    {
        internalServices.push_back(serviceRegistry);
        internalServices.push_back(IntrospectionPortService);
        internalServices.push_back(IntrospectionPortThroughputService);
        internalServices.push_back(IntrospectionSubscriberPortChangingDataService);
    }

    iox::capro::ServiceDescription getUniqueSD()
    {
        m_eventIdCounter++;
        if (m_eventIdCounter == std::numeric_limits<uint16_t>::max())
        {
            m_eventIdCounter = 1U;
            m_instIdCounter++; // not using max (wildcard)
            if (m_instIdCounter == std::numeric_limits<uint16_t>::max())
            {
                m_instIdCounter = 1U;
                m_sIdCounter++;
                if (m_sIdCounter == std::numeric_limits<uint16_t>::max())
                {
                    // ASSERT_TRUE(false); // limits of test reached no more unique ids possible
                }
            }
        }
        return {iox::capro::IdString_t(TruncateToCapacity, convert::toString(m_sIdCounter)),
                iox::capro::IdString_t(TruncateToCapacity, convert::toString(m_eventIdCounter)),
                iox::capro::IdString_t(TruncateToCapacity, convert::toString(m_instIdCounter))};
    }

    void acquireMaxNumberOfInterfaces(
        std::string runtimeName,
        std::function<void(iox::popo::InterfacePortData*)> f = std::function<void(iox::popo::InterfacePortData*)>())
    {
        for (unsigned int i = 0; i < iox::MAX_INTERFACE_NUMBER; i++)
        {
            auto newProcessName = runtimeName + iox::cxx::convert::toString(i);
            auto interfacePort = m_portManager->acquireInterfacePortData(
                iox::capro::Interfaces::INTERNAL, iox::RuntimeName_t(iox::TruncateToCapacity, newProcessName));
            ASSERT_NE(interfacePort, nullptr);
            if (f)
            {
                f(interfacePort);
            }
        }
    }

    void acquireMaxNumberOfConditionVariables(std::string runtimeName,
                                              std::function<void(iox::popo::ConditionVariableData*)> f =
                                                  std::function<void(iox::popo::ConditionVariableData*)>())
    {
        for (unsigned int i = 0; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; i++)
        {
            auto newProcessName = runtimeName + iox::cxx::convert::toString(i);
            auto condVar = m_portManager->acquireConditionVariableData(
                iox::RuntimeName_t(iox::TruncateToCapacity, newProcessName));
            ASSERT_FALSE(condVar.has_error());
            if (f)
            {
                f(condVar.value());
            }
        }
    }

    void
    acquireMaxNumberOfNodes(std::string nodeName,
                            std::string runtimeName,
                            std::function<void(iox::runtime::NodeData*, iox::NodeName_t, iox::RuntimeName_t)> f =
                                std::function<void(iox::runtime::NodeData*, iox::NodeName_t, iox::RuntimeName_t)>())
    {
        for (unsigned int i = 0U; i < iox::MAX_NODE_NUMBER; i++)
        {
            iox::RuntimeName_t newProcessName(iox::TruncateToCapacity, runtimeName + iox::cxx::convert::toString(i));
            iox::NodeName_t newNodeName(iox::TruncateToCapacity, nodeName + iox::cxx::convert::toString(i));
            auto node = m_portManager->acquireNodeData(newProcessName, newNodeName);
            ASSERT_FALSE(node.has_error());
            if (f)
            {
                f(node.value(), newNodeName, newProcessName);
            }
        }
    }

    void setupAndTestBlockingPublisher(const iox::RuntimeName_t& publisherRuntimeName,
                                       std::function<void()> testHook) noexcept;

    PublisherPortUser createPublisher(const PublisherOptions& options)
    {
        return PublisherPortUser(
            m_portManager
                ->acquirePublisherPortData(
                    {"1", "1", "1"}, options, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
                .value());
    }

    SubscriberPortUser createSubscriber(const SubscriberOptions& options)
    {
        return SubscriberPortUser(
            m_portManager->acquireSubscriberPortData({"1", "1", "1"}, options, "schlomo", PortConfigInfo()).value());
    }

    ClientPortUser createClient(const ClientOptions& options)
    {
        const ServiceDescription sd{"1", "1", "1"};
        const RuntimeName_t runtimeName{"guiseppe"};
        return ClientPortUser(
            *m_portManager->acquireClientPortData(sd, options, runtimeName, m_payloadDataSegmentMemoryManager, {})
                 .value());
    }

    ServerPortUser createServer(const ServerOptions& options)
    {
        const ServiceDescription sd{"1", "1", "1"};
        const RuntimeName_t runtimeName{"schlomo"};
        return ServerPortUser(
            *m_portManager->acquireServerPortData(sd, options, runtimeName, m_payloadDataSegmentMemoryManager, {})
                 .value());
    }
};

template <typename vector>
void setDestroyFlagAndClearContainer(vector& container)
{
    for (auto& item : container)
    {
        item->m_toBeDestroyed.store(true, std::memory_order_relaxed);
    }
    container.clear();
}
} // namespace iox_test_roudi_portmanager
