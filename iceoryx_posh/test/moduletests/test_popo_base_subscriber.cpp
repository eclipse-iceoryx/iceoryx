// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/modern_api/base_subscriber.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

#include "mocks/subscriber_mock.hpp"
#include "mocks/wait_set_mock.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData
{
    uint64_t val = 42;
};

template <typename T, typename port_t>
class StubbedBaseSubscriber : public iox::popo::BaseSubscriber<T, StubbedBaseSubscriber<T, port_t>, port_t>
{
  public:
    using SubscriberParent = iox::popo::BaseSubscriber<T, StubbedBaseSubscriber<T, port_t>, port_t>;

    using SubscriberParent::attachTo;
    using SubscriberParent::detachOf;
    using SubscriberParent::getServiceDescription;
    using SubscriberParent::getSubscriptionState;
    using SubscriberParent::getUid;
    using SubscriberParent::hasMissedSamples;
    using SubscriberParent::hasNewSamples;
    using SubscriberParent::releaseQueuedSamples;
    using SubscriberParent::subscribe;
    using SubscriberParent::take;
    using SubscriberParent::unsetTrigger;
    using SubscriberParent::unsubscribe;
    port_t& getMockedPort()
    {
        return SubscriberParent::m_port;
    }
};

using TestBaseSubscriber = StubbedBaseSubscriber<DummyData, MockSubscriberPortUser>;

// ========================= Base Publisher Tests ========================= //

class BaseSubscriberTest : public Test
{
  public:
    BaseSubscriberTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
    TestBaseSubscriber sut{};
};

TEST_F(BaseSubscriberTest, SubscribeCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), subscribe(iox::MAX_SUBSCRIBER_QUEUE_CAPACITY)).Times(1);
    // ===== Test ===== //
    sut.subscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, GetSubscriptionStateCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), getSubscriptionState).Times(1);
    // ===== Test ===== //
    sut.getSubscriptionState();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, UnsubscribeCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), unsubscribe).Times(1);
    // ===== Test ===== //
    sut.unsubscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, HasNewSamplesCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), hasNewChunks).Times(1);
    // ===== Test ===== //
    sut.hasNewSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceiveReturnsAllocatedMemoryChunksWrappedInSample)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk)
        .WillOnce(Return(ByMove(iox::cxx::success<iox::cxx::optional<const iox::mepoo::ChunkHeader*>>(
            const_cast<const iox::mepoo::ChunkHeader*>(chunk)))));
    // ===== Test ===== //
    auto result = sut.take();
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    EXPECT_EQ(true, result.value().has_value());
    EXPECT_EQ(reinterpret_cast<DummyData*>(chunk->payload()),
              result.value().value().get()); // Checks they point to the same memory location.
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceivedSamplesAreAutomaticallyDeletedWhenOutOfScope)
{
    // ===== Setup ===== //
    auto chunk =
        reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk)
        .WillOnce(Return(ByMove(iox::cxx::success<iox::cxx::optional<const iox::mepoo::ChunkHeader*>>(
            const_cast<const iox::mepoo::ChunkHeader*>(chunk)))));
    EXPECT_CALL(sut.getMockedPort(), releaseChunk).Times(AtLeast(1));
    // ===== Test ===== //
    {
        auto result = sut.take();
    }
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceiveForwardsErrorsFromUnderlyingPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk)
        .WillOnce(Return(ByMove(iox::cxx::error<iox::popo::ChunkReceiveError>(
            iox::popo::ChunkReceiveError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL))));
    // ===== Test ===== //
    auto result = sut.take();
    // ===== Verify ===== //
    EXPECT_EQ(true, result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceiveReturnsEmptyOptionalIfUnderlyingPortReturnsEmptyOptional)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk)
        .WillOnce(
            Return(ByMove(iox::cxx::success<iox::cxx::optional<const iox::mepoo::ChunkHeader*>>(iox::cxx::nullopt))));
    // ===== Test ===== //
    auto result = sut.take();
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    EXPECT_EQ(false, result.value().has_value());
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ClearReceiveBufferCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), releaseQueuedChunks).Times(1);
    // ===== Test ===== //
    sut.releaseQueuedSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, SetConditionVariableCallForwardedToUnderlyingSubscriberPort)
{
    iox::popo::ConditionVariableData condVar;
    WaitSetMock waitSet(&condVar);
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    // ===== Test ===== //
    sut.attachTo(waitSet, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, UnsetConditionVariableCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar;
    WaitSetMock* waitSet = new WaitSetMock(&condVar);
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    sut.attachTo(*waitSet, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    // ===== Test ===== //
    EXPECT_CALL(sut.getMockedPort(), unsetConditionVariable).Times(1);
    delete waitSet;
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, AttachingAttachedSubscriberToNewWaitsetDetachesItFromOriginalWaitset)
{
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar;
    WaitSetMock* waitSet = new WaitSetMock(&condVar);
    WaitSetMock* waitSet2 = new WaitSetMock(&condVar);
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    sut.attachTo(*waitSet, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    // ===== Test ===== //
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    EXPECT_CALL(sut.getMockedPort(), unsetConditionVariable).Times(1);
    sut.attachTo(*waitSet2, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 0);
    EXPECT_EQ(waitSet2->size(), 1);
    // ===== Cleanup ===== //
    EXPECT_CALL(sut.getMockedPort(), unsetConditionVariable).Times(1);
    delete waitSet;
}

TEST_F(BaseSubscriberTest, DetachingAttachedEventCleansup)
{
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar;
    WaitSetMock* waitSet = new WaitSetMock(&condVar);
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    sut.attachTo(*waitSet, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    // ===== Test ===== //
    EXPECT_CALL(sut.getMockedPort(), unsetConditionVariable).Times(1);
    sut.detachOf(iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 0);
    // ===== Cleanup ===== //
    delete waitSet;
}

TEST_F(BaseSubscriberTest, HasTriggeredCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), hasNewChunks).Times(1);
    // ===== Test ===== //
    sut.hasNewSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, GetServiceDescriptionCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, HasMissedSamplesCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), hasLostChunksSinceLastCall).Times(1);
    // ===== Test ===== //
    sut.hasMissedSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, DestroysUnderlyingPortOnDestruction)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), destroy).Times(1);
    // ===== Test ===== //
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
