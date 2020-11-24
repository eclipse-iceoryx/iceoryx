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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "mocks/wait_set_mock.hpp"
#include "test.hpp"
#include "testutils/timing_test.hpp"

#include <chrono>
#include <memory>
#include <thread>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::units::duration_literals;

class WaitSet_test : public Test
{
  public:
    ConditionVariableData m_condVarData;
    WaitSetMock m_sut{&m_condVarData};
    uint64_t m_resetTriggerId = 0U;
    WaitSet_test* m_triggerCallbackArgument1 = nullptr;
    WaitSet_test* m_triggerCallbackArgument2 = nullptr;
    mutable uint64_t m_returnTrueCounter = 0U;

    expected<Trigger, WaitSetError> acquireTrigger(WaitSetMock& waitset,
                                                   const uint64_t triggerId,
                                                   Trigger::Callback<WaitSet_test> callback = triggerCallback1)
    {
        return waitset.acquireTrigger(
            this, {this, &WaitSet_test::hasTriggered}, {this, &WaitSet_test::resetCallback}, triggerId, callback);
    }

    bool hasTriggered() const
    {
        if (m_returnTrueCounter == 0U)
        {
            return false;
        }
        else
        {
            --m_returnTrueCounter;
            return true;
        }
    }

    void resetCallback(const Trigger& trigger)
    {
        m_resetTriggerId = trigger.getTriggerId();
        const_cast<Trigger&>(trigger).reset();
    }

    static void triggerCallback1(WaitSet_test* const waitset)
    {
        waitset->m_triggerCallbackArgument1 = waitset;
    }

    static void triggerCallback2(WaitSet_test* const waitset)
    {
        waitset->m_triggerCallbackArgument2 = waitset;
    }

    void SetUp(){};

    void TearDown(){};
};

TEST_F(WaitSet_test, AcquireTriggerOnceIsSuccessful)
{
    EXPECT_FALSE(acquireTrigger(m_sut, 0).has_error());
}


TEST_F(WaitSet_test, AcquireMultipleTriggerIsSuccessful)
{
    auto trigger1 = acquireTrigger(m_sut, 0);
    auto trigger2 = acquireTrigger(m_sut, 0);
    auto trigger3 = acquireTrigger(m_sut, 0);

    EXPECT_FALSE(trigger1.has_error());
    EXPECT_FALSE(trigger2.has_error());
    EXPECT_FALSE(trigger3.has_error());
}

TEST_F(WaitSet_test, AcquireMaximumAllowedTriggersIsSuccessful)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTrigger(m_sut, 0));
        EXPECT_FALSE(trigger.back().has_error());
    }
}

TEST_F(WaitSet_test, AcquireMaximumAllowedPlusOneTriggerFails)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTrigger(m_sut, 0));
    }
    auto result = acquireTrigger(m_sut, 0);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(WaitSetError::TRIGGER_VECTOR_OVERFLOW));
}

TEST_F(WaitSet_test, AcquireTriggerReturnsTriggerWithValidOrigin)
{
    auto trigger = acquireTrigger(m_sut, 0);
    ASSERT_FALSE(trigger.has_error());
    EXPECT_EQ(trigger->getOrigin<WaitSet_test>(), this);
}

TEST_F(WaitSet_test, AcquireTriggerReturnsTriggerWithValidTriggerId)
{
    auto trigger = acquireTrigger(m_sut, 781);
    ASSERT_FALSE(trigger.has_error());
    EXPECT_EQ(trigger->getTriggerId(), 781);
}

TEST_F(WaitSet_test, AcquireTriggerReturnsTriggerWithValidCallback)
{
    auto trigger = acquireTrigger(m_sut, 0);
    ASSERT_FALSE(trigger.has_error());
    ASSERT_TRUE((*trigger)());
    EXPECT_THAT(m_triggerCallbackArgument1, this);
}

TEST_F(WaitSet_test, AcquireTriggerReturnsTriggerWithValidTriggerCallback)
{
    auto trigger = acquireTrigger(m_sut, 0);
    m_returnTrueCounter = 1;
    EXPECT_TRUE(trigger->hasTriggered());
    m_returnTrueCounter = 0;
    EXPECT_FALSE(trigger->hasTriggered());
}

TEST_F(WaitSet_test, ResetCallbackIsCalledWhenWaitsetGoesOutOfScope)
{
    iox::cxx::expected<Trigger, WaitSetError> trigger =
        iox::cxx::error<WaitSetError>(WaitSetError::TRIGGER_VECTOR_OVERFLOW);

    {
        WaitSetMock sut{&m_condVarData};
        trigger = acquireTrigger(sut, 421337);
    }

    EXPECT_THAT(m_resetTriggerId, Eq(421337));
}

TEST_F(WaitSet_test, TimerRemovesItselfFromWaitsetWhenGoingOutOfScope)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET - 1; ++i)
    {
        trigger.emplace_back(acquireTrigger(m_sut, 0));
    }

    {
        auto temporaryTrigger = acquireTrigger(m_sut, 0);
        // goes out of scope here and creates space again for an additional trigger
        // if this doesn't work we are unable to acquire another trigger since the
        // waitset is already full
    }

    auto anotherTrigger = acquireTrigger(m_sut, 0);
    EXPECT_FALSE(anotherTrigger.has_error());
}

TEST_F(WaitSet_test, MultipleTimerRemovingThemselfFromWaitsetWhenGoingOutOfScope)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET - 3; ++i)
    {
        trigger.emplace_back(acquireTrigger(m_sut, 0));
    }

    {
        auto temporaryTrigger1 = acquireTrigger(m_sut, 0);
        auto temporaryTrigger2 = acquireTrigger(m_sut, 0);
        auto temporaryTrigger3 = acquireTrigger(m_sut, 0);
    }

    auto anotherTrigger1 = acquireTrigger(m_sut, 0);
    auto anotherTrigger2 = acquireTrigger(m_sut, 0);
    auto anotherTrigger3 = acquireTrigger(m_sut, 0);
    EXPECT_FALSE(anotherTrigger3.has_error());
}

TEST_F(WaitSet_test, WaitBlocksWhenNothingTriggered)
{
    std::atomic_bool doStartWaiting{false};
    std::atomic_bool isThreadFinished{false};
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTrigger(m_sut, i + 5));
    }

    std::thread t([&] {
        m_returnTrueCounter = 0;
        trigger.front()->trigger();

        doStartWaiting.store(true);
        auto triggerVector = m_sut.wait();
        isThreadFinished.store(true);
    });

    while (!doStartWaiting.load())
        ;


    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(isThreadFinished.load());

    m_returnTrueCounter = 1;
    trigger.front()->trigger();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(isThreadFinished.load());

    t.join();
}

TEST_F(WaitSet_test, TimedWaitReturnsNothingWhenNothingTriggered)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTrigger(m_sut, i + 5));
    }

    m_returnTrueCounter = 0;
    trigger.front()->trigger();

    auto triggerVector = m_sut.timedWait(10_ms);
    ASSERT_THAT(triggerVector.size(), Eq(0));
}

void WaitReturnsTheOneTriggeredCondition(WaitSet_test* test,
                                         const std::function<WaitSet::TriggerStateVector()>& waitCall)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(test->acquireTrigger(test->m_sut, i + 5));
    }

    test->m_returnTrueCounter = 1;
    trigger.front()->trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(1));
    EXPECT_THAT(triggerVector[0].getTriggerId(), 5);
    EXPECT_TRUE(triggerVector[0].doesOriginateFrom(test));
    EXPECT_EQ(triggerVector[0].getOrigin<WaitSet_test>(), test);
}

TEST_F(WaitSet_test, WaitReturnsTheOneTriggeredCondition)
{
    WaitReturnsTheOneTriggeredCondition(this, [&] { return m_sut.wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsTheOneTriggeredCondition)
{
    WaitReturnsTheOneTriggeredCondition(this, [&] { return m_sut.timedWait(10_ms); });
}

void WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered(
    WaitSet_test* test, const std::function<WaitSet::TriggerStateVector()>& waitCall)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(test->acquireTrigger(test->m_sut, i * 4));
    }

    test->m_returnTrueCounter = 24;
    trigger.front()->trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(24));

    for (uint64_t i = 0; i < 24; ++i)
    {
        EXPECT_THAT(triggerVector[i].getTriggerId(), i * 4);
        EXPECT_TRUE(triggerVector[i].doesOriginateFrom(test));
        EXPECT_EQ(triggerVector[i].getOrigin<WaitSet_test>(), test);
    }
}

TEST_F(WaitSet_test, WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered)
{
    WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered(this, [&] { return m_sut.wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsAllTriggeredConditionWhenMultipleAreTriggered)
{
    WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered(this, [&] { return m_sut.timedWait(10_ms); });
}


void WaitReturnsAllTriggeredConditionWhenAllAreTriggered(WaitSet_test* test,
                                                         const std::function<WaitSet::TriggerStateVector()>& waitCall)
{
    iox::cxx::vector<expected<Trigger, WaitSetError>, iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET> trigger;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(test->acquireTrigger(test->m_sut, i * 3 + 2));
    }

    test->m_returnTrueCounter = iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET;
    trigger.front()->trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET));

    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        EXPECT_THAT(triggerVector[i].getTriggerId(), i * 3 + 2);
        EXPECT_TRUE(triggerVector[i].doesOriginateFrom(test));
        EXPECT_EQ(triggerVector[i].getOrigin<WaitSet_test>(), test);
    }
}

TEST_F(WaitSet_test, WaitReturnsAllTriggeredConditionWhenAllAreTriggered)
{
    WaitReturnsAllTriggeredConditionWhenAllAreTriggered(this, [&] { return m_sut.wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsAllTriggeredConditionWhenAllAreTriggered)
{
    WaitReturnsAllTriggeredConditionWhenAllAreTriggered(this, [&] { return m_sut.timedWait(10_ms); });
}

void WaitReturnsTriggersWithCorrectCallbacks(WaitSet_test* test,
                                             const std::function<WaitSet::TriggerStateVector()>& waitCall)
{
    auto trigger1 = test->acquireTrigger(test->m_sut, 0, WaitSet_test::triggerCallback1);
    auto trigger2 = test->acquireTrigger(test->m_sut, 0, WaitSet_test::triggerCallback2);

    test->m_returnTrueCounter = 2;
    trigger1->trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(2));

    test->m_triggerCallbackArgument1 = nullptr;
    triggerVector[0]();
    EXPECT_THAT(test->m_triggerCallbackArgument1, Eq(test));

    test->m_triggerCallbackArgument2 = nullptr;
    triggerVector[1]();
    EXPECT_THAT(test->m_triggerCallbackArgument2, Eq(test));
}

TEST_F(WaitSet_test, WaitReturnsTriggersWithCorrectCallbacks)
{
    WaitReturnsTriggersWithCorrectCallbacks(this, [&] { return m_sut.wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsTriggersWithCorrectCallbacks)
{
    WaitReturnsTriggersWithCorrectCallbacks(this, [&] { return m_sut.timedWait(10_ms); });
}

TEST_F(WaitSet_test, InitialWaitSetHasSizeZero)
{
    EXPECT_EQ(m_sut.size(), 0);
}

TEST_F(WaitSet_test, WaitSetCapacity)
{
    EXPECT_EQ(m_sut.capacity(), iox::MAX_NUMBER_OF_TRIGGERS_PER_WAITSET);
}

TEST_F(WaitSet_test, OneAcquireTriggerIncreasesSizeByOne)
{
    auto trigger1 = acquireTrigger(m_sut, 0);
    static_cast<void>(trigger1);

    EXPECT_EQ(m_sut.size(), 1);
}

TEST_F(WaitSet_test, MultipleAcquireTriggerIncreasesSizeCorrectly)
{
    auto trigger1 = acquireTrigger(m_sut, 0);
    auto trigger2 = acquireTrigger(m_sut, 0);
    auto trigger3 = acquireTrigger(m_sut, 0);
    auto trigger4 = acquireTrigger(m_sut, 0);

    EXPECT_EQ(m_sut.size(), 4);
}

TEST_F(WaitSet_test, TriggerGoesOutOfScopeReducesSize)
{
    auto trigger1 = acquireTrigger(m_sut, 0);
    auto trigger2 = acquireTrigger(m_sut, 0);
    {
        auto trigger3 = acquireTrigger(m_sut, 0);
        auto trigger4 = acquireTrigger(m_sut, 0);
    }

    EXPECT_EQ(m_sut.size(), 2);
}
