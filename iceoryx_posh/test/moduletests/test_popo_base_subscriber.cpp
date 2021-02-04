// Copyright (c) 2020, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/popo/base_subscriber.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

#include "mocks/chunk_mock.hpp"
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

    using SubscriberParent::disableEvent;
    using SubscriberParent::enableEvent;
    using SubscriberParent::getServiceDescription;
    using SubscriberParent::getSubscriptionState;
    using SubscriberParent::getUid;
    using SubscriberParent::hasMissedSamples;
    using SubscriberParent::hasSamples;
    using SubscriberParent::releaseQueuedSamples;
    using SubscriberParent::subscribe;
    using SubscriberParent::take;
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
    ChunkMock<DummyData> chunkMock;
    TestBaseSubscriber sut{};
};

TEST_F(BaseSubscriberTest, SubscribeCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), subscribe()).Times(1);
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
    sut.hasSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceiveReturnsAllocatedMemoryChunksWrappedInSample)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk)
        .WillOnce(Return(ByMove(iox::cxx::success<iox::cxx::optional<const iox::mepoo::ChunkHeader*>>(
            const_cast<const iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader())))));
    // ===== Test ===== //
    auto result = sut.take();
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    EXPECT_EQ(true, result.value().has_value());
    EXPECT_EQ(reinterpret_cast<DummyData*>(chunkMock.chunkHeader()->payload()),
              result.value().value().get()); // Checks they point to the same memory location.
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceivedSamplesAreAutomaticallyDeletedWhenOutOfScope)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk)
        .WillOnce(Return(ByMove(iox::cxx::success<iox::cxx::optional<const iox::mepoo::ChunkHeader*>>(
            const_cast<const iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader())))));
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
        .WillOnce(Return(ByMove(iox::cxx::error<iox::popo::ChunkReceiveResult>(
            iox::popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL))));
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

TEST_F(BaseSubscriberTest, AttachToWaitsetForwardedToUnderlyingSubscriberPort)
{
    iox::popo::ConditionVariableData condVar("Horscht");
    WaitSetMock waitSet(&condVar);
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    // ===== Test ===== //
    waitSet.attachEvent(sut, iox::popo::SubscriberEvent::HAS_SAMPLES);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, WaitSetUnsetConditionVariableWhenGoingOutOfScope)
{
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetMock> waitSet{new WaitSetMock(&condVar)};
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    waitSet->attachEvent(sut, iox::popo::SubscriberEvent::HAS_SAMPLES);
    // ===== Test ===== //
    EXPECT_CALL(sut.getMockedPort(), unsetConditionVariable).Times(1);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, AttachingAttachedSubscriberToNewWaitsetDetachesItFromOriginalWaitset)
{
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetMock> waitSet{new WaitSetMock(&condVar)};
    std::unique_ptr<WaitSetMock> waitSet2{new WaitSetMock(&condVar)};
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    waitSet->attachEvent(sut, iox::popo::SubscriberEvent::HAS_SAMPLES);
    // ===== Test ===== //
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    waitSet2->attachEvent(sut, iox::popo::SubscriberEvent::HAS_SAMPLES);
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 0U);
    EXPECT_EQ(waitSet2->size(), 1U);
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, DetachingAttachedEventCleansup)
{
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetMock> waitSet{new WaitSetMock(&condVar)};
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(&condVar)).Times(1);
    waitSet->attachEvent(sut, iox::popo::SubscriberEvent::HAS_SAMPLES);
    // ===== Test ===== //
    EXPECT_CALL(sut.getMockedPort(), unsetConditionVariable).Times(1);
    sut.disableEvent(iox::popo::SubscriberEvent::HAS_SAMPLES);
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 0U);
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, HasTriggeredCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), hasNewChunks).Times(1);
    // ===== Test ===== //
    sut.hasSamples();
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
