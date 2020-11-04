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

#include "iceoryx_posh/popo/modern_api/untyped_subscriber.hpp"
#include "mocks/subscriber_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

using TestUntypedSubscriber = iox::popo::UntypedSubscriberImpl<MockBaseSubscriber<void>>;

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
    TestUntypedSubscriber sut{{"", "", ""}};
};

TEST_F(UntypedSubscriberTest, GetsUIDViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, getUid).Times(1);
    // ===== Test ===== //
    sut.getUid();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, GetsServiceDescriptionViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, getServiceDescription).Times(1);
    // ===== Test ===== //
    sut.getServiceDescription();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, GetsSubscriptionStateViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, getSubscriptionState).Times(1);
    // ===== Test ===== //
    sut.getSubscriptionState();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, SubscribesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, subscribe).Times(1);
    // ===== Test ===== //
    sut.subscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, UnsubscribesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, unsubscribe).Times(1);
    // ===== Test ===== //
    sut.unsubscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, ChecksForNewSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasNewSamples).Times(1);
    // ===== Test ===== //
    sut.hasNewSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, ChecksForMissedSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasMissedSamples).Times(1);
    // ===== Test ===== //
    sut.hasMissedSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, ReceivesSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, take).Times(1).WillOnce(
        Return(ByMove(iox::cxx::success<iox::cxx::optional<iox::popo::Sample<const void>>>())));
    // ===== Test ===== //
    sut.take();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, ReleasesQueuedSamplesViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, releaseQueuedSamples).Times(1);
    // ===== Test ===== //
    sut.releaseQueuedSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, SetsConditionVariableViaBaseSubscriber)
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

TEST_F(UntypedSubscriberTest, UnsetsConditionVariableViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, unsetConditionVariable).Times(1);
    // ===== Test ===== //
    sut.unsetConditionVariable();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(UntypedSubscriberTest, ChecksIfConditionIsTriggeredViaBaseSubscriber)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut, hasTriggered).Times(1);
    // ===== Test ===== //
    sut.hasTriggered();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
