// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

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
    class SimpleEventClass
    {
      public:
        SimpleEventClass() = default;
        SimpleEventClass(const SimpleEventClass&) = delete;
        SimpleEventClass(SimpleEventClass&&) = delete;

        SimpleEventClass& operator=(const SimpleEventClass&) = delete;
        SimpleEventClass& operator=(SimpleEventClass&&) = delete;

        ~SimpleEventClass()
        {
        }

        void enableEvent(iox::popo::TriggerHandle&& handle) noexcept
        {
            m_handle = std::move(handle);
        }

        void invalidateTrigger(const uint64_t id)
        {
            m_invalidateTriggerId = id;
            m_handle.invalidate();
        }

        iox::cxx::ConstMethodCallback<bool> getHasTriggeredCallbackForEvent() const noexcept
        {
            return {*this, &SimpleEventClass::hasTriggered};
        }

        bool hasTriggered() const
        {
            return m_hasTriggered.exchange(false);
        }

        void disableEvent()
        {
            m_handle.reset();
        }

        uint64_t getUniqueId() const
        {
            return m_handle.getUniqueId();
        }

        void trigger()
        {
            m_hasTriggered.store(true);
            m_handle.trigger();
        }

        void resetTrigger()
        {
            m_hasTriggered.store(false);
        }

        iox::popo::TriggerHandle m_handle;
        mutable std::atomic_bool m_hasTriggered{false};
        static uint64_t m_invalidateTriggerId;

        SimpleEventClass* m_triggerCallbackArgument1 = nullptr;
        SimpleEventClass* m_triggerCallbackArgument2 = nullptr;
    };

    ConditionVariableData m_condVarData{"Horscht"};
    WaitSetMock m_sut{&m_condVarData};

    static void triggerCallback1(WaitSet_test::SimpleEventClass* const waitset)
    {
        waitset->m_triggerCallbackArgument1 = waitset;
    }

    static void triggerCallback2(WaitSet_test::SimpleEventClass* const waitset)
    {
        waitset->m_triggerCallbackArgument2 = waitset;
    }

    void SetUp()
    {
        WaitSet_test::SimpleEventClass::m_invalidateTriggerId = 0U;
    };

    void TearDown(){};

    using eventVector_t = iox::cxx::vector<SimpleEventClass, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET + 1>;
    eventVector_t m_simpleEvents{iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET + 1};
};
uint64_t WaitSet_test::SimpleEventClass::m_invalidateTriggerId = 0U;

TEST_F(WaitSet_test, AcquireTriggerOnceIsSuccessful)
{
    EXPECT_FALSE(m_sut.attachEvent(m_simpleEvents[0]).has_error());
}

TEST_F(WaitSet_test, AcquireMultipleTriggerIsSuccessful)
{
    auto result1 = m_sut.attachEvent(m_simpleEvents[0], 10U);
    auto result2 = m_sut.attachEvent(m_simpleEvents[1], 10U);
    auto result3 = m_sut.attachEvent(m_simpleEvents[2], 10U);

    EXPECT_FALSE(result1.has_error());
    EXPECT_FALSE(result2.has_error());
    EXPECT_FALSE(result3.has_error());
}

TEST_F(WaitSet_test, AcquireMaximumAllowedTriggersIsSuccessful)
{
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        auto result = m_sut.attachEvent(m_simpleEvents[i], 1U + i);
        EXPECT_FALSE(result.has_error());
    }
}

TEST_F(WaitSet_test, AcquireMaximumAllowedPlusOneTriggerFails)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        m_sut.attachEvent(m_simpleEvents[i], 1U + i);
    }
    auto result = m_sut.attachEvent(m_simpleEvents[iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET], 0U);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(WaitSetError::WAIT_SET_FULL));
}

TEST_F(WaitSet_test, AcquireSameTriggerTwiceResultsInError)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 0U;
    m_sut.attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);
    auto result2 = m_sut.attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.get_error(), Eq(WaitSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(WaitSet_test, AcquireSameTriggerWithNonNullIdTwiceResultsInError)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 121U;
    m_sut.attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);
    auto result2 = m_sut.attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.get_error(), Eq(WaitSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(WaitSet_test, AcquireSameTriggerWithDifferentIdResultsInError)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 2101U;
    constexpr uint64_t ANOTHER_USER_DEFINED_EVENT_ID = 9121U;
    m_sut.attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);
    auto result2 = m_sut.attachEvent(m_simpleEvents[0], ANOTHER_USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.get_error(), Eq(WaitSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(WaitSet_test, ResetCallbackIsCalledWhenWaitsetGoesOutOfScope)
{
    uint64_t uniqueTriggerId = 0U;
    SimpleEventClass simpleEvent;
    {
        WaitSetMock sut{&m_condVarData};
        constexpr uint64_t USER_DEFINED_EVENT_ID = 421337U;
        sut.attachEvent(simpleEvent, USER_DEFINED_EVENT_ID);
        uniqueTriggerId = simpleEvent.getUniqueId();
    }
    EXPECT_THAT(SimpleEventClass::m_invalidateTriggerId, Eq(uniqueTriggerId));
}

TEST_F(WaitSet_test, TriggerRemovesItselfFromWaitsetWhenGoingOutOfScope)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        m_sut.attachEvent(m_simpleEvents[i], 100U + i);
    }

    constexpr uint64_t USER_DEFINED_EVENT_ID = 0U;
    {
        SimpleEventClass temporaryTrigger;
        m_sut.attachEvent(temporaryTrigger, USER_DEFINED_EVENT_ID);
        // goes out of scope here and creates space again for an additional trigger
        // if this doesn't work we are unable to acquire another trigger since the
        // waitset is already full
    }

    auto result = m_sut.attachEvent(m_simpleEvents.back(), USER_DEFINED_EVENT_ID);
    EXPECT_FALSE(result.has_error());
}

TEST_F(WaitSet_test, MultipleTimerRemovingThemselfFromWaitsetWhenGoingOutOfScope)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 3U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        m_sut.attachEvent(m_simpleEvents[i], 100U + i);
    }

    constexpr uint64_t USER_DEFINED_EVENT_ID = 0U;
    {
        SimpleEventClass temporaryTrigger1, temporaryTrigger2, temporaryTrigger3;
        m_sut.attachEvent(temporaryTrigger1, USER_DEFINED_EVENT_ID);
        m_sut.attachEvent(temporaryTrigger2, USER_DEFINED_EVENT_ID);
        m_sut.attachEvent(temporaryTrigger3, USER_DEFINED_EVENT_ID);

        // goes out of scope here and creates space again for an additional trigger
        // if this doesn't work we are unable to acquire another trigger since the
        // waitset is already full
    }

    auto result0 = m_sut.attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);
    auto result1 = m_sut.attachEvent(m_simpleEvents[1], USER_DEFINED_EVENT_ID);
    auto result2 = m_sut.attachEvent(m_simpleEvents[2], USER_DEFINED_EVENT_ID);
    EXPECT_FALSE(result0.has_error());
    EXPECT_FALSE(result1.has_error());
    EXPECT_FALSE(result2.has_error());
}

TEST_F(WaitSet_test, WaitBlocksWhenNothingTriggered)
{
    std::atomic_bool doStartWaiting{false};
    std::atomic_bool isThreadFinished{false};
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        m_sut.attachEvent(m_simpleEvents[i], 5U + i);
    }

    std::thread t([&] {
        doStartWaiting.store(true);
        auto triggerVector = m_sut.wait();
        isThreadFinished.store(true);
    });

    while (!doStartWaiting.load())
        ;


    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(isThreadFinished.load());

    m_simpleEvents[0].trigger();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(isThreadFinished.load());

    t.join();
}

TEST_F(WaitSet_test, TimedWaitReturnsNothingWhenNothingTriggered)
{
    iox::cxx::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        m_sut.attachEvent(m_simpleEvents[i], 5U + i);
    }


    auto triggerVector = m_sut.timedWait(10_ms);
    ASSERT_THAT(triggerVector.size(), Eq(0U));
}

void WaitReturnsTheOneTriggeredCondition(WaitSet_test* test,
                                         const std::function<WaitSet<>::EventInfoVector()>& waitCall)
{
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        test->m_sut.attachEvent(test->m_simpleEvents[i], 5U + i);
    }

    test->m_simpleEvents[0].trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(1U));
    EXPECT_THAT(triggerVector[0U]->getEventId(), 5U);
    EXPECT_TRUE(triggerVector[0U]->doesOriginateFrom(&test->m_simpleEvents[0]));
    EXPECT_EQ(triggerVector[0U]->getOrigin<WaitSet_test::SimpleEventClass>(), &test->m_simpleEvents[0]);
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
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        test->m_sut.attachEvent(test->m_simpleEvents[i], 100U + i);
    }

    for (uint64_t i = 0U; i < 24U; ++i)
    {
        test->m_simpleEvents[i].trigger();
    }

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(24U));

    for (uint64_t i = 0U; i < 24U; ++i)
    {
        EXPECT_THAT(triggerVector[i]->getEventId(), 100U + i);
        EXPECT_TRUE(triggerVector[i]->doesOriginateFrom(&test->m_simpleEvents[i]));
        EXPECT_EQ(triggerVector[i]->getOrigin<WaitSet_test::SimpleEventClass>(), &test->m_simpleEvents[i]);
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
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        test->m_sut.attachEvent(test->m_simpleEvents[i], i * 3U + 2U);
    }

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        test->m_simpleEvents[i].trigger();
    }

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET));

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET; ++i)
    {
        EXPECT_THAT(triggerVector[i]->getEventId(), i * 3U + 2U);
        EXPECT_TRUE(triggerVector[i]->doesOriginateFrom(&test->m_simpleEvents[i]));
        EXPECT_EQ(triggerVector[i]->getOrigin<WaitSet_test::SimpleEventClass>(), &test->m_simpleEvents[i]);
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
    auto result1 = test->m_sut.attachEvent(test->m_simpleEvents[0], 1U, &WaitSet_test::triggerCallback1);
    auto result2 = test->m_sut.attachEvent(test->m_simpleEvents[1], 2U, &WaitSet_test::triggerCallback2);

    ASSERT_THAT(result1.has_error(), Eq(false));
    ASSERT_THAT(result2.has_error(), Eq(false));

    test->m_simpleEvents[0].trigger();
    test->m_simpleEvents[1].trigger();


    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(2U));

    (*triggerVector[0U])();
    EXPECT_THAT(test->m_simpleEvents[0].m_triggerCallbackArgument1, Eq(&test->m_simpleEvents[0]));

    (*triggerVector[1U])();
    EXPECT_THAT(test->m_simpleEvents[1].m_triggerCallbackArgument2, Eq(&test->m_simpleEvents[1]));
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
    m_sut.attachEvent(m_simpleEvents[0]);

    EXPECT_EQ(m_sut.size(), 1U);
}

TEST_F(WaitSet_test, MultipleAcquireTriggerIncreasesSizeCorrectly)
{
    m_sut.attachEvent(m_simpleEvents[0]);
    m_sut.attachEvent(m_simpleEvents[1]);
    m_sut.attachEvent(m_simpleEvents[2]);
    m_sut.attachEvent(m_simpleEvents[4]);

    EXPECT_EQ(m_sut.size(), 4U);
}

TEST_F(WaitSet_test, TriggerGoesOutOfScopeReducesSize)
{
    m_sut.attachEvent(m_simpleEvents[0]);
    m_sut.attachEvent(m_simpleEvents[1]);
    {
        SimpleEventClass simpleEvent1, simpleEvent2;
        m_sut.attachEvent(simpleEvent1);
        m_sut.attachEvent(simpleEvent2);
    }

    EXPECT_EQ(m_sut.size(), 2U);
}
