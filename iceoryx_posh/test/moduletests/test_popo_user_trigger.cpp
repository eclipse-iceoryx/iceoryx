// Copyright (c) 2020, 2021 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iox/duration.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::popo;

class WaitSetTest : public iox::popo::WaitSet<>
{
  public:
    WaitSetTest(iox::popo::ConditionVariableData& condVarData) noexcept
        : WaitSet(condVarData)
    {
    }
};

class UserTrigger_test : public Test
{
  public:
    UserTrigger m_sut;
    ConditionVariableData m_condVar{"Horscht"};
    ConditionVariableData m_condVar2{"Schnuppi"};
    WaitSetTest m_waitSet{m_condVar};
    WaitSetTest m_waitSet2{m_condVar2};

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

TEST_F(UserTrigger_test, IsNotTriggeredWhenCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "efe755c5-e84d-43a9-bbd9-7836133a119c");
    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, CannotBeTriggeredWhenNotAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad9d7d67-ef86-4d52-9752-1fdf9a8e12c6");
    m_sut.trigger();
    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, CannotBeTriggeredMultipleTimesWhenNotAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d921a26-0f29-499b-a6b5-d5175585e9e5");
    m_sut.trigger();
    m_sut.trigger();
    m_sut.trigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, CanBeTriggeredWhenAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "91c21a9f-4059-4b70-878a-d0ea17d56d1e");
    ASSERT_FALSE(m_waitSet.attachEvent(m_sut).has_error());
    m_sut.trigger();
    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, CanBeTriggeredMultipleTimesWhenAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "18440767-e3f9-4e8c-acf2-9875124289db");
    ASSERT_FALSE(m_waitSet.attachEvent(m_sut).has_error());
    m_sut.trigger();
    m_sut.trigger();
    m_sut.trigger();

    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(UserTrigger_test, UserTriggerGoesOutOfScopeCleansupAtWaitSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "99229230-90ee-45fc-b063-0ae6344d0308");
    {
        UserTrigger sut;
        ASSERT_FALSE(m_waitSet.attachEvent(sut).has_error());
    }

    EXPECT_EQ(m_waitSet.size(), 0U);
}

TEST_F(UserTrigger_test, ReattachedUserTriggerCleansUpWhenOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f3068c9-46a5-41ca-a967-953ca0e7116b");
    {
        UserTrigger sut;

        ASSERT_FALSE(m_waitSet.attachEvent(sut).has_error());
        ASSERT_FALSE(m_waitSet2.attachEvent(sut).has_error());
    }

    EXPECT_EQ(m_waitSet.size(), 0U);
    EXPECT_EQ(m_waitSet2.size(), 0U);
}

TEST_F(UserTrigger_test, AttachingToAnotherWaitSetCleansupFirstWaitset)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0f76d38-81a7-4ce3-8511-89391519ea8e");
    UserTrigger sut;

    ASSERT_FALSE(m_waitSet.attachEvent(m_sut).has_error());
    ASSERT_FALSE(m_waitSet2.attachEvent(m_sut).has_error());

    EXPECT_EQ(m_waitSet.size(), 0U);
    EXPECT_EQ(m_waitSet2.size(), 1U);
}

TEST_F(UserTrigger_test, AttachingToSameWaitsetTwiceLeadsToOneAttachment)
{
    ::testing::Test::RecordProperty("TEST_ID", "abf7d7cc-1409-4706-8669-0ea23e78ba26");
    UserTrigger sut;

    ASSERT_FALSE(m_waitSet.attachEvent(m_sut).has_error());
    ASSERT_TRUE(m_waitSet.attachEvent(m_sut).has_error());

    EXPECT_EQ(m_waitSet.size(), 1U);
}

TEST_F(UserTrigger_test, TriggersWaitSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "346dd5a8-a109-42ba-aab0-268dce37215f");
    using namespace iox::units::duration_literals;
    UserTrigger sut;

    ASSERT_FALSE(m_waitSet.attachEvent(sut, 4412U).has_error());
    sut.trigger();

    auto result = m_waitSet.timedWait(1_s);
    ASSERT_THAT(result.size(), Eq(1U));
    EXPECT_THAT(result[0U]->getNotificationId(), 4412U);
}

TEST_F(UserTrigger_test, DetachingFromAttachedWaitsetCleansUp)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3debaa9-f72d-49e1-82d9-bd1ff13530f7");
    UserTrigger sut;
    ASSERT_FALSE(m_waitSet.attachEvent(sut).has_error());

    m_waitSet.detachEvent(sut);

    EXPECT_EQ(m_waitSet.size(), 0U);
}

TEST_F(UserTrigger_test, UserTriggerCallbackCanBeCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "c95a5d7a-6de1-4419-9a19-0a1c6dd927d4");
    UserTrigger sut;
    ASSERT_FALSE(m_waitSet.attachEvent(sut, 123U, createNotificationCallback(UserTrigger_test::callback)).has_error());
    sut.trigger();

    auto triggerInfoVector = m_waitSet.wait();

    ASSERT_THAT(triggerInfoVector.size(), Eq(1));
    (*triggerInfoVector[0U])();
    EXPECT_THAT(m_callbackOrigin, &sut);
}

TEST_F(UserTrigger_test, UserTriggerCallbackCanBeCalledOverloadWithoutId)
{
    ::testing::Test::RecordProperty("TEST_ID", "c3af7155-5d27-468b-854b-0530f704cdf1");
    UserTrigger sut;
    ASSERT_FALSE(m_waitSet.attachEvent(sut, 0U, createNotificationCallback(UserTrigger_test::callback)).has_error());
    sut.trigger();

    auto triggerInfoVector = m_waitSet.wait();

    ASSERT_THAT(triggerInfoVector.size(), Eq(1U));
    (*triggerInfoVector[0U])();
    EXPECT_THAT(m_callbackOrigin, &sut);
}

} // namespace
