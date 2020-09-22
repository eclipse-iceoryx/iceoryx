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

#include "iceoryx_posh/popo/guard_condition.hpp"
#include "mocks/wait_set_mock.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/guard_condition.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

using namespace ::testing;

class iox_guard_cond_test : public Test
{
  public:
    void SetUp()
    {
        m_sut = iox_guard_cond_init(&m_sutStorage);
    }

    void TearDown()
    {
        iox_guard_cond_deinit(m_sut);
    }

    iox_guard_cond_storage_t m_sutStorage;
    iox_guard_cond_t m_sut;

    ConditionVariableData m_condVar;
    WaitSetMock m_waitSet{&m_condVar};
};

TEST_F(iox_guard_cond_test, isNotTriggeredWhenCreated)
{
    EXPECT_FALSE(iox_guard_cond_has_triggered(m_sut));
}

TEST_F(iox_guard_cond_test, cannotBeTriggeredWhenNotAttached)
{
    iox_guard_cond_trigger(m_sut);
    EXPECT_FALSE(iox_guard_cond_has_triggered(m_sut));
}

TEST_F(iox_guard_cond_test, canBeTriggeredWhenAttached)
{
    iox_ws_attach_condition(&m_waitSet, m_sut);
    iox_guard_cond_trigger(m_sut);
    EXPECT_TRUE(iox_guard_cond_has_triggered(m_sut));
    iox_ws_detach_all_conditions(&m_waitSet);
}

TEST_F(iox_guard_cond_test, resetTriggerWhenNotTriggeredIsNotTriggered)
{
    iox_guard_cond_reset_trigger(m_sut);
    EXPECT_FALSE(iox_guard_cond_has_triggered(m_sut));
}

TEST_F(iox_guard_cond_test, resetTriggerWhenTriggeredIsResultsInTriggered)
{
    iox_ws_attach_condition(&m_waitSet, m_sut);
    iox_guard_cond_trigger(m_sut);
    iox_guard_cond_reset_trigger(m_sut);
    EXPECT_FALSE(iox_guard_cond_has_triggered(m_sut));
    iox_ws_detach_all_conditions(&m_waitSet);
}
