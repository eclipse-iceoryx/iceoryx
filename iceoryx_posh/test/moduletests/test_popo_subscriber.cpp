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

#include "iceoryx_posh/popo/subscriber.hpp"
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

template <typename T, typename H, typename BaseSubscriber>
class StubbedSubscriber : public iox::popo::SubscriberImpl<T, H, BaseSubscriber>
{
  public:
    using SubscriberParent = iox::popo::SubscriberImpl<T, H, BaseSubscriber>;

    StubbedSubscriber(const iox::capro::ServiceDescription& service,
                      const iox::popo::SubscriberOptions& subscriberOptions = iox::popo::SubscriberOptions())
        : SubscriberParent(service, subscriberOptions)
    {
    }

    using SubscriberParent::port;
};

using TestSubscriber = StubbedSubscriber<DummyData, iox::mepoo::NoUserHeader, MockBaseSubscriber<DummyData>>;

class SubscriberTest : public Test
{
  public:
    SubscriberTest()
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
    TestSubscriber sut{{"", "", ""}, iox::popo::SubscriberOptions()};
};

TEST_F(SubscriberTest, GetsUIDViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b30966b-b50d-41f5-8be3-6f85ab14776d");
    // ===== Setup ===== //
    EXPECT_CALL(sut, getUid).WillOnce(Return(iox::popo::UniquePortId(iox::roudi::DEFAULT_UNIQUE_ROUDI_ID)));
    // ===== Test ===== //
    sut.getUid();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, GetsServiceDescriptionViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c08916d-69a4-436b-8ade-bcf43fdc7b6a");
    // ===== Setup ===== //
    EXPECT_CALL(sut, getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, GetsSubscriptionStateViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "4050a941-5b42-4a30-b00e-6a9f2c6aba0d");
    // ===== Setup ===== //
    EXPECT_CALL(sut, getSubscriptionState).Times(1);
    // ===== Test ===== //
    sut.getSubscriptionState();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, SubscribesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "067056dc-86a2-49a8-99fa-a4cac40d691f");
    // ===== Setup ===== //
    EXPECT_CALL(sut, subscribe).Times(1);
    // ===== Test ===== //
    sut.subscribe(1);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, UnsubscribesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "9da25521-3cb1-4c09-ad6d-166408fa37f2");
    // ===== Setup ===== //
    EXPECT_CALL(sut, unsubscribe).Times(1);
    // ===== Test ===== //
    sut.unsubscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, ChecksForNewSamplesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "27b37735-30db-422a-9b05-64c0a8daa18a");
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasData).Times(1);
    // ===== Test ===== //
    sut.hasData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, ChecksForMissedSamplesViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "a1c5f6f0-196a-457e-9990-cbeb89400619");
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasMissedData).Times(1);
    // ===== Test ===== //
    sut.hasMissedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, TakeReturnsAllocatedMemoryChunksWrappedInSample)
{
    ::testing::Test::RecordProperty("TEST_ID", "57507fcd-c7db-4b78-9e75-17c28c6ae5d7");
    // ===== Setup ===== //
    EXPECT_CALL(sut, takeChunk)
        .Times(1)
        .WillOnce(Return(ByMove(iox::ok(const_cast<const iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader())))));
    EXPECT_CALL(sut.port(), releaseChunk).Times(AtLeast(1));
    // ===== Test ===== //
    auto maybeSample = sut.take();
    // ===== Verify ===== //
    ASSERT_FALSE(maybeSample.has_error());
    EXPECT_EQ(maybeSample.value().get(), chunkMock.chunkHeader()->userPayload());
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, ReceivedSamplesAreAutomaticallyDeletedWhenOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "f32c401d-0620-4a4b-800f-eda94a493efd");
    // ===== Setup ===== //
    EXPECT_CALL(sut, takeChunk)
        .Times(1)
        .WillOnce(Return(ByMove(iox::ok(const_cast<const iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader())))));
    EXPECT_CALL(sut.port(), releaseChunk).Times(AtLeast(1));
    // ===== Test ===== //
    {
        EXPECT_FALSE(sut.take().has_error());
    }
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(SubscriberTest, ReleasesQueuedDataViaBaseSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "f30fe1ae-046c-48b3-b5cd-b9adbf9b864f");
    // ===== Setup ===== //
    EXPECT_CALL(sut, releaseQueuedData).Times(1);
    // ===== Test ===== //
    sut.releaseQueuedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

} // namespace
