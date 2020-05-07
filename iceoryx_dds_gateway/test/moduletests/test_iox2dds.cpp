#include "test.hpp"

#include "roudi_gtest.hpp"
#include <ioxdds/dds/data_writer.hpp>
#include <ioxdds/gateway/iox2dds.hpp>
#include <iceoryx_posh/internal/capro/capro_message.hpp>
#include <limits>

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

// ======================================== Helpers ======================================== //

using TestGateway = iox::gateway::dds::Iceoryx2DDSGateway<MockGenericGateway, MockSubscriber, MockDataWriter>;

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


static iox::gateway::dds::Channel<MockSubscriber, MockDataWriter> mockChannelFactory(iox::capro::ServiceDescription sd) noexcept
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

    return iox::gateway::dds::Channel<MockSubscriber, MockDataWriter>(sd, std::move(mockSubscriber), std::move(mockDataWriter));
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
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER,
                                        {"Introspection", iox::capro::AnyInstanceString, iox::capro::AnyEventString});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    gw->discover(msg);
    EXPECT_EQ(0, gw->getNumberOfChannels());
}

TEST_F(Iceoryx2DDSGatewayTest, IgnoresServiceMessages)
{
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(
        iox::capro::CaproMessageType::OFFER,
        {iox::capro::AnyServiceString, iox::capro::AnyInstanceString, iox::capro::AnyEventString});
    msg.m_subType = iox::capro::CaproMessageSubType::SERVICE;
    gw->discover(msg);
    EXPECT_EQ(0, gw->getNumberOfChannels());
}

TEST_F(Iceoryx2DDSGatewayTest, CreatesSubscriberAndDataWriterForOfferedServices)
{
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    gw->discover(msg);
    EXPECT_EQ(1, gw->getNumberOfChannels());
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelySubscribesToDataFromDetectedPublishers)
{
    // === Create Mock
    auto mockSubscriber = createMockSubscriber({"Radar", "Front-Right", "Reflections"});
    EXPECT_CALL(*mockSubscriber, subscribe).Times(1);
    stageMockSubscriber(std::move(mockSubscriber));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    gw->discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelyConnectsCreatedDataWritersToDDSNetwork)
{
    // === Create Mock
    auto mockWriter = createMockDataWriter({"Radar", "Front-Right", "Reflections"});
    EXPECT_CALL(*mockWriter, connect).Times(1);
    stageMockDataWriter(std::move(mockWriter));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    gw->discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, ForwardsFromPoshSubscriberToDDSDataWriter)
{
    // === Create Mock
    // Create a chunk in the heap
    char* buf = new char[sizeof(iox::mepoo::ChunkHeader) + sizeof(int)];
    auto chunk = new (buf) iox::mepoo::ChunkHeader();
    chunk->m_info.m_payloadSize = sizeof(int);
    auto payloadPtr = reinterpret_cast<int*>(buf + sizeof(iox::mepoo::ChunkHeader));
    *payloadPtr = 42; // Payload Value

    // Set up subscriber to provide this chunk when requested
    auto mockSubscriber = createMockSubscriber({"Radar", "Front-Right", "Reflections"});
    auto mockWriter = createMockDataWriter({"Radar", "Front-Right", "Reflections"});

    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(chunk), Return(true)));
    EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
    EXPECT_CALL(*mockWriter, write(SafeMatcherCast<uint8_t*>(Pointee(Eq(42))), chunk->m_info.m_payloadSize)).Times(1);

    stageMockSubscriber(std::move(mockSubscriber));
    stageMockDataWriter(std::move(mockWriter));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    gw->discover(msg);
    gw->forward();

    // === Clean-up
    delete[] buf;
}

TEST_F(Iceoryx2DDSGatewayTest, IgnoresMemoryChunksWithNoPayload)
{
    // === Create Mock
    // Create a chunk in the heap
    char* buf = new char[sizeof(iox::mepoo::ChunkHeader) + sizeof(int)];
    auto chunk = new (buf) iox::mepoo::ChunkHeader();
    chunk->m_info.m_payloadSize = 0;

    auto mockSubscriber = createMockSubscriber({"Radar", "Front-Right", "Reflections"});
    auto mockWriter = createMockDataWriter({"Radar", "Front-Right", "Reflections"});

    EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(chunk), Return(true)));
    EXPECT_CALL(*mockWriter, write).Times(Exactly(0));

    stageMockSubscriber(std::move(mockSubscriber));
    stageMockDataWriter(std::move(mockWriter));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    gw->discover(msg);
    gw->forward();
}

TEST_F(Iceoryx2DDSGatewayTest, ReleasesReferenceToMemoryChunkAfterSend)
{
    // === Create Mock
    // Create a chunk in the heap (required for write)
    char* buf = new char[sizeof(iox::mepoo::ChunkHeader) + sizeof(int)];
    auto chunk = new (buf) iox::mepoo::ChunkHeader();
    chunk->m_info.m_payloadSize = sizeof(int);
    auto payloadPtr = reinterpret_cast<int*>(buf + sizeof(iox::mepoo::ChunkHeader));
    *payloadPtr = 42; // Payload Value

    auto mockSubscriber = createMockSubscriber({"Radar", "Front-Right", "Reflections"});
    auto mockWriter = createMockDataWriter({"Radar", "Front-Right", "Reflections"});

    EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(chunk), Return(true)));
    {
        InSequence seq;
        EXPECT_CALL(*mockSubscriber, getChunk).Times(1);
        EXPECT_CALL(*mockWriter, write).Times(1);
        EXPECT_CALL(*mockSubscriber, releaseChunk).Times(1);
    }

    stageMockSubscriber(std::move(mockSubscriber));
    stageMockDataWriter(std::move(mockWriter));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    gw->discover(msg);
    gw->forward();
}

TEST_F(Iceoryx2DDSGatewayTest, DestroysCorrespondingSubscriberWhenAPublisherStopsOffering)
{
    // === Create Mock
    // Subscribers
    auto firstCreatedSubscriber = createMockSubscriber({"Radar", "Front-Right", "Reflections"});
    auto secondCreatedSubscriber = createMockSubscriber({"Radar", "Front-Right", "Reflections"});

    ON_CALL(*firstCreatedSubscriber, getServiceDescription).WillByDefault(
                Return(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"))
                );
    ON_CALL(*secondCreatedSubscriber, getServiceDescription).WillByDefault(
                Return(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"))
                );

    {
        InSequence seq;
        EXPECT_CALL(*firstCreatedSubscriber, subscribe).Times(1);
        EXPECT_CALL(*firstCreatedSubscriber, isDestroyed).Times(1);
        EXPECT_CALL(*secondCreatedSubscriber, subscribe).Times(1);
    }

    // Important ! Pass ownership, do not keep the reference here.
    stageMockSubscriber(std::move(firstCreatedSubscriber));
    stageMockSubscriber(std::move(secondCreatedSubscriber));

    // Messages
    auto offerMsg =
        iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    offerMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    auto stopOfferMsg =
        iox::capro::CaproMessage(iox::capro::CaproMessageType::STOP_OFFER, {"Radar", "Front-Right", "Reflections"});
    stopOfferMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    // === Test
    auto gw = std::make_shared<TestGateway>(mockChannelFactory);

    gw->discover(offerMsg);
    gw->discover(stopOfferMsg); // first subscriber must be deleted here
    gw->discover(offerMsg);
}
