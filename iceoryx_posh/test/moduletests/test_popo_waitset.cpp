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
#include "test.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::units::duration_literals;

class MockSubscriber : public Condition
{
  public:
    bool isConditionVariableAttached() noexcept override
    {
        return m_condVarAttached;
    }

    bool attachConditionVariable(ConditionVariableData* ConditionVariableDataPtr) noexcept override
    {
        m_condVarAttached = true;
        m_condVarPtr = ConditionVariableDataPtr;
        return m_condVarAttached;
    }

    bool hasTrigger() noexcept override
    {
        return m_wasTriggered;
    }

    void resetTrigger() noexcept override
    {
        m_wasTriggered = false;
    }

    bool detachConditionVariable() noexcept override
    {
        m_condVarAttached = false;
        return m_condVarAttached;
    }

    /// @note done in ChunkQueuePusher
    void notify()
    {
        m_wasTriggered = true;
        ConditionVariableSignaler signaler{m_condVarPtr};
        signaler.notifyOne();
    }

    /// @note members reside in ChunkQueueData in SHM
    bool m_condVarAttached{false};
    bool m_wasTriggered{false};
    ConditionVariableData* m_condVarPtr{nullptr};
};

class WaitSet_test : public Test
{
  public:
    static constexpr uint16_t MAX_NUMBER_OF_CONDITIONS_WITHOUT_GUARD = iox::popo::MAX_NUMBER_OF_CONDITIONS - 1;

    ConditionVariableData m_condVarData;
    WaitSet m_sut{&m_condVarData};
    vector<MockSubscriber, MAX_NUMBER_OF_CONDITIONS_WITHOUT_GUARD> m_subscriberVector;

    iox::posix::Semaphore m_syncSemaphore = iox::posix::Semaphore::create(0u).get_value();

    void SetUp()
    {
        MockSubscriber subscriber;
        while (m_subscriberVector.size() != m_subscriberVector.capacity())
        {
            m_subscriberVector.push_back(subscriber);
        }
    };

    void TearDown()
    {
        m_subscriberVector.clear();
        m_sut.clear();
        ConditionVariableWaiter waiter{&m_condVarData};
        waiter.reset();
    };
};

TEST_F(WaitSet_test, AttachSingleConditionSuccessful)
{
    EXPECT_TRUE(m_sut.attachCondition(m_subscriberVector.front()));
}

TEST_F(WaitSet_test, AttachSameConditionTwiceResultsInFailure)
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

TEST_F(WaitSet_test, TimedWaitWithInvalidTimeResultsInEmptyVector)
{
    auto emptyVector = m_sut.timedWait(0_ms);
    EXPECT_TRUE(emptyVector.empty());
}

TEST_F(WaitSet_test, NoAttachTimedWaitResultsInEmptyVector)
{
    auto emptyVector = m_sut.timedWait(1_ms);
    EXPECT_TRUE(emptyVector.empty());
}

TEST_F(WaitSet_test, TimedWaitWithNotificationResultsInImmediateTrigger)
{
    m_sut.attachCondition(m_subscriberVector.front());
    m_subscriberVector.front().notify();
    auto fulfilledConditions = m_sut.timedWait(1_ms);
    EXPECT_THAT(fulfilledConditions.size(), Eq(1));
    EXPECT_THAT(fulfilledConditions.front(), &m_subscriberVector.front());
}

TEST_F(WaitSet_test, TimeoutOfTimedWaitResultsInEmptyVector)
{
    m_sut.attachCondition(m_subscriberVector.front());
    auto fulfilledConditions = m_sut.timedWait(1_ms);
    EXPECT_THAT(fulfilledConditions.size(), Eq(0));
}

TEST_F(WaitSet_test, NotifyOneWhileWaitingResultsInTriggerMultiThreaded)
{
    std::atomic<int> counter{0};
    m_sut.attachCondition(m_subscriberVector.front());
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        auto fulfilledConditions = m_sut.wait();
        EXPECT_THAT(fulfilledConditions.size(), Eq(1));
        EXPECT_THAT(fulfilledConditions.front(), &m_subscriberVector.front());
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_subscriberVector.front().notify();
    waiter.join();
}

TEST_F(WaitSet_test, AttachManyNotifyOneWhileWaitingResultsInTriggerMultiThreaded)
{
    std::atomic<int> counter{0};
    m_sut.attachCondition(m_subscriberVector[0]);
    m_sut.attachCondition(m_subscriberVector[1]);
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        auto fulfilledConditions = m_sut.wait();
        EXPECT_THAT(fulfilledConditions.size(), Eq(1));
        EXPECT_THAT(fulfilledConditions.front(), &m_subscriberVector[0]);
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_subscriberVector[0].notify();
    waiter.join();
}

TEST_F(WaitSet_test, AttachManyNotifyManyBeforeWaitingResultsInTriggerMultiThreaded)
{
    std::atomic<int> counter{0};
    m_sut.attachCondition(m_subscriberVector[0]);
    m_sut.attachCondition(m_subscriberVector[1]);
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_syncSemaphore.wait();
        auto fulfilledConditions = m_sut.wait();
        EXPECT_THAT(fulfilledConditions.size(), Eq(2));
        EXPECT_THAT(fulfilledConditions[0], &m_subscriberVector[0]);
        EXPECT_THAT(fulfilledConditions[1], &m_subscriberVector[1]);
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    m_subscriberVector[0].notify();
    m_subscriberVector[1].notify();
    counter++;
    m_syncSemaphore.post();
    waiter.join();
}

TEST_F(WaitSet_test, AttachManyNotifyManyWhileWaitingResultsInTriggerMultiThreaded)
{
    std::atomic<int> counter{0};
    m_sut.attachCondition(m_subscriberVector[0]);
    m_sut.attachCondition(m_subscriberVector[1]);
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        auto fulfilledConditions = m_sut.wait();
        EXPECT_THAT(fulfilledConditions.size(), Eq(2));
        EXPECT_THAT(fulfilledConditions[0], &m_subscriberVector[0]);
        EXPECT_THAT(fulfilledConditions[1], &m_subscriberVector[1]);
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    m_subscriberVector[0].notify();
    m_subscriberVector[1].notify();
    counter++;
    waiter.join();
}


TEST_F(WaitSet_test, WaitWithoutNotifyResultsInBlockingMultiThreaded)
{
    std::atomic<int> counter{0};
    m_sut.attachCondition(m_subscriberVector.front());
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        m_sut.wait();
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_subscriberVector.front().notify();
    waiter.join();
}

TEST_F(WaitSet_test, NotifyGuardConditionWhileWaitingResultsInTriggerMultiThreaded)
{
    std::atomic<int> counter{0};
    auto& guardCond = m_sut.getGuardCondition();
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        auto fulfilledConditions = m_sut.wait();
        EXPECT_THAT(fulfilledConditions.size(), Eq(1));
        EXPECT_THAT(fulfilledConditions.front(), &guardCond);
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    guardCond.notify();
    waiter.join();
}

TEST_F(WaitSet_test, NotifyGuardConditionOnceTimedWaitResultsInResetOfTrigger)
{
    auto& guardCond = m_sut.getGuardCondition();
    guardCond.notify();
    auto fulfilledConditions1 = m_sut.timedWait(1_ms);
    EXPECT_THAT(fulfilledConditions1.size(), Eq(1));
    EXPECT_THAT(fulfilledConditions1.front(), &guardCond);
    auto fulfilledConditions2 = m_sut.timedWait(1_ms);
    EXPECT_THAT(fulfilledConditions2.size(), Eq(0));
}
