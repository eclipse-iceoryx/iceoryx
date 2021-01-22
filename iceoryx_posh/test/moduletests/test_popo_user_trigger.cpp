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
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "mocks/wait_set_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox;
using namespace iox::popo;

class UserTrigger_test : public Test
{
  public:
    UserTrigger m_sut;
    ConditionVariableData m_condVar{"Horscht"};
    ConditionVariableData m_condVar2{"Schnuppi"};
    WaitSetMock m_waitSet{&m_condVar};
    WaitSetMock m_waitSet2{&m_condVar2};

    void SetUp()
    {
        m_callbackOrigin = nullptr;
    }

    static UserTrigger* m_callbackOrigin;
    static void callback(UserTrigger* origin)
    {
        m_callbackOrigin = origin;
    }
};

UserTrigger* UserTrigger_test::m_callbackOrigin = nullptr;

TEST_F(UserTrigger_test, isNotTriggeredWhenCreated)
{
    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, canBeTriggeredWhenNotAttached)
{
    m_sut.trigger();
    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, canBeTriggeredMultipleTimesWhenNotAttached)
{
    m_sut.trigger();
    m_sut.trigger();
    m_sut.trigger();

    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, canBeTriggeredWhenAttached)
{
    m_waitSet.attachEvent(m_sut);
    m_sut.trigger();
    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, canBeTriggeredMultipleTimesWhenAttached)
{
    m_waitSet.attachEvent(m_sut);
    m_sut.trigger();
    m_sut.trigger();
    m_sut.trigger();

    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, resetTriggerWhenNotTriggeredIsNotTriggered)
{
    m_sut.resetTrigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, resetTriggerWhenTriggeredResultsInNotTriggered)
{
    m_waitSet.attachEvent(m_sut);
    m_sut.trigger();
    m_sut.resetTrigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, resetTriggerAndTriggerAgainResultsInTriggered)
{
    m_waitSet.attachEvent(m_sut);
    m_sut.trigger();
    m_sut.resetTrigger();
    m_sut.trigger();

    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, resetTriggerMultipleTimesWhenTriggeredResultsInNotTriggered)
{
    m_waitSet.attachEvent(m_sut);
    m_sut.trigger();
    m_sut.resetTrigger();
    m_sut.resetTrigger();
    m_sut.resetTrigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, UserTriggerGoesOutOfScopeCleansupAtWaitSet)
{
    {
        UserTrigger sut;
        m_waitSet.attachEvent(sut);
    }

    EXPECT_EQ(m_waitSet.size(), 0U);
}

TEST_F(UserTrigger_test, ReattachedUserTriggerCleansUpWhenOutOfScope)
{
    {
        UserTrigger sut;

        m_waitSet.attachEvent(sut);
        m_waitSet2.attachEvent(sut);
    }

    EXPECT_EQ(m_waitSet.size(), 0U);
    EXPECT_EQ(m_waitSet2.size(), 0U);
}

TEST_F(UserTrigger_test, AttachingToAnotherWaitSetCleansupFirstWaitset)
{
    UserTrigger sut;

    m_waitSet.attachEvent(m_sut);
    m_waitSet2.attachEvent(m_sut);

    EXPECT_EQ(m_waitSet.size(), 0U);
    EXPECT_EQ(m_waitSet2.size(), 1U);
}

TEST_F(UserTrigger_test, AttachingToSameWaitsetTwiceLeadsToOneAttachment)
{
    UserTrigger sut;

    m_waitSet.attachEvent(m_sut);
    m_waitSet.attachEvent(m_sut);

    EXPECT_EQ(m_waitSet.size(), 1U);
}

TEST_F(UserTrigger_test, TriggersWaitSet)
{
    using namespace iox::units::duration_literals;
    UserTrigger sut;

    m_waitSet.attachEvent(sut, 4412U);
    sut.trigger();

    auto result = m_waitSet.timedWait(1_s);
    ASSERT_THAT(result.size(), Eq(1U));
    EXPECT_THAT(result[0U]->getEventId(), 4412U);
}

TEST_F(UserTrigger_test, DetachingFromAttachedWaitsetCleansUp)
{
    UserTrigger sut;
    m_waitSet.attachEvent(sut);

    m_waitSet.detachEvent(sut);

    EXPECT_EQ(m_waitSet.size(), 0U);
}

TEST_F(UserTrigger_test, UserTriggerCallbackCanBeCalled)
{
    UserTrigger sut;
    m_waitSet.attachEvent(sut, 123U, &UserTrigger_test::callback);
    sut.trigger();

    auto triggerInfoVector = m_waitSet.wait();

    ASSERT_THAT(triggerInfoVector.size(), Eq(1));
    (*triggerInfoVector[0U])();
    EXPECT_THAT(m_callbackOrigin, &sut);
}

TEST_F(UserTrigger_test, UserTriggerCallbackCanBeCalledOverloadWithoutId)
{
    UserTrigger sut;
    m_waitSet.attachEvent(sut, 0U, &UserTrigger_test::callback);
    sut.trigger();

    auto triggerInfoVector = m_waitSet.wait();

    ASSERT_THAT(triggerInfoVector.size(), Eq(1U));
    (*triggerInfoVector[0U])();
    EXPECT_THAT(m_callbackOrigin, &sut);
}
