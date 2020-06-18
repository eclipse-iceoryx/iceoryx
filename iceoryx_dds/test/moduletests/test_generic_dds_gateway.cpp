#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_dds/gateway/gateway_config.hpp"

#include "test.hpp"

#include "helpers/stubbed_dds_gateway_generic.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Helpers ======================================== //
// We do not need real channel terminals to test the base class.
struct StubbedIceoryxTerminal
{
    StubbedIceoryxTerminal(iox::capro::ServiceDescription sd){};
};

struct StubbedDDSTerminal
{
    StubbedDDSTerminal(iox::dds::IdString sid, iox::dds::IdString iid, iox::dds::IdString eid){};
};

using TestChannel = iox::dds::Channel<StubbedIceoryxTerminal, StubbedDDSTerminal>;
using TestDDSGatewayGeneric = iox::dds::StubbedDDSGatewayGeneric<TestChannel>;

// ======================================== Fixture ======================================== //
class DDSGatewayGenericTest : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

// ======================================== Tests ======================================== //
TEST_F(DDSGatewayGenericTest, AddedChannelsAreStored)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testService);

    EXPECT_EQ(1, gw.getNumberOfChannels());
}

TEST_F(DDSGatewayGenericTest, DoesNotAddWhenChannelAlreadyExists)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testService);
    gw.addChannel(testService);

    EXPECT_EQ(1, gw.getNumberOfChannels());
}

TEST_F(DDSGatewayGenericTest, IgnoresWildcardServices)
{
    // ===== Setup
    auto completeWildcardService = iox::capro::ServiceDescription(iox::capro::AnyServiceString, iox::capro::AnyInstanceString, iox::capro::AnyEventString);
    auto wildcardServiceService = iox::capro::ServiceDescription(iox::capro::AnyServiceString, "instance", "event");
    auto wildcardInstanceService = iox::capro::ServiceDescription("service", iox::capro::AnyInstanceString, "event");
    auto wildcardEventService = iox::capro::ServiceDescription("service", "instance", iox::capro::AnyEventString);

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(completeWildcardService);
    gw.addChannel(wildcardServiceService);
    gw.addChannel(wildcardInstanceService);
    gw.addChannel(wildcardEventService);


    EXPECT_EQ(0, gw.getNumberOfChannels());
}

TEST_F(DDSGatewayGenericTest, ProperlyManagesMultipleChannels)
{
    // ===== Setup
    auto serviceOne = iox::capro::ServiceDescription("serviceOne", "instanceOne", "eventOne");
    auto serviceTwo = iox::capro::ServiceDescription("serviceTwo", "instanceTwo", "eventTwo");
    auto serviceThree = iox::capro::ServiceDescription("serviceThree", "instanceThree", "eventThree");
    auto serviceFour = iox::capro::ServiceDescription("serviceFour", "instanceFour", "eventFour");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(serviceOne);
    gw.addChannel(serviceTwo);
    gw.addChannel(serviceThree);
    gw.addChannel(serviceFour);


    EXPECT_EQ(4, gw.getNumberOfChannels());
    EXPECT_EQ(true, gw.findChannel(serviceOne).has_value());
    EXPECT_EQ(true, gw.findChannel(serviceTwo).has_value());
    EXPECT_EQ(true, gw.findChannel(serviceThree).has_value());
    EXPECT_EQ(true, gw.findChannel(serviceFour).has_value());

}

TEST_F(DDSGatewayGenericTest, HandlesMaxmimumChannelCapacity)
{
    // ===== Setup
    TestDDSGatewayGeneric gw{};

    // ===== Test
    for(auto i=0u; i<iox::dds::MAX_CHANNEL_NUMBER; i++)
    {
        auto result = gw.addChannel(iox::capro::ServiceDescription(
                              iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i)),
                              iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i)),
                              iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i))));
        EXPECT_EQ(false, result.has_error());
    }

    EXPECT_EQ(iox::dds::MAX_CHANNEL_NUMBER, gw.getNumberOfChannels());

}

TEST_F(DDSGatewayGenericTest, ThrowsErrorWhenExceedingMaximumChannelCapaicity)
{

    // ===== Setup
    TestDDSGatewayGeneric gw{};

    // ===== Test
    for(auto i=0u; i<iox::dds::MAX_CHANNEL_NUMBER; i++)
    {
        auto result = gw.addChannel(iox::capro::ServiceDescription(
                              iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i)),
                              iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i)),
                              iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i))));
        EXPECT_EQ(false, result.has_error());
    }

    auto result = gw.addChannel({"oneTooMany", "oneTooMany", "oneTooMany"});
    EXPECT_EQ(true, result.has_error());

}

TEST_F(DDSGatewayGenericTest, DiscardedChannelsAreNotStored)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testService);
    EXPECT_EQ(1, gw.getNumberOfChannels());
    gw.discardChannel(testService);
    EXPECT_EQ(0, gw.getNumberOfChannels());
}

TEST_F(DDSGatewayGenericTest, FindChannelReturnsCopyOfFoundChannel)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testService);
    auto foundChannel = gw.findChannel(testService);
    EXPECT_EQ(true, foundChannel.has_value());
    if (foundChannel.has_value())
    {
        EXPECT_EQ(testService, foundChannel.value().getService());
    }
}

TEST_F(DDSGatewayGenericTest, FindChannelGivesEmptyOptionalIfNoneFound)
{
    // ===== Setup
    auto storedChannelService = iox::capro::ServiceDescription("service", "instance", "event");
    auto notStoredChannelService = iox::capro::ServiceDescription("otherService", "otherInstance", "otherEvent");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(storedChannelService);
    auto foundChannel = gw.findChannel(notStoredChannelService);
    EXPECT_EQ(false, foundChannel.has_value());
}

TEST_F(DDSGatewayGenericTest, ForEachChannelExecutesGivenFunctionForAllStoredChannels)
{
    // ===== Setup
    auto testServiceA = iox::capro::ServiceDescription("serviceA", "instanceA", "eventA");
    auto testServiceB = iox::capro::ServiceDescription("serviceB", "instanceB", "eventB");
    auto testServiceC = iox::capro::ServiceDescription("serviceC", "instanceC", "eventC");

    auto count = 0u;
    auto f = [&count](TestChannel& channel) { count++; };

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testServiceA);
    gw.addChannel(testServiceB);
    gw.addChannel(testServiceC);
    gw.forEachChannel(f);

    EXPECT_EQ(3, count);
}
