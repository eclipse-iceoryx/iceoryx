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

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/subscriber_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using ::testing::_;

struct DummyData
{
    uint64_t val = 42;
};

class TestUntypedSubscriber : public iox::popo::UntypedSubscriberImpl<MockBaseSubscriber<void>>
{
  public:
    using SubscriberParent = iox::popo::UntypedSubscriberImpl<MockBaseSubscriber<void>>;

    TestUntypedSubscriber(const iox::capro::ServiceDescription& service,
                          const iox::popo::SubscriberOptions& subscriberOptions = iox::popo::SubscriberOptions())
        : SubscriberParent(service, subscriberOptions)
    {
    }

    using SubscriberParent::port;
};

class UntypedSubscriberTest : public Test
{
  public:
    UntypedSubscriberTest()
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
    TestUntypedSubscriber sut{{"", "", ""}};
};

TEST_F(UntypedSubscriberTest, GetsUIDViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "dfe1c24c-d012-4dba-8a24-1b3edbb436f4");
    // ===== Setup ===== //
    EXPECT_CALL(sut, getUid).WillOnce(Return(iox::popo::UniquePortId(iox::roudi::DEFAULT_UNIQUE_ROUDI_ID)));
    // ===== Test ===== //
    sut.getUid();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, GetsServiceDescriptionViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "bfbdea6a-7194-463f-9b38-ed11a5e2abc1");
    // ===== Setup ===== //
    EXPECT_CALL(sut, getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, GetsSubscriptionStateViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "da01f90a-da83-41f3-9621-b5b28a87504b");
    // ===== Setup ===== //
    EXPECT_CALL(sut, getSubscriptionState).Times(1);
    // ===== Test ===== //
    sut.getSubscriptionState();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, SubscribesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "c805d0d0-225b-46cc-a873-2fd399e75dc5");
    // ===== Setup ===== //
    EXPECT_CALL(sut, subscribe).Times(1);
    // ===== Test ===== //
    sut.subscribe(1);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, UnsubscribesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0470300-b2d5-4589-b93b-79efac76c56e");
    // ===== Setup ===== //
    EXPECT_CALL(sut, unsubscribe).Times(1);
    // ===== Test ===== //
    sut.unsubscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, ChecksForNewSamplesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "aff0709c-7486-4139-81de-5cfee29337f8");
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasData).Times(1);
    // ===== Test ===== //
    sut.hasData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, ChecksForMissedSamplesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "afa8f28c-35c7-4eff-921f-0a75685ef28e");
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasMissedData).Times(1);
    // ===== Test ===== //
    sut.hasMissedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, TakeReturnsAllocatedMemoryChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "e80e82f8-d573-407d-9640-b148d8679ed4");
    // ===== Setup ===== //
    EXPECT_CALL(sut, takeChunk)
        .Times(1)
        .WillOnce(Return(ByMove(iox::ok(const_cast<const iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader())))));
    EXPECT_CALL(sut.port(), releaseChunk).Times(AtLeast(1));
    // ===== Test ===== //
    auto maybeChunk = sut.take();
    // ===== Verify ===== //
    ASSERT_FALSE(maybeChunk.has_error());
    EXPECT_EQ(maybeChunk.value(), chunkMock.chunkHeader()->userPayload());
    // ===== Cleanup ===== //
    sut.release(maybeChunk.value());
}

TEST_F(UntypedSubscriberTest, ReleasesQueuedDataViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "66c0fb02-aa6d-48dd-8439-754e05cd29af");
    // ===== Setup ===== //
    EXPECT_CALL(sut, releaseQueuedData).Times(1);
    // ===== Test ===== //
    sut.releaseQueuedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

} // namespace
