#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_posh/capro/service_description.hpp"

#include "test.hpp"

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

// ======================================== Fixture ======================================== //
class ChannelTest : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

// ======================================== Tests ======================================== //
TEST_F(ChannelTest, ReturnsEmptyOptionalIfObjectPoolExhausted)
{
    auto channel = iox::dds::Channel<StubbedIceoryxTerminal, StubbedDDSTerminal>::create({"", "", ""});
}
