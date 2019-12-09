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

#include "mocks/chunk_mock.hpp"
#include "mocks/receiverport_mock.hpp"
#include "mocks/senderport_mock.hpp"
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
              static_cast<PortIntrospectionAccess<SenderPort_MOCK, ReceiverPort_MOCK>&>(*m_introspection))
    {
    }

    ~PortIntrospection_test()
    {
    }

    virtual void SetUp()
    {
        internal::CaptureStdout();
        m_senderPortImpl_mock = m_senderPortImpl.details;
        m_portThroughput_mock = m_portThroughput.details;
        m_receiverPortData_mock = m_portreceiverPortData.details;
        m_senderPortImpl_mock->isConnectedToMembersReturn = true;
        m_portThroughput_mock->isConnectedToMembersReturn = true;
        m_receiverPortData_mock->isConnectedToMembersReturn = true;
        ASSERT_THAT(m_introspection->registerSenderPort(
                        std::move(m_senderPortImpl), std::move(m_portThroughput), std::move(m_portreceiverPortData)),
                    Eq(true));
        EXPECT_THAT(m_senderPortImpl_mock->enableDoDeliverOnSubscription, Eq(1));
        EXPECT_THAT(m_portThroughput_mock->enableDoDeliverOnSubscription, Eq(1));
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

    std::shared_ptr<SenderPort_MOCK::mock_t> m_senderPortImpl_mock;
    std::shared_ptr<SenderPort_MOCK::mock_t> m_portThroughput_mock;
    std::shared_ptr<SenderPort_MOCK::mock_t> m_receiverPortData_mock;
    SenderPort_MOCK m_senderPortImpl;
    SenderPort_MOCK m_portThroughput;
    SenderPort_MOCK m_portreceiverPortData;
    std::unique_ptr<iox::roudi::PortIntrospection<SenderPort_MOCK, ReceiverPort_MOCK>> m_introspection{
        new iox::roudi::PortIntrospection<SenderPort_MOCK, ReceiverPort_MOCK>};
    PortIntrospectionAccess<SenderPort_MOCK, ReceiverPort_MOCK>& m_introspectionAccess;
};


TEST_F(PortIntrospection_test, registerSenderPort)
{
    SenderPort_MOCK senderPortImpl_mock;
    SenderPort_MOCK portThroughput_mock;
    SenderPort_MOCK portreceiverPortData_mock;
    auto introspection = std::unique_ptr<iox::roudi::PortIntrospection<SenderPort_MOCK, ReceiverPort_MOCK>>(
        new iox::roudi::PortIntrospection<SenderPort_MOCK, ReceiverPort_MOCK>);

    auto mockSender = senderPortImpl_mock.details;
    auto mockPort = portThroughput_mock.details;
    mockSender->isConnectedToMembersReturn = true;
    mockPort->isConnectedToMembersReturn = true;
    EXPECT_THAT(introspection->registerSenderPort(std::move(senderPortImpl_mock),
                                                  std::move(portThroughput_mock),
                                                  std::move(portreceiverPortData_mock)),
                Eq(true));

    SenderPort_MOCK senderPortImpl_mock2;
    SenderPort_MOCK portThroughput_mock2;
    SenderPort_MOCK portreceiverPortData_mock2;
    EXPECT_THAT(introspection->registerSenderPort(std::move(senderPortImpl_mock2),
                                                  std::move(portThroughput_mock2),
                                                  std::move(portreceiverPortData_mock2)),
                Eq(false));
    EXPECT_THAT(mockSender->enableDoDeliverOnSubscription, Eq(1));
    EXPECT_THAT(mockPort->enableDoDeliverOnSubscription, Eq(1));
}

TEST_F(PortIntrospection_test, sendPortData_EmptyList)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_senderPortImpl_mock->reserveSampleReturn = chunk->chunkHeader();

    m_introspectionAccess.sendPortData();

    // topic contains no sender or receiver ports but 0xFF bytes are overwritten

    EXPECT_THAT(m_senderPortImpl_mock->deliverChunk, Eq(1));
    EXPECT_THAT(chunk->sample()->m_senderList.size(), Eq(0));
    EXPECT_THAT(chunk->sample()->m_receiverList.size(), Eq(0));
}

TEST_F(PortIntrospection_test, sendThroughputData_EmptyList)
{
    using Topic = iox::roudi::PortThroughputIntrospectionFieldTopic;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_portThroughput_mock->reserveSampleReturn = chunk->chunkHeader();

    m_introspectionAccess.sendThroughputData();

    // topic contains no sender or receiver ports but 0xFF bytes are overwritten

    EXPECT_THAT(chunk->sample()->m_throughputList.size(), Eq(0));
    EXPECT_THAT(m_portThroughput_mock->deliverChunk, Eq(1));
}

TEST_F(PortIntrospection_test, sendData_OneSender)
{
    using PortData = iox::roudi::SenderPortData;
    using ThroughputData = iox::roudi::PortThroughputData;

    using PortDataTopic = iox::roudi::PortIntrospectionFieldTopic;
    using ThroughputTopic = iox::roudi::PortThroughputIntrospectionFieldTopic;

    using PortDataChunk = ChunkMock<PortDataTopic>;
    using ThroughputChunk = ChunkMock<ThroughputTopic>;

    auto portDataTopic = std::unique_ptr<PortDataChunk>(new PortDataChunk);
    auto throughputTopic = std::unique_ptr<ThroughputChunk>(new ThroughputChunk);

    m_senderPortImpl_mock->reserveSampleReturn = portDataTopic->chunkHeader();
    m_portThroughput_mock->reserveSampleReturn = throughputTopic->chunkHeader();

    SenderPort_MOCK senderPort;
    auto mockSenderPort = senderPort.details;
    std::string senderPortName("name");

    PortData expectedSenderPortData;
    expectedSenderPortData.m_name = iox::cxx::CString100(senderPortName.c_str());
    expectedSenderPortData.m_caproInstanceID = "1";
    expectedSenderPortData.m_caproServiceID = "2";
    expectedSenderPortData.m_caproEventMethodID = "3";

    constexpr uint64_t ExpectedUniqueID{1337};
    constexpr double NsPerSecond{1000000000.};
    constexpr uint64_t durationNs{100000000};
    SenderPort_MOCK::Throughput expectedThroughput;
    expectedThroughput.payloadSize = 73;
    expectedThroughput.chunkSize = 128;
    expectedThroughput.sequenceNumber = 13;
    expectedThroughput.lastDeliveryTimestamp = TimePointNs(DurationNs(0));
    expectedThroughput.currentDeliveryTimestamp = TimePointNs(DurationNs(durationNs));
    ThroughputData expectedThroughputData;
    expectedThroughputData.m_senderPortID = ExpectedUniqueID;
    expectedThroughputData.m_sampleSize = expectedThroughput.payloadSize;
    expectedThroughputData.m_chunkSize = expectedThroughput.chunkSize;
    expectedThroughputData.m_chunksPerMinute = 60. / (static_cast<double>(durationNs) / NsPerSecond);
    expectedThroughputData.m_lastSendIntervalInNanoseconds = durationNs;

    iox::capro::ServiceDescription service(expectedSenderPortData.m_caproServiceID,
                                           expectedSenderPortData.m_caproInstanceID,
                                           expectedSenderPortData.m_caproEventMethodID);

    iox::popo::SenderPortData senderPortData;
    senderPortData.m_throughputReadCache = expectedThroughput;
    senderPortData.m_processName = expectedSenderPortData.m_name;

    EXPECT_THAT(m_introspection->addSender(&senderPortData, senderPortName, service, ""), Eq(true));

    SenderPort_MOCK::globalDetails = std::make_shared<SenderPort_MOCK::mock_t>();
    SenderPort_MOCK::globalDetails->reserveSampleReturn = throughputTopic->chunkHeader();
    SenderPort_MOCK::globalDetails->getThroughputReturn = expectedThroughput;
    m_introspectionAccess.sendThroughputData();
    SenderPort_MOCK::globalDetails.reset();

    expectedThroughput.sequenceNumber++;
    expectedThroughput.lastDeliveryTimestamp = TimePointNs(DurationNs(durationNs));
    expectedThroughput.currentDeliveryTimestamp = TimePointNs(DurationNs(2 * durationNs));

    SenderPort_MOCK::globalDetails = std::make_shared<SenderPort_MOCK::mock_t>();
    SenderPort_MOCK::globalDetails->getUniqueIDReturn = ExpectedUniqueID;
    SenderPort_MOCK::globalDetails->reserveSampleReturn = portDataTopic->chunkHeader();
    m_introspectionAccess.sendPortData();
    SenderPort_MOCK::globalDetails.reset();

    // topic contains no sender or receiver ports but 0xFF bytes are overwritten

    ASSERT_THAT(portDataTopic->sample()->m_senderList.size(), Eq(1));
    auto sentSenderPortData = portDataTopic->sample()->m_senderList[0];
    EXPECT_THAT(sentSenderPortData.m_senderPortID, Eq(ExpectedUniqueID));
    EXPECT_THAT(sentSenderPortData.m_name, Eq(expectedSenderPortData.m_name));
    EXPECT_THAT(sentSenderPortData.m_caproInstanceID, Eq(expectedSenderPortData.m_caproInstanceID));
    EXPECT_THAT(sentSenderPortData.m_caproServiceID, Eq(expectedSenderPortData.m_caproServiceID));
    EXPECT_THAT(sentSenderPortData.m_caproEventMethodID, Eq(expectedSenderPortData.m_caproEventMethodID));

    SenderPort_MOCK::globalDetails = std::make_shared<SenderPort_MOCK::mock_t>();
    SenderPort_MOCK::globalDetails->getUniqueIDReturn = ExpectedUniqueID;
    SenderPort_MOCK::globalDetails->reserveSampleReturn = throughputTopic->chunkHeader();
    SenderPort_MOCK::globalDetails->getThroughputReturn = expectedThroughput;
    m_introspectionAccess.sendThroughputData();
    SenderPort_MOCK::globalDetails.reset();


    ASSERT_THAT(throughputTopic->sample()->m_throughputList.size(), Eq(1));
    auto sentThroughputData = throughputTopic->sample()->m_throughputList[0];
    EXPECT_THAT(sentThroughputData.m_senderPortID, Eq(ExpectedUniqueID));
    EXPECT_THAT(sentThroughputData.m_sampleSize, Eq(expectedThroughputData.m_sampleSize));
    EXPECT_THAT(sentThroughputData.m_chunkSize, Eq(expectedThroughputData.m_chunkSize));
    EXPECT_THAT(sentThroughputData.m_chunksPerMinute, DoubleEq(expectedThroughputData.m_chunksPerMinute));
    EXPECT_THAT(sentThroughputData.m_lastSendIntervalInNanoseconds,
                Eq(expectedThroughputData.m_lastSendIntervalInNanoseconds));
}


TEST_F(PortIntrospection_test, addAndRemoveSender)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using PortData = iox::roudi::SenderPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_senderPortImpl_mock->reserveSampleReturn = chunk->chunkHeader();

    SenderPort_MOCK port1;
    SenderPort_MOCK port2;
    auto mockPort1 = port1.details;
    auto mockPort2 = port2.details;

    std::string name1("name1");
    std::string name2("name2");

    // prepare expected outputs
    PortData expected1;
    expected1.m_name = iox::cxx::CString100(name1.c_str());
    expected1.m_caproInstanceID = "1";
    expected1.m_caproServiceID = "2";
    expected1.m_caproEventMethodID = "3";
    expected1.m_runnable = iox::cxx::CString100("4");

    PortData expected2;
    expected2.m_name = iox::cxx::CString100(name2.c_str());
    expected2.m_caproInstanceID = "abc";
    expected2.m_caproServiceID = "def";
    expected2.m_caproEventMethodID = "ghi";
    expected2.m_runnable = iox::cxx::CString100("jkl");

    // prepare inputs
    iox::capro::ServiceDescription service1(
        expected1.m_caproServiceID, expected1.m_caproInstanceID, expected1.m_caproEventMethodID);
    iox::capro::ServiceDescription service2(
        expected2.m_caproServiceID, expected2.m_caproInstanceID, expected2.m_caproEventMethodID);

    // test adding of ports

    // remark: duplicate sender port insertions are not possible

    iox::popo::SenderPortData portData1, portData2;
    EXPECT_THAT(m_introspection->addSender(&portData1, name1, service1, "4"), Eq(true));
    EXPECT_THAT(m_introspection->addSender(&portData1, name1, service1, "4"), Eq(false));
    EXPECT_THAT(m_introspection->addSender(&portData2, name2, service2, "jkl"), Eq(true));
    EXPECT_THAT(m_introspection->addSender(&portData2, name2, service2, "jkl"), Eq(false));

    mockPort1->getUniqueIDReturn = 1;
    mockPort2->getUniqueIDReturn = 2;

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

    EXPECT_THAT(m_senderPortImpl_mock->deliverChunk, Eq(4u));
}

TEST_F(PortIntrospection_test, addAndRemoveReceiver)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using PortData = iox::roudi::ReceiverPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_senderPortImpl_mock->reserveSampleReturn = chunk->chunkHeader();

    std::string name1("name1");
    std::string name2("name2");

    // prepare expected outputs
    PortData expected1;
    expected1.m_name = iox::cxx::CString100(name1.c_str());
    expected1.m_caproInstanceID = "1";
    expected1.m_caproServiceID = "2";
    expected1.m_caproEventMethodID = "3";
    expected1.m_senderIndex = -1;
    expected1.m_runnable = iox::cxx::CString100("4");

    PortData expected2;
    expected2.m_name = iox::cxx::CString100(name2.c_str());
    expected2.m_caproInstanceID = "4";
    expected2.m_caproServiceID = "5";
    expected2.m_caproEventMethodID = "6";
    expected2.m_senderIndex = -1;
    expected2.m_runnable = iox::cxx::CString100("7");

    // prepare inputs
    iox::capro::ServiceDescription service1(
        expected1.m_caproServiceID, expected1.m_caproInstanceID, expected1.m_caproEventMethodID);
    iox::capro::ServiceDescription service2(
        expected2.m_caproServiceID, expected2.m_caproInstanceID, expected2.m_caproEventMethodID);

    // test adding of ports

    // remark: duplicate receiver insertions are possible but will not be transmitted via send
    iox::popo::ReceiverPortData recData1;
    iox::popo::ReceiverPortData recData2;
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

    EXPECT_THAT(m_senderPortImpl_mock->deliverChunk, Eq(4u));
}


TEST_F(PortIntrospection_test, reportMessageToEstablishConnection)
{
    using Topic = iox::roudi::PortIntrospectionFieldTopic;
    using ReceiverPortData = iox::roudi::ReceiverPortData;
    using SenderPortData = iox::roudi::SenderPortData;

    auto chunk = std::unique_ptr<ChunkMock<Topic>>(new ChunkMock<Topic>);

    m_senderPortImpl_mock->reserveSampleReturn = chunk->chunkHeader();

    std::string nameReceiver("receiver");
    std::string nameSender("sender");

    // prepare expected outputs
    ReceiverPortData expectedReceiver;
    expectedReceiver.m_name = iox::cxx::CString100(nameReceiver.c_str());
    expectedReceiver.m_caproInstanceID = "1";
    expectedReceiver.m_caproServiceID = "2";
    expectedReceiver.m_caproEventMethodID = "3";
    expectedReceiver.m_senderIndex = -1;

    SenderPortData expectedSender;
    expectedSender.m_name = iox::cxx::CString100(nameSender.c_str());
    expectedSender.m_caproInstanceID = "1";
    expectedSender.m_caproServiceID = "2";
    expectedSender.m_caproEventMethodID = "3";

    // prepare inputs
    iox::capro::ServiceDescription service(
        expectedSender.m_caproServiceID, expectedSender.m_caproInstanceID, expectedSender.m_caproEventMethodID);

    // test adding of sender and receiver port of same service to establish a connection (requires same service id)
    m_senderPortImpl.details = m_senderPortImpl_mock;
    iox::popo::ReceiverPortData recData1;
    EXPECT_THAT(m_introspection->addReceiver(&recData1, nameReceiver, service, ""), Eq(true));
    iox::popo::SenderPortData senderPortData;
    EXPECT_THAT(m_introspection->addSender(&senderPortData, nameSender, service, ""), Eq(true));

    m_senderPortImpl_mock->getUniqueIDReturn = 42;

    m_introspectionAccess.sendPortData();

    auto sample = chunk->sample();

    {
        // expect unconnected sender and receiver (service is equal but m_senderIndex == -1 in receiver)

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
        // expect unconnected sender and receiver, since there was a SUB but no ACK
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
        // expect connected sender and receiver, since there was a SUB followed by ACK
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
        // expect connected sender and receiver, since there was a SUB followed by ACK
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
        // expect unconnected sender and receiver, since there was a SUB without ACK
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
        // expect unconnected sender and receiver, since there was a SUB followed by NACK
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
        // expect unconnected sender and receiver, since there was a SUB without ACK
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
        // expect connected sender and receiver, since there was a SUB followed by ACK
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
        // expect connected sender and receiver, since there was a SUB followed by ACK followed by another message (SUB)
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
        // expect disconnected sender and receiver, since there was a STOP_OFFER
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
    m_senderPortImpl_mock->reserveSampleReturn = chunkPortData->chunkHeader();

    using PortThroughput = iox::roudi::PortThroughputIntrospectionFieldTopic;
    ChunkMock<PortThroughput> chunkPortThroughput;
    m_portThroughput_mock->reserveSampleReturn = chunkPortThroughput.chunkHeader();

    using ReceiverPortChanging = iox::roudi::ReceiverPortChangingIntrospectionFieldTopic;
    ChunkMock<ReceiverPortChanging> chunkReceiverPortChanging;
    m_receiverPortData_mock->reserveSampleReturn = chunkReceiverPortChanging.chunkHeader();

    // we use the deliverChunk call to check how often the thread calls the send method
    m_introspection->setSendInterval(10);
    m_introspection->run();
    /// @todo this time can be reduced when the sleep mechanism of the port introspection thread is replace by a trigger
    /// queue
    std::this_thread::sleep_for(std::chrono::milliseconds(555)); // within this time, the thread should have run 6 times
    m_introspection->stop();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(555)); // if the thread doesn't stop, we have 12 runs after the sleep period
    EXPECT_THAT(m_senderPortImpl_mock->deliverChunk, Eq(1));
    EXPECT_TRUE(4 <= m_portThroughput_mock->deliverChunk && m_portThroughput_mock->deliverChunk <= 8);
    EXPECT_TRUE(4 <= m_receiverPortData_mock->deliverChunk && m_receiverPortData_mock->deliverChunk <= 8);
}
