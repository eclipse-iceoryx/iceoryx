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

#include "iceoryx_posh/popo/user_trigger.hpp"
#include "mocks/wait_set_mock.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

using namespace ::testing;

class iox_user_trigger_test : public Test
{
  public:
    void SetUp()
    {
        m_sut = iox_user_trigger_init(&m_sutStorage);
        wasTriggerCallbackCalled = false;
    }

    void TearDown()
    {
        iox_user_trigger_deinit(m_sut);
    }

    static void triggerCallback(iox_user_trigger_t)
    {
        wasTriggerCallbackCalled = true;
    }

    iox_user_trigger_storage_t m_sutStorage;
    iox_user_trigger_t m_sut;

    ConditionVariableData m_condVar;
    WaitSetMock m_waitSet{&m_condVar};
    static bool wasTriggerCallbackCalled;
};

bool iox_user_trigger_test::wasTriggerCallbackCalled = false;

TEST_F(iox_user_trigger_test, isNotTriggeredWhenCreated)
{
    EXPECT_FALSE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, canBeTriggeredWhenNotAttached)
{
    iox_user_trigger_trigger(m_sut);
    EXPECT_TRUE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, canBeTriggeredWhenAttached)
{
    iox_user_trigger_attach_to_waitset(m_sut, &m_waitSet, 0, NULL);
    iox_user_trigger_trigger(m_sut);
    EXPECT_TRUE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, resetTriggerWhenNotTriggeredIsNotTriggered)
{
    iox_user_trigger_reset_trigger(m_sut);
    EXPECT_FALSE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, resetTriggerWhenTriggeredIsResultsInNotTriggered)
{
    iox_user_trigger_attach_to_waitset(m_sut, &m_waitSet, 0, NULL);
    iox_user_trigger_trigger(m_sut);
    iox_user_trigger_reset_trigger(m_sut);
    EXPECT_FALSE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, triggeringWaitSetResultsInCorrectTriggerId)
{
    iox_user_trigger_attach_to_waitset(m_sut, &m_waitSet, 88191, NULL);
    iox_user_trigger_trigger(m_sut);

    auto triggerVector = m_waitSet.wait();

    ASSERT_THAT(triggerVector.size(), Eq(1));
    EXPECT_EQ(triggerVector[0].getTriggerId(), 88191);
}

TEST_F(iox_user_trigger_test, triggeringWaitSetResultsInCorrectCallback)
{
    iox_user_trigger_attach_to_waitset(m_sut, &m_waitSet, 0, iox_user_trigger_test::triggerCallback);
    iox_user_trigger_trigger(m_sut);

    auto triggerVector = m_waitSet.wait();

    ASSERT_THAT(triggerVector.size(), Eq(1));
    triggerVector[0]();

    EXPECT_TRUE(wasTriggerCallbackCalled);
}

TEST_F(iox_user_trigger_test, attachingToAnotherWaitSetCleansupFirstWaitset)
{
    WaitSetMock m_waitSet2{&m_condVar};
    iox_user_trigger_attach_to_waitset(m_sut, &m_waitSet, 0, NULL);

    iox_user_trigger_attach_to_waitset(m_sut, &m_waitSet2, 0, NULL);

    EXPECT_EQ(m_waitSet.size(), 0);
    EXPECT_EQ(m_waitSet2.size(), 1);
}

TEST_F(iox_user_trigger_test, detachingItFromWaitsetCleansup)
{
    WaitSetMock m_waitSet2{&m_condVar};
    iox_user_trigger_attach_to_waitset(m_sut, &m_waitSet, 0, NULL);

    iox_user_trigger_detach(m_sut);

    EXPECT_EQ(m_waitSet.size(), 0);
}

