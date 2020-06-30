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

#include "iceoryx_posh/internal/popo/waitset/condition.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/waitset/guard_condition.hpp"
#include "iceoryx_posh/internal/popo/waitset/wait_set.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "mocks/posh_runtime_mock.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;

class MockSubscriber : public Condition
{
  public:
    bool isConditionVariableAttached() noexcept
    {
    }

    bool attachConditionVariable(ConditionVariableData* ConditionVariableDataPtr) noexcept
    {
    }

    bool detachConditionVariable() noexcept
    {
    }

    bool notify()
    {
    }
};

class WaitSet_test : public Test
{
  public:
    static constexpr uint16_t MAX_NUMBER_OF_CONDITIONS_WITHOUT_GUARD = iox::popo::MAX_NUMBER_OF_CONDITIONS - 1;

    MockSubscriber m_subscriber;
    ConditionVariableData m_condVarData;
    WaitSet m_sut;
    vector<MockSubscriber, MAX_NUMBER_OF_CONDITIONS_WITHOUT_GUARD> m_subscriberVector;

    void SetUp()
    {
        for (auto currentSubscriber : m_subscriberVector)
        {
            m_subscriberVector.push_back(m_subscriber);
        }
    };

    void TearDown()
    {
        m_subscriberVector.clear();
    };
};

// TEST_F(WaitSet_test, NoAttachResultsInError)
// {
// }

TEST_F(WaitSet_test, AttachSingleConditionSuccessful)
{
    EXPECT_TRUE(m_sut.attachCondition(m_subscriberVector.front()));
}

TEST_F(WaitSet_test, AttachSameConditionTwiceResultsInOneFailure)
{
    m_sut.attachCondition(m_subscriberVector.front());
    EXPECT_FALSE(m_sut.attachCondition(m_subscriberVector.front()));
}

TEST_F(WaitSet_test, AttachMultipleConditionSuccessful)
{
    for (auto currentSubscriber : m_subscriberVector)
    {
        EXPECT_TRUE(m_sut.attachCondition(currentSubscriber));
    }
}

TEST_F(WaitSet_test, AttachTooManyConditionsResultsInFailure)
{
    for (auto currentSubscriber : m_subscriberVector)
    {
        m_sut.attachCondition(currentSubscriber);
    }

    Condition extraCondition;
    EXPECT_FALSE(m_sut.attachCondition(extraCondition));
}

TEST_F(WaitSet_test, DetachSingleConditionSuccessful)
{
    m_sut.attachCondition(m_subscriberVector.front());
    EXPECT_TRUE(m_sut.detachCondition(m_subscriberVector.front()));
}

TEST_F(WaitSet_test, DetachMultipleleConditionsSuccessful)
{
    for (auto currentSubscriber : m_subscriberVector)
    {
        m_sut.attachCondition(currentSubscriber);
    }
    for (auto currentSubscriber : m_subscriberVector)
    {
        EXPECT_TRUE(m_sut.detachCondition(currentSubscriber));
    }
}

TEST_F(WaitSet_test, DetachConditionNotInListResultsInFailure)
{
    EXPECT_FALSE(m_sut.detachCondition(m_subscriberVector.front()));
}

TEST_F(WaitSet_test, DetachUnknownConditionResultsInFailure)
{
    m_sut.attachCondition(m_subscriberVector.front());
    EXPECT_FALSE(m_sut.detachCondition(m_subscriberVector.back()));
}

// TEST_F(WaitSet_test, TimedWaitWithSignalAlreadySetResultsInImmediateTrigger){}
// TEST_F(WaitSet_test, SignalSetWhileWaitingInTimedWaitResultsInTrigger){}
// TEST_F(WaitSet_test, TimeoutOfTimedWaitResultsInTrigger){}
// TEST_F(WaitSet_test, TimedWaitWithInvalidConditionResultsInFailure){}
// TEST_F(WaitSet_test, TimedWaitWithInvalidTimeResultsInFailure){}
// TEST_F(WaitSet_test, TimeoutOfTimedWaitResultsInTrigger){}

// TEST_F(WaitSet_test, WaitWithoutSignalResultsInBlocking){}
// TEST_F(WaitSet_test, WaitWithSignalAlreadySetResultsInImmediateTrigger){}
// TEST_F(WaitSet_test, SignalSetWhileWaitWaitResultsInTrigger){}
// TEST_F(WaitSet_test, WaitWithInvalidConditionResultsInBlocking){}
