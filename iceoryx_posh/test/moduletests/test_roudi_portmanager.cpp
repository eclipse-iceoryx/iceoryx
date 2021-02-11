// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

    iox::ProcessName_t m_Process{"TestProcess"};

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
    iox::ProcessName_t p1 = "test1";
    decltype(iox::MAX_PUBLISHERS) pubForP1 = iox::MAX_PUBLISHERS;
    std::vector<iox::popo::PublisherPortData*> avaPublisher1(pubForP1);
    PublisherOptions publisherOptions{1, "run1"};

    for (unsigned int i = 0; i < pubForP1; i++)
    {
        auto publisherPortDataResult = m_portManager->acquirePublisherPortData(
            getUniqueSD(), publisherOptions, p1, m_payloadMemoryManager, PortConfigInfo());

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
            getUniqueSD(), publisherOptions, p1, m_payloadMemoryManager, PortConfigInfo());
        EXPECT_TRUE(errorHandlerCalled);
        ASSERT_TRUE(publisherPortDataResult.has_error());
        EXPECT_THAT(publisherPortDataResult.get_error(), Eq(PortPoolError::PUBLISHER_PORT_LIST_FULL));
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfSubscribersFails)
{
    iox::ProcessName_t p1 = "test1";

    decltype(iox::MAX_SUBSCRIBERS) subForP1 = iox::MAX_SUBSCRIBERS;
    std::vector<iox::popo::SubscriberPortData*> avaSubscriber1(subForP1);
    SubscriberOptions subscriberOptions{1, 1, "run1"};

    for (unsigned int i = 0; i < subForP1; i++)
    {
        auto subscriberPortDataResult =
            m_portManager->acquireSubscriberPortData(getUniqueSD(), subscriberOptions, p1, PortConfigInfo());
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
            m_portManager->acquireSubscriberPortData(getUniqueSD(), subscriberOptions, p1, PortConfigInfo());
        EXPECT_TRUE(errorHandlerCalled);
        EXPECT_THAT(subscriberPortDataResult.get_error(), Eq(PortPoolError::SUBSCRIBER_PORT_LIST_FULL));
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfInterfacesFails)
{
    std::string itf = "itf";

    for (unsigned int i = 0; i < iox::MAX_INTERFACE_NUMBER; i++)
    {
        auto newItfName = itf + std::to_string(i);
        auto interp = m_portManager->acquireInterfacePortData(
            iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        EXPECT_NE(interp, nullptr);
    }

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

TEST_F(PortManager_test, deleteInterfacePortfromMaximumNumberAndAddOneIsSuccessful)
{
    std::string itf = "itf";

    for (unsigned int i = 0; i < iox::MAX_INTERFACE_NUMBER; i++)
    {
        auto newItfName = itf + std::to_string(i);
        auto interp = m_portManager->acquireInterfacePortData(
            iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        EXPECT_NE(interp, nullptr);
    }

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newItfName = itf + std::to_string(testi);
        // this is done because there is no removeInterfaceData method in the PortManager class
        m_portManager->deletePortsOfProcess(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));

        auto interfacePointer = m_portManager->acquireInterfacePortData(
            iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        EXPECT_NE(interfacePointer, nullptr);
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfApplicationsFails)
{
    std::string app = "app";

    for (unsigned int i = 0; i < iox::MAX_PROCESS_NUMBER; i++)
    {
        auto newAppName = app + std::to_string(i);
        auto appp =
            m_portManager->acquireApplicationPortData(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));
        EXPECT_NE(appp, nullptr);
    }

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

TEST_F(PortManager_test, deleteApplicationPortfromMaximumNumberAndAddOneIsSuccessful)
{
    std::string app = "app";

    for (unsigned int i = 0; i < iox::MAX_PROCESS_NUMBER; i++)
    {
        auto newAppName = app + std::to_string(i);
        auto appp =
            m_portManager->acquireApplicationPortData(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));
        EXPECT_NE(appp, nullptr);
    }

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newAppName = app + std::to_string(testi);
        // this is done because there is no removeApplicationData method in the PortManager class
        m_portManager->deletePortsOfProcess(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));

        auto appPointer =
            m_portManager->acquireApplicationPortData(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));
        EXPECT_NE(appPointer, nullptr);
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfConditionVariablesFails)
{
    std::string process = "HypnoToadForEver";

    for (unsigned int i = 0; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; i++)
    {
        auto newProcessName = process + std::to_string(i);
        auto conditionVariableDataResult = m_portManager->acquireConditionVariableData(
            iox::ProcessName_t(iox::cxx::TruncateToCapacity, newProcessName));
        EXPECT_FALSE(conditionVariableDataResult.has_error());
    }

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

TEST_F(PortManager_test, deleteConditionVariablePortfromMaximumNumberAndAddOneIsSuccessful)
{
    std::string process = "HypnoToadForEver";

    for (unsigned int i = 0; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; i++)
    {
        auto newProcessName = process + std::to_string(i);
        auto conditionVariableDataResult = m_portManager->acquireConditionVariableData(
            iox::ProcessName_t(iox::cxx::TruncateToCapacity, newProcessName));
        EXPECT_FALSE(conditionVariableDataResult.has_error());
    }

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newProcessName = process + std::to_string(testi);
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

    std::string process = "HypnoToadForEver";

    // first aquire all possible condition variables
    for (unsigned int i = 0; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; i++)
    {
        auto conditionVariableDataResult = m_portManager->acquireConditionVariableData("JustFollowTheHypnoToad");
        ASSERT_FALSE(conditionVariableDataResult.has_error());
        condVarContainer.push_back(conditionVariableDataResult.value());
    }

    // so now no one should be available
    {
        auto conditionVariableDataResult = m_portManager->acquireConditionVariableData("JustFollowTheHypnoToad");
        EXPECT_TRUE(conditionVariableDataResult.has_error());
    }

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(condVarContainer);
    m_portManager->doDiscovery();

    // so we should able to get some more now
    for (unsigned int i = 0; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; i++)
    {
        auto conditionVariableDataResult = m_portManager->acquireConditionVariableData("JustFollowTheHypnoToad");
        EXPECT_FALSE(conditionVariableDataResult.has_error());
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfNodesFails)
{
    std::string process = "Process";
    std::string node = "Node";

    // first acquire all possible NodeData
    for (unsigned int i = 0U; i < iox::MAX_NODE_NUMBER; i++)
    {
        iox::ProcessName_t newProcessName(iox::cxx::TruncateToCapacity, process + std::to_string(i));
        iox::NodeName_t newNodeName(iox::cxx::TruncateToCapacity, node + std::to_string(i));
        auto nodeData = m_portManager->acquireNodeData(newProcessName, newNodeName);
        ASSERT_THAT(nodeData.has_error(), Eq(false));
        EXPECT_THAT(nodeData.value()->m_node, StrEq(newNodeName));
        EXPECT_THAT(nodeData.value()->m_process, StrEq(newProcessName));
    }

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

TEST_F(PortManager_test, deleteNodePortfromMaximumNumberandAddOneIsSuccessful)
{
    std::string process = "Process";
    std::string node = "Node";

    // first acquire all possible NodeData
    for (unsigned int i = 0U; i < iox::MAX_NODE_NUMBER; i++)
    {
        iox::ProcessName_t newProcessName(iox::cxx::TruncateToCapacity, process + std::to_string(i));
        iox::NodeName_t newNodeName(iox::cxx::TruncateToCapacity, node + std::to_string(i));
        auto nodeData = m_portManager->acquireNodeData(newProcessName, newNodeName);
        ASSERT_THAT(nodeData.has_error(), Eq(false));
    }

    // delete one and add one NodeData should be possible now
    unsigned int i = 0U;
    iox::ProcessName_t newProcessName(iox::cxx::TruncateToCapacity, process + std::to_string(i));
    iox::NodeName_t newNodeName(iox::cxx::TruncateToCapacity, node + std::to_string(i));
    // this is done because there is no removeNodeData method in the PortManager class
    m_portManager->deletePortsOfProcess(newProcessName);

    auto nodeData = m_portManager->acquireNodeData(newProcessName, newNodeName);
    EXPECT_THAT(nodeData.has_error(), Eq(false));
    EXPECT_THAT(nodeData.value()->m_node, StrEq(newNodeName));
    EXPECT_THAT(nodeData.value()->m_process, StrEq(newProcessName));
}


TEST_F(PortManager_test, AcquireNodeDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    iox::ProcessName_t process = "Humuhumunukunukuapua'a";
    iox::NodeName_t node = "Taumatawhakatangihangakoauauotamateaturipukakapikimaungahoronukupokaiwhenuakitanatahu";
    std::vector<iox::runtime::NodeData*> nodeContainer;

    // first acquire all possible NodeData
    for (unsigned int i = 0U; i < iox::MAX_NODE_NUMBER; i++)
    {
        auto nodeData = m_portManager->acquireNodeData(process, node);
        ASSERT_FALSE(nodeData.has_error());
        nodeContainer.push_back(nodeData.value());
    }

    // so now no NodeData should be available
    auto nodeData = m_portManager->acquireNodeData(process, node);
    EXPECT_THAT(nodeData.has_error(), Eq(true));

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(nodeContainer);
    m_portManager->doDiscovery();

    // so we should be able to get some more now
    for (unsigned int i = 0U; i < iox::MAX_NODE_NUMBER; i++)
    {
        auto nodeData = m_portManager->acquireNodeData(process, node);
        EXPECT_THAT(nodeData.has_error(), Eq(false));
    }
}

TEST_F(PortManager_test, DestroyPortsInProcessP2ChangesStatesOfPortsInProcessP1)
{
    iox::ProcessName_t p1 = "myProcess1";
    iox::ProcessName_t p2 = "myProcess2";
    iox::capro::ServiceDescription cap1(1, 1, 1);
    iox::capro::ServiceDescription cap2(2, 2, 2);
    PublisherOptions publisherOptions{1, "node"};
    SubscriberOptions subscriberOptions{1, 1, "node"};

    // two processes p1 and p2 each with a publisher and subscriber that match to the other process
    auto publisherData1 =
        m_portManager->acquirePublisherPortData(cap1, publisherOptions, p1, m_payloadMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData1 =
        m_portManager->acquireSubscriberPortData(cap2, subscriberOptions, p1, PortConfigInfo()).value();

    auto publisherData2 =
        m_portManager->acquirePublisherPortData(cap2, publisherOptions, p2, m_payloadMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData2 =
        m_portManager->acquireSubscriberPortData(cap1, subscriberOptions, p2, PortConfigInfo()).value();

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

    // destroy the ports of process p2 and check if states of ports in p1 changed as expected
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
}

TEST_F(PortManager_test, CleanupProcessP2ChangesStatesOfPortsInProcessP1)
{
    iox::ProcessName_t p1 = "myProcess1";
    iox::ProcessName_t p2 = "myProcess2";
    iox::capro::ServiceDescription cap1(1, 1, 1);
    iox::capro::ServiceDescription cap2(2, 2, 2);
    PublisherOptions publisherOptions{1, "node"};
    SubscriberOptions subscriberOptions{1, 1, "node"};

    // two processes p1 and p2 each with a publisher and subscriber that match to the other process
    auto publisherData1 =
        m_portManager->acquirePublisherPortData(cap1, publisherOptions, p1, m_payloadMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData1 =
        m_portManager->acquireSubscriberPortData(cap2, subscriberOptions, p1, PortConfigInfo()).value();

    auto publisherData2 =
        m_portManager->acquirePublisherPortData(cap2, publisherOptions, p2, m_payloadMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData2 =
        m_portManager->acquireSubscriberPortData(cap1, subscriberOptions, p2, PortConfigInfo()).value();

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

    // cleanup process p2 and check if states of ports in p1 changed  as expected
    {
        m_portManager->deletePortsOfProcess(p2);
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

TEST_F(PortManager_test, AcquireInterfacePortDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    std::vector<iox::popo::InterfacePortData*> interfaceContainer;
    std::string itf = "itf";

    // first aquire all possible condition variables
    for (uint32_t i = 0U; i < iox::MAX_INTERFACE_NUMBER; i++)
    {
        auto newItfName = itf + std::to_string(i);
        auto interp = m_portManager->acquireInterfacePortData(
            iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        ASSERT_NE(interp, nullptr);
        interfaceContainer.push_back(interp);
    }

    // so now no one should be available
    {
        auto interp = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, m_Process);
        EXPECT_EQ(interp, nullptr);
    }

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(interfaceContainer);
    m_portManager->doDiscovery();

    // so we should able to get some more now
    for (uint32_t i = 0U; i < iox::MAX_INTERFACE_NUMBER; i++)
    {
        auto interp = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, m_Process);
        EXPECT_NE(interp, nullptr);
    }
}

TEST_F(PortManager_test, OfferPublisherServiceUpdatesServiceRegistryChangeCounter)
{
    auto serviceCounter = m_portManager->serviceRegistryChangeCounter();
    auto initialCount = serviceCounter->load();
    PublisherOptions publisherOptions{1};

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData({1, 1, 1}, publisherOptions, m_Process, m_payloadMemoryManager, PortConfigInfo())
            .value());

    publisher.offer();
    m_portManager->doDiscovery();

    EXPECT_EQ(initialCount + 1 == serviceCounter->load(), true);
}
