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

#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "mocks/wait_set_mock.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/condition.h"
}

#include "test.hpp"

using namespace ::testing;

class iox_cond_test : public Test
{
  public:
    class : public Condition
    {
      public:
        bool setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept override
        {
            setConditionVariableArgument = conditionVariableDataPtr;
            return setConditionVariableReturn;
        }

        bool hasTriggered() const noexcept override
        {
            return hasTriggeredReturn;
        }

        bool unsetConditionVariable() noexcept override
        {
            return unsetConditionVariableReturn;
        }

        bool hasTriggeredReturn = true;
        bool unsetConditionVariableReturn = true;
        bool setConditionVariableReturn = true;

        ConditionVariableData* setConditionVariableArgument = nullptr;
    } sut;
};

TEST_F(iox_cond_test, HasTriggered)
{
    sut.hasTriggeredReturn = false;
    EXPECT_FALSE(iox_cond_has_triggered(&sut));

    sut.hasTriggeredReturn = true;
    EXPECT_TRUE(iox_cond_has_triggered(&sut));
}

TEST_F(iox_cond_test, ConditionVariableNotAttachedAfterConstruction)
{
    EXPECT_FALSE(iox_cond_is_condition_variable_attached(&sut));
}

TEST_F(iox_cond_test, AttachingConditionVariable)
{
    ConditionVariableData data;
    WaitSetMock ws(&data);
    ws.attachCondition(sut);

    EXPECT_TRUE(iox_cond_is_condition_variable_attached(&sut));
}
