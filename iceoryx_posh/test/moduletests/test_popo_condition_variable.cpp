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

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_signaler.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_waiter.hpp"

#include "test.hpp"
#include <atomic>
#include <memory>
#include <thread>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::units::duration_literals;

class ConditionVariable_test : public Test
{
  public:
    ConditionVariableData m_condVarData;
    ConditionVariableWaiter m_waiter{&m_condVarData};
    ConditionVariableSignaler m_signaler{&m_condVarData};

    iox::posix::Semaphore m_syncSemaphore = iox::posix::Semaphore::create(0u).get_value();

    void SetUp(){};
    void TearDown()
    {
        // Reset condition variable
        m_waiter.reset();
    };
};

TEST_F(ConditionVariable_test, TimedWaitWithInvalidTimeResultsInFailure)
{
    EXPECT_FALSE(m_waiter.timedWait(0_ms));
}

TEST_F(ConditionVariable_test, NoNotifyResultsInTimeoutSingleThreaded)
{
    EXPECT_FALSE(m_waiter.timedWait(10_ms));
}

TEST_F(ConditionVariable_test, NotifyOnceResultsInNoTimeoutSingleThreaded)
{
    m_signaler.notifyOne();
    EXPECT_TRUE(m_waiter.timedWait(10_ms));
}

TEST_F(ConditionVariable_test, NotifyOnceResultsInNoWaitSingleThreaded)
{
    m_signaler.notifyOne();
    m_waiter.wait();
    // We expect that the next line is reached
    EXPECT_TRUE(true);
}

TEST_F(ConditionVariable_test, NotifyTwiceResultsInNoWaitSingleThreaded)
{
    m_signaler.notifyOne();
    m_signaler.notifyOne();
    m_waiter.wait();
    m_waiter.wait();
    // We expect that the next line is reached
    EXPECT_TRUE(true);
}

TEST_F(ConditionVariable_test, WaitAndNotifyResultsInImmediateTriggerMultiThreaded)
{
    std::atomic<int> counter{0};
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        m_waiter.wait();
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notifyOne();
    waiter.join();
}

TEST_F(ConditionVariable_test, ResetResultsInBlockingWaitMultiThreaded)
{
    std::atomic<int> counter{0};
    m_signaler.notifyOne();
    m_waiter.reset();
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        m_waiter.wait();
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notifyOne();
    waiter.join();
}

TEST_F(ConditionVariable_test, ResetWithoutNotifiyResultsInBlockingWaitMultiThreaded)
{
    std::atomic<int> counter{0};
    m_waiter.reset();
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        m_waiter.wait();
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notifyOne();
    waiter.join();
}

TEST_F(ConditionVariable_test, NotifyWhileWaitingResultsNoTimeoutMultiThreaded)
{
    std::atomic<int> counter{0};
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        EXPECT_TRUE(m_waiter.timedWait(10_ms));
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notifyOne();
    waiter.join();
}

TEST_F(ConditionVariable_test, DISABLED_MoveConditionVariableSignalerIsSuccessful)
{
    /// @todo move c'tor currently deleted
}

TEST_F(ConditionVariable_test, DISABLED_MoveConditionVariableWaiterIsSuccessful)
{
    /// @todo move c'tor currently deleted
}

TEST_F(ConditionVariable_test, DISABLED_MoveAssignConditionVariableSignalerIsSuccessful)
{
    /// @todo move assign currently deleted
}

TEST_F(ConditionVariable_test, DISABLED_MoveAssignConditionVariableWaiterIsSuccessful)
{
    /// @todo move assign currently deleted
}
