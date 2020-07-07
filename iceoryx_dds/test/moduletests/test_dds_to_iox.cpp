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

#include "helpers/fixture_dds_gateway.hpp"

#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_dds/gateway/dds_to_iox.hpp"

#include "mocks/chunk_mock.hpp"
#include "mocks/google_mocks.hpp"
#include "roudi_gtest.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Helpers ======================================== //
using TestChannel = iox::dds::Channel<MockPublisher, MockDataReader>;
using TestGateway = iox::dds::DDS2IceoryxGateway<TestChannel, MockGenericDDSGateway<TestChannel>>;

// ======================================== Fixture ======================================== //
class DDS2IceoryxGatewayTest : public DDSGatewayTestFixture<MockPublisher, MockDataReader>
{
};

// ======================================== Tests ======================================== //
TEST_F(DDS2IceoryxGatewayTest, ChannelsAreCreatedForConfiguredServices)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    auto dataSize = 8u;
    iox::dds::GatewayConfig config{};
    config.m_configuredServices.push_back(iox::dds::GatewayConfig::ServiceEntry{testService, dataSize});

    TestGateway gw{};
    EXPECT_CALL(gw, findChannel).WillOnce(Return(iox::cxx::nullopt_t()));
    EXPECT_CALL(gw, addChannel(testService)).WillOnce(Return(channelFactory(testService)));

    // === Test
    gw.loadConfiguration(config);
}

TEST_F(DDS2IceoryxGatewayTest, ImmediatelyOffersConfiguredPublishers)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    auto dataSize = 8u;

    iox::dds::GatewayConfig config{};
    config.m_configuredServices.push_back(iox::dds::GatewayConfig::ServiceEntry{testService, dataSize});

    auto mockPublisher = createMockIceoryxTerminal(testService);
    EXPECT_CALL(*mockPublisher, offer).Times(1);
    stageMockIceoryxTerminal(std::move(mockPublisher));

    TestGateway gw{};
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel(testService)).WillByDefault(Return(channelFactory(testService)));

    // === Test
    gw.loadConfiguration(config);
}

TEST_F(DDS2IceoryxGatewayTest, ImmediatelyConnectsConfiguredDataReaders)
{
    // === Setup
    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
    auto dataSize = 8u;

    iox::dds::GatewayConfig config{};
    config.m_configuredServices.push_back(iox::dds::GatewayConfig::ServiceEntry{testService});

    auto mockDataReader = createMockDDSTerminal(testService);
    EXPECT_CALL(*mockDataReader, connect).Times(1);
    stageMockDDSTerminal(std::move(mockDataReader));

    TestGateway gw{};
    ON_CALL(gw, findChannel).WillByDefault(Return(iox::cxx::nullopt_t()));
    ON_CALL(gw, addChannel(testService)).WillByDefault(Return(channelFactory(testService)));

    // === Test
    gw.loadConfiguration(config);
}

TEST_F(DDS2IceoryxGatewayTest, ForwardsReceivedBytesIntoReservedMemoryChunks)
{
    // Will activate test when bug with returning an expected in a mock is resolved.
    if (false)
    {
        // === Setup
        auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
        auto dataSize = 8u;

        // Setup data reader to provide a sample
        auto mockDataReader = createMockDDSTerminal(testService);
        auto mockPublisher = createMockIceoryxTerminal(testService);

        // NOTE: This line does does not compile with the following error. Will debug later...
        // error: cannot convert ‘iox::cxx::success<long unsigned int>’ to ‘long unsigned int’ in initialization
        // : value(std::forward<Targs>(args)...)
        // ON_CALL(*mockDataReader, read(_, _, _, _)).WillByDefault(Return(iox::cxx::success<uint64_t>(1)));

        EXPECT_CALL(*mockPublisher, sendChunk).Times(1);

        stageMockDDSTerminal(std::move(mockDataReader));
        stageMockIceoryxTerminal(std::move(mockPublisher));

        auto testChannel = channelFactory(testService).get_value();
        TestGateway gw{};

        // === Test
        gw.forward(testChannel);
    }
}

TEST_F(DDS2IceoryxGatewayTest, OnlyRequestsOneSampleAtATime)
{
    if (false)
    {
        // === Setup
        auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
        auto dataSize = 8u;

        uint8_t buffer[64];

        // Setup data reader to provide a sample
        auto mockDataReader = createMockDDSTerminal(testService);
        auto mockPublisher = createMockIceoryxTerminal(testService);

        ON_CALL(*mockPublisher, allocateChunk).WillByDefault(Return(&buffer));

        // NOTE: This line does does not compile with the following error. Will debug later...
        // error: cannot convert ‘iox::cxx::success<long unsigned int>’ to ‘long unsigned int’ in initialization
        // : value(std::forward<Targs>(args)...).
        // ON_CALL(*mockDataReader, read(_, _, _, _)).WillByDefault(Return(iox::cxx::success<uint64_t>(1)));

        stageMockDDSTerminal(std::move(mockDataReader));
        stageMockIceoryxTerminal(std::move(mockPublisher));

        auto testChannel = channelFactory(testService).get_value();
        TestGateway gw{};

        // === Test
        gw.forward(testChannel);
    }
}

//TEST_F(DDS2IceoryxGatewayTest, DoesNotForwardAcrossChannelsWithNoDataSize)
//{
//    // === Setup
//    auto testService = iox::capro::ServiceDescription({"Radar", "Front-Right", "Reflections"});
//    auto dataSize = 0u;

//    auto mockDataReader = createMockDDSTerminal(testService);
//    auto mockPublisher = createMockIceoryxTerminal(testService);

//    EXPECT_CALL(*mockDataReader, take(_, _, _, _)).Times(0);
//    EXPECT_CALL(*mockPublisher, sendChunk).Times(0);

//    stageMockDDSTerminal(std::move(mockDataReader));
//    stageMockIceoryxTerminal(std::move(mockPublisher));

//    TestGateway gw{};

//    // === Test
//    auto testChannel = channelFactory(testService, dataSize).get_value();
//    gw.forward(testChannel);
//}
