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
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#undef protected
#undef private

#include "iceoryx_posh/iceoryx_posh_types.hpp" // MAX_PORT_NUMBER
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/roudi/port_manager.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"

#include <cstdint>
#include <limits> // std::numeric_limits

using namespace ::testing;
using ::testing::Return;

using iox::popo::ReceiverPort;
using iox::popo::SenderPort;
using iox::roudi::IceOryxRouDiMemoryManager;
using iox::roudi::PortPoolError;
using iox::roudi::PortManager;

class CShmMangerTester : public PortManager
{
  public:
    CShmMangerTester(IceOryxRouDiMemoryManager* roudiMemoryManager)
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
    CShmMangerTester* m_shmManager{nullptr};

    uint16_t m_instIdCounter, m_eventIdCounter, m_sIdCounter;
    void SetUp() override
    {
        testing::internal::CaptureStderr();
        m_instIdCounter = m_sIdCounter = 1;
        m_eventIdCounter = 0;
        // starting at {1,1,1}

        auto config = iox::RouDiConfig_t().setDefaults();
        config.roudi.m_verifySharedMemoryPlacement = false;
        m_roudiMemoryManager = new IceOryxRouDiMemoryManager(config);
        m_roudiMemoryManager->createAndAnnounceMemory();
        m_shmManager = new CShmMangerTester(m_roudiMemoryManager);

        auto user = iox::posix::PosixGroup::getGroupOfCurrentProcess().getName();
        m_payloadMemoryManager =
            m_roudiMemoryManager->segmentManager().value()->getSegmentInformationForUser(user).m_memoryManager;

        // clearing the introspection, is not in d'tor -> SEGFAULT in delete sporadically
        m_shmManager->stopPortIntrospection();
        m_shmManager->deletePortsOfProcess(iox::MQ_ROUDI_NAME);
    }

    void TearDown() override
    {
        delete m_shmManager;
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
};


TEST_F(PortManager_test, doDiscovery_singleShotSenderFirst)
{
    SenderPort sender(m_shmManager->acquireSenderPortData({1, 1, 1}, "/guiseppe", m_payloadMemoryManager).get_value());
    ASSERT_TRUE(sender);
    sender.activate();
    // no doDiscovery() at this position is intentional

    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);

    m_shmManager->doDiscovery();

    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1u));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // is the correct receiver in the receiver list
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));

    // is the receiver connected
    EXPECT_TRUE(receiver1.isSubscribed());
}

TEST_F(PortManager_test, doDiscovery_singleShotReceiverFirst)
{
    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);
    // no doDiscovery() at this position is intentional

    SenderPort sender(m_shmManager->acquireSenderPortData({1, 1, 1}, "/guiseppe", m_payloadMemoryManager).get_value());
    ASSERT_TRUE(sender);
    sender.activate();

    m_shmManager->doDiscovery();

    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1u));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // is the correct receiver in the receiver list
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));

    // is the receiver connected
    EXPECT_TRUE(receiver1.isSubscribed());
}

TEST_F(PortManager_test, doDiscovery_singleShotReceiverFirstWithDiscovery)
{
    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);
    m_shmManager->doDiscovery();

    SenderPort sender(m_shmManager->acquireSenderPortData({1, 1, 1}, "/guiseppe", m_payloadMemoryManager).get_value());
    ASSERT_TRUE(sender);
    sender.activate();

    m_shmManager->doDiscovery();

    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1u));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // is the correct receiver in the receiver list
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));

    // is the receiver connected
    EXPECT_TRUE(receiver1.isSubscribed());
}

TEST_F(PortManager_test, doDiscovery_rightOrdering)
{
    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);
    m_shmManager->doDiscovery();

    SenderPort sender(m_shmManager->acquireSenderPortData({1, 1, 1}, "/guiseppe", m_payloadMemoryManager).get_value());
    ASSERT_TRUE(sender);
    sender.activate();

    ReceiverPort receiver2(m_shmManager->acquireReceiverPortData({1, 1, 1}, "/ignatz"));
    ASSERT_TRUE(receiver2);
    receiver2.subscribe(true);
    m_shmManager->doDiscovery();

    // check if all receivers are subscribed
    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(2u));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // check if the receivers are in the right order
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));
    it++;
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver2.getMembers()->m_processName));

    // check if the receivers know that they are subscribed
    EXPECT_TRUE(receiver1.isSubscribed());
    EXPECT_TRUE(receiver2.isSubscribed());
}

TEST_F(PortManager_test, SenderReceiverOverflow)
{
    std::string p1 = "/test1";
    std::string r1 = "run1";
    decltype(iox::MAX_PORT_NUMBER) forP1 = iox::MAX_PORT_NUMBER;
    std::vector<iox::popo::SenderPortData*> avaSender1(forP1);
    std::vector<iox::popo::ReceiverPortData*> avaReceiver1(forP1);


    for (unsigned int i = 0; i < forP1; i++)
    {
        auto rec = m_shmManager->acquireReceiverPortData(getUniqueSD(), p1, r1);
        ASSERT_THAT(rec, Ne(nullptr));
        avaReceiver1[i] = rec;
        auto sen = m_shmManager->acquireSenderPortData(getUniqueSD(), p1, m_payloadMemoryManager, r1);
        ASSERT_FALSE(sen.has_error());
        avaSender1[i] = sen.get_value();
    }

    { // test if overflow errors get hit

        bool errorHandlerCalled = false;
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
            [&errorHandlerCalled](const iox::Error error [[gnu::unused]],
                                  const std::function<void()>,
                                  const iox::ErrorLevel) { errorHandlerCalled = true; });
        auto rec = m_shmManager->acquireReceiverPortData(getUniqueSD(), p1, r1);
        EXPECT_TRUE(errorHandlerCalled);
        EXPECT_THAT(rec, Eq(nullptr));

        errorHandlerCalled = false;
        auto sen = m_shmManager->acquireSenderPortData(getUniqueSD(), p1, m_payloadMemoryManager, r1);
        EXPECT_TRUE(errorHandlerCalled);
        ASSERT_TRUE(sen.has_error());
        EXPECT_THAT(sen.get_error(), Eq(PortPoolError::SENDER_PORT_LIST_FULL));
    }
}

TEST_F(PortManager_test, InterfaceAndApplicationsOverflow)
{
    // overflow of interface and applications
    std::string itf = "/itf";
    std::string app = "/app";

    for (unsigned int i = 0; i < iox::MAX_INTERFACE_NUMBER; i++)
    {
        auto interp = m_shmManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, itf + std::to_string(i));
        EXPECT_THAT(interp, Ne(nullptr));
    }
    for (unsigned int i = 0; i < iox::MAX_PROCESS_NUMBER; i++)
    {
        auto appp = m_shmManager->acquireApplicationPortData(app + std::to_string(i));
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
        auto interp = m_shmManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, "/itfPenguin");
        EXPECT_THAT(interp, Eq(nullptr));
        EXPECT_TRUE(errorHandlerCalled);

        errorHandlerCalled = false;
        auto appp = m_shmManager->acquireApplicationPortData("/appPenguin");
        EXPECT_THAT(appp, Eq(nullptr));
        EXPECT_TRUE(errorHandlerCalled);
    }

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        m_shmManager->deletePortsOfProcess(itf + std::to_string(testi));
        m_shmManager->deletePortsOfProcess(app + std::to_string(testi));

        auto interp =
            m_shmManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, itf + std::to_string(testi));
        EXPECT_THAT(interp, Ne(nullptr));

        auto appp = m_shmManager->acquireApplicationPortData(app + std::to_string(testi));
        EXPECT_THAT(appp, Ne(nullptr));
    }
}

TEST_F(PortManager_test, PortDestroy)
{
    std::string p1 = "/myProcess1";
    std::string p2 = "/myProcess2";
    iox::capro::ServiceDescription cap1(1, 1, 1);
    iox::capro::ServiceDescription cap2(2, 2, 2);

    // two processes p1 and p2 each with a sender and receiver that match to the other process
    auto senderData1 = m_shmManager->acquireSenderPortData(cap1, p1, m_payloadMemoryManager).get_value();
    auto receiverData1 = m_shmManager->acquireReceiverPortData(cap2, p1);

    auto senderData2 = m_shmManager->acquireSenderPortData(cap2, p2, m_payloadMemoryManager).get_value();
    auto receiverData2 = m_shmManager->acquireReceiverPortData(cap1, p2);

    // let them connect
    {
        SenderPort sender1(senderData1);
        ASSERT_TRUE(sender1);
        sender1.activate();
        ReceiverPort receiver1(receiverData1);
        ASSERT_TRUE(receiver1);
        receiver1.subscribe(true);

        SenderPort sender2(senderData2);
        ASSERT_TRUE(sender2);
        sender2.activate();
        ReceiverPort receiver2(receiverData2);
        ASSERT_TRUE(receiver2);
        receiver2.subscribe(true);

        m_shmManager->doDiscovery();

        ASSERT_THAT(sender1.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1u));
        EXPECT_TRUE(receiver1.isSubscribed());

        ASSERT_THAT(sender2.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1u));
        EXPECT_TRUE(receiver1.isSubscribed());
    }

    // destroy the ports of process p2 and check if states of ports in p1 changed as expected
    {
        SenderPort sender1(senderData1);
        ASSERT_TRUE(sender1);
        ReceiverPort receiver1(receiverData1);
        ASSERT_TRUE(receiver1);

        SenderPort sender2(senderData2);
        ASSERT_TRUE(sender2);
        sender2.destroy();
        ReceiverPort receiver2(receiverData2);
        ASSERT_TRUE(receiver2);
        receiver2.destroy();

        m_shmManager->doDiscovery();

        ASSERT_THAT(sender1.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(0u));
        EXPECT_FALSE(receiver1.isSubscribed());
    }

    // re-create the ports of process p2
    senderData2 = m_shmManager->acquireSenderPortData(cap2, p2, m_payloadMemoryManager).get_value();
    receiverData2 = m_shmManager->acquireReceiverPortData(cap1, p2);

    // let them connect
    {
        SenderPort sender1(senderData1);
        ASSERT_TRUE(sender1);
        ReceiverPort receiver1(receiverData1);
        ASSERT_TRUE(receiver1);

        SenderPort sender2(senderData2);
        ASSERT_TRUE(sender2);
        sender2.activate();
        ReceiverPort receiver2(receiverData2);
        ASSERT_TRUE(receiver2);
        receiver2.subscribe(true);

        m_shmManager->doDiscovery();

        ASSERT_THAT(sender1.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1u));
        EXPECT_TRUE(receiver1.isSubscribed());

        ASSERT_THAT(sender2.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1u));
        EXPECT_TRUE(receiver1.isSubscribed());
    }

    // cleanup process p2 and check if states of ports in p1 changed  as expected
    {
        m_shmManager->deletePortsOfProcess(p2);
        SenderPort sender1(senderData1);
        ASSERT_TRUE(sender1);
        ReceiverPort receiver1(receiverData1);
        ASSERT_TRUE(receiver1);

        ASSERT_THAT(sender1.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(0u));
        EXPECT_FALSE(receiver1.isSubscribed());
    }
}
