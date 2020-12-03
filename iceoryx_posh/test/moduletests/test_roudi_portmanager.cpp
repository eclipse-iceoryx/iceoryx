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

#include "test.hpp"

#define protected public
#define private public
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#undef protected
#undef private

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

using iox::popo::PublisherPortUser;
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
        m_portManager->deletePortsOfProcess(iox::MQ_ROUDI_NAME);
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


TEST_F(PortManager_test, doDiscovery_singleShotPublisherFirst)
{
    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData({1, 1, 1}, 1, "/guiseppe", m_payloadMemoryManager, "runnable", PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();
    // no doDiscovery() at this position is intentional

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, 1, "/schlomo", "runnable", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, doDiscovery_singleShotSubscriberFirst)
{
    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, 1, "/schlomo", "runnable", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();
    // no doDiscovery() at this position is intentional

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData({1, 1, 1}, 1, "/guiseppe", m_payloadMemoryManager, "runnable", PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, doDiscovery_singleShotSubscriberFirstWithDiscovery)
{
    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, 1, "/schlomo", "runnable", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();
    m_portManager->doDiscovery();

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData({1, 1, 1}, 1, "/guiseppe", m_payloadMemoryManager, "runnable", PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, doDiscovery_rightOrdering)
{
    SubscriberPortUser subscriber1(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, 1, "/schlomo", "runnable", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber1);
    subscriber1.subscribe();

    m_portManager->doDiscovery();

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData({1, 1, 1}, 1, "/guiseppe", m_payloadMemoryManager, "runnable", PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    SubscriberPortUser subscriber2(
        m_portManager->acquireSubscriberPortData({1, 1, 1}, 1, "/ingnatz", "runnable", PortConfigInfo()).value());
    ASSERT_TRUE(subscriber2);
    subscriber2.subscribe();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
    EXPECT_THAT(subscriber2.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, PublisherSubscriberOverflow)
{
    iox::ProcessName_t p1 = "/test1";
    iox::RunnableName_t r1 = "run1";
    decltype(iox::MAX_PUBLISHERS) pubForP1 = iox::MAX_PUBLISHERS;
    decltype(iox::MAX_SUBSCRIBERS) subForP1 = iox::MAX_SUBSCRIBERS;
    std::vector<iox::popo::PublisherPortData*> avaPublisher1(pubForP1);
    std::vector<iox::popo::SubscriberPortData*> avaSubscriber1(subForP1);


    for (unsigned int i = 0; i < pubForP1; i++)
    {
        auto sen =
            m_portManager->acquirePublisherPortData(getUniqueSD(), 1, p1, m_payloadMemoryManager, r1, PortConfigInfo());

        ASSERT_FALSE(sen.has_error());
        avaPublisher1[i] = sen.value();
    }

    for (unsigned int i = 0; i < subForP1; i++)
    {
        auto rec = m_portManager->acquireSubscriberPortData(getUniqueSD(), 1, p1, r1, PortConfigInfo());
        ASSERT_THAT(rec.has_error(), Eq(false));
        avaSubscriber1[i] = rec.value();
    }

    { // test if overflow errors get hit

        bool errorHandlerCalled = false;
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error error [[gnu::unused]],
                                  const std::function<void()>,
                                  const iox::ErrorLevel) { errorHandlerCalled = true; });
        auto rec = m_portManager->acquireSubscriberPortData(getUniqueSD(), 1, p1, r1, PortConfigInfo());
        EXPECT_TRUE(errorHandlerCalled);
        EXPECT_THAT(rec.get_error(), Eq(PortPoolError::SUBSCRIBER_PORT_LIST_FULL));

        errorHandlerCalled = false;
        auto sen =
            m_portManager->acquirePublisherPortData(getUniqueSD(), 1, p1, m_payloadMemoryManager, r1, PortConfigInfo());
        EXPECT_TRUE(errorHandlerCalled);
        ASSERT_TRUE(sen.has_error());
        EXPECT_THAT(sen.get_error(), Eq(PortPoolError::PUBLISHER_PORT_LIST_FULL));
    }
}

TEST_F(PortManager_test, InterfaceAndApplicationsOverflow)
{
    // overflow of interface and applications
    std::string itf = "/itf";
    std::string app = "/app";

    for (unsigned int i = 0; i < iox::MAX_INTERFACE_NUMBER; i++)
    {
        auto newItfName = itf + std::to_string(i);
        auto interp = m_portManager->acquireInterfacePortData(
            iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        EXPECT_THAT(interp, Ne(nullptr));
    }
    for (unsigned int i = 0; i < iox::MAX_PROCESS_NUMBER; i++)
    {
        auto newAppName = app + std::to_string(i);
        auto appp =
            m_portManager->acquireApplicationPortData(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));
        EXPECT_THAT(appp, Ne(nullptr));
    }

    // test if overflow errors get hit
    {
        auto errorHandlerCalled{false};
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
                errorHandlerCalled = true;
            });

        errorHandlerCalled = false;
        auto interfacePointer =
            m_portManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, "/itfPenguin");
        EXPECT_THAT(interfacePointer, Eq(nullptr));
        EXPECT_TRUE(errorHandlerCalled);

        errorHandlerCalled = false;
        auto appPointer = m_portManager->acquireApplicationPortData("/appPenguin");
        EXPECT_THAT(appPointer, Eq(nullptr));
        EXPECT_TRUE(errorHandlerCalled);
    }

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newItfName = itf + std::to_string(testi);
        auto newAppName = app + std::to_string(testi);
        m_portManager->deletePortsOfProcess(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        m_portManager->deletePortsOfProcess(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));

        auto interfacePointer = m_portManager->acquireInterfacePortData(
            iox::capro::Interfaces::INTERNAL, iox::ProcessName_t(iox::cxx::TruncateToCapacity, newItfName));
        EXPECT_THAT(interfacePointer, Ne(nullptr));

        auto appPointer =
            m_portManager->acquireApplicationPortData(iox::ProcessName_t(iox::cxx::TruncateToCapacity, newAppName));
        EXPECT_THAT(appPointer, Ne(nullptr));
    }
}

TEST_F(PortManager_test, PortDestroy)
{
    iox::ProcessName_t p1 = "/myProcess1";
    iox::ProcessName_t p2 = "/myProcess2";
    iox::capro::ServiceDescription cap1(1, 1, 1);
    iox::capro::ServiceDescription cap2(2, 2, 2);

    // two processes p1 and p2 each with a publisher and subscriber that match to the other process
    auto publisherData1 =
        m_portManager->acquirePublisherPortData(cap1, 1, p1, m_payloadMemoryManager, "runnable", PortConfigInfo())
            .value();
    auto subscriberData1 = m_portManager->acquireSubscriberPortData(cap2, 1, p1, "runnable", PortConfigInfo()).value();

    auto publisherData2 =
        m_portManager->acquirePublisherPortData(cap2, 1, p2, m_payloadMemoryManager, "runnable", PortConfigInfo())
            .value();
    auto subscriberData2 = m_portManager->acquireSubscriberPortData(cap1, 1, p2, "runnable", PortConfigInfo()).value();

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

    // re-create the ports of process p2
    publisherData2 =
        m_portManager->acquirePublisherPortData(cap2, 1, p2, m_payloadMemoryManager, "runnable", PortConfigInfo())
            .value();
    subscriberData2 = m_portManager->acquireSubscriberPortData(cap1, 1, p2, "runnable", PortConfigInfo()).value();

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
