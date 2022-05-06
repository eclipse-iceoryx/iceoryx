// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/base_publisher.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using ::testing::_;

template <typename port_t>
class StubbedBasePublisher : public iox::popo::BasePublisher<port_t>
{
  public:
    StubbedBasePublisher(iox::capro::ServiceDescription)
        : iox::popo::BasePublisher<port_t>::BasePublisher(){};

    using iox::popo::BasePublisher<port_t>::port;
};

using TestBasePublisher = StubbedBasePublisher<MockPublisherPortUser>;

class BasePublisherTest : public Test
{
  public:
    BasePublisherTest()
    {
    }

    void SetUp()
    {
        EXPECT_CALL(sut.port(), destroy).WillRepeatedly(Return());
    }

    void TearDown()
    {
    }

  protected:
    TestBasePublisher sut{{"", "", ""}};
};


TEST_F(BasePublisherTest, OfferDoesOfferServiceOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "9fac841c-d067-47ec-8626-73ef7d4aa8db");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), offer).Times(1);
    // ===== Test ===== //
    sut.offer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, StopOfferDoesStopOfferServiceOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5c7b795-b996-4e87-9f2b-96fa0e01c4c3");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), stopOffer).Times(1);
    // ===== Test ===== //
    sut.stopOffer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, isOfferedDoesCheckIfPortIsOfferedOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "323b75cb-539e-4888-9b2f-f3f0bcdc1d3d");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), isOffered).Times(1);
    // ===== Test ===== //
    sut.isOffered();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, isOfferedDoesCheckIfUnderylingPortHasSubscribers)
{
    ::testing::Test::RecordProperty("TEST_ID", "b361e985-5187-4e51-a833-697d08cb0588");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), hasSubscribers).Times(1);
    // ===== Test ===== //
    sut.hasSubscribers();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, GetServiceDescriptionCallForwardedToUnderlyingPublisherPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "c3b989a9-61d5-4d8f-81b0-eacb0e368a14");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BasePublisherTest, DestroysUnderlyingPortOnDestruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ecca6de-7331-493b-8985-cc37af368dba");
    EXPECT_CALL(sut.port(), destroy).Times(1);
}

} // namespace
