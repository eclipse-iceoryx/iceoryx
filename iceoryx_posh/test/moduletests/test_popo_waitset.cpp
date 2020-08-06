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
#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_posh/popo/guard_condition.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "test.hpp"
#include "timing_test.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::units::duration_literals;

class MockSubscriber : public Condition
{
  public:
    bool setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept override
    {
        m_condVarPtr = conditionVariableDataPtr;
        return true;
    }

    bool hasTriggered() const noexcept override
    {
        return m_wasTriggered;
    }

    bool unsetConditionVariable() noexcept override
    {
        m_condVarPtr = nullptr;
        return true;
    }

    /// @note done in ChunkQueuePusher
    void notify()
    {
        // We don't need to check if the WaitSet is still alive as it follows RAII and will inform every Condition about
        // a possible destruction
        m_wasTriggered = true;
        ConditionVariableSignaler signaler{m_condVarPtr};
        signaler.notifyOne();
    }

    /// @note members reside in ChunkQueueData in SHM
    bool m_wasTriggered{false};
    ConditionVariableData* m_condVarPtr{nullptr};
};

class WaitSet_test : public Test
{
  public:
    ConditionVariableData m_condVarData;
    WaitSet m_sut{&m_condVarData};
    vector<MockSubscriber, iox::MAX_NUMBER_OF_CONDITIONS> m_subscriberVector;

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
        m_sut.detachAllConditions();
        m_subscriberVector.clear();
        ConditionVariableWaiter waiter{&m_condVarData};
        waiter.reset();
    };
};

TEST_F(WaitSet_test, AttachSingleConditionSuccessful)
{
    EXPECT_FALSE(m_sut.attachCondition(m_subscriberVector.front()).has_error());
}

TEST_F(WaitSet_test, AttachSameConditionTwiceResultsInFailure)
{
    m_sut.attachCondition(m_subscriberVector.front());
    EXPECT_THAT(m_sut.attachCondition(m_subscriberVector.front()).get_error(),
                Eq(WaitSetError::CONDITION_VARIABLE_ALREADY_SET));
}

TEST_F(WaitSet_test, AttachConditionAndDestroyResultsInLifetimeFailure)
{
    auto errorHandlerCalled{false};
    iox::Error receivedError;
    WaitSet* m_sut2 = static_cast<WaitSet*>(malloc(sizeof(WaitSet)));
    new (m_sut2) WaitSet{&m_condVarData};

    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled,
         &receivedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
            receivedError = error;
        });

    {
        MockSubscriber scopedCondition;
        m_sut2->attachCondition(scopedCondition);
    }

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(receivedError, Eq(iox::Error::kPOPO__WAITSET_CONDITION_LIFETIME_ISSUE));
    free(m_sut2);
}

TEST_F(WaitSet_test, AttachConditionAndDestroyWaitSetResultsInDetach)
{
    {
        WaitSet m_sut2{&m_condVarData};
        m_sut2.attachCondition(m_subscriberVector.front());
    }
    EXPECT_FALSE(m_subscriberVector.front().isConditionVariableAttached());
}

TEST_F(WaitSet_test, DISABLED_AttachConditionAndMoveIsSuccessful)
{
    /// @todo move c'tor currently deleted
}


TEST_F(WaitSet_test, DISABLED_AttachConditionAndMoveAssignIsSuccessful)
{
    /// @todo move assign currently deleted
}

TEST_F(WaitSet_test, AttachMaximumAllowedConditionsSuccessful)
{
    for (auto& currentSubscriber : m_subscriberVector)
    {
        EXPECT_FALSE(m_sut.attachCondition(currentSubscriber).has_error());
    }
}

TEST_F(WaitSet_test, AttachTooManyConditionsResultsInFailure)
{
    for (auto& currentSubscriber : m_subscriberVector)
    {
        m_sut.attachCondition(currentSubscriber);
    }

    MockSubscriber extraCondition;
    EXPECT_THAT(m_sut.attachCondition(extraCondition).get_error(), Eq(WaitSetError::CONDITION_VECTOR_OVERFLOW));
}

TEST_F(WaitSet_test, DetachSingleConditionSuccessful)
{
    m_sut.attachCondition(m_subscriberVector.front());
    EXPECT_TRUE(m_sut.detachCondition(m_subscriberVector.front()));
}

TEST_F(WaitSet_test, DetachMultipleConditionsSuccessful)
{
    for (auto& currentSubscriber : m_subscriberVector)
    {
        m_sut.attachCondition(currentSubscriber);
    }
    for (auto& currentSubscriber : m_subscriberVector)
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

TEST_F(WaitSet_test, AttachConditionInTwoWaitSetsResultsInAlreadySetError)
{
    WaitSet m_sut2{&m_condVarData};
    m_sut.attachCondition(m_subscriberVector.front());
    EXPECT_THAT(m_sut2.attachCondition(m_subscriberVector.front()).get_error(),
                Eq(WaitSetError::CONDITION_VARIABLE_ALREADY_SET));
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

TEST_F(WaitSet_test, TimedWaitWithMaximumNumberOfConditionsResultsInReturnOfMaximumNumberOfConditions)
{
    for (auto& currentSubscriber : m_subscriberVector)
    {
        m_sut.attachCondition(currentSubscriber);
        currentSubscriber.notify();
    }
    auto fulfilledConditions = m_sut.timedWait(1_ms);
    EXPECT_THAT(fulfilledConditions.size(), Eq(iox::MAX_NUMBER_OF_CONDITIONS));
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

TIMING_TEST_F(WaitSet_test, AttachManyNotifyManyBeforeWaitingResultsInTriggerMultiThreaded, Repeat(5), [&] {
    std::atomic<int> counter{0};
    m_sut.attachCondition(m_subscriberVector[0]);
    m_sut.attachCondition(m_subscriberVector[1]);
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_syncSemaphore.wait();
        auto fulfilledConditions = m_sut.wait();
        TIMING_TEST_ASSERT_TRUE(fulfilledConditions.size() == 2);
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
});


TIMING_TEST_F(WaitSet_test, AttachManyNotifyManyWhileWaitingResultsInTriggerMultiThreaded, Repeat(5), [&] {
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
});


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
    GuardCondition guardCond;
    m_sut.attachCondition(guardCond);
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
    guardCond.setTrigger();
    waiter.join();
    m_sut.detachCondition(guardCond);
}

TEST_F(WaitSet_test, NotifyGuardConditionOnceTimedWaitResultsInResetOfTrigger)
{
    GuardCondition guardCond;
    m_sut.attachCondition(guardCond);
    guardCond.setTrigger();
    auto fulfilledConditions1 = m_sut.timedWait(1_ms);
    EXPECT_THAT(fulfilledConditions1.size(), Eq(1));
    EXPECT_THAT(fulfilledConditions1.front(), &guardCond);
    guardCond.resetTrigger();
    auto fulfilledConditions2 = m_sut.timedWait(1_ms);
    EXPECT_THAT(fulfilledConditions2.size(), Eq(0));
    m_sut.detachCondition(guardCond);
}
