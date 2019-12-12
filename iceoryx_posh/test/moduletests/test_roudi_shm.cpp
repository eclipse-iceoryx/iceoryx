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
#include "iceoryx_posh/internal/roudi/shared_memory_manager.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include <cstdint>
#include <limits> // std::numeric_limits

using namespace ::testing;
using ::testing::Return;

using iox::popo::ReceiverPort;
using iox::popo::SenderPort;
using iox::roudi::SharedMemoryManager;

class CShmMangerTester : public SharedMemoryManager
{
  public:
    CShmMangerTester(const iox::RouDiConfig_t& f_config)
        : SharedMemoryManager(f_config)
    {
    }

  private:
    FRIEND_TEST(SharedMemoryManager_test, CheckDeleteOfPortsFromProcess1);
    FRIEND_TEST(SharedMemoryManager_test, CheckDeleteOfPortsFromProcess2);
};

class SharedMemoryManager_test : public Test
{
  public:
    CShmMangerTester* m_shmManager;
    uint16_t m_instIdCounter, m_eventIdCounter, m_sIdCounter;
    void SetUp() override
    {
        testing::internal::CaptureStderr();
        m_instIdCounter = m_sIdCounter = 1;
        m_eventIdCounter = 0;
        // starting at {1,1,1}

        auto config = iox::RouDiConfig_t().setDefaults();
        config.roudi.m_verifySharedMemoryPlacement = false;
        m_shmManager = new CShmMangerTester(config);
        // clearing the introspection, is not in d'tor -> SEGFAULT in delete sporadically
        m_shmManager->stopPortIntrospection();
        m_shmManager->deletePortsOfProcess(iox::roudi::PORT_INTROSPECTION_MQ_APP_NAME);
    }

    void TearDown() override
    {
        iox::RelativePointer::unregisterAll();
        delete m_shmManager;

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


TEST_F(SharedMemoryManager_test, doDiscovery_singleShotSenderFirst)
{
    SenderPort sender(
        m_shmManager->acquireSenderPortData({1, 1, 1},
                                            iox::Interfaces::INTERNAL,
                                            "/guiseppe",
                                            &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager));
    ASSERT_TRUE(sender);
    sender.activate();
    // no doDiscovery() at this position is intentional

    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, iox::Interfaces::INTERNAL, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);

    m_shmManager->doDiscovery();

    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // is the correct receiver in the receiver list
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));

    // is the receiver connected
    EXPECT_TRUE(receiver1.isSubscribed());
}

TEST_F(SharedMemoryManager_test, doDiscovery_singleShotReceiverFirst)
{
    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, iox::Interfaces::INTERNAL, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);
    // no doDiscovery() at this position is intentional

    SenderPort sender(
        m_shmManager->acquireSenderPortData({1, 1, 1},
                                            iox::Interfaces::INTERNAL,
                                            "/guiseppe",
                                            &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager));
    ASSERT_TRUE(sender);
    sender.activate();

    m_shmManager->doDiscovery();

    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // is the correct receiver in the receiver list
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));

    // is the receiver connected
    EXPECT_TRUE(receiver1.isSubscribed());
}

TEST_F(SharedMemoryManager_test, doDiscovery_singleShotReceiverFirstWithDiscovery)
{
    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, iox::Interfaces::INTERNAL, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);
    m_shmManager->doDiscovery();

    SenderPort sender(
        m_shmManager->acquireSenderPortData({1, 1, 1},
                                            iox::Interfaces::INTERNAL,
                                            "/guiseppe",
                                            &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager));
    ASSERT_TRUE(sender);
    sender.activate();

    m_shmManager->doDiscovery();

    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(1));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // is the correct receiver in the receiver list
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));

    // is the receiver connected
    EXPECT_TRUE(receiver1.isSubscribed());
}

TEST_F(SharedMemoryManager_test, doDiscovery_rightOrdering)
{
    ReceiverPort receiver1(m_shmManager->acquireReceiverPortData({1, 1, 1}, iox::Interfaces::INTERNAL, "/schlomo"));
    ASSERT_TRUE(receiver1);
    receiver1.subscribe(true);
    m_shmManager->doDiscovery();

    SenderPort sender(
        m_shmManager->acquireSenderPortData({1, 1, 1},
                                            iox::Interfaces::INTERNAL,
                                            "/guiseppe",
                                            &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager));
    ASSERT_TRUE(sender);
    sender.activate();

    ReceiverPort receiver2(m_shmManager->acquireReceiverPortData({1, 1, 1}, iox::Interfaces::INTERNAL, "/ignatz"));
    ASSERT_TRUE(receiver2);
    receiver2.subscribe(true);
    m_shmManager->doDiscovery();

    // check if all receivers are subscribed
    ASSERT_THAT(sender.getMembers()->m_receiverHandler.m_receiverVector.size(), Eq(2));
    auto it = sender.getMembers()->m_receiverHandler.m_receiverVector.begin();

    // check if the receivers are in the right order
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver1.getMembers()->m_processName));
    it++;
    EXPECT_THAT(iox::popo::ReceiverPort(*it).getMembers()->m_processName, Eq(receiver2.getMembers()->m_processName));

    // check if the receivers know that they are subscribed
    EXPECT_TRUE(receiver1.isSubscribed());
    EXPECT_TRUE(receiver2.isSubscribed());
}

TEST_F(SharedMemoryManager_test, DISABLED_CheckDeleteOfPortsFromProcess1)
{
    /// @todo refactor this part of the code, this is a hard whitebox test which
    ///         in the end tests nothing! You are not allowed to gain access to
    ///         the middleware port lists in this test, think of something else!
    ///
    //    std::string p1 = "/test1";
    //    std::string p2 = "/test2";
    //    decltype(iox::MAX_PORT_NUMBER) introspectionPorts = 0; // stopped atm
    //    decltype(iox::MAX_PORT_NUMBER) forP1 = iox::MAX_PORT_NUMBER / 2;
    //    decltype(iox::MAX_PORT_NUMBER) forP2 = iox::MAX_PORT_NUMBER - forP1 - introspectionPorts;
    //    std::vector<iox::popo::SenderPortData*> avaSender1(forP1);
    //    std::vector<iox::popo::ReceiverPortData*> avaReceiver1(forP1);
    //    std::vector<iox::popo::SenderPortData*> avaSender2(forP2);
    //    std::vector<iox::popo::ReceiverPortData*> avaReceiver2(forP2);
    //
    //    m_shmManager->acquireInterfacePortData(iox::Interfaces::INTERNAL, p1);
    //    m_shmManager->acquireInterfacePortData(iox::Interfaces::INTERNAL, p2);
    //
    //    for (unsigned int i = 0; i < forP1; i++)
    //    {
    //        auto rec = m_shmManager->acquireReceiverPortData(getUniqueSD(), iox::Interfaces::INTERNAL, p1);
    //        ASSERT_THAT(rec, Ne(nullptr));
    //        avaReceiver1[i] = rec;
    //        auto sen = m_shmManager->acquireSenderPortData(
    //            getUniqueSD(),
    //            iox::Interfaces::INTERNAL,
    //            p1,
    //            &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager);
    //        ASSERT_THAT(sen, Ne(nullptr));
    //        avaSender1[i] = sen;
    //    }
    //    for (unsigned int i = 0; i < forP2; i++)
    //    {
    //        auto rec = m_shmManager->acquireReceiverPortData(getUniqueSD(), iox::Interfaces::INTERNAL, p2);
    //        ASSERT_THAT(rec, Ne(nullptr));
    //        avaReceiver2[i] = rec;
    //        auto sen = m_shmManager->acquireSenderPortData(
    //            getUniqueSD(),
    //            iox::Interfaces::INTERNAL,
    //            p2,
    //            &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager);
    //        avaSender2[i] = sen;
    //        ASSERT_THAT(sen, Ne(nullptr));
    //    }
    //
    //    { // test if overflow errors get hit
    //        error_handling_MOCK::activateMock();
    //        EXPECT_CALL(*error_handling_MOCK::mock, errorHandler(_, _, _, _)).WillOnce(Return());
    //        auto rec = m_shmManager->acquireReceiverPortData(getUniqueSD(), iox::Interfaces::INTERNAL, p1);
    //        EXPECT_THAT(rec, Eq(nullptr));
    //
    //        EXPECT_CALL(*error_handling_MOCK::mock, errorHandler(_, _, _, _)).WillOnce(Return());
    //        auto sen = m_shmManager->acquireSenderPortData(
    //            getUniqueSD(),
    //            iox::Interfaces::INTERNAL,
    //            p1,
    //            &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager);
    //        EXPECT_THAT(sen, Eq(nullptr));
    //    }
    //
    //    // now we delete process and check if the ports of the living app are there and the others are deleted
    //    m_shmManager->deletePortsOfProcess(p2);
    //    iox::roudi::MiddlewareShm::middlewareReceiverList_t* avaRec =
    //        &(m_shmManager->m_ShmInterface.getShmInterface()->m_middlewareReceiverList);
    //    for (auto& rec : avaReceiver1) // check if the available process receivers are still there
    //    {
    //        bool found = false;
    //        for (auto& livingList : *avaRec)
    //        {
    //            if (rec == &livingList) // we compare the pointer to shm
    //            {
    //                found = true;
    //                break;
    //            }
    //        }
    //        ASSERT_TRUE(found);
    //    }
    //    for (auto& rec : avaReceiver2) // check if the removed process receivers are gone
    //    {
    //        bool found = false;
    //        for (auto& livingList : *avaRec)
    //        {
    //            if (rec == &livingList) // we compare the pointer to shm
    //            {
    //                found = true;
    //                break;
    //            }
    //        }
    //        ASSERT_FALSE(found);
    //    }
    //    iox::roudi::MiddlewareShm::middlewareSenderList_t* avaSend =
    //        &(m_shmManager->m_ShmInterface.getShmInterface()->m_middlewareSenderList);
    //    for (auto& rec : avaSender1) // check if the available process receivers are still there
    //    {
    //        bool found = false;
    //        for (auto& livingList : *avaSend)
    //        {
    //            if (rec == &livingList) // we compare the pointer to shm
    //            {
    //                found = true;
    //                break;
    //            }
    //        }
    //        ASSERT_TRUE(found);
    //    }
    //    for (auto& rec : avaSender2) // check if the available process receivers are still there
    //    {
    //        bool found = false;
    //        for (auto& livingList : *avaSend)
    //        {
    //            if (rec == &livingList) // we compare the pointer to shm
    //            {
    //                found = true;
    //                break;
    //            }
    //        }
    //        ASSERT_FALSE(found);
    //    }
}

TEST_F(SharedMemoryManager_test, DISABLED_CheckDeleteOfPortsFromProcess2)
{
    /// @todo refactor this part of the code, this is a hard whitebox test which
    ///         in the end tests nothing! You are not allowed to gain access to
    ///         the middleware port lists in this test, think of something else!
    ///
    // // same as test before but now we delete p1 over an application port
    // std::string p1 = "/test1";
    // std::string p2 = "/test2";
    // decltype(iox::MAX_PORT_NUMBER) introspectionPorts = 0; // stopped atm
    // decltype(iox::MAX_PORT_NUMBER) forP1 = iox::MAX_PORT_NUMBER / 2;
    // decltype(iox::MAX_PORT_NUMBER) forP2 = iox::MAX_PORT_NUMBER - forP1 - introspectionPorts;
    // std::vector<iox::popo::SenderPort*> avaSender1(forP1);
    // std::vector<iox::popo::ReceiverPort*> avaReceiver1(forP1);
    // std::vector<iox::popo::SenderPort*> avaSender2(forP2);
    // std::vector<iox::popo::ReceiverPort*> avaReceiver2(forP2);

    // m_shmManager->acquireApplicationPortData(iox::Interfaces::INTERNAL, p1);
    // m_shmManager->acquireApplicationPortData(iox::Interfaces::INTERNAL, p2);

    // for (unsigned int i = 0; i < forP1; i++)
    // {
    //     auto rec = m_shmManager->addReceiverPort(getUniqueSD(), iox::Interfaces::INTERNAL, p1);
    //     ASSERT_THAT(rec, Ne(nullptr));
    //     avaReceiver1[i] = rec;
    //     auto sen =
    //         m_shmManager->addSenderPort(getUniqueSD(),
    //                                     iox::Interfaces::INTERNAL,
    //                                     p1,
    //                                     &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager);
    //     ASSERT_THAT(sen, Ne(nullptr));
    //     avaSender1[i] = sen;
    // }
    // for (unsigned int i = 0; i < forP2; i++)
    // {
    //     auto rec = m_shmManager->addReceiverPort(getUniqueSD(), iox::Interfaces::INTERNAL, p2);
    //     ASSERT_THAT(rec, Ne(nullptr));
    //     avaReceiver2[i] = rec;
    //     auto sen =
    //         m_shmManager->addSenderPort(getUniqueSD(),
    //                                     iox::Interfaces::INTERNAL,
    //                                     p2,
    //                                     &m_shmManager->getShmInterface().getShmInterface()->m_roudiMemoryManager);
    //     avaSender2[i] = sen;
    //     ASSERT_THAT(sen, Ne(nullptr));
    // }

    // // now we delete process and check if the ports of the living app are there and the others are deleted
    // m_shmManager->deletePortsOfProcess(p1);
    // iox::roudi::MiddlewareShm::middlewareReceiverList_t* avaRec =
    //     &(m_shmManager->m_ShmInterface.getShmInterface()->m_middlewareReceiverList);
    // for (auto& rec : avaReceiver2) // check if the available process receivers are still there
    // {
    //     bool found = false;
    //     for (auto& livingList : *avaRec)
    //     {
    //         if (rec == &livingList) // we compare the pointer to shm
    //         {
    //             found = true;
    //             break;
    //         }
    //     }
    //     ASSERT_TRUE(found);
    // }
    // for (auto& rec : avaReceiver1) // check if the removed process receivers are gone
    // {
    //     bool found = false;
    //     for (auto& livingList : *avaRec)
    //     {
    //         if (rec == &livingList) // we compare the pointer to shm
    //         {
    //             found = true;
    //             break;
    //         }
    //     }
    //     ASSERT_FALSE(found);
    // }
    // iox::roudi::MiddlewareShm::middlewareSenderList_t* avaSend =
    //     &(m_shmManager->m_ShmInterface.getShmInterface()->m_middlewareSenderList);
    // for (auto& rec : avaSender2) // check if the available process receivers are still there
    // {
    //     bool found = false;
    //     for (auto& livingList : *avaSend)
    //     {
    //         if (rec == &livingList) // we compare the pointer to shm
    //         {
    //             found = true;
    //             break;
    //         }
    //     }
    //     ASSERT_TRUE(found);
    // }
    // for (auto& rec : avaSender1) // check if the available process receivers are still there
    // {
    //     bool found = false;
    //     for (auto& livingList : *avaSend)
    //     {
    //         if (rec == &livingList) // we compare the pointer to shm
    //         {
    //             found = true;
    //             break;
    //         }
    //     }
    //     ASSERT_FALSE(found);
    // }
}

TEST_F(SharedMemoryManager_test, InterfaceAndApplicationsOverflow)
{
    // overflow of interface and applications
    std::string itf = "/itf";
    std::string app = "/app";

    for (unsigned int i = 0; i < iox::MAX_INTERFACE_NUMBER; i++)
    {
        auto interp = m_shmManager->acquireInterfacePortData(iox::Interfaces::INTERNAL, itf + std::to_string(i));
        EXPECT_THAT(interp, Ne(nullptr));
    }
    for (unsigned int i = 0; i < iox::MAX_PROCESS_NUMBER; i++)
    {
        auto appp = m_shmManager->acquireApplicationPortData(iox::Interfaces::INTERNAL, app + std::to_string(i));
        EXPECT_THAT(appp, Ne(nullptr));
    }

    // test if overflow errors get hit
    {
        auto errorHandlerCalled{false};
        auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler([&errorHandlerCalled](
            const iox::Error, const std::function<void()>, const iox::ErrorLevel) { errorHandlerCalled = true; });

        errorHandlerCalled = false;
        auto interp = m_shmManager->acquireInterfacePortData(iox::Interfaces::INTERNAL, "/itfPenguin");
        EXPECT_THAT(interp, Eq(nullptr));
        EXPECT_TRUE(errorHandlerCalled);

        errorHandlerCalled = false;
        auto appp = m_shmManager->acquireApplicationPortData(iox::Interfaces::INTERNAL, "/appPenguin");
        EXPECT_THAT(appp, Eq(nullptr));
        EXPECT_TRUE(errorHandlerCalled);
    }

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        m_shmManager->deletePortsOfProcess(itf + std::to_string(testi));
        m_shmManager->deletePortsOfProcess(app + std::to_string(testi));

        auto interp = m_shmManager->acquireInterfacePortData(iox::Interfaces::INTERNAL, itf + std::to_string(testi));
        EXPECT_THAT(interp, Ne(nullptr));

        auto appp = m_shmManager->acquireApplicationPortData(iox::Interfaces::INTERNAL, app + std::to_string(testi));
        EXPECT_THAT(appp, Ne(nullptr));
    }
}
