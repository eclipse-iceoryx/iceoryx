// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/gateway/channel.hpp"
#include "iceoryx_posh/gateway/gateway_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/detail/convert.hpp"
#include "iox/duration.hpp"
#include "iox/std_string_support.hpp"

#include "test.hpp"

#include "stubs/stub_gateway_generic.hpp"

namespace
{
using namespace ::testing;
using namespace iox::units::duration_literals;
using ::testing::_;

// ======================================== Helpers ======================================== //

using iox::into;
using iox::lossy;
using iox::capro::IdString_t;

// We do not need real channel terminals to test the base class.
struct StubbedIceoryxTerminal
{
    struct Options
    {
    };
    StubbedIceoryxTerminal(const iox::capro::ServiceDescription&, const Options&){};
};

struct StubbedExternalTerminal
{
    StubbedExternalTerminal(IdString_t, IdString_t, IdString_t){};
};

using TestChannel = iox::gw::Channel<StubbedIceoryxTerminal, StubbedExternalTerminal>;
using TestGatewayGeneric = iox::gw::StubbedGatewayGeneric<TestChannel>;

// ======================================== Fixture ======================================== //
class GatewayGenericTest : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};

    std::unique_ptr<TestGatewayGeneric> sut{new TestGatewayGeneric{}};
};

// ======================================== Tests ======================================== //
TEST_F(GatewayGenericTest, AddedChannelsAreStored)
{
    ::testing::Test::RecordProperty("TEST_ID", "da5e4a66-4f88-48bb-a684-4d10a19684b5");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    // ===== Test
    ASSERT_FALSE(sut->addChannel(testService, StubbedIceoryxTerminal::Options()).has_error());

    EXPECT_EQ(1U, sut->getNumberOfChannels());
}

TEST_F(GatewayGenericTest, DoesNotAddDuplicateChannels)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdd568b4-b377-48a3-8d2a-7f131bf1bba6");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    // ===== Test
    ASSERT_FALSE(sut->addChannel(testService, StubbedIceoryxTerminal::Options()).has_error());
    ASSERT_FALSE(sut->addChannel(testService, StubbedIceoryxTerminal::Options()).has_error());

    EXPECT_EQ(1U, sut->getNumberOfChannels());
}

TEST_F(GatewayGenericTest, IgnoresWildcardServices)
{
    ::testing::Test::RecordProperty("TEST_ID", "27984b07-f100-4cc3-9092-7f8afb8adca0");
    // ===== Setup
    auto completeWildcardService = iox::capro::ServiceDescription("*", "*", "*");
    auto wildcardServiceService = iox::capro::ServiceDescription("*", "instance", "event");
    auto wildcardInstanceService = iox::capro::ServiceDescription("service", "*", "event");
    auto wildcardEventService = iox::capro::ServiceDescription("service", "instance", "*");

    // ===== Test
    auto resultOne = sut->addChannel(completeWildcardService, StubbedIceoryxTerminal::Options());
    auto resultTwo = sut->addChannel(wildcardServiceService, StubbedIceoryxTerminal::Options());
    auto resultThree = sut->addChannel(wildcardInstanceService, StubbedIceoryxTerminal::Options());
    auto resultFour = sut->addChannel(wildcardEventService, StubbedIceoryxTerminal::Options());

    EXPECT_EQ(iox::gw::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultOne.error());
    EXPECT_EQ(iox::gw::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultTwo.error());
    EXPECT_EQ(iox::gw::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultThree.error());
    EXPECT_EQ(iox::gw::GatewayError::UNSUPPORTED_SERVICE_TYPE, resultFour.error());

    EXPECT_EQ(0U, sut->getNumberOfChannels());
}

TEST_F(GatewayGenericTest, ProperlyManagesMultipleChannels)
{
    ::testing::Test::RecordProperty("TEST_ID", "d6126772-9069-47c4-b8b0-4325eecba208");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto serviceOne = iox::capro::ServiceDescription("serviceOne", "instanceOne", "eventOne");
    auto serviceTwo = iox::capro::ServiceDescription("serviceTwo", "instanceTwo", "eventTwo");
    auto serviceThree = iox::capro::ServiceDescription("serviceThree", "instanceThree", "eventThree");
    auto serviceFour = iox::capro::ServiceDescription("serviceFour", "instanceFour", "eventFour");

    // ===== Test
    ASSERT_FALSE(sut->addChannel(serviceOne, StubbedIceoryxTerminal::Options()).has_error());
    ASSERT_FALSE(sut->addChannel(serviceTwo, StubbedIceoryxTerminal::Options()).has_error());
    ASSERT_FALSE(sut->addChannel(serviceThree, StubbedIceoryxTerminal::Options()).has_error());
    ASSERT_FALSE(sut->addChannel(serviceFour, StubbedIceoryxTerminal::Options()).has_error());


    EXPECT_EQ(4U, sut->getNumberOfChannels());
    EXPECT_EQ(true, sut->findChannel(serviceOne).has_value());
    EXPECT_EQ(true, sut->findChannel(serviceTwo).has_value());
    EXPECT_EQ(true, sut->findChannel(serviceThree).has_value());
    EXPECT_EQ(true, sut->findChannel(serviceFour).has_value());
}

TEST_F(GatewayGenericTest, HandlesMaxmimumChannelCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b4385e8-c717-4368-8121-b7d526fd22ac");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));

    // ===== Test
    for (auto i = 0U; i < iox::MAX_CHANNEL_NUMBER; i++)
    {
        auto result = sut->addChannel(
            iox::capro::ServiceDescription(into<lossy<iox::capro::IdString_t>>(iox::convert::toString(i)),
                                           into<lossy<iox::capro::IdString_t>>(iox::convert::toString(i)),
                                           into<lossy<iox::capro::IdString_t>>(iox::convert::toString(i))),
            StubbedIceoryxTerminal::Options());
        EXPECT_EQ(false, result.has_error());
    }

    EXPECT_EQ(iox::MAX_CHANNEL_NUMBER, sut->getNumberOfChannels());
}

TEST_F(GatewayGenericTest, ThrowsErrorWhenExceedingMaximumChannelCapaicity)
{
    ::testing::Test::RecordProperty("TEST_ID", "f73c1fe0-d5d3-4527-9acb-29692c5fd19f");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));

    // ===== Test
    for (auto i = 0U; i < iox::MAX_CHANNEL_NUMBER; i++)
    {
        auto result = sut->addChannel(
            iox::capro::ServiceDescription(into<lossy<iox::capro::IdString_t>>(iox::convert::toString(i)),
                                           into<lossy<iox::capro::IdString_t>>(iox::convert::toString(i)),
                                           into<lossy<iox::capro::IdString_t>>(iox::convert::toString(i))),
            StubbedIceoryxTerminal::Options());
        EXPECT_EQ(false, result.has_error());
    }

    auto result = sut->addChannel({"oneTooMany", "oneTooMany", "oneTooMany"}, StubbedIceoryxTerminal::Options());
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::gw::GatewayError::UNSUCCESSFUL_CHANNEL_CREATION, result.error());
}

TEST_F(GatewayGenericTest, ThrowsErrorWhenAttemptingToRemoveNonexistantChannel)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd9f8279-5876-418a-9606-7413a9c2df35");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto testServiceA = iox::capro::ServiceDescription("serviceA", "instanceA", "eventA");
    auto testServiceB = iox::capro::ServiceDescription("serviceB", "instanceB", "eventB");
    auto testServiceC = iox::capro::ServiceDescription("serviceC", "instanceC", "eventC");

    // ===== Test
    ASSERT_FALSE(sut->addChannel(testServiceA, StubbedIceoryxTerminal::Options()).has_error());
    ASSERT_FALSE(sut->addChannel(testServiceB, StubbedIceoryxTerminal::Options()).has_error());
    EXPECT_EQ(2U, sut->getNumberOfChannels());

    auto result = sut->discardChannel(testServiceC);
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(2U, sut->getNumberOfChannels());
}

TEST_F(GatewayGenericTest, DiscardedChannelsAreNotStored)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9f4cfcc-210d-4a61-83bf-d12da0ff7480");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    // ===== Test
    ASSERT_FALSE(sut->addChannel(testService, StubbedIceoryxTerminal::Options()).has_error());
    EXPECT_EQ(1U, sut->getNumberOfChannels());
    auto result = sut->discardChannel(testService);
    EXPECT_EQ(false, result.has_error());
    EXPECT_EQ(0U, sut->getNumberOfChannels());
}

TEST_F(GatewayGenericTest, FindChannelReturnsCopyOfFoundChannel)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c97dd37-cdd4-4e93-b202-36cdcf3c1029");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto testService = iox::capro::ServiceDescription("service", "instance", "event");

    // ===== Test
    ASSERT_FALSE(sut->addChannel(testService, StubbedIceoryxTerminal::Options()).has_error());
    auto foundChannel = sut->findChannel(testService);
    EXPECT_EQ(true, foundChannel.has_value());
    if (foundChannel.has_value())
    {
        EXPECT_EQ(testService, foundChannel.value().getServiceDescription());
    }
}

TEST_F(GatewayGenericTest, FindChannelGivesEmptyOptionalIfNoneFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "83cccfe0-68e6-4a35-b098-35c6c49c7743");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto storedChannelService = iox::capro::ServiceDescription("service", "instance", "event");
    auto notStoredChannelService = iox::capro::ServiceDescription("otherService", "otherInstance", "otherEvent");

    // ===== Test
    ASSERT_FALSE(sut->addChannel(storedChannelService, StubbedIceoryxTerminal::Options()).has_error());
    auto foundChannel = sut->findChannel(notStoredChannelService);
    EXPECT_EQ(false, foundChannel.has_value());
}

TEST_F(GatewayGenericTest, ForEachChannelExecutesGivenFunctionForAllStoredChannels)
{
    ::testing::Test::RecordProperty("TEST_ID", "df23a642-f247-42c8-8df3-4daba32bc397");
    // ===== Setup
    EXPECT_CALL(*sut, getInterface()).WillRepeatedly(Return(iox::capro::Interfaces::INTERNAL));
    auto testServiceA = iox::capro::ServiceDescription("serviceA", "instanceA", "eventA");
    auto testServiceB = iox::capro::ServiceDescription("serviceB", "instanceB", "eventB");
    auto testServiceC = iox::capro::ServiceDescription("serviceC", "instanceC", "eventC");

    auto count = 0U;
    auto f = [&count](TestChannel&) { count++; };

    // ===== Test
    ASSERT_FALSE(sut->addChannel(testServiceA, StubbedIceoryxTerminal::Options()).has_error());
    ASSERT_FALSE(sut->addChannel(testServiceB, StubbedIceoryxTerminal::Options()).has_error());
    ASSERT_FALSE(sut->addChannel(testServiceC, StubbedIceoryxTerminal::Options()).has_error());
    sut->forEachChannel(f);

    EXPECT_EQ(3U, count);
}

} // namespace
