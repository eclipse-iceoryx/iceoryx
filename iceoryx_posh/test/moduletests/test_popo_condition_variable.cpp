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
#include "iceoryx_posh/internal/popo/waitset/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition_variable_signaler.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition_variable_waiter.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;

class ConditionVariable_test : public Test
{
  public:
    static constexpr size_t MEGABYTE = 1 << 20;
    static constexpr size_t MEMORY_SIZE = 1 * MEGABYTE;
    const uint64_t HISTORY_SIZE = 16;
    static constexpr uint32_t MAX_NUMBER_QUEUES = 128;
    char memory[MEMORY_SIZE];
    iox::posix::Allocator allocator{memory, MEMORY_SIZE};
    MemPool mempool{128, 20, &allocator, &allocator};
    MemPool chunkMgmtPool{128, 20, &allocator, &allocator};

    ConditionVariableData m_condVarData;
    ConditionVariableWaiter m_waiter{&m_condVarData};
    ConditionVariableSignaler m_signaler{&m_condVarData};

    void SetUp(){};
    void TearDown(){};
};

TEST_F(ConditionVariable_test, NoSignalResultsInWait)
{
}

TEST_F(ConditionVariable_test, SignalResultsInCall)
{
    // m_signaler.signal();
}

// TEST_F(ConditionVariable_test, ResetConditionSuccessful){}
// TEST_F(ConditionVariable_test, ResetConditionWithInvalidConditionResultsInFailure){}

// TEST_F(ConditionVariable_test, NotifyConditionSuccessful){}
// TEST_F(ConditionVariable_test, NotifyWithInvalidConditionResultsInFailure){}
// TEST_F(ConditionVariable_test, NotifyWithInvalidConditionStateResultsInFailure){}

// TEST_F(ConditionVariable_test, TimedWaitWithSignalAlreadySetResultsInImmediateTrigger){}
// TEST_F(ConditionVariable_test, SignalSetWhileWaitingInTimedWaitResultsInTrigger){}
// TEST_F(ConditionVariable_test, TimeoutOfTimedWaitResultsInTrigger){}
// TEST_F(ConditionVariable_test, TimedWaitWithInvalidConditionResultsInFailure){}
// TEST_F(ConditionVariable_test, TimedWaitWithInvalidTimeResultsInFailure){}
// TEST_F(ConditionVariable_test, TimedWaitWithInvalidConditionStateResultsInFailure){}

// TEST_F(ConditionVariable_test, WaitWithoutSignalResultsInBlocking){}
// TEST_F(ConditionVariable_test, WaitWithSignalAlreadySetResultsInImmediateTrigger){}
// TEST_F(ConditionVariable_test, SignalSetWhileWaitWaitResultsInTrigger){}
// TEST_F(ConditionVariable_test, WaitWithInvalidConditionResultsInBlocking){}
// TEST_F(ConditionVariable_test, WaitWithInvalidConditionStateResultsInFailure){}
