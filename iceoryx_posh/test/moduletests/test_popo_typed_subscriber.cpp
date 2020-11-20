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

#include "iceoryx_posh/popo/modern_api/typed_subscriber.hpp"
#include "mocks/subscriber_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData
{
    uint64_t val = 42;
};

using TestTypedSubscriber = iox::popo::TypedSubscriber<DummyData, MockBaseSubscriber<DummyData>>;

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
    TestTypedSubscriber sut{{"", "", ""}};
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
    sut.subscribe();
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
    EXPECT_CALL(sut, hasNewSamples).Times(1);
    // ===== Test ===== //
    sut.hasNewSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ChecksForMissedSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasMissedSamples).Times(1);
    // ===== Test ===== //
    sut.hasMissedSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ReceivesSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, take).Times(1).WillOnce(
        Return(ByMove(iox::cxx::success<iox::cxx::optional<iox::popo::Sample<const DummyData>>>())));
    // ===== Test ===== //
    sut.take();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ReleasesQueuedSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, releaseQueuedSamples).Times(1);
    // ===== Test ===== //
    sut.releaseQueuedSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, SetsConditionVariableViaBaseSubscriber)
{
    // ===== Setup ===== //
    auto conditionVariable = new iox::popo::ConditionVariableData();
    EXPECT_CALL(sut, setConditionVariable).Times(1);
    // ===== Test ===== //
    sut.setConditionVariable(conditionVariable);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    delete conditionVariable;
}

TEST_F(TypedSubscriberTest, UnsetsConditionVariableViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, unsetConditionVariable).Times(1);
    // ===== Test ===== //
    sut.unsetConditionVariable();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(TypedSubscriberTest, ChecksIfConditionIsTriggeredViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasTriggered).Times(1);
    // ===== Test ===== //
    sut.hasTriggered();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
