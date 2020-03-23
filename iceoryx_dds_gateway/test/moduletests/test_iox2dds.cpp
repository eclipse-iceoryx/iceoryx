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
    MockDataWriter(std::string service, std::string instance, std::string event){};
    MOCK_METHOD0(connect, void(void));
    MOCK_METHOD2(write, bool(uint8_t*, uint64_t));
    MOCK_CONST_METHOD0(getServiceId, std::string(void));
    MOCK_CONST_METHOD0(getInstanceId, std::string(void));
    MOCK_CONST_METHOD0(getEventId, std::string(void));
};

// ======================================== Helpers ======================================== //

using TestGateway = iox::gateway::dds::Iceoryx2DDSGateway<MockGenericGateway, MockSubscriber, MockDataWriter>;

// Holds mocks created by tests to be returned by mock factories.
static std::vector<std::unique_ptr<MockSubscriber>> stagedMockSubscribers;
static std::vector<std::unique_ptr<MockDataWriter>> stagedMockWriters;

// Marks where in the array to look for a valid mock.
// Indexes lower than the marker are assumed to have been moved out and thus unspecified.
static size_t subscriberMarker = 0;
static size_t dataWriterMarker = 0;

// Create a new DataWriter mock
std::unique_ptr<MockDataWriter> createMockDataWriter(std::string service, std::string instance, std::string event)
{
    return std::unique_ptr<MockDataWriter>(new MockDataWriter(
        std::forward<std::string>(service), std::forward<std::string>(instance), std::forward<std::string>(event)));
}

// Stage the given mock to be provided by the DataWriter mock factory
void stageMockDataWriter(std::unique_ptr<MockDataWriter>&& mock)
{
    stagedMockWriters.emplace_back(std::move(mock));
};

// Create a new Subscriber mock
std::unique_ptr<MockSubscriber> createMockSubscriber(const iox::capro::ServiceDescription& sd)
{
    return std::unique_ptr<MockSubscriber>(new MockSubscriber(std::forward<const iox::capro::ServiceDescription&>(sd)));
}

// Stage the given mock to be provided by the Subscriber mock factory
void stageMockSubscriber(std::unique_ptr<MockSubscriber>&& mock)
{
    stagedMockSubscribers.emplace_back(std::move(mock));
};

// ======================================== Mock Factories ======================================== //
static std::unique_ptr<MockDataWriter>
mockDataWriterFactory(std::string service, std::string instance, std::string event) noexcept
{
    if (dataWriterMarker < stagedMockWriters.size())
    {
        return std::move(stagedMockWriters.at(dataWriterMarker++));
    }
    else
    {
        // Create a mock if there are none staged.
        // This occurs when a mock is required by the test class, but it has no expectations specified in the test.
        return createMockDataWriter(service, instance, event);
    }
};

static std::unique_ptr<MockSubscriber> mockSubscriberFactory(iox::capro::ServiceDescription sd) noexcept
{
    if (subscriberMarker < stagedMockSubscribers.size())
    {
        return std::move(stagedMockSubscribers.at(subscriberMarker++));
    }
    else
    {
        // Create a mock if there are none staged.
        // This occurs when a mock is required by the test class, but it has no expectations specified in the test.
        return createMockSubscriber(sd);
    }
};

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
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER,
                                        {"Introspection", iox::capro::AnyInstanceString, iox::capro::AnyEventString});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    gw->discover(msg);
    EXPECT_EQ(0, gw->getNumberOfSubscribers());
    EXPECT_EQ(0, gw->getNumberOfDataWriters());
}

TEST_F(Iceoryx2DDSGatewayTest, IgnoresServiceMessages)
{
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
    auto msg = iox::capro::CaproMessage(
        iox::capro::CaproMessageType::OFFER,
        {iox::capro::AnyServiceString, iox::capro::AnyInstanceString, iox::capro::AnyEventString});
    msg.m_subType = iox::capro::CaproMessageSubType::SERVICE;
    gw->discover(msg);
    EXPECT_EQ(0, gw->getNumberOfSubscribers());
    EXPECT_EQ(0, gw->getNumberOfDataWriters());
}

TEST_F(Iceoryx2DDSGatewayTest, CreatesSubscriberAndDataWriterForOfferedServices)
{
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    gw->discover(msg);
    EXPECT_EQ(1, gw->getNumberOfSubscribers());
    EXPECT_EQ(1, gw->getNumberOfDataWriters());
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelySubscribesToDataFromDetectedPublishers)
{
    // === Create Mock
    auto mockSubscriber = createMockSubscriber(iox::capro::ServiceDescription("", "", ""));
    EXPECT_CALL(*mockSubscriber, subscribe).Times(1);
    stageMockSubscriber(std::move(mockSubscriber));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    msg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    gw->discover(msg);
}

TEST_F(Iceoryx2DDSGatewayTest, ImmediatelyConnectsCreatedDataWritersToDDSNetwork)
{
    // === Create Mock
    auto mockWriter = createMockDataWriter("", "", "");
    EXPECT_CALL(*mockWriter, connect).Times(1);
    stageMockDataWriter(std::move(mockWriter));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
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
    auto mockSubscriber = createMockSubscriber(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"));
    auto mockWriter = createMockDataWriter("Radar", "Front-Right", "Reflections");

    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(chunk), Return(true)));
    EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
    EXPECT_CALL(*mockWriter, write(SafeMatcherCast<uint8_t*>(Pointee(Eq(42))), chunk->m_info.m_payloadSize)).Times(1);

    stageMockSubscriber(std::move(mockSubscriber));
    stageMockDataWriter(std::move(mockWriter));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
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

    auto mockSubscriber = createMockSubscriber(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"));
    auto mockWriter = createMockDataWriter("Radar", "Front-Right", "Reflections");

    EXPECT_CALL(*mockSubscriber, hasNewChunks).Times(1);
    ON_CALL(*mockSubscriber, hasNewChunks).WillByDefault(Return(true));
    ON_CALL(*mockSubscriber, getChunk).WillByDefault(DoAll(SetArgPointee<0>(chunk), Return(true)));
    EXPECT_CALL(*mockWriter, write).Times(Exactly(0));

    stageMockSubscriber(std::move(mockSubscriber));
    stageMockDataWriter(std::move(mockWriter));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
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

    auto mockSubscriber = createMockSubscriber(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"));
    auto mockWriter = createMockDataWriter("Radar", "Front-Right", "Reflections");

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
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
    auto msg = iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    gw->discover(msg);
    gw->forward();
}

TEST_F(Iceoryx2DDSGatewayTest, DestroysCorrespondingSubscriberWhenAPublisherStopsOffering)
{
    // === Create Mock
    auto mockSubscriber1 = createMockSubscriber(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"));
    auto mockSubscriber2 = createMockSubscriber(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"));

    ON_CALL(*mockSubscriber1, getServiceDescription).WillByDefault(
                Return(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"))
                );
    ON_CALL(*mockSubscriber2, getServiceDescription).WillByDefault(
                Return(iox::capro::ServiceDescription("Radar", "Front-Right", "Reflections"))
                );

    {
        InSequence seq;
        EXPECT_CALL(*mockSubscriber1, subscribe).Times(1);
        EXPECT_CALL(*mockSubscriber1, isDestroyed).Times(1);
        EXPECT_CALL(*mockSubscriber2, subscribe).Times(1);
    }

    stageMockSubscriber(std::move(mockSubscriber1));
    stageMockSubscriber(std::move(mockSubscriber2));

    // === Test
    auto gw = std::make_shared<TestGateway>(mockSubscriberFactory, mockDataWriterFactory);
    auto offerMsg =
        iox::capro::CaproMessage(iox::capro::CaproMessageType::OFFER, {"Radar", "Front-Right", "Reflections"});
    auto stopOfferMsg =
        iox::capro::CaproMessage(iox::capro::CaproMessageType::STOP_OFFER, {"Radar", "Front-Right", "Reflections"});

    offerMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;
    stopOfferMsg.m_subType = iox::capro::CaproMessageSubType::EVENT;

    gw->discover(offerMsg);
    gw->discover(stopOfferMsg); // initial subscriber must be deleted here
    gw->discover(offerMsg);
}
