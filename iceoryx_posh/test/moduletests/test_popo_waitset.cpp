// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
    std::vector<std::unique_ptr<expected<TriggerHandle, WaitSetError>>> m_triggerHandle;
    ConditionVariableData m_condVarData{"Horscht"};
    WaitSetMock m_sut{&m_condVarData};
    uint64_t m_resetTriggerId = 0U;
    WaitSet_test* m_triggerCallbackArgument1 = nullptr;
    WaitSet_test* m_triggerCallbackArgument2 = nullptr;
    mutable uint64_t m_returnTrueCounter = 0U;

    expected<TriggerHandle, WaitSetError>* acquireTriggerHandle(
        WaitSetMock& waitset, const uint64_t eventId, Trigger::Callback<WaitSet_test> callback = triggerCallback1)
    {
        m_triggerHandle.emplace_back(
            std::make_unique<expected<TriggerHandle, WaitSetError>>(waitset.acquireTriggerHandle(
                this, {*this, &WaitSet_test::hasTriggered}, {*this, &WaitSet_test::resetCallback}, eventId, callback)));
        return m_triggerHandle.back().get();
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

    void resetCallback(const uint64_t uniqueTriggerId)
    {
        m_resetTriggerId = uniqueTriggerId;
        for (uint64_t i = 0U; i < m_triggerHandle.size(); ++i)
        {
            if (m_triggerHandle[i]->value().getUniqueId() == uniqueTriggerId)
            {
                m_triggerHandle[i]->value().invalidate();
                m_triggerHandle.erase(m_triggerHandle.begin() + i);
                return;
            }
        }
    }

    void removeTrigger(const uint64_t uniqueTriggerId)
    {
        m_resetTriggerId = uniqueTriggerId;
        for (uint64_t i = 0U; i < m_triggerHandle.size(); ++i)
        {
            if (m_triggerHandle[i]->value().getUniqueId() == uniqueTriggerId)
            {
                m_triggerHandle.erase(m_triggerHandle.begin() + i);
            }
        }
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
    EXPECT_FALSE(acquireTriggerHandle(m_sut, 0U)->has_error());
}

TEST_F(WaitSet_test, AcquireMultipleTriggerIsSuccessful)
{
    auto trigger1 = acquireTriggerHandle(m_sut, 10U);
    auto trigger2 = acquireTriggerHandle(m_sut, 11U);
    auto trigger3 = acquireTriggerHandle(m_sut, 12U);

    EXPECT_FALSE(trigger1->has_error());
    EXPECT_FALSE(trigger2->has_error());
    EXPECT_FALSE(trigger3->has_error());
}

TEST_F(WaitSet_test, AcquireMaximumAllowedTriggersIsSuccessful)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTriggerHandle(m_sut, 1U + i));
        EXPECT_FALSE(trigger.back()->has_error());
    }
}

TEST_F(WaitSet_test, AcquireMaximumAllowedPlusOneTriggerFails)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTriggerHandle(m_sut, 5U + i));
    }
    auto result = acquireTriggerHandle(m_sut, 0U);
    ASSERT_TRUE(result->has_error());
    EXPECT_THAT(result->get_error(), Eq(WaitSetError::WAIT_SET_FULL));
}

TEST_F(WaitSet_test, AcquireSameTriggerTwiceResultsInError)
{
    acquireTriggerHandle(m_sut, 0U);
    auto trigger2 = acquireTriggerHandle(m_sut, 0U);

    ASSERT_TRUE(trigger2->has_error());
    EXPECT_THAT(trigger2->get_error(), Eq(WaitSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(WaitSet_test, AcquireSameTriggerWithNonNullIdTwiceResultsInError)
{
    acquireTriggerHandle(m_sut, 121U);
    auto trigger2 = acquireTriggerHandle(m_sut, 121U);

    ASSERT_TRUE(trigger2->has_error());
    EXPECT_THAT(trigger2->get_error(), Eq(WaitSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(WaitSet_test, ResetCallbackIsCalledWhenWaitsetGoesOutOfScope)
{
    uint64_t uniqueTriggerId = 0U;
    {
        WaitSetMock sut{&m_condVarData};
        uniqueTriggerId = acquireTriggerHandle(sut, 421337U)->value().getUniqueId();
    }

    EXPECT_THAT(m_resetTriggerId, Eq(uniqueTriggerId));
}

TEST_F(WaitSet_test, TriggerRemovesItselfFromWaitsetWhenGoingOutOfScope)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTriggerHandle(m_sut, 100U + i));
    }

    {
        auto temporaryTrigger = acquireTriggerHandle(m_sut, 0U);
        // goes out of scope here and creates space again for an additional trigger
        // if this doesn't work we are unable to acquire another trigger since the
        // waitset is already full
        removeTrigger(temporaryTrigger->value().getUniqueId());
    }

    auto anotherTrigger = acquireTriggerHandle(m_sut, 0U);
    EXPECT_FALSE(anotherTrigger->has_error());
}

TEST_F(WaitSet_test, MultipleTimerRemovingThemselfFromWaitsetWhenGoingOutOfScope)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i + 3U < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTriggerHandle(m_sut, 100U + i));
    }

    {
        auto temporaryTrigger1 = acquireTriggerHandle(m_sut, 1U);
        auto temporaryTrigger2 = acquireTriggerHandle(m_sut, 2U);
        auto temporaryTrigger3 = acquireTriggerHandle(m_sut, 3U);

        removeTrigger(temporaryTrigger1->value().getUniqueId());
        removeTrigger(temporaryTrigger2->value().getUniqueId());
        removeTrigger(temporaryTrigger3->value().getUniqueId());
    }

    acquireTriggerHandle(m_sut, 5U);
    acquireTriggerHandle(m_sut, 6U);
    auto anotherTrigger3 = acquireTriggerHandle(m_sut, 7U);
    EXPECT_FALSE(anotherTrigger3->has_error());
}

TEST_F(WaitSet_test, WaitBlocksWhenNothingTriggered)
{
    std::atomic_bool doStartWaiting{false};
    std::atomic_bool isThreadFinished{false};
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTriggerHandle(m_sut, i + 5U));
    }

    std::thread t([&] {
        m_returnTrueCounter = 0U;
        trigger.front()->value().trigger();

        doStartWaiting.store(true);
        auto triggerVector = m_sut.wait();
        isThreadFinished.store(true);
    });

    while (!doStartWaiting.load())
        ;


    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(isThreadFinished.load());

    m_returnTrueCounter = 1U;
    trigger.front()->value().trigger();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(isThreadFinished.load());

    t.join();
}

TEST_F(WaitSet_test, TimedWaitReturnsNothingWhenNothingTriggered)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(acquireTriggerHandle(m_sut, i + 5U));
    }

    m_returnTrueCounter = 0U;
    trigger.front()->value().trigger();

    auto triggerVector = m_sut.timedWait(10_ms);
    ASSERT_THAT(triggerVector.size(), Eq(0U));
}

void WaitReturnsTheOneTriggeredCondition(WaitSet_test* test,
                                         const std::function<WaitSet<>::EventInfoVector()>& waitCall)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(test->acquireTriggerHandle(test->m_sut, i + 5U));
    }

    test->m_returnTrueCounter = 1U;
    trigger.front()->value().trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(1U));
    EXPECT_THAT(triggerVector[0U]->getEventId(), 5U);
    EXPECT_TRUE(triggerVector[0U]->doesOriginateFrom(test));
    EXPECT_EQ(triggerVector[0U]->getOrigin<WaitSet_test>(), test);
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
    WaitSet_test* test, const std::function<WaitSet<>::EventInfoVector()>& waitCall)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(test->acquireTriggerHandle(test->m_sut, 100U + i));
    }

    test->m_returnTrueCounter = 24U;
    trigger.front()->value().trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(24U));

    for (uint64_t i = 0U; i < 24U; ++i)
    {
        EXPECT_THAT(triggerVector[i]->getEventId(), 100U + i);
        EXPECT_TRUE(triggerVector[i]->doesOriginateFrom(test));
        EXPECT_EQ(triggerVector[i]->getOrigin<WaitSet_test>(), test);
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
                                                         const std::function<WaitSet<>::EventInfoVector()>& waitCall)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        trigger.emplace_back(test->acquireTriggerHandle(test->m_sut, i * 3U + 2U));
    }

    test->m_returnTrueCounter = iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET;
    trigger.front()->value().trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET));

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        EXPECT_THAT(triggerVector[i]->getEventId(), i * 3U + 2U);
        EXPECT_TRUE(triggerVector[i]->doesOriginateFrom(test));
        EXPECT_EQ(triggerVector[i]->getOrigin<WaitSet_test>(), test);
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
                                             const std::function<WaitSet<>::EventInfoVector()>& waitCall)
{
    auto trigger1 = test->acquireTriggerHandle(test->m_sut, 1U, WaitSet_test::triggerCallback1);
    auto trigger2 = test->acquireTriggerHandle(test->m_sut, 2U, WaitSet_test::triggerCallback2);

    ASSERT_THAT(trigger1->has_error(), Eq(false));
    ASSERT_THAT(trigger2->has_error(), Eq(false));

    test->m_returnTrueCounter = 2U;
    trigger1->value().trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(2U));

    test->m_triggerCallbackArgument1 = nullptr;
    (*triggerVector[0U])();
    EXPECT_THAT(test->m_triggerCallbackArgument1, Eq(test));

    test->m_triggerCallbackArgument2 = nullptr;
    (*triggerVector[1U])();
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
    EXPECT_EQ(m_sut.size(), 0U);
}

TEST_F(WaitSet_test, WaitSetCapacity)
{
    EXPECT_EQ(m_sut.capacity(), iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET);
}

TEST_F(WaitSet_test, OneAcquireTriggerIncreasesSizeByOne)
{
    auto trigger1 = acquireTriggerHandle(m_sut, 0U);
    static_cast<void>(trigger1);

    EXPECT_EQ(m_sut.size(), 1U);
}

TEST_F(WaitSet_test, MultipleAcquireTriggerIncreasesSizeCorrectly)
{
    acquireTriggerHandle(m_sut, 5U);
    acquireTriggerHandle(m_sut, 6U);
    acquireTriggerHandle(m_sut, 7U);
    acquireTriggerHandle(m_sut, 8U);

    EXPECT_EQ(m_sut.size(), 4U);
}

TEST_F(WaitSet_test, TriggerGoesOutOfScopeReducesSize)
{
    acquireTriggerHandle(m_sut, 1U);
    acquireTriggerHandle(m_sut, 2U);
    {
        auto trigger3 = acquireTriggerHandle(m_sut, 3U);
        auto trigger4 = acquireTriggerHandle(m_sut, 4U);
        removeTrigger(trigger3->value().getUniqueId());
        removeTrigger(trigger4->value().getUniqueId());
    }

    EXPECT_EQ(m_sut.size(), 2U);
}

TEST_F(WaitSet_test, MovingAssignTriggerReducesSize)
{
    auto trigger1 = acquireTriggerHandle(m_sut, 0U);
    TriggerHandle trigger2;
    trigger2 = std::move(trigger1->value());

    EXPECT_EQ(m_sut.size(), 1U);
}

TEST_F(WaitSet_test, MoveCTorTriggerDoesNotChangeSize)
{
    auto trigger1 = acquireTriggerHandle(m_sut, 0U);
    auto trigger2(std::move(*trigger1));

    EXPECT_EQ(m_sut.size(), 1U);
}
