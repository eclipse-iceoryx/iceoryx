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

#include "iceoryx_dds/dds/data_writer.hpp"
#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_dds/gateway/gateway_config.hpp"
#include "iceoryx_dds/gateway/iox_to_dds.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include "mocks/chunk_mock.hpp"
#include "mocks/google_mocks.hpp"
#include "roudi_gtest.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SetArgPointee;

// ======================================== Helpers ======================================== //

using TestGateway =
    iox::dds::Iceoryx2DDSGateway<iox::dds::Channel<MockSubscriber, MockDataWriter>, MockGenericDDSGateway>;

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


static iox::cxx::expected<iox::dds::Channel<MockSubscriber, MockDataWriter>, iox::dds::GatewayError>
createTestChannel(iox::capro::ServiceDescription sd) noexcept
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

    return iox::cxx::success<iox::dds::Channel<MockSubscriber, MockDataWriter>>(
        iox::dds::Channel<MockSubscriber, MockDataWriter>(sd, std::move(mockSubscriber), std::move(mockDataWriter)));
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
TEST_F(Iceoryx2DDSGatewayTest, ChannelsAreCreatedForConfiguredServices)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    iox::dds::GatewayConfig config{};
    config.m_configuredServices.push_back(testService);

    TestGateway gw{};
    EXPECT_CALL(gw, findChannel).WillOnce(Return(iox::cxx::nullopt_t()));
    EXPECT_CALL(gw, addChannel).WillOnce(Return(createTestChannel(testService)));

    // === Test
    gw.loadConfiguration(config);
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelySubscribesToDataFromConfiguredServices)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    iox::dds::GatewayConfig config{};
    config.m_configuredServices.push_back(testService);

    auto mockSubscriber = createMockSubscriber(testService);
    EXPECT_CALL(*mockSubscriber, subscribe).Times(1);
    stageMockSubscriber(std::move(mockSubscriber));

    TestGateway gw{};
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(createTestChannel(testService)));

    // === Test
    gw.loadConfiguration(config);
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelyConnectsCreatedDataWritersForConfiguredServices)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    iox::dds::GatewayConfig config{};
    config.m_configuredServices.push_back(testService);

    auto mockWriter = createMockDataWriter(testService);
    EXPECT_CALL(*mockWriter, connect).Times(1);
    stageMockDataWriter(std::move(mockWriter));

    TestGateway gw{};
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel).WillByDefault(Return(createTestChannel(testService)));

    // === Test
    gw.loadConfiguration(config);
}

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

TEST_F(Iceoryx2DDSGatewayTest, ChannelsAreCreatedForDiscoveredServices)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    TestGateway gw{};
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    EXPECT_CALL(gw, findChannel).WillOnce(Return(iox::cxx::nullopt_t()));
    EXPECT_CALL(gw, addChannel).WillOnce(Return(createTestChannel(testService)));

    // === Test
    gw.discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelySubscribesToDataFromDiscoveredServices)
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

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelyConnectsCreatedDataWritersForDiscoveredServices)
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

TEST_F(Iceoryx2DDSGatewayTest, ForwardsChunkFromSubscriberToDataWriter)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    // Prepare a mock mempool chunk
    ChunkMock<int> mockChunk{42};

    // Set up subscriber to provide the chunk
    auto mockSubscriber = createMockSubscriber(testService);
    EXPECT_CALL(*mockSubscriber, hasNewChunks).WillOnce(Return(true));
    EXPECT_CALL(*mockSubscriber, getChunk).WillOnce(DoAll(SetArgPointee<0>(mockChunk.chunkHeader()), Return(true)));
    stageMockSubscriber(std::move(mockSubscriber));

    // Verify expected write to the data writer
    auto mockWriter = createMockDataWriter(testService);
    EXPECT_CALL(*mockWriter,
                write(SafeMatcherCast<uint8_t*>(Pointee(Eq(42))), mockChunk.chunkHeader()->m_info.m_payloadSize))
        .Times(1);
    stageMockDataWriter(std::move(mockWriter));

    auto testChannel = createTestChannel(testService);
    TestGateway gw{};

    // === Test
    gw.forward(testChannel.get_value());
}

TEST_F(Iceoryx2DDSGatewayTest, IgnoresMemoryChunksWithNoPayload)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    // Prepare a mock mempool chunk
    ChunkMock<int> mockChunk{0};
    mockChunk.chunkHeader()->m_info.m_payloadSize = 0;

    // Set up subscriber to provide the chunk
    auto mockSubscriber = createMockSubscriber(testService);
    EXPECT_CALL(*mockSubscriber, hasNewChunks).WillOnce(Return(true));
    EXPECT_CALL(*mockSubscriber, getChunk).WillOnce(DoAll(SetArgPointee<0>(mockChunk.chunkHeader()), Return(true)));
    stageMockSubscriber(std::move(mockSubscriber));

    // Verify expected write to the data writer
    auto mockWriter = createMockDataWriter(testService);
    EXPECT_CALL(*mockWriter, write).Times(Exactly(0));
    stageMockDataWriter(std::move(mockWriter));

    auto testChannel = createTestChannel(testService);
    TestGateway gw{};

    // === Test
    gw.forward(testChannel.get_value());
}

TEST_F(Iceoryx2DDSGatewayTest, ReleasesReferenceToMemoryChunkAfterSend)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    // Prepare a mock mempool chunk
    ChunkMock<int> mockChunk{42};

    // Set up expect sequence of interactions with subscriber and data writer
    auto mockSubscriber = createMockSubscriber(testService);
    auto mockWriter = createMockDataWriter(testService);
    {
        InSequence seq;
        EXPECT_CALL(*mockSubscriber, hasNewChunks).WillOnce(Return(true));
        EXPECT_CALL(*mockSubscriber, getChunk).WillOnce(DoAll(SetArgPointee<0>(mockChunk.chunkHeader()), Return(true)));
        EXPECT_CALL(*mockWriter, write).Times(1);
        EXPECT_CALL(*mockSubscriber, releaseChunk).Times(1);
    }

    stageMockSubscriber(std::move(mockSubscriber));
    stageMockDataWriter(std::move(mockWriter));

    auto testChannel = createTestChannel(testService);
    TestGateway gw{};

    // === Test
    gw.forward(testChannel.get_value());
}

TEST_F(Iceoryx2DDSGatewayTest, DestroysCorrespondingSubscriberWhenAPublisherStopsOffering)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});

    // Subscribers
    auto firstCreatedSubscriber = createMockSubscriber(testService);
    auto secondCreatedSubscriber = createMockSubscriber(testService);
    {
        InSequence seq;
        EXPECT_CALL(*firstCreatedSubscriber, subscribe).Times(1);
        EXPECT_CALL(*secondCreatedSubscriber, subscribe).Times(1);
    }

    stageMockSubscriber(std::move(firstCreatedSubscriber));
    stageMockSubscriber(std::move(secondCreatedSubscriber));

    // Messages
    auto offerMsg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, testService);
    offerMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    auto stopOfferMsg = iox::capro::CaproMessage(iox::capro::CaproMessageType::STOP_OFFER, testService);
    stopOfferMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    // Get the test channels here as we need to use them in expectations
    auto testChannelOne = createTestChannel(testService);
    auto testChannelTwo = createTestChannel(testService);

    TestGateway gw{};
    EXPECT_CALL(gw, findChannel)
        .WillOnce(Return(iox::cxx::nullopt_t()))
        .WillOnce(Return(
            iox::cxx::make_optional<iox::dds::Channel<MockSubscriber, MockDataWriter>>(testChannelOne.get_value())))
        .WillOnce(Return(iox::cxx::nullopt_t()));
    EXPECT_CALL(gw, addChannel).WillOnce(Return(testChannelOne)).WillOnce(Return(testChannelTwo));
    EXPECT_CALL(gw, discardChannel)
            .WillOnce(Return(iox::cxx::success<>()));

    // === Test
    gw.discover(offerMsg);
    gw.discover(stopOfferMsg); // first subscriber must be deleted here
    gw.discover(offerMsg);
}
