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

#include "test.hpp"

using namespace ::testing;
using namespace iox;
using namespace iox::popo;

class GuardCondition_test : public Test
{
  public:
    GuardCondition m_sut;
    ConditionVariableData m_condVar;
    WaitSetMock m_waitSet{&m_condVar};
};

TEST_F(GuardCondition_test, isNotTriggeredWhenCreated)
{
    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(GuardCondition_test, cannotBeTriggeredWhenNotAttached)
{
    m_sut.trigger();
    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(GuardCondition_test, cannotBeTriggeredMultipleTimesWhenNotAttached)
{
    m_sut.trigger();
    m_sut.trigger();
    m_sut.trigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(GuardCondition_test, canBeTriggeredWhenAttached)
{
    m_waitSet.attachCondition(m_sut);
    m_sut.trigger();
    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(GuardCondition_test, canBeTriggeredMultipleTimesWhenAttached)
{
    m_waitSet.attachCondition(m_sut);
    m_sut.trigger();
    m_sut.trigger();
    m_sut.trigger();

    EXPECT_TRUE(m_sut.hasTriggered());
}

TEST_F(GuardCondition_test, resetTriggerWhenNotTriggeredIsNotTriggered)
{
    m_sut.resetTrigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(GuardCondition_test, resetTriggerWhenTriggeredResultsInNotTriggered)
{
    m_waitSet.attachCondition(m_sut);
    m_sut.trigger();
    m_sut.resetTrigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}

TEST_F(GuardCondition_test, resetTriggerMultipleTimesWhenTriggeredResultsInNotTriggered)
{
    m_waitSet.attachCondition(m_sut);
    m_sut.trigger();
    m_sut.resetTrigger();
    m_sut.resetTrigger();
    m_sut.resetTrigger();

    EXPECT_FALSE(m_sut.hasTriggered());
}
