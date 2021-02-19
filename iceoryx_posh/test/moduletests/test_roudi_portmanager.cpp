// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/roudi/port_manager.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"

#include <cstdint>
#include <limits> // std::numeric_limits

using namespace ::testing;
using ::testing::Return;

using iox::popo::PublisherOptions;
using iox::popo::PublisherPortUser;
using iox::popo::SubscriberOptions;
using iox::popo::SubscriberPortUser;
using iox::roudi::IceOryxRouDiMemoryManager;
using iox::roudi::PortManager;
using iox::roudi::PortPoolError;
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
};

class PortManager_test : public Test
{
  public:
    iox::mepoo::MemoryManager* m_payloadMemoryManager{nullptr};
    IceOryxRouDiMemoryManager* m_roudiMemoryManager{nullptr};
    PortManagerTester* m_portManager{nullptr};

    uint16_t m_instIdCounter, m_eventIdCounter, m_sIdCounter;

    iox::ProcessName_t m_ProcessName{"TestProcess"};

    void SetUp() override
    {
        testing::internal::CaptureStderr();
        m_instIdCounter = m_sIdCounter = 1;
        m_eventIdCounter = 0;
        // starting at {1,1,1}

        auto config = iox::RouDiConfig_t().setDefaults();
        m_roudiMemoryManager = new IceOryxRouDiMemoryManager(config);
        m_roudiMemoryManager->createAndAnnounceMemory();
        m_portManager = new PortManagerTester(m_roudiMemoryManager);

        auto user = iox::posix::PosixUser::getUserOfCurrentProcess().getName();
        m_payloadMemoryManager =
            m_roudiMemoryManager->segmentManager().value()->getSegmentInformationForUser(user).m_memoryManager;

        // clearing the introspection, is not in d'tor -> SEGFAULT in delete sporadically
        m_portManager->stopPortIntrospection();
        m_portManager->deletePortsOfProcess(iox::roudi::IPC_CHANNEL_ROUDI_NAME);
    }

    void TearDown() override
    {
        delete m_portManager;
        delete m_roudiMemoryManager;
        iox::RelativePointer::unregisterAll();

        if (Test::HasFailure())
        {
            std::cout << testing::internal::GetCapturedStderr() << std::endl;
        }
        else
        {
            (void)testing::internal::GetCapturedStderr();
        }
    }
    iox::capro::ServiceDescription getUniqueSD()
    {
        m_eventIdCounter++;
        if (m_eventIdCounter == std::numeric_limits<uint16_t>::max())
        {
            m_eventIdCounter = 1;
            m_instIdCounter++; // not using max (wildcard)
            if (m_instIdCounter == std::numeric_limits<uint16_t>::max())
            {
                m_instIdCounter = 1;
                m_sIdCounter++;
                if (m_sIdCounter == std::numeric_limits<uint16_t>::max())
                {
                    // ASSERT_TRUE(false); // limits of test reached no more unique ids possible
                }
            }
        }
        return {m_sIdCounter, m_eventIdCounter, m_instIdCounter};
    }

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};

    void acquireMaxNumberOfInterfaces(
        std::string itfName,
        std::function<void(iox::popo::InterfacePortData*)> f = std::function<void(iox::popo::InterfacePortData*)>())
    {
        for (unsigned int i = 0; i < iox::MAX_INTERFACE_NUMBER; i++)
        {
            auto newItfName = itfName + std::to_string(i);
            auto interfaceport = m_portManager->acquireInterfacePortData(
                iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
            ASSERT_NE(interfaceport, nullptr);
            if (f)
            {
                f(interfaceport);
            }
        }
    }

    void acquireMaxNumberOfApplications(
        std::string appName,
        std::function<void(iox::popo::ApplicationPortData*)> f = std::function<void(iox::popo::ApplicationPortData*)>())
    {
        for (unsigned int i = 0; i < iox::MAX_PROCESS_NUMBER; i++)
        {
            auto newAppName = appName + std::to_string(i);
            auto applicationport =
                m_portManager->acquireApplicationPortData(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));
            ASSERT_NE(applicationport, nullptr);
            if (f)
            {
                f(applicationport);
            }
        }
    }

    void acquireMaxNumberOfConditionVariables(std::string processName,
                                              std::function<void(iox::popo::ConditionVariableData*)> f =
                                                  std::function<void(iox::popo::ConditionVariableData*)>())
    {
        for (unsigned int i = 0; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; i++)
        {
            auto newProcessName = processName + std::to_string(i);
            auto condVarport = m_portManager->acquireConditionVariableData(
                iox::ProcessName_t(iox::cxx::TruncateToCapacity, newProcessName));
            ASSERT_FALSE(condVarport.has_error());
            if (f)
            {
                f(condVarport.value());
            }
        }
    }

    void
    acquireMaxNumberOfNodes(std::string nodeName,
                            std::string processName,
                            std::function<void(iox::runtime::NodeData*, iox::NodeName_t, iox::ProcessName_t)> f =
                                std::function<void(iox::runtime::NodeData*, iox::NodeName_t, iox::ProcessName_t)>())
    {
        for (unsigned int i = 0U; i < iox::MAX_NODE_NUMBER; i++)
        {
            iox::ProcessName_t newProcessName(iox::cxx::TruncateToCapacity, processName + std::to_string(i));
            iox::NodeName_t newNodeName(iox::cxx::TruncateToCapacity, nodeName + std::to_string(i));
            auto nodePort = m_portManager->acquireNodeData(newProcessName, newNodeName);
            ASSERT_FALSE(nodePort.has_error());
            if (f)
            {
                f(nodePort.value(), newNodeName, newProcessName);
            }
        }
    }
};

template <typename vector>
void setDestroyFlagAndClearContainer(vector& Container)
{
    for (auto item : Container)
    {
        item->m_toBeDestroyed.store(true, std::memory_order_relaxed);
    }
    Container.clear();
}

TEST_F(PortManager_test, doDiscovery_singleShotPublisherFirst)
{
    PublisherOptions publisherOptions{1, "node"};
    SubscriberOptions subscriberOptions{1, 1, "node"};

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {1, 1, 1}, publisherOptions, "guiseppe", m_payloadMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();
    // no doDiscovery() at this position is intentional

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, subscriberOptions, "schlomo", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, doDiscovery_singleShotSubscriberFirst)
{
    PublisherOptions publisherOptions{1, "node"};
    SubscriberOptions subscriberOptions{1, 1, "node"};

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, subscriberOptions, "schlomo", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();
    // no doDiscovery() at this position is intentional

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {1, 1, 1}, publisherOptions, "guiseppe", m_payloadMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, doDiscovery_singleShotSubscriberFirstWithDiscovery)
{
    PublisherOptions publisherOptions{1, "node"};
    SubscriberOptions subscriberOptions{1, 1, "node"};

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, subscriberOptions, "schlomo", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();
    m_portManager->doDiscovery();

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {1, 1, 1}, publisherOptions, "guiseppe", m_payloadMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, doDiscovery_rightOrdering)
{
    PublisherOptions publisherOptions{1, "node"};
    SubscriberOptions subscriberOptions{1, 1, "node"};

    SubscriberPortUser subscriber1(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, subscriberOptions, "schlomo", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber1);
    subscriber1.subscribe();

    m_portManager->doDiscovery();

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {1, 1, 1}, publisherOptions, "guiseppe", m_payloadMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    SubscriberPortUser subscriber2(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, subscriberOptions, "ingnatz", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber2);
    subscriber2.subscribe();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
    EXPECT_THAT(subscriber2.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfPublishersFails)
{
    iox::ProcessName_t processName1 = "test1";
    decltype(iox::MAX_PUBLISHERS) pubForP1 = iox::MAX_PUBLISHERS;
    std::vector<iox::popo::PublisherPortData*> avaPublisher1(pubForP1);
    PublisherOptions publisherOptions{1, "run1"};

    for (unsigned int i = 0; i < pubForP1; i++)
    {
        auto publisherPortDataResult = m_portManager->acquirePublisherPortData(
            getUniqueSD(), publisherOptions, processName1, m_payloadMemoryManager, PortConfigInfo());

        ASSERT_FALSE(publisherPortDataResult.has_error());
        avaPublisher1[i] = publisherPortDataResult.value();
    }

    { // test if overflow errors get hit

        bool errorHandlerCalled = false;
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error error [[gnu::unused]],
                                  const std::function<void()>,
                                  const iox::ErrorLevel) { errorHandlerCalled = true; });

        auto publisherPortDataResult = m_portManager->acquirePublisherPortData(
            getUniqueSD(), publisherOptions, processName1, m_payloadMemoryManager, PortConfigInfo());
        EXPECT_TRUE(errorHandlerCalled);
        ASSERT_TRUE(publisherPortDataResult.has_error());
        EXPECT_THAT(publisherPortDataResult.get_error(), Eq(PortPoolError::PUBLISHER_PORT_LIST_FULL));
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfSubscribersFails)
{
    iox::ProcessName_t processName1 = "test1";

    decltype(iox::MAX_SUBSCRIBERS) subForP1 = iox::MAX_SUBSCRIBERS;
    std::vector<iox::popo::SubscriberPortData*> avaSubscriber1(subForP1);
    SubscriberOptions subscriberOptions{1, 1, "run1"};

    for (unsigned int i = 0; i < subForP1; i++)
    {
        auto subscriberPortDataResult =
            m_portManager->acquireSubscriberPortData(getUniqueSD(), subscriberOptions, processName1, PortConfigInfo());
        ASSERT_THAT(subscriberPortDataResult.has_error(), Eq(false));
        avaSubscriber1[i] = subscriberPortDataResult.value();
    }

    { // test if overflow errors get hit

        bool errorHandlerCalled = false;
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error error [[gnu::unused]],
                                  const std::function<void()>,
                                  const iox::ErrorLevel) { errorHandlerCalled = true; });
        auto subscriberPortDataResult =
            m_portManager->acquireSubscriberPortData(getUniqueSD(), subscriberOptions, processName1, PortConfigInfo());
        EXPECT_TRUE(errorHandlerCalled);
        EXPECT_THAT(subscriberPortDataResult.get_error(), Eq(PortPoolError::SUBSCRIBER_PORT_LIST_FULL));
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfInterfacesFails)
{
    std::string itfName = "itf";

    // first aquire all possible Interfaces
    acquireMaxNumberOfInterfaces(itfName);

    // test if overflow errors get hit
    {
        auto errorHandlerCalled{false};
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
                errorHandlerCalled = true;
            });

        auto interfacePointer = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, "itfPenguin");
        EXPECT_EQ(interfacePointer, nullptr);
        EXPECT_TRUE(errorHandlerCalled);
    }
}

TEST_F(PortManager_test, DeleteInterfacePortfromMaximumNumberAndAddOneIsSuccessful)
{
    std::string itfName = "itf";

    // first aquire all possible Interfaces
    acquireMaxNumberOfInterfaces(itfName);

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newItfName = itfName + std::to_string(testi);
        // this is done because there is no removeInterfaceData method in the PortManager class
        m_portManager->deletePortsOfProcess(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));

        auto interfacePointer = m_portManager->acquireInterfacePortData(
            iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        EXPECT_NE(interfacePointer, nullptr);
    }
}

TEST_F(PortManager_test, AcquireInterfacePortDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    std::vector<iox::popo::InterfacePortData*> interfaceContainer;
    std::string itfName = "itf";

    // first aquire all possible interfaces
    acquireMaxNumberOfInterfaces(itfName, [&](auto InterafcePort) { interfaceContainer.push_back(InterafcePort); });

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(interfaceContainer);
    m_portManager->doDiscovery();

    // so we should able to get some more now
    acquireMaxNumberOfInterfaces(itfName);
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfApplicationsFails)
{
    std::string appName = "app";

    // first aquire all possible applications
    acquireMaxNumberOfApplications(appName);

    // test if overflow errors get hit
    {
        auto errorHandlerCalled{false};
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
                errorHandlerCalled = true;
            });

        auto appPointer = m_portManager->acquireApplicationPortData("appPenguin");
        EXPECT_EQ(appPointer, nullptr);
        EXPECT_TRUE(errorHandlerCalled);
    }
}

TEST_F(PortManager_test, DeleteApplicationPortfromMaximumNumberAndAddOneIsSuccessful)
{
    std::string appName = "app";

    // first aquire all possible applications
    acquireMaxNumberOfApplications(appName);

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newAppName = appName + std::to_string(testi);
        // this is done because there is no removeApplicationData method in the PortManager class
        m_portManager->deletePortsOfProcess(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));

        auto appPointer =
            m_portManager->acquireApplicationPortData(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));
        EXPECT_NE(appPointer, nullptr);
    }
}

TEST_F(PortManager_test, AcquireApplicationDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    std::vector<iox::popo::ApplicationPortData*> ApplicationContainer;

    std::string appName = "app";

    // first aquire all possible applications
    acquireMaxNumberOfApplications(appName,
                                   [&](auto Applicationport) { ApplicationContainer.push_back(Applicationport); });

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(ApplicationContainer);
    m_portManager->doDiscovery();

    // so we should able to get some more now
    acquireMaxNumberOfApplications(appName);
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfConditionVariablesFails)
{
    std::string processName = "HypnoToadForEver";

    // first aquire all possible condition variables
    acquireMaxNumberOfConditionVariables(processName);

    // test if overflow errors get hit
    {
        auto errorHandlerCalled{false};
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
                errorHandlerCalled = true;
            });

        auto conditionVariableDataResult = m_portManager->acquireConditionVariableData("AnotherToad");
        EXPECT_TRUE(conditionVariableDataResult.has_error());
        EXPECT_TRUE(errorHandlerCalled);
        EXPECT_THAT(conditionVariableDataResult.get_error(), Eq(PortPoolError::CONDITION_VARIABLE_LIST_FULL));
    }
}

TEST_F(PortManager_test, DeleteConditionVariablePortfromMaximumNumberAndAddOneIsSuccessful)
{
    std::string processName = "HypnoToadForEver";

    // first aquire all possible condition variables
    acquireMaxNumberOfConditionVariables(processName);

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newProcessName = processName + std::to_string(testi);
        // this is done because there is no removeConditionVariableData method in the PortManager class
        m_portManager->deletePortsOfProcess(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newProcessName));

        auto conditionVariableDataResult = m_portManager->acquireConditionVariableData(
            iox::ProcessName_t(iox::cxx::TruncateToCapacity, newProcessName));
        EXPECT_FALSE(conditionVariableDataResult.has_error());
    }
}

TEST_F(PortManager_test, AcquireConditionVariablesDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    std::vector<iox::popo::ConditionVariableData*> condVarContainer;

    std::string processName = "HypnoToadForEver";

    // first aquire all possible condition variables
    acquireMaxNumberOfConditionVariables(processName,
                                         [&](auto CondVarport) { condVarContainer.push_back(CondVarport); });

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(condVarContainer);
    m_portManager->doDiscovery();

    // so we should able to get some more now
    acquireMaxNumberOfConditionVariables(processName);
}

TEST_F(PortManager_test, AcquireMaxNumberOfNodePorts)
{
    std::string processName = "Process";
    std::string nodeName = "Node";

    acquireMaxNumberOfNodes(nodeName, processName, [&](auto NodePort, auto newNodeName, auto newProcessName) {
        EXPECT_THAT(NodePort->m_node, StrEq(newNodeName));
        EXPECT_THAT(NodePort->m_process, StrEq(newProcessName));
    });
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfNodesFails)
{
    std::string processName = "Process";
    std::string nodeName = "Node";

    // first acquire all possible NodeData
    acquireMaxNumberOfNodes(nodeName, processName);

    // test if overflow errors get hit
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    auto nodeData = m_portManager->acquireNodeData("AnotherProcess", "AnotherNode");
    EXPECT_THAT(nodeData.has_error(), Eq(true));
    EXPECT_THAT(errorHandlerCalled, Eq(true));
    EXPECT_THAT(nodeData.get_error(), Eq(PortPoolError::NODE_DATA_LIST_FULL));
}

TEST_F(PortManager_test, DeleteNodePortfromMaximumNumberandAddOneIsSuccessful)
{
    std::string processName = "Process";
    std::string nodeName = "Node";

    // first acquire all possible NodeData
    acquireMaxNumberOfNodes(nodeName, processName);

    // delete one and add one NodeData should be possible now
    unsigned int i = 0U;
    iox::ProcessName_t newProcessName(iox::cxx::TruncateToCapacity, processName + std::to_string(i));
    iox::NodeName_t newNodeName(iox::cxx::TruncateToCapacity, nodeName + std::to_string(i));
    // this is done because there is no removeNodeData method in the PortManager class
    m_portManager->deletePortsOfProcess(newProcessName);

    auto nodeData = m_portManager->acquireNodeData(newProcessName, newNodeName);
    EXPECT_THAT(nodeData.has_error(), Eq(false));
    EXPECT_THAT(nodeData.value()->m_node, StrEq(newNodeName));
    EXPECT_THAT(nodeData.value()->m_process, StrEq(newProcessName));
}


TEST_F(PortManager_test, AcquireNodeDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    iox::ProcessName_t processName = "Humuhumunukunukuapua'a";
    iox::NodeName_t nodeName = "Taumatawhakatangihangakoauauotamateaturipukakapikimaungahoronukupokaiwhenuakitanatahu";
    std::vector<iox::runtime::NodeData*> nodeContainer;

    // first acquire all possible NodeData
    acquireMaxNumberOfNodes(nodeName, processName, [&](auto NodePort, auto newNodeName, auto newProcessName) {
        nodeContainer.push_back(NodePort);
        EXPECT_THAT(NodePort->m_node, StrEq(newNodeName));
        EXPECT_THAT(NodePort->m_process, StrEq(newProcessName));
    });

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(nodeContainer);
    m_portManager->doDiscovery();

    // so we should be able to get some more now
    acquireMaxNumberOfNodes(nodeName, processName);
}

TEST_F(PortManager_test, PortsDestroyInProcess2ChangeStatesOfPortsInProcess1)
{
    iox::ProcessName_t processName1 = "myProcess1";
    iox::ProcessName_t processName2 = "myProcess2";
    iox::capro::ServiceDescription cap1(1, 1, 1);
    iox::capro::ServiceDescription cap2(2, 2, 2);
    PublisherOptions publisherOptions{1, "node"};
    SubscriberOptions subscriberOptions{1, 1, "node"};

    // two processes process1 and process2 each with a publisher and subscriber that match to the other process
    auto publisherData1 =
        m_portManager
            ->acquirePublisherPortData(cap1, publisherOptions, processName1, m_payloadMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData1 =
        m_portManager->acquireSubscriberPortData(cap2, subscriberOptions, processName1, PortConfigInfo()).value();

    auto publisherData2 =
        m_portManager
            ->acquirePublisherPortData(cap2, publisherOptions, processName2, m_payloadMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData2 =
        m_portManager->acquireSubscriberPortData(cap1, subscriberOptions, processName2, PortConfigInfo()).value();

    // let them connect
    {
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        publisher1.offer();
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);
        subscriber1.subscribe();

        PublisherPortUser publisher2(publisherData2);
        ASSERT_TRUE(publisher2);
        publisher2.offer();
        SubscriberPortUser subscriber2(subscriberData2);
        ASSERT_TRUE(subscriber2);
        subscriber2.subscribe();

        m_portManager->doDiscovery();

        ASSERT_TRUE(publisher1.hasSubscribers());
        ASSERT_TRUE(publisher2.hasSubscribers());
        EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
        EXPECT_THAT(subscriber2.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
    }

    // destroy the ports of process2 and check if states of ports in process1 changed as expected
    {
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);

        PublisherPortUser publisher2(publisherData2);
        ASSERT_TRUE(publisher2);
        publisher2.destroy();
        SubscriberPortUser subscriber2(subscriberData2);
        ASSERT_TRUE(subscriber2);
        subscriber2.destroy();

        m_portManager->doDiscovery();

        ASSERT_FALSE(publisher1.hasSubscribers());
        if (std::is_same<iox::build::CommunicationPolicy, iox::build::OneToManyPolicy>::value)
        {
            EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::WAIT_FOR_OFFER));
        }
    }

    // re-create the ports of process processName2
    publisherData2 =
        m_portManager
            ->acquirePublisherPortData(cap2, publisherOptions, processName2, m_payloadMemoryManager, PortConfigInfo())
            .value();
    subscriberData2 =
        m_portManager->acquireSubscriberPortData(cap1, subscriberOptions, processName2, PortConfigInfo()).value();

    // let them connect
    {
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);

        PublisherPortUser publisher2(publisherData2);
        ASSERT_TRUE(publisher2);
        publisher2.offer();
        SubscriberPortUser subscriber2(subscriberData2);
        ASSERT_TRUE(subscriber2);
        subscriber2.subscribe();

        m_portManager->doDiscovery();

        ASSERT_TRUE(publisher1.hasSubscribers());
        ASSERT_TRUE(publisher2.hasSubscribers());
        EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
        EXPECT_THAT(subscriber2.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
    }

    // cleanup process process2 and check if states of ports in process1 changed  as expected
    {
        m_portManager->deletePortsOfProcess(processName2);
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);

        ASSERT_FALSE(publisher1.hasSubscribers());
        if (std::is_same<iox::build::CommunicationPolicy, iox::build::OneToManyPolicy>::value)
        {
            EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::WAIT_FOR_OFFER));
        }
    }
}

TEST_F(PortManager_test, OfferPublisherServiceUpdatesServiceRegistryChangeCounter)
{
    auto serviceCounter = m_portManager->serviceRegistryChangeCounter();
    ASSERT_NE(serviceCounter, nullptr);

    auto initialCount = serviceCounter->load();
    PublisherOptions publisherOptions{1};

    auto publisherportdata = m_portManager->acquirePublisherPortData(
        {1, 1, 1}, publisherOptions, m_ProcessName, m_payloadMemoryManager, PortConfigInfo());
    ASSERT_FALSE(publisherportdata.has_error());

    PublisherPortUser publisher(publisherportdata.value());

    publisher.offer();
    m_portManager->doDiscovery();

    EXPECT_EQ(serviceCounter->load(), initialCount + 1);
}
