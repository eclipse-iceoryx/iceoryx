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

#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_dds/gateway/gateway_config.hpp"

#include "test.hpp"

#include "stubs/stubbed_dds_gateway_generic.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Helpers ======================================== //
// We do not need real channel terminals to test the base class.
struct StubbedIceoryxTerminal
{
    StubbedIceoryxTerminal(iox::capro::ServiceDescription){};
};

struct StubbedDDSTerminal
{
    StubbedDDSTerminal(iox::dds::IdString, iox::dds::IdString, iox::dds::IdString){};
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

TEST_F(DDSGatewayGenericTest, DoesNotAddDuplicateChannels)
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
    auto completeWildcardService = iox::capro::ServiceDescription(
        iox::capro::AnyServiceString, iox::capro::AnyInstanceString, iox::capro::AnyEventString);
    auto wildcardServiceService = iox::capro::ServiceDescription(iox::capro::AnyServiceString, "instance", "event");
    auto wildcardInstanceService = iox::capro::ServiceDescription("service", iox::capro::AnyInstanceString, "event");
    auto wildcardEventService = iox::capro::ServiceDescription("service", "instance", iox::capro::AnyEventString);

    TestDDSGatewayGeneric gw{};

    // ===== Test
    auto resultOne = gw.addChannel(completeWildcardService);
    auto resultTwo = gw.addChannel(wildcardServiceService);
    auto resultThree = gw.addChannel(wildcardInstanceService);
    auto resultFour = gw.addChannel(wildcardEventService);

    EXPECT_EQ(iox::dds::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultOne.get_error());
    EXPECT_EQ(iox::dds::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultTwo.get_error());
    EXPECT_EQ(iox::dds::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultThree.get_error());
    EXPECT_EQ(iox::dds::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultFour.get_error());

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
    for (auto i = 0u; i < iox::dds::MAX_CHANNEL_NUMBER; i++)
    {
        auto result = gw.addChannel(
            iox::capro::ServiceDescription(iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i)),
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
    for (auto i = 0u; i < iox::dds::MAX_CHANNEL_NUMBER; i++)
    {
        auto result = gw.addChannel(
            iox::capro::ServiceDescription(iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i)),
                                           iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i)),
                                           iox::capro::IdString(iox::cxx::TruncateToCapacity, std::to_string(i))));
        EXPECT_EQ(false, result.has_error());
    }

    auto result = gw.addChannel({"oneTooMany", "oneTooMany", "oneTooMany"});
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::dds::GatewayError::UNSUCCESSFUL_CHANNEL_CREATION, result.get_error());
}

TEST_F(DDSGatewayGenericTest, ThrowsErrorWhenAttemptingToRemoveNonexistantChannel)
{
    // ===== Setup
    auto testServiceA = iox::capro::ServiceDescription("serviceA", "instanceA", "eventA");
    auto testServiceB = iox::capro::ServiceDescription("serviceB", "instanceB", "eventB");
    auto testServiceC = iox::capro::ServiceDescription("serviceC", "instanceC", "eventC");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testServiceA);
    gw.addChannel(testServiceB);
    EXPECT_EQ(2, gw.getNumberOfChannels());

    auto result = gw.discardChannel(testServiceC);
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(2, gw.getNumberOfChannels());
}

TEST_F(DDSGatewayGenericTest, DiscardedChannelsAreNotStored)
{
    // ===== Setup
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testService);
    EXPECT_EQ(1, gw.getNumberOfChannels());
    auto result = gw.discardChannel(testService);
    EXPECT_EQ(false, result.has_error());
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
        EXPECT_EQ(testService, foundChannel.value().getServiceDescription());
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
    auto f = [&count](TestChannel&) { count++; };

    TestDDSGatewayGeneric gw{};

    // ===== Test
    gw.addChannel(testServiceA);
    gw.addChannel(testServiceB);
    gw.addChannel(testServiceC);
    gw.forEachChannel(f);

    EXPECT_EQ(3, count);
}
