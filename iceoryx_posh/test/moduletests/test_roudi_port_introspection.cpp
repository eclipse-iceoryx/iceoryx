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

#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "mocks/chunk_mock.hpp"
#include "mocks/publisher_mock.hpp"
#include "mocks/subscriber_mock.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::Return;

#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

using iox::mepoo::DurationNs;
using iox::mepoo::TimePointNs;

#include <cstdint>

class MockPublisherPortUserIntrospection : public MockPublisherPortUser
{
  public:
    using Topic = iox::roudi::PortIntrospectionFieldTopic;

    MockPublisherPortUserIntrospection() = default;
    MockPublisherPortUserIntrospection(iox::popo::PublisherPortData*){};

    ChunkMock<Topic> m_chunk;

    iox::cxx::expected<iox::mepoo::ChunkHeader*, iox::popo::AllocationError> tryAllocateChunk(const uint32_t)
    {
        return iox::cxx::success<iox::mepoo::ChunkHeader*>(m_chunk.chunkHeader());
    }
};

template <typename PublisherPort, typename SubscriberPort>
class PortIntrospectionAccess : public iox::roudi::PortIntrospection<PublisherPort, SubscriberPort>
{
  public:
    using iox::roudi::PortIntrospection<PublisherPort, SubscriberPort>::sendPortData;

    void sendThroughputData()
    {
        iox::roudi::PortIntrospection<PublisherPort, SubscriberPort>::sendThroughputData();
    }
    iox::cxx::optional<PublisherPort>& getPublisherPort()
    {
        return this->m_publisherPort;
    }
    iox::cxx::optional<PublisherPort>& getPublisherPortThroughput()
    {
        return this->m_publisherPortThroughput;
    }
};

class PortIntrospection_test : public Test
{
  public:
    PortIntrospection_test()
    {
    }

    ~PortIntrospection_test()
    {
    }

    virtual void SetUp()
    {
        internal::CaptureStdout();
        ASSERT_THAT(m_introspectionAccess.registerPublisherPort(&m_publisherPortDataPortGeneric,
                                                                &m_publisherPortDataThroughput,
                                                                &m_publisherPortDataSubscriberData),
                    Eq(true));
    }

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    bool comparePortData(const iox::roudi::SubscriberPortData& a, const iox::roudi::SubscriberPortData& b)
    {
        auto nameA = std::string(a.m_name);
        auto nameB = std::string(b.m_name);

        if (nameA.compare(nameB) != 0)
            return false;
        if (a.m_caproInstanceID != b.m_caproInstanceID)
            return false;
        if (a.m_caproServiceID != b.m_caproServiceID)
            return false;
        if (a.m_caproEventMethodID != b.m_caproEventMethodID)
            return false;
        if (a.m_publisherIndex != b.m_publisherIndex)
            return false;
        if (a.m_runnable != b.m_runnable)
            return false;

        return true;
    }

    bool comparePortData(const iox::roudi::PublisherPortData& a, const iox::roudi::PublisherPortData& b)
    {
        auto nameA = std::string(a.m_name);
        auto nameB = std::string(b.m_name);

        if (nameA.compare(nameB) != 0)
            return false;
        if (a.m_caproInstanceID != b.m_caproInstanceID)
            return false;
        if (a.m_caproServiceID != b.m_caproServiceID)
            return false;
        if (a.m_caproEventMethodID != b.m_caproEventMethodID)
            return false;
        if (a.m_runnable != b.m_runnable)
            return false;

        return true;
    }

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};

    iox::mepoo::MemoryManager m_memoryManager;
    iox::capro::ServiceDescription m_serviceDescription;
    iox::popo::PublisherPortData m_publisherPortDataPortGeneric{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataThroughput{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataSubscriberData{m_serviceDescription, "Foo", &m_memoryManager};

    PortIntrospectionAccess<MockPublisherPortUserIntrospection, MockSubscriberPortUser> m_introspectionAccess;
};


TEST_F(PortIntrospection_test, registerPublisherPort)
{
    iox::popo::PublisherPortData m_publisherPortDataPortGeneric{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataThroughput{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataSubscriberData{m_serviceDescription, "Foo", &m_memoryManager};

    auto introspection =
        std::unique_ptr<iox::roudi::PortIntrospection<MockPublisherPortUserIntrospection, MockSubscriberPortUser>>(
            new iox::roudi::PortIntrospection<MockPublisherPortUserIntrospection, MockSubscriberPortUser>);

    EXPECT_THAT(introspection->registerPublisherPort(&m_publisherPortDataPortGeneric,
                                                     &m_publisherPortDataThroughput,
                                                     &m_publisherPortDataSubscriberData),
                Eq(true));

    iox::popo::PublisherPortData m_publisherPortDataPortGeneric2{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataThroughput2{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataSubscriberData2{m_serviceDescription, "Foo", &m_memoryManager};

    EXPECT_THAT(introspection->registerPublisherPort(&m_publisherPortDataPortGeneric2,
                                                     &m_publisherPortDataThroughput2,
                                                     &m_publisherPortDataSubscriberData2),
                Eq(false));
}


TEST_F(PortIntrospection_test, sendPortData_EmptyList)
{
    EXPECT_CALL(m_introspectionAccess.getPublisherPort().value(), sendChunk(_)).Times(1);

    m_introspectionAccess.sendPortData();

    EXPECT_THAT(m_introspectionAccess.getPublisherPort().value().m_chunk.sample()->m_publisherList.size(), Eq(0));
    EXPECT_THAT(m_introspectionAccess.getPublisherPort().value().m_chunk.sample()->m_subscriberList.size(), Eq(0));
}

TEST_F(PortIntrospection_test, addAndRemovePublisher)
{
    using PortData = iox::roudi::PublisherPortData;

    const iox::ProcessName_t processName1("name1");
    const iox::ProcessName_t processName2("name2");
    const iox::RunnableName_t runnableName1("4");
    const iox::RunnableName_t runnableName2("jkl");

    // prepare expected outputs
    PortData expected1;
    expected1.m_name = processName1;
    expected1.m_caproInstanceID = "1";
    expected1.m_caproServiceID = "2";
    expected1.m_caproEventMethodID = "3";
    expected1.m_runnable = runnableName1;

    PortData expected2;
    expected2.m_name = processName2;
    expected2.m_caproInstanceID = "abc";
    expected2.m_caproServiceID = "def";
    expected2.m_caproEventMethodID = "ghi";
    expected2.m_runnable = runnableName2;

    // prepare inputs
    iox::capro::ServiceDescription service1(
        expected1.m_caproServiceID, expected1.m_caproInstanceID, expected1.m_caproEventMethodID);
    iox::capro::ServiceDescription service2(
        expected2.m_caproServiceID, expected2.m_caproInstanceID, expected2.m_caproEventMethodID);

    EXPECT_CALL(m_introspectionAccess.getPublisherPort().value(), sendChunk(_)).Times(4);

    // test adding of ports
    // remark: duplicate publisher port insertions are not possible
    iox::popo::PublisherPortData portData1{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData portData2{m_serviceDescription, "Foo", &m_memoryManager};
    EXPECT_THAT(m_introspectionAccess.addPublisher(&portData1, processName1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspectionAccess.addPublisher(&portData1, processName1, service1, "4"), Eq(false));
    EXPECT_THAT(m_introspectionAccess.addPublisher(&portData2, processName2, service2, "jkl"), Eq(true));
    EXPECT_THAT(m_introspectionAccess.addPublisher(&portData2, processName2, service2, "jkl"), Eq(false));

    m_introspectionAccess.sendPortData();

    auto sample = m_introspectionAccess.getPublisherPort().value().m_chunk.sample();

    {
        ASSERT_THAT(m_introspectionAccess.getPublisherPort().value().m_chunk.sample()->m_publisherList.size(), Eq(2));
        ASSERT_THAT(m_introspectionAccess.getPublisherPort().value().m_chunk.sample()->m_subscriberList.size(), Eq(0));

        auto& publisherInfo1 = sample->m_publisherList[0];
        auto& publisherInfo2 = sample->m_publisherList[1];

        // remark: we cannot ensure that the order is the same as the order in
        // which the ports where added  we therefore expect to find both ports
        // with the corresponding ids (we need to check whether multiple port
        // insertions also work correctly, therefore we need at least two ports)

        if (comparePortData(publisherInfo1, expected1))
        {
            EXPECT_THAT(comparePortData(publisherInfo2, expected2), Eq(true));
        }
        else
        {
            EXPECT_THAT(comparePortData(publisherInfo2, expected1), Eq(true));
        }
    }

    // test removal of ports

    EXPECT_THAT(m_introspectionAccess.removePublisher(processName1, service1), Eq(true));
    EXPECT_THAT(m_introspectionAccess.removePublisher(processName1, service1), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(m_introspectionAccess.getPublisherPort().value().m_chunk.sample()->m_publisherList.size(), Eq(1));
        ASSERT_THAT(m_introspectionAccess.getPublisherPort().value().m_chunk.sample()->m_subscriberList.size(), Eq(0));

        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expected2), Eq(true));
    }

    EXPECT_THAT(m_introspectionAccess.removePublisher(processName2, service2), Eq(true));
    EXPECT_THAT(m_introspectionAccess.removePublisher(processName2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    EXPECT_THAT(m_introspectionAccess.removePublisher(processName2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    sample->~PortIntrospectionFieldTopic();
}

TEST_F(PortIntrospection_test, addAndRemoveSubscriber)
{
    using PortData = iox::roudi::SubscriberPortData;

    const iox::ProcessName_t processName1("name1");
    const iox::ProcessName_t processName2("name2");
    const iox::RunnableName_t runnableName1("4");
    const iox::RunnableName_t runnableName2("7");

    // prepare expected outputs
    PortData expected1;
    expected1.m_name = processName1;
    expected1.m_caproInstanceID = "1";
    expected1.m_caproServiceID = "2";
    expected1.m_caproEventMethodID = "3";
    expected1.m_publisherIndex = -1;
    expected1.m_runnable = runnableName1;

    PortData expected2;
    expected2.m_name = processName2;
    expected2.m_caproInstanceID = "4";
    expected2.m_caproServiceID = "5";
    expected2.m_caproEventMethodID = "6";
    expected2.m_publisherIndex = -1;
    expected2.m_runnable = runnableName2;

    // prepare inputs
    iox::capro::ServiceDescription service1(
        expected1.m_caproServiceID, expected1.m_caproInstanceID, expected1.m_caproEventMethodID);
    iox::capro::ServiceDescription service2(
        expected2.m_caproServiceID, expected2.m_caproInstanceID, expected2.m_caproEventMethodID);

    EXPECT_CALL(m_introspectionAccess.getPublisherPort().value(), sendChunk(_)).Times(4);

    // test adding of ports
    // remark: duplicate subscriber insertions are possible but will not be transmitted via send
    iox::popo::SubscriberPortData recData1{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    iox::popo::SubscriberPortData recData2{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    EXPECT_THAT(m_introspectionAccess.addSubscriber(&recData1, processName1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspectionAccess.addSubscriber(&recData1, processName1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspectionAccess.addSubscriber(&recData2, processName2, service2, "7"), Eq(true));
    EXPECT_THAT(m_introspectionAccess.addSubscriber(&recData2, processName2, service2, "7"), Eq(true));

    m_introspectionAccess.sendPortData();

    auto sample = m_introspectionAccess.getPublisherPort().value().m_chunk.sample();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(2));

        auto& subscriberInfo1 = sample->m_subscriberList[0];
        auto& subscriberInfo2 = sample->m_subscriberList[1];

        // remark: we cannot ensure that the order is the same as the order in
        // which the ports where added  we therefore expect to find both ports
        // with the corresponding ids (we need to check whether multiple port
        // insertions also work correctly, therefore we need at least two ports)

        if (comparePortData(subscriberInfo1, expected1))
        {
            EXPECT_THAT(comparePortData(subscriberInfo2, expected2), Eq(true));
        }
        else
        {
            EXPECT_THAT(comparePortData(subscriberInfo2, expected1), Eq(true));
        }
    }

    // test removal of ports

    EXPECT_THAT(m_introspectionAccess.removeSubscriber(processName1, service1), Eq(true));
    EXPECT_THAT(m_introspectionAccess.removeSubscriber(processName1, service1), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        auto& publisherInfo = sample->m_subscriberList[0];

        EXPECT_THAT(comparePortData(publisherInfo, expected2), Eq(true));
    }

    EXPECT_THAT(m_introspectionAccess.removeSubscriber(processName2, service2), Eq(true));
    EXPECT_THAT(m_introspectionAccess.removeSubscriber(processName2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    EXPECT_THAT(m_introspectionAccess.removeSubscriber(processName2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    sample->~PortIntrospectionFieldTopic();
}


TEST_F(PortIntrospection_test, reportMessageToEstablishConnection)
{
    using SubscriberPortData = iox::roudi::SubscriberPortData;
    using PublisherPortData = iox::roudi::PublisherPortData;

    const iox::ProcessName_t nameSubscriber("subscriber");
    const iox::ProcessName_t namePublisher("publisher");

    // prepare expected outputs
    SubscriberPortData expectedSubscriber;
    expectedSubscriber.m_name = nameSubscriber;
    expectedSubscriber.m_caproInstanceID = "1";
    expectedSubscriber.m_caproServiceID = "2";
    expectedSubscriber.m_caproEventMethodID = "3";
    expectedSubscriber.m_publisherIndex = -1;

    PublisherPortData expectedPublisher;
    expectedPublisher.m_name = namePublisher;
    expectedPublisher.m_caproInstanceID = "1";
    expectedPublisher.m_caproServiceID = "2";
    expectedPublisher.m_caproEventMethodID = "3";

    // prepare inputs
    iox::capro::ServiceDescription service(expectedPublisher.m_caproServiceID,
                                           expectedPublisher.m_caproInstanceID,
                                           expectedPublisher.m_caproEventMethodID);

    EXPECT_CALL(m_introspectionAccess.getPublisherPort().value(), sendChunk(_)).Times(10);

    // test adding of publisher or subscriber port of same service to establish a connection (requires same service id)
    iox::popo::SubscriberPortData recData1{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    EXPECT_THAT(m_introspectionAccess.addSubscriber(&recData1, nameSubscriber, service, ""), Eq(true));
    iox::popo::PublisherPortData publisherPortData{m_serviceDescription, "Foo", &m_memoryManager};
    EXPECT_THAT(m_introspectionAccess.addPublisher(&publisherPortData, namePublisher, service, ""), Eq(true));

    m_introspectionAccess.sendPortData();

    auto sample = m_introspectionAccess.getPublisherPort().value().m_chunk.sample();

    {
        // expect unconnected publisher or subscriber (service is equal but m_publisherIndex == -1 in subscriber)

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    // report messeges to establish a connection
    // remark: essentially a black box test of the internal state machine

    iox::capro::CaproMessageType type = iox::capro::CaproMessageType::SUB;
    iox::capro::CaproMessage message(type, service);
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB but no ACK
        expectedSubscriber.m_publisherIndex = -1;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::ACK;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK
        expectedSubscriber.m_publisherIndex = 0;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::UNSUB;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK
        expectedSubscriber.m_publisherIndex = -1;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::SUB;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB without ACK
        expectedSubscriber.m_publisherIndex = -1;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::NACK;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB followed by NACK
        expectedSubscriber.m_publisherIndex = -1;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::SUB;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB without ACK
        expectedSubscriber.m_publisherIndex = -1;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::ACK;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK
        expectedSubscriber.m_publisherIndex = 0;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::SUB;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK followed by another message
        // (SUB)
        expectedSubscriber.m_publisherIndex = 0;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::STOP_OFFER;
    m_introspectionAccess.reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect disconnected publisher or subscriber, since there was a STOP_OFFER
        expectedSubscriber.m_publisherIndex = -1;

        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_subscriberList[0], expectedSubscriber), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expectedPublisher), Eq(true));
    }

    sample->~PortIntrospectionFieldTopic();
}


TEST_F(PortIntrospection_test, DISABLED_thread)
{
    using PortData = iox::roudi::PortIntrospectionFieldTopic;
    auto chunkPortData = std::unique_ptr<ChunkMock<PortData>>(new ChunkMock<PortData>);

    using PortThroughput = iox::roudi::PortThroughputIntrospectionFieldTopic;
    ChunkMock<PortThroughput> chunkPortThroughput;

    using SubscriberPortChanging = iox::roudi::SubscriberPortChangingIntrospectionFieldTopic;
    ChunkMock<SubscriberPortChanging> chunkSubscriberPortChanging;

    EXPECT_CALL(m_introspectionAccess.getPublisherPort().value(), sendChunk(_)).Times(AtLeast(4));

    // we use the deliverChunk call to check how often the thread calls the send method
    m_introspectionAccess.setSendInterval(10);
    m_introspectionAccess.run();
    /// @todo this time can be reduced when the sleep mechanism of the port introspection thread is replace by a trigger
    /// queue
    std::this_thread::sleep_for(std::chrono::milliseconds(555)); // within this time, the thread should have run 6 times
    m_introspectionAccess.stop();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(555)); // if the thread doesn't stop, we have 12 runs after the sleep period
}
