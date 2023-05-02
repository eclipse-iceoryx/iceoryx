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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/base_subscriber.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"
#include "iox/unique_ptr.hpp"

#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/subscriber_mock.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

// anonymous namespace to prevent linker issues or sanitizer false positives
// if a struct with the same name is used in other tests
namespace
{
struct DummyData
{
    uint64_t val = 42;
};

class WaitSetTest : public iox::popo::WaitSet<>
{
  public:
    WaitSetTest(iox::popo::ConditionVariableData& condVarData) noexcept
        : WaitSet(condVarData)
    {
    }
};


template <typename port_t>
class StubbedBaseSubscriber : public iox::popo::BaseSubscriber<port_t>
{
  public:
    using SubscriberParent = iox::popo::BaseSubscriber<port_t>;

    using SubscriberParent::disableEvent;
    using SubscriberParent::disableState;
    using SubscriberParent::enableEvent;
    using SubscriberParent::enableState;
    using SubscriberParent::takeChunk;

    using SubscriberParent::port;
};

using TestBaseSubscriber = StubbedBaseSubscriber<MockSubscriberPortUser>;

// ========================= Base Publisher Tests ========================= //

class BaseSubscriberTest : public Test
{
  public:
    BaseSubscriberTest()
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
    ChunkMock<DummyData> chunkMock;
    TestBaseSubscriber sut{};
};

TEST_F(BaseSubscriberTest, SubscribeCallForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "bee5b6ab-c08c-4cb5-b39b-dd75b2fb1b40");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), subscribe()).Times(1);
    // ===== Test ===== //
    sut.subscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, GetSubscriptionStateCallForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fc3be1a-cd85-44f6-8596-c7a2273eabab");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), getSubscriptionState).Times(1);
    // ===== Test ===== //
    sut.getSubscriptionState();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, UnsubscribeCallForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5793a32-2785-4dd3-b9ff-411070f67a5a");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), unsubscribe).Times(1);
    // ===== Test ===== //
    sut.unsubscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, HasDataCallForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "a1c39c3a-3347-4072-a3d3-02e3cc264ae5");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), hasNewChunks).Times(1);
    // ===== Test ===== //
    sut.hasData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceiveReturnsAllocatedMemoryChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "5e3c00e1-bd7c-49bf-adaf-f0d83cd4ab99");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), tryGetChunk)
        .WillOnce(Return(ByMove(iox::ok(const_cast<const iox::mepoo::ChunkHeader*>(chunkMock.chunkHeader())))));
    // ===== Test ===== //
    auto result = sut.takeChunk();
    // ===== Verify ===== //
    ASSERT_EQ(false, result.has_error());
    EXPECT_EQ(result.value(), chunkMock.chunkHeader());
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ReceiveForwardsErrorsFromUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff175cb2-ad97-4ba9-ab32-cd73618b0b8b");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), tryGetChunk)
        .WillOnce(Return(ByMove(iox::err(iox::popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL))));
    // ===== Test ===== //
    auto result = sut.takeChunk();
    // ===== Verify ===== //
    ASSERT_EQ(true, result.has_error());
    EXPECT_EQ(iox::popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL, result.error());
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, ClearReceiveBufferCallForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "975653e3-4644-4a2e-8bc6-7af9830e3863");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), releaseQueuedChunks).Times(1);
    // ===== Test ===== //
    sut.releaseQueuedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, AttachStateToWaitsetForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b4c16fd-bb9d-4a4e-bc55-521be5c1ae18");
    iox::popo::ConditionVariableData condVar("Horscht");
    WaitSetTest waitSet(condVar);
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    EXPECT_CALL(sut.port(), hasNewChunks()).WillRepeatedly(Return(false));
    // ===== Test ===== //
    ASSERT_FALSE(waitSet.attachState(sut, iox::popo::SubscriberState::HAS_DATA).has_error());
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable()).Times(1);
}

TEST_F(BaseSubscriberTest, AttachEventToWaitsetForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "2588c558-9982-418d-ac4b-0d512103d0e5");
    iox::popo::ConditionVariableData condVar("Horscht");
    WaitSetTest waitSet(condVar);
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    // ===== Test ===== //
    ASSERT_FALSE(waitSet.attachEvent(sut, iox::popo::SubscriberEvent::DATA_RECEIVED).has_error());
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable()).Times(1);
}

TEST_F(BaseSubscriberTest, WaitSetUnsetStateBasedConditionVariableWhenGoingOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "9af5c23d-7584-4142-bd1b-1eaca706d887");
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetTest> waitSet{new WaitSetTest(condVar)};
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    EXPECT_CALL(sut.port(), hasNewChunks()).WillRepeatedly(Return(false));
    ASSERT_FALSE(waitSet->attachState(sut, iox::popo::SubscriberState::HAS_DATA).has_error());
    // ===== Test ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable).Times(1);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, WaitSetUnsetEventBasedConditionVariableWhenGoingOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "d0a1d958-9681-4c88-88d8-4a6e4485d101");
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetTest> waitSet{new WaitSetTest(condVar)};
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    ASSERT_FALSE(waitSet->attachEvent(sut, iox::popo::SubscriberEvent::DATA_RECEIVED).has_error());
    // ===== Test ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable).Times(1);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, AttachingAttachedStateSubscriberToNewWaitsetDetachesItFromOriginalWaitset)
{
    ::testing::Test::RecordProperty("TEST_ID", "301c7202-cb9c-436c-ba6d-5c370eab9e5d");
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetTest> waitSet{new WaitSetTest(condVar)};
    std::unique_ptr<WaitSetTest> waitSet2{new WaitSetTest(condVar)};
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    EXPECT_CALL(sut.port(), hasNewChunks()).WillRepeatedly(Return(false));
    ASSERT_FALSE(waitSet->attachState(sut, iox::popo::SubscriberState::HAS_DATA).has_error());
    // ===== Test ===== //
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    ASSERT_FALSE(waitSet2->attachState(sut, iox::popo::SubscriberState::HAS_DATA).has_error());
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 0U);
    EXPECT_EQ(waitSet2->size(), 1U);
    // ===== Cleanup ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable()).Times(1);
}

TEST_F(BaseSubscriberTest, AttachingEventToAttachedStateSubscriberDetachesState)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4b37424-10ec-4217-9e64-7006e6aebad9");
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetTest> waitSet{new WaitSetTest(condVar)};
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    EXPECT_CALL(sut.port(), hasNewChunks()).WillRepeatedly(Return(false));
    ASSERT_FALSE(waitSet->attachState(sut, iox::popo::SubscriberState::HAS_DATA).has_error());
    // ===== Test ===== //
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    ASSERT_FALSE(waitSet->attachEvent(sut, iox::popo::SubscriberEvent::DATA_RECEIVED).has_error());
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 1U);
    // ===== Cleanup ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable()).Times(1);
}

TEST_F(BaseSubscriberTest, DetachingAttachedStateCleansup)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bff4985-752e-47c9-9232-e2382086db29");
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetTest> waitSet{new WaitSetTest(condVar)};
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    EXPECT_CALL(sut.port(), hasNewChunks()).WillRepeatedly(Return(false));
    ASSERT_FALSE(waitSet->attachState(sut, iox::popo::SubscriberState::HAS_DATA).has_error());
    // ===== Test ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable).Times(1);
    sut.disableState(iox::popo::SubscriberState::HAS_DATA);
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 0U);
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, DetachingAttachedEventCleansup)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9b7a7e4-4374-4634-ba3d-6ffb833c5974");
    // ===== Setup ===== //
    iox::popo::ConditionVariableData condVar("Horscht");
    std::unique_ptr<WaitSetTest> waitSet{new WaitSetTest(condVar)};
    EXPECT_CALL(sut.port(), setConditionVariable(_, _)).Times(1);
    ASSERT_FALSE(waitSet->attachEvent(sut, iox::popo::SubscriberEvent::DATA_RECEIVED).has_error());
    // ===== Test ===== //
    EXPECT_CALL(sut.port(), unsetConditionVariable).Times(1);
    sut.disableEvent(iox::popo::SubscriberEvent::DATA_RECEIVED);
    // ===== Verify ===== //
    EXPECT_EQ(waitSet->size(), 0U);
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, GetServiceDescriptionCallForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "93c5087c-2ba4-46fe-95d7-b619b49d3fe8");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, HasMissedSamplesCallForwardedToUnderlyingSubscriberPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "90427619-7b26-4dc2-950b-9192be99f20a");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), hasLostChunksSinceLastCall).Times(1);
    // ===== Test ===== //
    sut.hasMissedData();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(BaseSubscriberTest, DestroysUnderlyingPortOnDestruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a3004af-4ccd-4df0-bdd8-6e22e97d2428");
    // ===== Setup ===== //
    EXPECT_CALL(sut.port(), destroy).Times(1);
    // ===== Test ===== //
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

} // namespace
