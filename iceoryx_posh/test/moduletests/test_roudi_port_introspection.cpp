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
// MockPublisherPortUser, MockSubscriberPortUser
class PortIntrospectionAccess : public iox::roudi::PortIntrospection
{
  public:
    void sendPortData()
    {
        iox::roudi::PortIntrospection::sendPortData();
    }
    void sendThroughputData()
    {
        iox::roudi::PortIntrospection::sendThroughputData();
    }
};

class PortIntrospection_test : public Test
{
  public:
    PortIntrospection_test()
        : m_introspectionAccess(static_cast<PortIntrospectionAccess&>(*m_introspection))
    {
    }

    ~PortIntrospection_test()
    {
    }

    virtual void SetUp()
    {
        internal::CaptureStdout();
        ASSERT_THAT(m_introspection->registerPublisherPort(&m_publisherPortDataPortGeneric,
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

    MockPublisherPortUser m_publisherPortImpl_mock;
    MockPublisherPortUser m_portThroughput_mock;
    MockPublisherPortUser m_subscriberPortData_mock;
    std::unique_ptr<iox::roudi::PortIntrospection> m_introspection{new iox::roudi::PortIntrospection};
    PortIntrospectionAccess& m_introspectionAccess;
};


TEST_F(PortIntrospection_test, registerPublisherPort)
{
    iox::popo::PublisherPortData m_publisherPortDataPortGeneric{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataThroughput{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataSubscriberData{m_serviceDescription, "Foo", &m_memoryManager};

    auto introspection = std::unique_ptr<iox::roudi::PortIntrospection>(new iox::roudi::PortIntrospection);

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
    using Topic = iox::roudi::PortIntrospectionFieldTopic;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_introspectionAccess.sendPortData();

    // topic contains no publisher or subscriber ports but 0xFF bytes are overwritten

    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(1);
    EXPECT_THAT(chunk->sample()->m_publisherList.size(), Eq(0));
    EXPECT_THAT(chunk->sample()->m_subscriberList.size(), Eq(0));
}

TEST_F(PortIntrospection_test, sendThroughputData_EmptyList)
{
    /// @todo #252 re-add port throughput for v1.0?
    using Topic = iox::roudi::PortThroughputIntrospectionFieldTopic;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_introspectionAccess.sendThroughputData();

    // topic contains no publisher or subscriber ports but 0xFF bytes are overwritten

    EXPECT_THAT(chunk->sample()->m_throughputList.size(), Eq(0));
    EXPECT_CALL(m_portThroughput_mock, sendChunk(_)).Times(1);
}

TEST_F(PortIntrospection_test, sendData_OnePublisher)
{
    /// @todo #252 re-add port throughput for v1.0?

    // using PortData = iox::roudi::PublisherPortData;
    // using ThroughputData = iox::roudi::PortThroughputData;

    // using PortDataTopic = iox::roudi::PortIntrospectionFieldTopic;
    // using ThroughputTopic = iox::roudi::PortThroughputIntrospectionFieldTopic;

    // using PortDataChunk = ChunkMock<PortDataTopic>;
    // using ThroughputChunk = ChunkMock<ThroughputTopic>;

    // auto portDataTopic = std::unique_ptr<PortDataChunk>(new PortDataChunk);
    // auto throughputTopic = std::unique_ptr<ThroughputChunk>(new ThroughputChunk);

    // MockPublisherPortUser publisherPort;
    // std::string publisherPortName("name");

    // PortData expectedPublisherPortData;
    // expectedPublisherPortData.m_name = iox::cxx::string<100>(iox::cxx::TruncateToCapacity,
    // publisherPortName.c_str()); expectedPublisherPortData.m_caproInstanceID = "1";
    // expectedPublisherPortData.m_caproServiceID = "2";
    // expectedPublisherPortData.m_caproEventMethodID = "3";

    // constexpr uint64_t ExpectedUniqueID{1337};
    // constexpr double NsPerSecond{1000000000.};
    // constexpr uint64_t durationNs{100000000};
    // MockPublisherPortUser::Throughput expectedThroughput;
    // expectedThroughput.payloadSize = 73;
    // expectedThroughput.chunkSize = 128;
    // expectedThroughput.sequenceNumber = 13;
    // expectedThroughput.lastDeliveryTimestamp = TimePointNs(DurationNs(0));
    // expectedThroughput.currentDeliveryTimestamp = TimePointNs(DurationNs(durationNs));
    // ThroughputData expectedThroughputData;
    // expectedThroughputData.m_publisherPortID = ExpectedUniqueID;
    // expectedThroughputData.m_sampleSize = expectedThroughput.payloadSize;
    // expectedThroughputData.m_chunkSize = expectedThroughput.chunkSize;
    // expectedThroughputData.m_chunksPerMinute = 60. / (static_cast<double>(durationNs) / NsPerSecond);
    // expectedThroughputData.m_lastSendIntervalInNanoseconds = durationNs;

    // iox::capro::ServiceDescription service(expectedPublisherPortData.m_caproServiceID,
    //                                        expectedPublisherPortData.m_caproInstanceID,
    //                                        expectedPublisherPortData.m_caproEventMethodID);

    // iox::popo::PublisherPortData publisherPortData;
    // publisherPortData.m_throughputReadCache = expectedThroughput;
    // publisherPortData.m_processName = expectedPublisherPortData.m_name;

    // EXPECT_THAT(m_introspection->addPublisher(&publisherPortData, publisherPortName, service, ""), Eq(true));

    // PublisherPort_MOCK::globalDetails = std::make_shared<PublisherPort_MOCK::mock_t>();
    // PublisherPort_MOCK::globalDetails->reserveSampleReturn = throughputTopic->chunkHeader();
    // PublisherPort_MOCK::globalDetails->getThroughputReturn = expectedThroughput;
    // m_introspectionAccess.sendThroughputData();

    // expectedThroughput.sequenceNumber++;
    // expectedThroughput.lastDeliveryTimestamp = TimePointNs(DurationNs(durationNs));
    // expectedThroughput.currentDeliveryTimestamp = TimePointNs(DurationNs(2 * durationNs));

    // m_introspectionAccess.sendPortData();

    // topic contains no publisher or subscriber ports but 0xFF bytes are overwritten

    // ASSERT_THAT(portDataTopic->sample()->m_publisherList.size(), Eq(1));
    // auto sentPublisherPortData = portDataTopic->sample()->m_publisherList[0];
    // EXPECT_THAT(sentPublisherPortData.m_publisherPortID, Eq(ExpectedUniqueID));
    // EXPECT_THAT(sentPublisherPortData.m_name, Eq(expectedPublisherPortData.m_name));
    // EXPECT_THAT(sentPublisherPortData.m_caproInstanceID, Eq(expectedPublisherPortData.m_caproInstanceID));
    // EXPECT_THAT(sentPublisherPortData.m_caproServiceID, Eq(expectedPublisherPortData.m_caproServiceID));
    // EXPECT_THAT(sentPublisherPortData.m_caproEventMethodID, Eq(expectedPublisherPortData.m_caproEventMethodID));

    // PublisherPort_MOCK::globalDetails = std::make_shared<PublisherPort_MOCK::mock_t>();
    // PublisherPort_MOCK::globalDetails->getUniqueIDReturn = ExpectedUniqueID;
    // PublisherPort_MOCK::globalDetails->reserveSampleReturn = throughputTopic->chunkHeader();
    // PublisherPort_MOCK::globalDetails->getThroughputReturn = expectedThroughput;
    // m_introspectionAccess.sendThroughputData();
    // PublisherPort_MOCK::globalDetails.reset();


    // ASSERT_THAT(throughputTopic->sample()->m_throughputList.size(), Eq(1));
    // auto sentThroughputData = throughputTopic->sample()->m_throughputList[0];
    // EXPECT_THAT(sentThroughputData.m_publisherPortID, Eq(ExpectedUniqueID));
    // EXPECT_THAT(sentThroughputData.m_sampleSize, Eq(expectedThroughputData.m_sampleSize));
    // EXPECT_THAT(sentThroughputData.m_chunkSize, Eq(expectedThroughputData.m_chunkSize));
    // EXPECT_THAT(sentThroughputData.m_chunksPerMinute, DoubleEq(expectedThroughputData.m_chunksPerMinute));
    // EXPECT_THAT(sentThroughputData.m_lastSendIntervalInNanoseconds,
    //             Eq(expectedThroughputData.m_lastSendIntervalInNanoseconds));
}


TEST_F(PortIntrospection_test, addAndRemovePublisher)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using PortData = iox::roudi::PublisherPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    iox::cxx::string<100> name1("name1");
    iox::cxx::string<100> name2("name2");

    // prepare expected outputs
    PortData expected1;
    expected1.m_name = name1;
    expected1.m_caproInstanceID = "1";
    expected1.m_caproServiceID = "2";
    expected1.m_caproEventMethodID = "3";
    expected1.m_runnable = iox::cxx::string<100>("4");

    PortData expected2;
    expected2.m_name = name2;
    expected2.m_caproInstanceID = "abc";
    expected2.m_caproServiceID = "def";
    expected2.m_caproEventMethodID = "ghi";
    expected2.m_runnable = iox::cxx::string<100>("jkl");

    // prepare inputs
    iox::capro::ServiceDescription service1(
        expected1.m_caproServiceID, expected1.m_caproInstanceID, expected1.m_caproEventMethodID);
    iox::capro::ServiceDescription service2(
        expected2.m_caproServiceID, expected2.m_caproInstanceID, expected2.m_caproEventMethodID);

    // test adding of ports

    // remark: duplicate publisher port insertions are not possible

    iox::popo::PublisherPortData portData1{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData portData2{m_serviceDescription, "Foo", &m_memoryManager};
    EXPECT_THAT(m_introspection->addPublisher(&portData1, name1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspection->addPublisher(&portData1, name1, service1, "4"), Eq(false));
    EXPECT_THAT(m_introspection->addPublisher(&portData2, name2, service2, "jkl"), Eq(true));
    EXPECT_THAT(m_introspection->addPublisher(&portData2, name2, service2, "jkl"), Eq(false));

    m_introspectionAccess.sendPortData();

    auto sample = chunk->sample();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(2));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));

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

    EXPECT_THAT(m_introspection->removePublisher(name1, service1), Eq(true));
    EXPECT_THAT(m_introspection->removePublisher(name1, service1), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(1));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));

        EXPECT_THAT(comparePortData(sample->m_publisherList[0], expected2), Eq(true));
    }

    EXPECT_THAT(m_introspection->removePublisher(name2, service2), Eq(true));
    EXPECT_THAT(m_introspection->removePublisher(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    EXPECT_THAT(m_introspection->removePublisher(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    sample->~PortIntrospectionFieldTopic();

    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(4);
}

TEST_F(PortIntrospection_test, addAndRemoveSubscriber)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using PortData = iox::roudi::SubscriberPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    iox::cxx::string<100> name1("name1");
    iox::cxx::string<100> name2("name2");

    // prepare expected outputs
    PortData expected1;
    expected1.m_name = name1;
    expected1.m_caproInstanceID = "1";
    expected1.m_caproServiceID = "2";
    expected1.m_caproEventMethodID = "3";
    expected1.m_publisherIndex = -1;
    expected1.m_runnable = iox::cxx::string<100>("4");

    PortData expected2;
    expected2.m_name = name2;
    expected2.m_caproInstanceID = "4";
    expected2.m_caproServiceID = "5";
    expected2.m_caproEventMethodID = "6";
    expected2.m_publisherIndex = -1;
    expected2.m_runnable = iox::cxx::string<100>("7");

    // prepare inputs
    iox::capro::ServiceDescription service1(
        expected1.m_caproServiceID, expected1.m_caproInstanceID, expected1.m_caproEventMethodID);
    iox::capro::ServiceDescription service2(
        expected2.m_caproServiceID, expected2.m_caproInstanceID, expected2.m_caproEventMethodID);

    // test adding of ports

    // remark: duplicate subscriber insertions are possible but will not be transmitted via send
    iox::popo::SubscriberPortData recData1{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    iox::popo::SubscriberPortData recData2{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    EXPECT_THAT(m_introspection->addSubscriber(&recData1, name1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspection->addSubscriber(&recData1, name1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspection->addSubscriber(&recData2, name2, service2, "7"), Eq(true));
    EXPECT_THAT(m_introspection->addSubscriber(&recData2, name2, service2, "7"), Eq(true));

    m_introspectionAccess.sendPortData();

    auto sample = chunk->sample();

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

    EXPECT_THAT(m_introspection->removeSubscriber(name1, service1), Eq(true));
    EXPECT_THAT(m_introspection->removeSubscriber(name1, service1), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(1));

        auto& publisherInfo = sample->m_subscriberList[0];

        EXPECT_THAT(comparePortData(publisherInfo, expected2), Eq(true));
    }

    EXPECT_THAT(m_introspection->removeSubscriber(name2, service2), Eq(true));
    EXPECT_THAT(m_introspection->removeSubscriber(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    EXPECT_THAT(m_introspection->removeSubscriber(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_publisherList.size(), Eq(0));
        ASSERT_THAT(sample->m_subscriberList.size(), Eq(0));
    }

    sample->~PortIntrospectionFieldTopic();

    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(4);
}


TEST_F(PortIntrospection_test, reportMessageToEstablishConnection)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using SubscriberPortData = iox::roudi::SubscriberPortData;
    using PublisherPortData = iox::roudi::PublisherPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    std::string nameSubscriber("subscriber");
    std::string namePublisher("publisher");

    // prepare expected outputs
    SubscriberPortData expectedSubscriber;
    expectedSubscriber.m_name = iox::cxx::string<100>(iox::cxx::TruncateToCapacity, nameSubscriber.c_str());
    expectedSubscriber.m_caproInstanceID = "1";
    expectedSubscriber.m_caproServiceID = "2";
    expectedSubscriber.m_caproEventMethodID = "3";
    expectedSubscriber.m_publisherIndex = -1;

    PublisherPortData expectedPublisher;
    expectedPublisher.m_name = iox::cxx::string<100>(iox::cxx::TruncateToCapacity, namePublisher.c_str());
    expectedPublisher.m_caproInstanceID = "1";
    expectedPublisher.m_caproServiceID = "2";
    expectedPublisher.m_caproEventMethodID = "3";

    // prepare inputs
    iox::capro::ServiceDescription service(expectedPublisher.m_caproServiceID,
                                           expectedPublisher.m_caproInstanceID,
                                           expectedPublisher.m_caproEventMethodID);

    // test adding of publisher or subscriber port of same service to establish a connection (requires same service id)
    iox::popo::SubscriberPortData recData1{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    EXPECT_THAT(m_introspection->addSubscriber(&recData1, nameSubscriber, service, ""), Eq(true));
    iox::popo::PublisherPortData publisherPortData{m_serviceDescription, "Foo", &m_memoryManager};
    EXPECT_THAT(m_introspection->addPublisher(&publisherPortData, namePublisher, service, ""), Eq(true));

    m_introspectionAccess.sendPortData();

    auto sample = chunk->sample();

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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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
    m_introspection->reportMessage(message);
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


TEST_F(PortIntrospection_test, thread)
{
    using PortData = iox::roudi::PortIntrospectionFieldTopic;
    auto chunkPortData = std::unique_ptr<ChunkMock<PortData>>(new ChunkMock<PortData>);

    using PortThroughput = iox::roudi::PortThroughputIntrospectionFieldTopic;
    ChunkMock<PortThroughput> chunkPortThroughput;

    using SubscriberPortChanging = iox::roudi::SubscriberPortChangingIntrospectionFieldTopic;
    ChunkMock<SubscriberPortChanging> chunkSubscriberPortChanging;

    // we use the deliverChunk call to check how often the thread calls the send method
    m_introspection->setSendInterval(10);
    m_introspection->run();
    /// @todo this time can be reduced when the sleep mechanism of the port introspection thread is replace by a trigger
    /// queue
    std::this_thread::sleep_for(std::chrono::milliseconds(555)); // within this time, the thread should have run 6 times
    m_introspection->stop();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(555)); // if the thread doesn't stop, we have 12 runs after the sleep period
    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk(_)).Times(1);
    EXPECT_CALL(m_portThroughput_mock, sendChunk(_)).Times(AtLeast(4));
    EXPECT_CALL(m_subscriberPortData_mock, sendChunk(_)).Times(AtLeast(4));
}
