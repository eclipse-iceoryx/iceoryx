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

template <typename SenderPort, typename ReceiverPort>
class PortIntrospectionAccess : public iox::roudi::PortIntrospection<SenderPort, ReceiverPort>
{
  public:
    void sendPortData()
    {
        iox::roudi::PortIntrospection<SenderPort, ReceiverPort>::sendPortData();
    }
    void sendThroughputData()
    {
        iox::roudi::PortIntrospection<SenderPort, ReceiverPort>::sendThroughputData();
    }
};

class PortIntrospection_test : public Test
{
  public:
    PortIntrospection_test()
        : m_introspectionAccess(
            static_cast<PortIntrospectionAccess<MockPublisherPortUser, MockSubscriberPortUser>&>(*m_introspection))
    {
    }

    ~PortIntrospection_test()
    {
    }

    virtual void SetUp()
    {
        internal::CaptureStdout();
        ASSERT_THAT(m_introspection->registerSenderPort(&m_publisherPortDataPortGeneric,
                                                        &m_publisherPortDataThroughput,
                                                        &m_publisherPortDataReceiverData),
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

    bool comparePortData(const iox::roudi::ReceiverPortData& a, const iox::roudi::ReceiverPortData& b)
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
        if (a.m_senderIndex != b.m_senderIndex)
            return false;
        if (a.m_runnable != b.m_runnable)
            return false;

        return true;
    }

    bool comparePortData(const iox::roudi::SenderPortData& a, const iox::roudi::SenderPortData& b)
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
    iox::popo::PublisherPortData m_publisherPortDataReceiverData{m_serviceDescription, "Foo", &m_memoryManager};

    MockPublisherPortUser m_publisherPortImpl_mock;
    MockPublisherPortUser m_portThroughput_mock;
    MockPublisherPortUser m_receiverPortData_mock;
    std::unique_ptr<iox::roudi::PortIntrospection<MockPublisherPortUser, MockSubscriberPortUser>> m_introspection{
        new iox::roudi::PortIntrospection<MockPublisherPortUser, MockSubscriberPortUser>};
    PortIntrospectionAccess<MockPublisherPortUser, MockSubscriberPortUser>& m_introspectionAccess;
};


TEST_F(PortIntrospection_test, registerSenderPort)
{
    iox::popo::PublisherPortData m_publisherPortDataPortGeneric{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataThroughput{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataReceiverData{m_serviceDescription, "Foo", &m_memoryManager};

    auto introspection = std::unique_ptr<iox::roudi::PortIntrospection<MockPublisherPortUser, MockSubscriberPortUser>>(
        new iox::roudi::PortIntrospection<MockPublisherPortUser, MockSubscriberPortUser>);

    EXPECT_THAT(introspection->registerSenderPort(
                    &m_publisherPortDataPortGeneric, &m_publisherPortDataThroughput, &m_publisherPortDataReceiverData),
                Eq(true));

    iox::popo::PublisherPortData m_publisherPortDataPortGeneric2{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataThroughput2{m_serviceDescription, "Foo", &m_memoryManager};
    iox::popo::PublisherPortData m_publisherPortDataReceiverData2{m_serviceDescription, "Foo", &m_memoryManager};

    EXPECT_THAT(introspection->registerSenderPort(&m_publisherPortDataPortGeneric2,
                                                  &m_publisherPortDataThroughput2,
                                                  &m_publisherPortDataReceiverData2),
                Eq(false));
}

TEST_F(PortIntrospection_test, sendPortData_EmptyList)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_introspectionAccess.sendPortData();

    // topic contains no publisher or subscriber ports but 0xFF bytes are overwritten

    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk).Times(1);
    EXPECT_THAT(chunk->sample()->m_senderList.size(), Eq(0));
    EXPECT_THAT(chunk->sample()->m_receiverList.size(), Eq(0));
}

TEST_F(PortIntrospection_test, sendThroughputData_EmptyList)
{
    /// @todo #252 re-add port throughput for v1.0?
    using Topic = iox::roudi::PortThroughputIntrospectionFieldTopic;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_introspectionAccess.sendThroughputData();

    // topic contains no publisher or subscriber ports but 0xFF bytes are overwritten

    EXPECT_THAT(chunk->sample()->m_throughputList.size(), Eq(0));
    EXPECT_CALL(m_portThroughput_mock, sendChunk).Times(1);
}

TEST_F(PortIntrospection_test, sendData_OneSender)
{
    /// @todo #252 re-add port throughput for v1.0?

    // using PortData = iox::roudi::SenderPortData;
    // using ThroughputData = iox::roudi::PortThroughputData;

    // using PortDataTopic = iox::roudi::PortIntrospectionFieldTopic;
    // using ThroughputTopic = iox::roudi::PortThroughputIntrospectionFieldTopic;

    // using PortDataChunk = ChunkMock<PortDataTopic>;
    // using ThroughputChunk = ChunkMock<ThroughputTopic>;

    // auto portDataTopic = std::unique_ptr<PortDataChunk>(new PortDataChunk);
    // auto throughputTopic = std::unique_ptr<ThroughputChunk>(new ThroughputChunk);

    // MockPublisherPortUser publisherPort;
    // std::string publisherPortName("name");

    // PortData expectedSenderPortData;
    // expectedSenderPortData.m_name = iox::cxx::string<100>(iox::cxx::TruncateToCapacity, publisherPortName.c_str());
    // expectedSenderPortData.m_caproInstanceID = "1";
    // expectedSenderPortData.m_caproServiceID = "2";
    // expectedSenderPortData.m_caproEventMethodID = "3";

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

    // iox::capro::ServiceDescription service(expectedSenderPortData.m_caproServiceID,
    //                                        expectedSenderPortData.m_caproInstanceID,
    //                                        expectedSenderPortData.m_caproEventMethodID);

    // iox::popo::SenderPortData publisherPortData;
    // publisherPortData.m_throughputReadCache = expectedThroughput;
    // publisherPortData.m_processName = expectedSenderPortData.m_name;

    // EXPECT_THAT(m_introspection->addSender(&publisherPortData, publisherPortName, service, ""), Eq(true));

    // SenderPort_MOCK::globalDetails = std::make_shared<SenderPort_MOCK::mock_t>();
    // SenderPort_MOCK::globalDetails->reserveSampleReturn = throughputTopic->chunkHeader();
    // SenderPort_MOCK::globalDetails->getThroughputReturn = expectedThroughput;
    // m_introspectionAccess.sendThroughputData();

    // expectedThroughput.sequenceNumber++;
    // expectedThroughput.lastDeliveryTimestamp = TimePointNs(DurationNs(durationNs));
    // expectedThroughput.currentDeliveryTimestamp = TimePointNs(DurationNs(2 * durationNs));

    // m_introspectionAccess.sendPortData();

    // topic contains no publisher or subscriber ports but 0xFF bytes are overwritten

    // ASSERT_THAT(portDataTopic->sample()->m_senderList.size(), Eq(1));
    // auto sentSenderPortData = portDataTopic->sample()->m_senderList[0];
    // EXPECT_THAT(sentSenderPortData.m_publisherPortID, Eq(ExpectedUniqueID));
    // EXPECT_THAT(sentSenderPortData.m_name, Eq(expectedSenderPortData.m_name));
    // EXPECT_THAT(sentSenderPortData.m_caproInstanceID, Eq(expectedSenderPortData.m_caproInstanceID));
    // EXPECT_THAT(sentSenderPortData.m_caproServiceID, Eq(expectedSenderPortData.m_caproServiceID));
    // EXPECT_THAT(sentSenderPortData.m_caproEventMethodID, Eq(expectedSenderPortData.m_caproEventMethodID));

    // SenderPort_MOCK::globalDetails = std::make_shared<SenderPort_MOCK::mock_t>();
    // SenderPort_MOCK::globalDetails->getUniqueIDReturn = ExpectedUniqueID;
    // SenderPort_MOCK::globalDetails->reserveSampleReturn = throughputTopic->chunkHeader();
    // SenderPort_MOCK::globalDetails->getThroughputReturn = expectedThroughput;
    // m_introspectionAccess.sendThroughputData();
    // SenderPort_MOCK::globalDetails.reset();


    // ASSERT_THAT(throughputTopic->sample()->m_throughputList.size(), Eq(1));
    // auto sentThroughputData = throughputTopic->sample()->m_throughputList[0];
    // EXPECT_THAT(sentThroughputData.m_publisherPortID, Eq(ExpectedUniqueID));
    // EXPECT_THAT(sentThroughputData.m_sampleSize, Eq(expectedThroughputData.m_sampleSize));
    // EXPECT_THAT(sentThroughputData.m_chunkSize, Eq(expectedThroughputData.m_chunkSize));
    // EXPECT_THAT(sentThroughputData.m_chunksPerMinute, DoubleEq(expectedThroughputData.m_chunksPerMinute));
    // EXPECT_THAT(sentThroughputData.m_lastSendIntervalInNanoseconds,
    //             Eq(expectedThroughputData.m_lastSendIntervalInNanoseconds));
}


TEST_F(PortIntrospection_test, addAndRemoveSender)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using PortData = iox::roudi::SenderPortData;

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
    EXPECT_THAT(m_introspection->addSender(&portData1, name1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspection->addSender(&portData1, name1, service1, "4"), Eq(false));
    EXPECT_THAT(m_introspection->addSender(&portData2, name2, service2, "jkl"), Eq(true));
    EXPECT_THAT(m_introspection->addSender(&portData2, name2, service2, "jkl"), Eq(false));

    m_introspectionAccess.sendPortData();

    auto sample = chunk->sample();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(2));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(0));

        auto& senderInfo1 = sample->m_senderList[0];
        auto& senderInfo2 = sample->m_senderList[1];

        // remark: we cannot ensure that the order is the same as the order in
        // which the ports where added  we therefore expect to find both ports
        // with the corresponding ids (we need to check whether multiple port
        // insertions also work correctly, therefore we need at least two ports)

        if (comparePortData(senderInfo1, expected1))
        {
            EXPECT_THAT(comparePortData(senderInfo2, expected2), Eq(true));
        }
        else
        {
            EXPECT_THAT(comparePortData(senderInfo2, expected1), Eq(true));
        }
    }

    // test removal of ports

    EXPECT_THAT(m_introspection->removeSender(name1, service1), Eq(true));
    EXPECT_THAT(m_introspection->removeSender(name1, service1), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(0));

        EXPECT_THAT(comparePortData(sample->m_senderList[0], expected2), Eq(true));
    }

    EXPECT_THAT(m_introspection->removeSender(name2, service2), Eq(true));
    EXPECT_THAT(m_introspection->removeSender(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(0));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(0));
    }

    EXPECT_THAT(m_introspection->removeSender(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(0));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(0));
    }

    sample->~PortIntrospectionFieldTopic();

    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk).Times(4);
}

TEST_F(PortIntrospection_test, addAndRemoveReceiver)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using PortData = iox::roudi::ReceiverPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    iox::cxx::string<100> name1("name1");
    iox::cxx::string<100> name2("name2");

    // prepare expected outputs
    PortData expected1;
    expected1.m_name = name1;
    expected1.m_caproInstanceID = "1";
    expected1.m_caproServiceID = "2";
    expected1.m_caproEventMethodID = "3";
    expected1.m_senderIndex = -1;
    expected1.m_runnable = iox::cxx::string<100>("4");

    PortData expected2;
    expected2.m_name = name2;
    expected2.m_caproInstanceID = "4";
    expected2.m_caproServiceID = "5";
    expected2.m_caproEventMethodID = "6";
    expected2.m_senderIndex = -1;
    expected2.m_runnable = iox::cxx::string<100>("7");

    // prepare inputs
    iox::capro::ServiceDescription service1(
        expected1.m_caproServiceID, expected1.m_caproInstanceID, expected1.m_caproEventMethodID);
    iox::capro::ServiceDescription service2(
        expected2.m_caproServiceID, expected2.m_caproInstanceID, expected2.m_caproEventMethodID);

    // test adding of ports

    // remark: duplicate receiver insertions are possible but will not be transmitted via send
    iox::popo::ReceiverPortData recData1{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    iox::popo::ReceiverPortData recData2{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    EXPECT_THAT(m_introspection->addReceiver(&recData1, name1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspection->addReceiver(&recData1, name1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspection->addReceiver(&recData2, name2, service2, "7"), Eq(true));
    EXPECT_THAT(m_introspection->addReceiver(&recData2, name2, service2, "7"), Eq(true));

    m_introspectionAccess.sendPortData();

    auto sample = chunk->sample();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(0));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(2));

        auto& receiverInfo1 = sample->m_receiverList[0];
        auto& receiverInfo2 = sample->m_receiverList[1];

        // remark: we cannot ensure that the order is the same as the order in
        // which the ports where added  we therefore expect to find both ports
        // with the corresponding ids (we need to check whether multiple port
        // insertions also work correctly, therefore we need at least two ports)

        if (comparePortData(receiverInfo1, expected1))
        {
            EXPECT_THAT(comparePortData(receiverInfo2, expected2), Eq(true));
        }
        else
        {
            EXPECT_THAT(comparePortData(receiverInfo2, expected1), Eq(true));
        }
    }

    // test removal of ports

    EXPECT_THAT(m_introspection->removeReceiver(name1, service1), Eq(true));
    EXPECT_THAT(m_introspection->removeReceiver(name1, service1), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(0));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        auto& senderInfo = sample->m_receiverList[0];

        EXPECT_THAT(comparePortData(senderInfo, expected2), Eq(true));
    }

    EXPECT_THAT(m_introspection->removeReceiver(name2, service2), Eq(true));
    EXPECT_THAT(m_introspection->removeReceiver(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(0));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(0));
    }

    EXPECT_THAT(m_introspection->removeReceiver(name2, service2), Eq(false));

    m_introspectionAccess.sendPortData();

    {
        ASSERT_THAT(sample->m_senderList.size(), Eq(0));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(0));
    }

    sample->~PortIntrospectionFieldTopic();

    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk).Times(4);
}


TEST_F(PortIntrospection_test, reportMessageToEstablishConnection)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using ReceiverPortData = iox::roudi::ReceiverPortData;
    using SenderPortData = iox::roudi::SenderPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    std::string nameReceiver("receiver");
    std::string nameSender("sender");

    // prepare expected outputs
    ReceiverPortData expectedReceiver;
    expectedReceiver.m_name = iox::cxx::string<100>(iox::cxx::TruncateToCapacity, nameReceiver.c_str());
    expectedReceiver.m_caproInstanceID = "1";
    expectedReceiver.m_caproServiceID = "2";
    expectedReceiver.m_caproEventMethodID = "3";
    expectedReceiver.m_senderIndex = -1;

    SenderPortData expectedSender;
    expectedSender.m_name = iox::cxx::string<100>(iox::cxx::TruncateToCapacity, nameSender.c_str());
    expectedSender.m_caproInstanceID = "1";
    expectedSender.m_caproServiceID = "2";
    expectedSender.m_caproEventMethodID = "3";

    // prepare inputs
    iox::capro::ServiceDescription service(
        expectedSender.m_caproServiceID, expectedSender.m_caproInstanceID, expectedSender.m_caproEventMethodID);

    // test adding of publisher or subscriber port of same service to establish a connection (requires same service id)
    iox::popo::ReceiverPortData recData1{
        m_serviceDescription, "Foo", iox::cxx::VariantQueueTypes::FiFo_MultiProducerSingleConsumer};
    EXPECT_THAT(m_introspection->addReceiver(&recData1, nameReceiver, service, ""), Eq(true));
    iox::popo::PublisherPortData publisherPortData{m_serviceDescription, "Foo", &m_memoryManager};
    EXPECT_THAT(m_introspection->addSender(&publisherPortData, nameSender, service, ""), Eq(true));

    m_introspectionAccess.sendPortData();

    auto sample = chunk->sample();

    {
        // expect unconnected publisher or subscriber (service is equal but m_senderIndex == -1 in receiver)

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    // report messeges to establish a connection
    // remark: essentially a black box test of the internal state machine

    iox::capro::CaproMessageType type = iox::capro::CaproMessageType::SUB;
    iox::capro::CaproMessage message(type, service);
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB but no ACK
        expectedReceiver.m_senderIndex = -1;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::ACK;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK
        expectedReceiver.m_senderIndex = 0;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::UNSUB;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK
        expectedReceiver.m_senderIndex = -1;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::SUB;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB without ACK
        expectedReceiver.m_senderIndex = -1;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::NACK;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB followed by NACK
        expectedReceiver.m_senderIndex = -1;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::SUB;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect unconnected publisher or subscriber, since there was a SUB without ACK
        expectedReceiver.m_senderIndex = -1;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::ACK;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK
        expectedReceiver.m_senderIndex = 0;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::SUB;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect connected publisher or subscriber, since there was a SUB followed by ACK followed by another message
        // (SUB)
        expectedReceiver.m_senderIndex = 0;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    message.m_type = iox::capro::CaproMessageType::STOP_OFFER;
    m_introspection->reportMessage(message);
    m_introspectionAccess.sendPortData();

    {
        // expect disconnected publisher or subscriber, since there was a STOP_OFFER
        expectedReceiver.m_senderIndex = -1;

        ASSERT_THAT(sample->m_senderList.size(), Eq(1));
        ASSERT_THAT(sample->m_receiverList.size(), Eq(1));

        EXPECT_THAT(comparePortData(sample->m_receiverList[0], expectedReceiver), Eq(true));
        EXPECT_THAT(comparePortData(sample->m_senderList[0], expectedSender), Eq(true));
    }

    sample->~PortIntrospectionFieldTopic();
}


TEST_F(PortIntrospection_test, thread)
{
    using PortData = iox::roudi::PortIntrospectionFieldTopic;
    auto chunkPortData = std::unique_ptr<ChunkMock<PortData>>(new ChunkMock<PortData>);

    using PortThroughput = iox::roudi::PortThroughputIntrospectionFieldTopic;
    ChunkMock<PortThroughput> chunkPortThroughput;

    using ReceiverPortChanging = iox::roudi::ReceiverPortChangingIntrospectionFieldTopic;
    ChunkMock<ReceiverPortChanging> chunkReceiverPortChanging;

    // we use the deliverChunk call to check how often the thread calls the send method
    m_introspection->setSendInterval(10);
    m_introspection->run();
    /// @todo this time can be reduced when the sleep mechanism of the port introspection thread is replace by a trigger
    /// queue
    std::this_thread::sleep_for(std::chrono::milliseconds(555)); // within this time, the thread should have run 6 times
    m_introspection->stop();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(555)); // if the thread doesn't stop, we have 12 runs after the sleep period
    EXPECT_CALL(m_publisherPortImpl_mock, sendChunk).Times(1);
    EXPECT_CALL(m_portThroughput_mock, sendChunk).Times(AtLeast(4));
    EXPECT_CALL(m_receiverPortData_mock, sendChunk).Times(AtLeast(4));
}
