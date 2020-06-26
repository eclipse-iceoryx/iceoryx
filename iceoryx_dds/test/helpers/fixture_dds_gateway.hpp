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

#ifndef TEST_HELPERS_FIXTURE_DDS_GATEWAY_H
#define TEST_HELPERS_FIXTURE_DDS_GATEWAY_H

#include "iceoryx_dds/gateway/channel.hpp"

#include "mocks/google_mocks.hpp"
#include "roudi_gtest.hpp"
#include "test.hpp"

class DDSGatewayTestFixture : public Test
{
  public:
    // Holds mocks created by tests to be used by the channel factory.
    std::vector<std::shared_ptr<MockSubscriber>> stagedMockSubscribers;
    std::vector<std::shared_ptr<MockDataWriter>> stagedMockWriters;

    // Marks where in the array to look for a valid mock.
    // Indexes lower than the marker are assumed to have been moved out and thus undefined.
    size_t subscriberMarker = 0;
    size_t dataWriterMarker = 0;

    void SetUp(){};
    void TearDown()
    {
        stagedMockSubscribers.clear();
        subscriberMarker = 0;
        stagedMockWriters.clear();
        dataWriterMarker = 0;
    };

    // Create a new DataWriter mock
    std::shared_ptr<MockDataWriter> createMockDataWriter(const iox::capro::ServiceDescription& sd)
    {
        return std::shared_ptr<MockDataWriter>(new MockDataWriter(sd));
    }
    // Stage the given mock to be used in the channel factory
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
    // Stage the given mock to be used in the channel factory
    void stageMockSubscriber(std::shared_ptr<MockSubscriber>&& mock)
    {
        stagedMockSubscribers.emplace_back(std::move(mock));
    };

    // Creates channels to be used in tests.
    // Channels will contain staged mocks, or empty mocks if none are staged.
    // The factory method can be passed to test gateways, allowing injection of mocks.
    iox::cxx::expected<iox::dds::Channel<MockSubscriber, MockDataWriter>, iox::dds::GatewayError>
    channelFactory(iox::capro::ServiceDescription sd) noexcept
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

};

#endif // TEST_HELPERS_FIXTURE_DDS_GATEWAY_H
