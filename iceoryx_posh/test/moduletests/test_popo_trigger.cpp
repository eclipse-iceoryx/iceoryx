// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/trigger.hpp"

#include "test.hpp"
#include <gtest/gtest-param-test.h>

using namespace iox;
using namespace iox::popo;
using namespace ::testing;

typedef TriggerState* CreateBaseTrigger();

namespace internalTesting
{
void* originValue;
uint64_t triggerIdValue = 0U;
TriggerState::Callback<void> callbackValue = nullptr;
cxx::ConstMethodCallback<bool> hasTriggerCallbackValue;
cxx::MethodCallback<void, const Trigger&> resetCallbackValue;
ConditionVariableData conditionVariableDataValue;

int triggerCallbackValueSetter = 0;

void triggerCallback(int* const origin)
{
    *origin = triggerCallbackValueSetter;
}

template <typename T>
TriggerState* createTriggerState()
{
    return new TriggerState(
        static_cast<T*>(originValue), triggerIdValue, reinterpret_cast<TriggerState::Callback<T>>(callbackValue));
}

template <typename T>
TriggerState* createTrigger()
{
    return new Trigger(static_cast<T*>(originValue),
                       &conditionVariableDataValue,
                       hasTriggerCallbackValue,
                       resetCallbackValue,
                       triggerIdValue,
                       reinterpret_cast<TriggerState::Callback<T>>(callbackValue));
}

class TriggerStateInheritance_test : public TestWithParam<CreateBaseTrigger*>
{
  public:
    TriggerStateInheritance_test()
    {
    }

    ~TriggerStateInheritance_test()
    {
        delete m_sut;
    }

    void createSut()
    {
        m_sut = (*GetParam())();
    }

    TriggerState* m_sut{nullptr};
};


TEST_P(TriggerStateInheritance_test, getTriggerIdReturnsValidTriggerId)
{
    triggerIdValue = 1234;
    createSut();

    EXPECT_EQ(m_sut->getTriggerId(), 1234);
}

TEST_P(TriggerStateInheritance_test, doesOriginateFromStatesOriginCorrectly)
{
    int bla;
    float fuu;
    originValue = &bla;
    createSut();

    EXPECT_EQ(m_sut->doesOriginateFrom(&bla), true);
    EXPECT_EQ(m_sut->doesOriginateFrom(&fuu), false);
}

TEST_P(TriggerStateInheritance_test, getOriginReturnsCorrectOriginWhenHavingCorrectType)
{
    int bla;
    originValue = &bla;
    createSut();

    EXPECT_EQ(m_sut->getOrigin<int>(), &bla);
    EXPECT_EQ(const_cast<const TriggerState*>(m_sut)->getOrigin<int>(), &bla);
}

TEST_P(TriggerStateInheritance_test, triggerCallbackIsWorking)
{
    int bla;
    originValue = &bla;
    triggerCallbackValueSetter = 4242;
    callbackValue = reinterpret_cast<TriggerState::Callback<void>>(triggerCallback);
    createSut();

    EXPECT_TRUE((*m_sut)());
    EXPECT_EQ(bla, 4242);
}

INSTANTIATE_TEST_CASE_P(TriggerStateAndChilds,
                        TriggerStateInheritance_test,
                        Values(&createTriggerState<int>, &createTrigger<int>));

class TriggerState_test : public Test
{
  public:
    virtual void SetUp()
    {
        internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    TriggerState m_sut;
};

TEST_F(TriggerState_test, DefaultCTorConstructsEmptyTriggerState)
{
    int bla;
    TriggerState sut;

    EXPECT_EQ(sut.getTriggerId(), TriggerState::INVALID_TRIGGER_ID);
    EXPECT_EQ(sut.doesOriginateFrom(&bla), false);
    EXPECT_EQ(sut.getOrigin<void>(), nullptr);
    EXPECT_EQ(const_cast<const TriggerState&>(sut).getOrigin<void>(), nullptr);
    EXPECT_EQ(sut(), false);
}

} // namespace internalTesting
