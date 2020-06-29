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

#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition_variable_signaler.hpp"
#include "iceoryx_posh/internal/popo/waitset/guard_condition.hpp"
#include "iceoryx_posh/internal/popo/waitset/wait_set.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;

class WaitSet_test : public Test
{
  public:
    static constexpr uint16_t MAX_NUMBER_OF_CONDITIONS_WITHOUT_GUARD = iox::popo::MAX_NUMBER_OF_CONDITIONS - 1;
    static constexpr size_t MEGABYTE = 1 << 20;
    static constexpr size_t MEMORY_SIZE = 1 * MEGABYTE;
    const uint64_t HISTORY_SIZE = 16;
    static constexpr uint32_t MAX_NUMBER_QUEUES = 128;
    char memory[MEMORY_SIZE];
    iox::posix::Allocator allocator{memory, MEMORY_SIZE};
    MemPool mempool{128, 20, &allocator, &allocator};
    MemPool chunkMgmtPool{128, 20, &allocator, &allocator};

    Condition m_condition;
    ConditionVariableData m_condVarData;
    WaitSet m_waitset;
    ConditionVariableSignaler m_signaler{&m_condVarData};
    vector<Condition, MAX_NUMBER_OF_CONDITIONS_WITHOUT_GUARD> m_conditionVector;

    void SetUp()
    {
        m_condition.attachConditionVariable(&m_condVarData);
        for (auto currentCondition : m_conditionVector)
        {
            m_conditionVector.push_back(m_condition);
        }
    };
    void TearDown()
    {
        m_conditionVector.clear();
    };
};

// TEST_F(WaitSet_test, NoAttachResultsInError)
// {
// }

TEST_F(WaitSet_test, AttachSingleConditionSuccessful)
{
    EXPECT_TRUE(m_waitset.attachCondition(m_conditionVector.front()));
}

TEST_F(WaitSet_test, AttachSameConditionTwiceResultsInOneFailure)
{
    m_waitset.attachCondition(m_conditionVector.front());
    EXPECT_FALSE(m_waitset.attachCondition(m_conditionVector.front()));
}

TEST_F(WaitSet_test, AttachMultipleConditionSuccessful)
{
    for (auto currentCondition : m_conditionVector)
    {
        EXPECT_TRUE(m_waitset.attachCondition(currentCondition));
    }
}

TEST_F(WaitSet_test, AttachTooManyConditionsResultsInFailure)
{
    for (auto currentCondition : m_conditionVector)
    {
        m_waitset.attachCondition(currentCondition);
    }

    Condition extraCondition;
    EXPECT_FALSE(m_waitset.attachCondition(extraCondition));
}

TEST_F(WaitSet_test, DetachSingleConditionSuccessful)
{
    m_waitset.attachCondition(m_conditionVector.front());
    EXPECT_TRUE(m_waitset.detachCondition(m_conditionVector.front()));
}

TEST_F(WaitSet_test, DetachMultipleleConditionsSuccessful)
{
    for (auto currentCondition : m_conditionVector)
    {
        m_waitset.attachCondition(currentCondition);
    }
    for (auto currentCondition : m_conditionVector)
    {
        EXPECT_TRUE(m_waitset.detachCondition(currentCondition));
    }
}

TEST_F(WaitSet_test, DetachConditionNotInListResultsInFailure)
{
    EXPECT_FALSE(m_waitset.detachCondition(m_conditionVector.front()));
}

TEST_F(WaitSet_test, DetachUnknownConditionResultsInFailure)
{
    m_waitset.attachCondition(m_conditionVector.front());
    EXPECT_FALSE(m_waitset.detachCondition(m_conditionVector.back()));
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
