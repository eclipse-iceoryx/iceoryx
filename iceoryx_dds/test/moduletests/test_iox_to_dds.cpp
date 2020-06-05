// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include <limits>

#include <iceoryx_posh/internal/capro/capro_message.hpp>
#include <iceoryx_utils/cxx/optional.hpp>
#include <iceoryx_dds/dds/data_writer.hpp>
#include <iceoryx_dds/gateway/channel.hpp>
#include <iceoryx_dds/gateway/iox_to_dds.hpp>

#include "mocks/chunk_mock.hpp"
#include "roudi_gtest.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::InSequence;

// ======================================== Mocks ======================================== //
class MockGenericGateway
{
  public:
    MockGenericGateway(const iox::capro::Interfaces i){};
    MOCK_METHOD1(getCaProMessage, bool(iox::capro::CaproMessage&));
};

class MockSubscriber
{
  public:
    MockSubscriber(const iox::capro::ServiceDescription& sd){};
    MOCK_METHOD0(isDestroyed, void()); // Allows testing for destruction
    virtual ~MockSubscriber()
    {
        isDestroyed();
    }
    MOCK_CONST_METHOD0(hasNewChunks, bool());
    MOCK_METHOD0(getServiceDescription, iox::capro::ServiceDescription());
    MOCK_METHOD1(getChunk, bool(const iox::mepoo::ChunkHeader**));
    MOCK_METHOD1(releaseChunk, bool(const void* const payload));
    MOCK_METHOD1(subscribe, void(const uint32_t));
};

class MockDataWriter : public iox::dds::DataWriter<MockDataWriter>
{
  public:
    MockDataWriter(const iox::capro::ServiceDescription& sd){};
    MOCK_METHOD0(connect, void(void));
    MOCK_METHOD2(write, bool(uint8_t*, uint64_t));
    MOCK_CONST_METHOD0(getServiceId, std::string(void));
    MOCK_CONST_METHOD0(getInstanceId, std::string(void));
    MOCK_CONST_METHOD0(getEventId, std::string(void));
};

class MockGenericDDSGateway
{
public:
  MockGenericDDSGateway(){};
  MockGenericDDSGateway(const iox::capro::Interfaces i){};
  MOCK_METHOD1(getCaProMessage, bool(iox::capro::CaproMessage&));
  MOCK_METHOD1(addChannel, iox::dds::Channel<MockSubscriber, MockDataWriter>(const iox::capro::ServiceDescription&));
  MOCK_METHOD1(discardChannel, void(const iox::capro::ServiceDescription&));
  MOCK_METHOD1(findChannel, iox::cxx::optional<iox::dds::Channel<MockSubscriber, MockDataWriter>>(const iox::capro::ServiceDescription&));
  MOCK_METHOD1(forEachChannel, void(const std::function<void(iox::dds::Channel<MockSubscriber, MockDataWriter>&)>));
};

// ======================================== Helpers ======================================== //

using TestGateway = iox::dds::Iceoryx2DDSGateway<iox::dds::Channel<MockSubscriber, MockDataWriter>, MockGenericDDSGateway>;

// Holds mocks created by tests to be returned by mock factories.
static std::vector<std::shared_ptr<MockSubscriber>> stagedMockSubscribers;
static std::vector<std::shared_ptr<MockDataWriter>> stagedMockWriters;

// Marks where in the array to look for a valid mock.
// Indexes lower than the marker are assumed to have been moved out and thus unspecified.
static size_t subscriberMarker = 0;
static size_t dataWriterMarker = 0;

// Create a new DataWriter mock
std::shared_ptr<MockDataWriter> createMockDataWriter(const iox::capro::ServiceDescription& sd)
{
    return std::shared_ptr<MockDataWriter>(new MockDataWriter(sd));
}

// Stage the given mock to be provided by the DataWriter mock factory
void stageMockDataWriter(std::shared_ptr<MockDataWriter>&& mock)
{
    // Pass ownership - do not hold a reference here.
    stagedMockWriters.emplace_back(std::move(mock));
};

// Create a new Subscriber mock
std::shared_ptr<MockSubscriber> createMockSubscriber(const iox::capro::ServiceDescription& sd)
{
    return std::shared_ptr<MockSubscriber>(new MockSubscriber(sd));
}

// Stage the given mock to be provided by the Subscriber mock factory
void stageMockSubscriber(std::shared_ptr<MockSubscriber>&& mock)
{
    stagedMockSubscribers.emplace_back(std::move(mock));
};

// ======================================== Mock Factories ======================================== //


static iox::dds::Channel<MockSubscriber, MockDataWriter> createTestChannel(iox::capro::ServiceDescription sd) noexcept
{
    // Get or create a mock subscriber
    std::shared_ptr<MockSubscriber> mockSubscriber;
    if (subscriberMarker < stagedMockSubscribers.size())
    {
        // Important - must pass ownership to receiver so object is deleted when the receiver is done with it.
        mockSubscriber = std::move(stagedMockSubscribers.at(subscriberMarker++));
    }
    else
    {
        mockSubscriber = createMockSubscriber(sd);
    }

    // Get or create a mock data writer
    std::shared_ptr<MockDataWriter> mockDataWriter;
    if (dataWriterMarker < stagedMockWriters.size())
    {
        // Important - must pass ownership to receiver so object is deleted when the receiver is done with it.
        mockDataWriter = std::move(stagedMockWriters.at(dataWriterMarker++));
    }
    else
    {
        mockDataWriter = createMockDataWriter(sd);
    }

    return iox::dds::Channel<MockSubscriber, MockDataWriter>(sd, std::move(mockSubscriber), std::move(mockDataWriter));
}

// ======================================== Fixture ======================================== //
class Iceoryx2DDSGatewayTest : public Test
{
  public:
    void SetUp(){};
    void TearDown()
    {
        stagedMockSubscribers.clear();
        subscriberMarker = 0;
        stagedMockWriters.clear();
        dataWriterMarker = 0;
    };
};

// ======================================== Tests ======================================== //
TEST_F(Iceoryx2DDSGatewayTest, IgnoresIntrospectionPorts)
{
    // === Setup
    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER,
                                        {"Introspection", iox::capro::AnyInstanceString, iox::capro::AnyEventString});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    EXPECT_CALL(gw, addChannel).Times(0);

    // === Test
    gw.discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, IgnoresServiceMessages)
{
    // === Setup
    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(
        iox::capro::CaproMessageType::OFFER,
        {iox::capro::AnyServiceString, iox::capro::AnyInstanceString, iox::capro::AnyEventString});
    msg.m_subType = iox::capro::CaproMessageSubType::SERVICE;

    EXPECT_CALL(gw, addChannel).Times(0);

    // === Test
    gw.discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, CreatesSubscriberAndDataWriterForOfferedServices)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(createTestChannel(testService)));

    EXPECT_CALL(gw, addChannel).Times(1);

    // === Test
    gw.discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelySubscribesToDataFromDetectedPublishers)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    auto mockSubscriber = createMockSubscriber(testService);
    EXPECT_CALL(*mockSubscriber, subscribe).Times(1);
    stageMockSubscriber(std::move(mockSubscriber));

    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    // Mock methods of the mock generic dds gateway base class
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(createTestChannel(testService)));

    // === Test
    gw.discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelyConnectsCreatedDataWritersToDDSNetwork)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    auto mockWriter = createMockDataWriter(testService);
    EXPECT_CALL(*mockWriter, connect).Times(1);
    stageMockDataWriter(std::move(mockWriter));

    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    // Mock methods of the mock generic dds gateway base class
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(createTestChannel(testService)));

    // === Test
    gw.discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, ForwardsFromPoshSubscriberToDDSDataWriter)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    // Prepare a mock mempool chunk
    ChunkMock<int> mockChunk{42};

    // Set up subscriber to provide this chunk when requested
    auto mockSubscriber = createMockSubscriber(testService);
    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(mockChunk.chunkHeader()), Return(true)));
    EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
    stageMockSubscriber(std::move(mockSubscriber));

    // We expect one attempt to write the mock chunk to the mock writer
    auto mockWriter = createMockDataWriter(testService);
    EXPECT_CALL(*mockWriter,
                write(SafeMatcherCast<uint8_t*>(Pointee(Eq(42))), mockChunk.chunkHeader()->m_info.m_payloadSize))
        .Times(1);
    stageMockDataWriter(std::move(mockWriter));

    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    auto testChannel = createTestChannel(testService); // Keep a copy for use in mutliple mocks
    TestGateway gw{};

    // Mock methods of the mock generic dds gateway base class
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(testChannel));

    // Capture the function containing the logic the implementation wants to run and run it on the test channel
    // to verify correctness.
    EXPECT_CALL(gw, forEachChannel).WillOnce([&testChannel](const std::function<void(iox::dds::Channel<MockSubscriber, MockDataWriter>&)> f){
        f(testChannel);
    });

    // === Test
    gw.discover(msg);   // Trigger setup of channel with mock subscriber and data writer.
    gw.forward();       // Trigger forwarding of mock chunk from mock subscriber to mock data writer.
}

TEST_F(Iceoryx2DDSGatewayTest, IgnoresMemoryChunksWithNoPayload)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    // Prepare a mock mempool chunk zero size
    ChunkMock<int> mockChunk{42};
    mockChunk.chunkHeader()->m_info.m_payloadSize = 0;

    // Set up subscriber to provide this chunk when requested
    auto mockSubscriber = createMockSubscriber(testService);
    EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(mockChunk.chunkHeader()), Return(true)));
    stageMockSubscriber(std::move(mockSubscriber));

    // We expect exactly zero attempts to write to the data writer since the mempool chunk is size zero
    auto mockWriter = createMockDataWriter(testService);
    EXPECT_CALL(*mockWriter, write).Times(Exactly(0));
    stageMockDataWriter(std::move(mockWriter));

    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    auto testChannel = createTestChannel(testService);

    // Mock methods of the mock generic dds gateway base class
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(testChannel));

    // Capture the function containing the logic the implementation wants to run and run it on the test channel
    // to verify correctness.
    EXPECT_CALL(gw, forEachChannel).WillOnce([&testChannel](const std::function<void(iox::dds::Channel<MockSubscriber, MockDataWriter>&)> f){
        f(testChannel);
    });

    // === Test
    gw.discover(msg);
    gw.forward();
}

TEST_F(Iceoryx2DDSGatewayTest, ReleasesReferenceToMemoryChunkAfterSend)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    ChunkMock<int> mockChunk{42};

    auto mockSubscriber = createMockSubscriber(testService);
    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(mockChunk.chunkHeader()), Return(true)));

    auto mockWriter = createMockDataWriter(testService);

    {
        InSequence seq;
        EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
        EXPECT_CALL(*mockSubscriber, getChunk).Times(1);
        EXPECT_CALL(*mockWriter, write).Times(1);
        EXPECT_CALL(*mockSubscriber, releaseChunk).Times(1);
    }

    stageMockSubscriber(std::move(mockSubscriber));
    stageMockDataWriter(std::move(mockWriter));

    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    auto testChannel = createTestChannel(testService);

    // Mock methods of the mock generic dds gateway base class
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(testChannel));

    // Capture the function containing the logic the implementation wants to run and run it on the test channel
    // to verify correctness.
    EXPECT_CALL(gw, forEachChannel).WillOnce([&testChannel](const std::function<void(iox::dds::Channel<MockSubscriber, MockDataWriter>&)> f){
        f(testChannel);
    });

    // === Test
    gw.discover(msg);
    gw.forward();

}

TEST_F(Iceoryx2DDSGatewayTest, DestroysCorrespondingSubscriberWhenAPublisherStopsOffering)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    // Subscribers
    auto firstCreatedSubscriber = createMockSubscriber(testService);
    auto secondCreatedSubscriber = createMockSubscriber(testService);
    ON_CALL(*firstCreatedSubscriber, getServiceDescription)
        .WillByDefault(Return(testService));
    ON_CALL(*secondCreatedSubscriber, getServiceDescription)
        .WillByDefault(Return(testService));
    {
        InSequence seq;
        EXPECT_CALL(*firstCreatedSubscriber, subscribe).Times(1);
        EXPECT_CALL(*secondCreatedSubscriber, subscribe).Times(1);
    }

    stageMockSubscriber(std::move(firstCreatedSubscriber));
    stageMockSubscriber(std::move(secondCreatedSubscriber));

    // Messages
    auto offerMsg =
        iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    offerMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    auto stopOfferMsg =
        iox::capro::CaproMessage(iox::capro::CaproMessageType::STOP_OFFER, testService);
    stopOfferMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    // Get the test channels here as we need to use them in expectations
    auto testChannelOne = createTestChannel(testService);
    auto testChannelTwo = createTestChannel(testService);

    TestGateway gw{};
    // EXPECT_CALL is used here because it allows specifying different return values for consecutive calls
    EXPECT_CALL(gw, findChannel)
            .WillOnce(Return(iox::cxx::nullopt_t()))
            .WillOnce(Return(iox::cxx::make_optional<iox::dds::Channel<MockSubscriber, MockDataWriter>>(testChannelOne)))
            .WillOnce(Return(iox::cxx::nullopt_t()));
    EXPECT_CALL(gw, addChannel)
            .WillOnce(Return(testChannelOne))
            .WillOnce(Return(testChannelTwo));
    EXPECT_CALL(gw, discardChannel)
            .Times(1);

    // === Test
    gw.discover(offerMsg);
    gw.discover(stopOfferMsg); // first subscriber must be deleted here
    gw.discover(offerMsg);
}
