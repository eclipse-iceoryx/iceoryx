// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020-2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/typed_subscriber.hpp"
#include "mocks/chunk_mock.hpp"
#include "mocks/subscriber_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData
{
    uint64_t val = 42;
};

using TestTypedSubscriber = iox::popo::TypedSubscriber<DummyData, MockBaseSubscriber, MockSubscriberPortUser>;

class TypedSubscriberTest : public Test
{
  public:
    TypedSubscriberTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
    ChunkMock<DummyData> chunkMock;
    TestTypedSubscriber sut{{"", "", ""}, iox::popo::SubscriberOptions()};
};

TEST_F(TypedSubscriberTest, GetsUIDViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, getUid).Times(1);
    // ===== Test ===== //
    sut.getUid();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, GetsServiceDescriptionViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, GetsSubscriptionStateViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, getSubscriptionState).Times(1);
    // ===== Test ===== //
    sut.getSubscriptionState();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, SubscribesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, subscribe).Times(1);
    // ===== Test ===== //
    sut.subscribe(1);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, UnsubscribesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, unsubscribe).Times(1);
    // ===== Test ===== //
    sut.unsubscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ChecksForNewSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasData).Times(1);
    // ===== Test ===== //
    sut.hasData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ChecksForMissedSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasMissedData).Times(1);
    // ===== Test ===== //
    sut.hasMissedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ReceivesSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, takeChunk)
        .Times(1)
        .WillOnce(Return(ByMove(iox::cxx::success<const iox::mepoo::ChunkHeader*>(
            const_cast<const iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader())))));
    // ===== Test ===== //
    auto maybeSample = sut.take();
    // ===== Verify ===== //
    EXPECT_FALSE(maybeSample.has_error());
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ReleasesQueuedDataViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, releaseQueuedData).Times(1);
    // ===== Test ===== //
    sut.releaseQueuedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
