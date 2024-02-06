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

#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "mocks/wait_set_mock.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::testing;

extern "C" {
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

namespace
{
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

    ConditionVariableData m_condVar{"Horscht"};
    WaitSetMock m_waitSet{m_condVar};
    static bool wasTriggerCallbackCalled;
};

bool iox_user_trigger_test::wasTriggerCallbackCalled = false;

TEST_F(iox_user_trigger_test, initUserTriggerWithNullptrForStorageReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1529267-af64-43eb-a0f8-4db6a8557b6e");
    EXPECT_EQ(iox_user_trigger_init(nullptr), nullptr);
}

TEST_F(iox_user_trigger_test, isNotTriggeredWhenCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "10fbcb9f-f9ef-4886-b154-757f62a5ec2f");
    EXPECT_FALSE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, cannotBeTriggeredWhenNotAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "d988e34b-8b45-4dcb-b663-32eee5f9d9df");
    iox_user_trigger_trigger(m_sut);
    EXPECT_FALSE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, canBeTriggeredWhenAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "d48b92b0-ab26-4a36-9c83-68699ca3e1b0");
    iox_ws_attach_user_trigger_event(&m_waitSet, m_sut, 0U, triggerCallback);
    iox_user_trigger_trigger(m_sut);
    EXPECT_TRUE(iox_user_trigger_has_triggered(m_sut));
}

TEST_F(iox_user_trigger_test, triggeringWaitSetResultsInCorrectNotificationId)
{
    ::testing::Test::RecordProperty("TEST_ID", "03858b17-f08c-4fba-b973-03a651fcb3c6");
    iox_ws_attach_user_trigger_event(&m_waitSet, m_sut, 88191U, triggerCallback);
    iox_user_trigger_trigger(m_sut);

    auto eventVector = m_waitSet.wait();

    ASSERT_THAT(eventVector.size(), Eq(1U));
    EXPECT_EQ(eventVector[0U]->getNotificationId(), 88191U);
}

TEST_F(iox_user_trigger_test, triggeringWaitSetResultsInCorrectCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "cfb59955-a3dd-4514-805d-9718072bd99b");
    iox_ws_attach_user_trigger_event(&m_waitSet, m_sut, 0U, iox_user_trigger_test::triggerCallback);
    iox_user_trigger_trigger(m_sut);

    auto eventVector = m_waitSet.wait();

    ASSERT_THAT(eventVector.size(), Eq(1U));
    (*eventVector[0U])();

    EXPECT_TRUE(wasTriggerCallbackCalled);
}

TEST_F(iox_user_trigger_test, attachingToAnotherWaitSetCleansupFirstWaitset)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fb7b119-b0ca-4bcc-9776-189a4468822e");
    WaitSetMock m_waitSet2{m_condVar};
    iox_ws_attach_user_trigger_event(&m_waitSet, m_sut, 0U, triggerCallback);

    iox_ws_attach_user_trigger_event(&m_waitSet2, m_sut, 0U, triggerCallback);

    EXPECT_EQ(m_waitSet.size(), 0U);
    EXPECT_EQ(m_waitSet2.size(), 1U);
}

TEST_F(iox_user_trigger_test, disable_trigger_eventingItFromWaitsetCleansup)
{
    ::testing::Test::RecordProperty("TEST_ID", "10d8d416-57f5-4c9f-aa71-7ee917e3d97e");
    WaitSetMock m_waitSet2{m_condVar};
    iox_ws_attach_user_trigger_event(&m_waitSet, m_sut, 0U, triggerCallback);

    iox_ws_detach_user_trigger_event(&m_waitSet, m_sut);

    EXPECT_EQ(m_waitSet.size(), 0U);
}

TEST_F(iox_user_trigger_test, userTriggerDeinitWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f418a98-c3d5-4dc7-a550-21e0d2f6adee");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_user_trigger_deinit(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_user_trigger_test, userTriggerTriggerWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "da67d02e-b801-40be-b640-c3aaabc4b3a5");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_user_trigger_trigger(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_user_trigger_test, userTriggerHasTriggeredWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b79eb0f-6102-402e-b55c-f339e2eb9b77");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_user_trigger_has_triggered(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

} // namespace
