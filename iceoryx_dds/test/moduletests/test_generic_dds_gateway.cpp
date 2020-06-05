#include <iceoryx_dds/dds/dds_types.hpp>
#include <iceoryx_dds/gateway/channel.hpp>

#include "test.hpp"

#include "helpers/stubbed_dds_gateway_generic.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Helpers ======================================== //
// We do not need real channel terminals to test the base class.
struct StubbedIceoryxTerminal{
    StubbedIceoryxTerminal(iox::capro::ServiceDescription sd){};
};

struct StubbedDDSTerminal{
    StubbedDDSTerminal(iox::dds::IdString sid, iox::dds::IdString iid, iox::dds::IdString eid){};
};

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
    auto testService = iox::capro::ServiceDescription("", "", "");

    iox::dds::StubbedDDSGatewayGeneric<iox::dds::Channel<StubbedIceoryxTerminal, StubbedDDSTerminal>> gw{};

    // ===== Test
    gw.addChannel(testService);

    EXPECT_EQ(1, gw.getNumberOfChannels());

}

TEST_F(DDSGatewayGenericTest, DiscardedChannelsAreNotStored)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("", "", "");

    iox::dds::StubbedDDSGatewayGeneric<iox::dds::Channel<StubbedIceoryxTerminal, StubbedDDSTerminal>> gw{};

    // ===== Test
    gw.addChannel(testService);
    gw.discardChannel(testService);

    EXPECT_EQ(0, gw.getNumberOfChannels());

}

TEST_F(DDSGatewayGenericTest, FindChannelReturnsCopyOfFoundChannel)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    iox::dds::StubbedDDSGatewayGeneric<iox::dds::Channel<StubbedIceoryxTerminal, StubbedDDSTerminal>> gw{};

    // ===== Test
    gw.addChannel(testService);
    auto foundChannel = gw.findChannel(testService);
    EXPECT_EQ(1, gw.getNumberOfChannels());
    EXPECT_EQ(true, foundChannel.has_value());
    if(foundChannel.has_value())
    {
        EXPECT_EQ(testService, foundChannel.value().getService());
    }

}

TEST_F(DDSGatewayGenericTest, FindChannelGivesEmptyOptionalIfNoneFound)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("", "", "");

    iox::dds::StubbedDDSGatewayGeneric<iox::dds::Channel<StubbedIceoryxTerminal, StubbedDDSTerminal>> gw{};

    // ===== Test
    auto foundChannel = gw.findChannel(testService);
    EXPECT_EQ(false, foundChannel.has_value());
}

