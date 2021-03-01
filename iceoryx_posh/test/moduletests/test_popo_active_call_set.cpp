// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/popo/active_call_set.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/concurrent/smart_lock.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
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

namespace
{
enum SimpleEvent
{
    StoepselBachelorParty,
    Hypnotoad
};

uint64_t g_invalidateTriggerId = 0U;
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
        m_handleStoepsel.reset();
        m_handleHypnotoad.reset();
    }

    void enableEvent(iox::popo::TriggerHandle&& handle, const SimpleEvent event) noexcept
    {
        switch (event)
        {
        case SimpleEvent::StoepselBachelorParty:
            m_handleStoepsel = std::move(handle);
            break;
        case SimpleEvent::Hypnotoad:
            m_handleHypnotoad = std::move(handle);
            break;
        }
    }

    void enableEvent(iox::popo::TriggerHandle&& handle) noexcept
    {
        m_handleNoEventEnum = std::move(handle);
    }

    void invalidateTrigger(const uint64_t id) noexcept
    {
        g_invalidateTriggerId = id;
        if (m_handleHypnotoad.getUniqueId() == id)
        {
            m_handleHypnotoad.invalidate();
        }
        else if (m_handleStoepsel.getUniqueId() == id)
        {
            m_handleStoepsel.invalidate();
        }
        else if (m_handleNoEventEnum.getUniqueId() == id)
        {
            m_handleNoEventEnum.invalidate();
        }
    }

    void disableEvent(const SimpleEvent event) noexcept
    {
        switch (event)
        {
        case SimpleEvent::StoepselBachelorParty:
            m_handleStoepsel.reset();
            break;
        case SimpleEvent::Hypnotoad:
            m_handleHypnotoad.reset();
            break;
        }
    }

    void disableEvent() noexcept
    {
        m_handleNoEventEnum.reset();
    }
    void triggerStoepsel() noexcept
    {
        m_hasTriggered.store(true);
        m_handleStoepsel.trigger();
    }

    void resetTrigger() noexcept
    {
        m_hasTriggered.store(false);
    }

    iox::popo::TriggerHandle m_handleHypnotoad;
    iox::popo::TriggerHandle m_handleStoepsel;
    iox::popo::TriggerHandle m_handleNoEventEnum;
    mutable std::atomic_bool m_hasTriggered{false};
};

class TestActiveCallSet : public ActiveCallSet
{
  public:
    TestActiveCallSet(EventVariableData* data) noexcept
        : ActiveCallSet(data)
    {
    }
};

struct EventAndSutPair_t
{
    SimpleEventClass* object;
    TestActiveCallSet* sut;
};

struct TriggerSourceAndCount
{
    std::atomic<SimpleEventClass*> m_source{nullptr};
    std::atomic<uint64_t> m_count{0U};
};

iox::concurrent::smart_lock<std::vector<EventAndSutPair_t>> g_toBeAttached;
iox::concurrent::smart_lock<std::vector<EventAndSutPair_t>> g_toBeDetached;
std::array<TriggerSourceAndCount, iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> g_triggerCallbackArg;
uint64_t g_triggerCallbackRuntimeInMs = 0U;
iox::cxx::optional<iox::posix::Semaphore> g_callbackBlocker;

class ActiveCallSet_test : public Test
{
  public:
    template <uint64_t N>
    static void triggerCallback(SimpleEventClass* const event) noexcept
    {
        g_triggerCallbackArg[N].m_source = event;
        ++g_triggerCallbackArg[N].m_count;

        if (g_callbackBlocker)
        {
            g_callbackBlocker->wait();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(g_triggerCallbackRuntimeInMs));
    }

    static void attachCallback(SimpleEventClass* const) noexcept
    {
        for (auto& e : g_toBeAttached.GetCopy())
        {
            e.sut->attachEvent(*e.object, SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);
        }
    }

    static void detachCallback(SimpleEventClass* const) noexcept
    {
        for (auto& e : g_toBeDetached.GetCopy())
        {
            e.sut->detachEvent(*e.object, SimpleEvent::StoepselBachelorParty);
        }
    }

    static void notifyAndThenDetachStoepselCallback(SimpleEventClass* const) noexcept
    {
        for (auto& e : g_toBeDetached.GetCopy())
        {
            e.object->triggerStoepsel();
            e.sut->detachEvent(*e.object, SimpleEvent::StoepselBachelorParty);
        }
    }


    void SetUp()
    {
        g_callbackBlocker.reset();
        for (auto& e : g_triggerCallbackArg)
        {
            e.m_source = nullptr;
            e.m_count = 0U;
        }
        m_sut.emplace(&m_eventVarData);
        g_invalidateTriggerId = 0U;
        g_triggerCallbackRuntimeInMs = 0U;
        g_toBeAttached->clear();
        g_toBeDetached->clear();
    };

    void activateTriggerCallbackBlocker() noexcept
    {
        g_callbackBlocker.emplace(
            iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value());
    }

    void unblockTriggerCallback(const uint64_t numberOfUnblocks) noexcept
    {
        for (uint64_t i = 0U; i < numberOfUnblocks; ++i)
        {
            ASSERT_TRUE(static_cast<bool>(g_callbackBlocker));
            g_callbackBlocker->post();
        }
    }

    void TearDown(){};


    static constexpr uint64_t OVERFLOW_TEST_APPENDIX = 1U;
    using eventArray_t = SimpleEventClass[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET + OVERFLOW_TEST_APPENDIX];
    eventArray_t m_simpleEvents;
    EventVariableData m_eventVarData{"Maulbeerblättle"};
    iox::cxx::optional<TestActiveCallSet> m_sut;

    static constexpr uint64_t CALLBACK_WAIT_IN_MS = 100U;
};

constexpr uint64_t ActiveCallSet_test::CALLBACK_WAIT_IN_MS;
constexpr uint64_t ActiveCallSet_test::OVERFLOW_TEST_APPENDIX;

template <uint64_t N>
struct AttachEvent
{
    template <typename EventType>
    static void doIt(TestActiveCallSet& sut, std::vector<SimpleEventClass>& events, const EventType event)
    {
        EXPECT_THAT(sut.attachEvent(events[N], event, ActiveCallSet_test::triggerCallback<N>).has_error(), Eq(false));
        AttachEvent<N - 1U>::doIt(sut, events, event);
    }
};

template <>
struct AttachEvent<0U>
{
    template <typename EventType>
    static void doIt(TestActiveCallSet& sut, std::vector<SimpleEventClass>& events, const EventType event)
    {
        EXPECT_THAT(sut.attachEvent(events[0U], event, ActiveCallSet_test::triggerCallback<0U>).has_error(), Eq(false));
    }
};
} // namespace


//////////////////////////////////
// BEGIN attach / detach
//////////////////////////////////
TEST_F(ActiveCallSet_test, CapacityIsEqualToMAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET)
{
    EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET));
}

TEST_F(ActiveCallSet_test, IsEmptyWhenConstructed)
{
    EXPECT_THAT(m_sut->size(), Eq(0U));
}

TEST_F(ActiveCallSet_test, AttachingWithoutEnumIfEnoughSpaceAvailableWorks)
{
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0U], ActiveCallSet_test::triggerCallback<0U>).has_error());
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, AttachWithoutEnumTillCapacityIsFullWorks)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0U>).has_error());
        EXPECT_THAT(m_sut->size(), Eq(i + 1));
    }
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity()));
}

TEST_F(ActiveCallSet_test, DetachDecreasesSize)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0U>);
    }
    m_sut->detachEvent(m_simpleEvents[0U]);
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1U));
}

TEST_F(ActiveCallSet_test, AttachWithoutEnumOneMoreThanCapacityFails)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0U>);
        EXPECT_THAT(m_sut->size(), Eq(i + 1));
    }
    auto result = m_sut->attachEvent(m_simpleEvents[m_sut->capacity()], ActiveCallSet_test::triggerCallback<0U>);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::ACTIVE_CALL_SET_FULL));
}

TEST_F(ActiveCallSet_test, AttachingWithEnumIfEnoughSpaceAvailableWorks)
{
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0U], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>)
                     .has_error());
    EXPECT_THAT(m_sut->size(), Eq(1));
}

TEST_F(ActiveCallSet_test, AttachWithEnumTillCapacityIsFullWorks)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_FALSE(
            m_sut->attachEvent(m_simpleEvents[i], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>)
                .has_error());
        EXPECT_THAT(m_sut->size(), Eq(i + 1));
    }
}

TEST_F(ActiveCallSet_test, AttachWithEnumOneMoreThanCapacityFails)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);
    }
    auto result = m_sut->attachEvent(
        m_simpleEvents[m_sut->capacity()], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::ACTIVE_CALL_SET_FULL));
}

TEST_F(ActiveCallSet_test, DetachMakesSpaceForAnotherAttachWithEventEnum)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);
    }

    m_sut->detachEvent(m_simpleEvents[0U], SimpleEvent::Hypnotoad);
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[m_sut->capacity()],
                                   SimpleEvent::Hypnotoad,
                                   ActiveCallSet_test::triggerCallback<0U>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, DetachMakesSpaceForAnotherAttachWithoutEventEnum)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0U>);
    }

    m_sut->detachEvent(m_simpleEvents[0U]);
    EXPECT_FALSE(
        m_sut->attachEvent(m_simpleEvents[m_sut->capacity()], ActiveCallSet_test::triggerCallback<0U>).has_error());
}

TEST_F(ActiveCallSet_test, AttachingEventWithoutEventTypeLeadsToAttachedNoEventEnumTriggerHandle)
{
    m_sut->attachEvent(m_simpleEvents[0U], ActiveCallSet_test::triggerCallback<0U>);
    EXPECT_TRUE(m_simpleEvents[0U].m_handleNoEventEnum.isValid());
}

TEST_F(ActiveCallSet_test, AttachingEventWithEventTypeLeadsToAttachedTriggerHandle)
{
    m_sut->attachEvent(m_simpleEvents[0U], SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    EXPECT_TRUE(m_simpleEvents[0U].m_handleStoepsel.isValid());
}

TEST_F(ActiveCallSet_test, AttachingSameEventWithEventEnumTwiceFails)
{
    m_sut->attachEvent(m_simpleEvents[0U], SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    auto result = m_sut->attachEvent(
        m_simpleEvents[0U], SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(ActiveCallSet_test, AttachingSameEventWithoutEventEnumTwiceFails)
{
    m_sut->attachEvent(m_simpleEvents[0U], ActiveCallSet_test::triggerCallback<0U>);

    auto result = m_sut->attachEvent(m_simpleEvents[0U], ActiveCallSet_test::triggerCallback<0U>);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(ActiveCallSet_test, AttachingSameClassWithTwoDifferentEventsWorks)
{
    m_sut->attachEvent(m_simpleEvents[0U], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);

    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   ActiveCallSet_test::triggerCallback<0U>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, DetachingSameClassWithDifferentEventEnumChangesNothing)
{
    m_sut->attachEvent(m_simpleEvents[0U], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);

    m_sut->detachEvent(m_simpleEvents[0U], SimpleEvent::StoepselBachelorParty);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, DetachingDifferentClassWithSameEventEnumChangesNothing)
{
    m_sut->attachEvent(m_simpleEvents[0U], SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);

    m_sut->detachEvent(m_simpleEvents[1U], SimpleEvent::Hypnotoad);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, AttachingWithoutEnumTillCapacityFilledSetsUpNoEventEnumTriggerHandle)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0U>);
    }

    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_TRUE(m_simpleEvents[i].m_handleNoEventEnum.isValid());
    }
}

TEST_F(ActiveCallSet_test, DTorDetachesAllAttachedEvents)
{
    auto capacity = m_sut->capacity();
    for (uint64_t i = 0U; i < capacity; ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0U>);
    }

    m_sut.reset();

    for (uint64_t i = 0U; i < capacity; ++i)
    {
        EXPECT_FALSE(m_simpleEvents[i].m_handleHypnotoad.isValid());
    }
}

TEST_F(ActiveCallSet_test, AttachedEventDTorDetachesItself)
{
    {
        SimpleEventClass fuu;
        m_sut->attachEvent(fuu, ActiveCallSet_test::triggerCallback<0U>);
    }

    EXPECT_THAT(m_sut->size(), Eq(0U));
}

TEST_F(ActiveCallSet_test, AttachingSimpleEventWithoutEnumSetsNoEventEnumTriggerHandle)
{
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, ActiveCallSet_test::triggerCallback<0U>);

    EXPECT_TRUE(static_cast<bool>(fuu.m_handleNoEventEnum));
}

TEST_F(ActiveCallSet_test, DetachingSimpleEventResetsTriggerHandle)
{
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->detachEvent(fuu);

    EXPECT_FALSE(static_cast<bool>(fuu.m_handleHypnotoad));
}

TEST_F(ActiveCallSet_test, AttachingEventWithEnumSetsTriggerHandle)
{
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    EXPECT_TRUE(static_cast<bool>(fuu.m_handleStoepsel));
}

TEST_F(ActiveCallSet_test, DetachingEventWithEnumResetsTriggerHandle)
{
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->detachEvent(fuu, SimpleEvent::StoepselBachelorParty);

    EXPECT_FALSE(static_cast<bool>(fuu.m_handleStoepsel));
}

TEST_F(ActiveCallSet_test, DetachingNonAttachedEventResetsNothing)
{
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->detachEvent(fuu, SimpleEvent::Hypnotoad);

    EXPECT_TRUE(static_cast<bool>(fuu.m_handleStoepsel));
}

///////////////////////////////////
// END
///////////////////////////////////

///////////////////////////////////
// BEGIN calling callbacks
///////////////////////////////////
TIMING_TEST_F(ActiveCallSet_test, CallbackIsCalledAfterNotify, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count == 1U);
});

TIMING_TEST_F(ActiveCallSet_test, CallbackIsCalledOnlyOnceWhenTriggered, Repeat(5), [&] {
    SimpleEventClass fuu1;
    SimpleEventClass fuu2;
    m_sut->attachEvent(fuu1, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->attachEvent(fuu2, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<1U>);

    fuu1.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    fuu2.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == &fuu1);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count == 1U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source == &fuu2);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count == 1U);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerWhileInCallbackLeadsToAnotherCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    constexpr uint64_t NUMBER_OF_TRIGGER_UNBLOCKS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    fuu.triggerStoepsel();
    unblockTriggerCallback(NUMBER_OF_TRIGGER_UNBLOCKS);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count == 2U);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerWhileInCallbackLeadsToAnotherCallbackOnce, Repeat(5), [&] {
    SimpleEventClass fuu;
    SimpleEventClass bar;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->attachEvent(bar, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<1U>);

    constexpr uint64_t NUMBER_OF_TRIGGER_UNBLOCKS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    fuu.triggerStoepsel();
    bar.triggerStoepsel();
    unblockTriggerCallback(NUMBER_OF_TRIGGER_UNBLOCKS);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count == 2U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source == &bar);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count == 1U);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerMultipleTimesWhileInCallbackLeadsToAnotherCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    constexpr uint64_t NUMBER_OF_RETRIGGERS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (uint64_t i = 0U; i < NUMBER_OF_RETRIGGERS; ++i)
    {
        fuu.triggerStoepsel();
    }
    unblockTriggerCallback(NUMBER_OF_RETRIGGERS);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count == 2U);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerMultipleTimesWhileInCallbackLeadsToAnotherCallbackOnce, Repeat(5), [&] {
    SimpleEventClass fuu;
    SimpleEventClass bar;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->attachEvent(bar, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<1U>);

    constexpr uint64_t NUMBER_OF_RETRIGGERS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (uint64_t i = 0U; i < NUMBER_OF_RETRIGGERS; ++i)
    {
        fuu.triggerStoepsel();
    }
    bar.triggerStoepsel();
    unblockTriggerCallback(NUMBER_OF_RETRIGGERS + 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count == 2U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source == &bar);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count == 1U);
});

TIMING_TEST_F(ActiveCallSet_test, NoTriggerLeadsToNoCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(fuu, SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == nullptr);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count == 0U);
});

TIMING_TEST_F(ActiveCallSet_test, TriggeringAllEventsCallsAllCallbacks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U>::doIt(
        *m_sut, events, SimpleEvent::StoepselBachelorParty);

    activateTriggerCallbackBlocker();
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (auto& e : events)
    {
        e.triggerStoepsel();
    }

    // 10 times more callback runs allowed to allow potential overtriggering
    unblockTriggerCallback(10U * iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_source == &events[0U]);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_count == 2U);
    for (uint64_t i = 1U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source == &events[i]);
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_count == 1U);
    }
});

TIMING_TEST_F(ActiveCallSet_test, TriggeringAllEventsCallsAllCallbacksOnce, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1>::doIt(
        *m_sut, events, SimpleEvent::StoepselBachelorParty);

    activateTriggerCallbackBlocker();
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (auto& e : events)
    {
        e.triggerStoepsel();
    }

    // 10 times more callback runs allowed to allow potential overtriggering
    unblockTriggerCallback(10U * iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_source == &events[0U]);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_count == 3U);
    for (uint64_t i = 1U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source == &events[i]);
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_count == 1U);
    }
});
//////////////////////////////////
// END
//////////////////////////////////

//////////////////////////////////
// BEGIN concurrent attach / detach
//////////////////////////////////
TIMING_TEST_F(ActiveCallSet_test, AttachingWhileCallbackIsRunningWorks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    m_sut->attachEvent(events[1U], SimpleEvent::StoepselBachelorParty, triggerCallback<1U>);
    events[1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS * 2U));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source == &events[1U]);
});

TIMING_TEST_F(ActiveCallSet_test, AttachingMultipleWhileCallbackIsRunningWorks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    m_sut->attachEvent(events[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U],
                       SimpleEvent::StoepselBachelorParty,
                       triggerCallback<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U>);

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 2U>::doIt(
        *m_sut, events, SimpleEvent::StoepselBachelorParty);

    g_triggerCallbackRuntimeInMs = 0U;
    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        events[i].triggerStoepsel();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source == &events[i]);
    }
});

TIMING_TEST_F(ActiveCallSet_test, DetachingWhileCallbackIsRunningWorks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    g_triggerCallbackArg[0U].m_source = nullptr;
    m_sut->detachEvent(events[0U], SimpleEvent::StoepselBachelorParty);
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == nullptr);
});

TIMING_TEST_F(ActiveCallSet_test, DetachingWhileCallbackIsRunningBlocksDetach, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);
    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 4U));

    auto begin = std::chrono::system_clock::now();
    m_sut->detachEvent(events[0U], SimpleEvent::StoepselBachelorParty);
    auto end = std::chrono::system_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    TIMING_TEST_EXPECT_TRUE(static_cast<uint64_t>(elapsed.count()) > CALLBACK_WAIT_IN_MS / 2U);
});

TIMING_TEST_F(ActiveCallSet_test, EventDestructorBlocksWhenCallbackIsRunning, Repeat(5), [&] {
    SimpleEventClass* event = new SimpleEventClass();
    m_sut->attachEvent(*event, SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);
    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    event->triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 4U));

    auto begin = std::chrono::system_clock::now();
    delete event;
    auto end = std::chrono::system_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    TIMING_TEST_EXPECT_TRUE(static_cast<uint64_t>(elapsed.count()) > CALLBACK_WAIT_IN_MS / 2U);
});


TIMING_TEST_F(ActiveCallSet_test, DetachingMultipleWhileCallbackIsRunningWorks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U>::doIt(
        *m_sut, events, SimpleEvent::StoepselBachelorParty);

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    g_triggerCallbackRuntimeInMs = 0U;
    for (auto& e : events)
    {
        m_sut->detachEvent(e, SimpleEvent::StoepselBachelorParty);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    for (auto& t : g_triggerCallbackArg)
    {
        t.m_source = nullptr;
    }
    for (auto& e : events)
    {
        e.triggerStoepsel();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source == nullptr);
    }
});

TIMING_TEST_F(ActiveCallSet_test, AttachingDetachingRunsIndependentOfCallback, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_sut->attachEvent(events[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U],
                       SimpleEvent::StoepselBachelorParty,
                       triggerCallback<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U>);
    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 2U>::doIt(
        *m_sut, events, SimpleEvent::StoepselBachelorParty);

    for (auto& e : events)
    {
        m_sut->detachEvent(e, SimpleEvent::StoepselBachelorParty);
    }

    // EXPECT_* (assert step) is inside of doIt call. We expect that every event can
    // be attached
    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 2U>::doIt(
        *m_sut, events, SimpleEvent::StoepselBachelorParty);
});
//////////////////////////////////
// END
//////////////////////////////////

//////////////////////////////////
// BEGIN attach / detach in callbacks
//////////////////////////////////
TIMING_TEST_F(ActiveCallSet_test, DetachingSelfInCallbackWorks, Repeat(5), [&] {
    g_toBeDetached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    g_toBeDetached->push_back({&events[0U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, detachCallback);

    events[0U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_sut->size() == 0U);
});

TIMING_TEST_F(ActiveCallSet_test, DetachingNonSelfEventInCallbackWorks, Repeat(5), [&] {
    g_toBeDetached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    g_toBeDetached->push_back({&events[1U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, detachCallback);
    m_sut->attachEvent(events[1U], SimpleEvent::StoepselBachelorParty, triggerCallback<1U>);

    events[0U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_sut->size() == 1U);
});

TIMING_TEST_F(ActiveCallSet_test, DetachedCallbacksAreNotBeingCalledWhenTriggeredBefore, Repeat(5), [&] {
    g_toBeDetached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    g_toBeDetached->push_back({&events[1U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, notifyAndThenDetachStoepselCallback);
    m_sut->attachEvent(events[1U], SimpleEvent::StoepselBachelorParty, triggerCallback<1U>);

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    g_triggerCallbackArg[1U].m_source = nullptr;
    g_triggerCallbackRuntimeInMs = 0U;

    events[0U].triggerStoepsel();
    events[1U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source == nullptr);
});

TIMING_TEST_F(ActiveCallSet_test, AttachingInCallbackWorks, Repeat(1), [&] {
    g_toBeAttached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    g_toBeAttached->push_back({&events[1U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, attachCallback);

    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    events[1U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source == &events[1U]);
});
//////////////////////////////////
// END
//////////////////////////////////

