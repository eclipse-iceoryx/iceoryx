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

class ActiveCallSet_test : public Test
{
  public:
    enum SimpleEvent
    {
        StoepselBachelorParty,
        Hypnotoad
    };

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
            if (event == SimpleEvent::StoepselBachelorParty)
            {
                m_handleStoepsel = std::move(handle);
            }
            else
            {
                m_handleHypnotoad = std::move(handle);
            }
        }

        void enableEvent(iox::popo::TriggerHandle&& handle) noexcept
        {
            m_handleHypnotoad = std::move(handle);
        }

        void invalidateTrigger(const uint64_t id) noexcept
        {
            m_invalidateTriggerId = id;
            if (m_handleHypnotoad.getUniqueId() == id)
            {
                m_handleHypnotoad.invalidate();
            }
            else if (m_handleStoepsel.getUniqueId() == id)
            {
                m_handleStoepsel.invalidate();
            }
        }

        void disableEvent(const SimpleEvent event) noexcept
        {
            if (event == SimpleEvent::StoepselBachelorParty)
            {
                m_handleStoepsel.reset();
            }
            else
            {
                m_handleHypnotoad.reset();
            }
        }

        void disableEvent() noexcept
        {
            m_handleHypnotoad.reset();
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
        mutable std::atomic_bool m_hasTriggered{false};
        static uint64_t m_invalidateTriggerId;
    };

    class ActiveCallSetMock : public ActiveCallSet
    {
      public:
        ActiveCallSetMock(EventVariableData* data) noexcept
            : ActiveCallSet(data)
        {
        }
    };

    EventVariableData m_eventVarData{"Maulbeerblatt"};
    iox::cxx::optional<ActiveCallSetMock> m_sut;

    template <uint64_t N>
    static void triggerCallback(ActiveCallSet_test::SimpleEventClass* const event) noexcept
    {
        ActiveCallSet_test::m_triggerCallbackArg[N] = event;
        std::this_thread::sleep_for(std::chrono::milliseconds(m_triggerCallbackRuntimeInMs));
    }

    static void attachCallback(ActiveCallSet_test::SimpleEventClass* const) noexcept
    {
        for (auto& e : m_toBeAttached)
        {
            e.sut->attachEvent(*e.object, SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);
        }
    }

    static void detachCallback(ActiveCallSet_test::SimpleEventClass* const) noexcept
    {
        for (auto& e : m_toBeDetached)
        {
            e.sut->detachEvent(*e.object, SimpleEvent::StoepselBachelorParty);
        }
    }

    static void notifyAndThenDetachCallback(ActiveCallSet_test::SimpleEventClass* const) noexcept
    {
        for (auto& e : m_toBeDetached)
        {
            e.object->triggerStoepsel();
            e.sut->detachEvent(*e.object, SimpleEvent::StoepselBachelorParty);
        }
    }


    void SetUp()
    {
        for (auto& e : m_triggerCallbackArg)
        {
            e = nullptr;
        }
        m_sut.emplace(&m_eventVarData);
        ActiveCallSet_test::SimpleEventClass::m_invalidateTriggerId = 0U;
        ActiveCallSet_test::m_triggerCallbackRuntimeInMs = 0U;
        ActiveCallSet_test::m_toBeAttached.clear();
        ActiveCallSet_test::m_toBeDetached.clear();
    };

    void TearDown(){};

    struct ToBeAttached_t
    {
        SimpleEventClass* object;
        ActiveCallSetMock* sut;
    };

    // + 1U to test the maximum capacity
    using eventVector_t = iox::cxx::vector<SimpleEventClass, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET + 1U>;
    eventVector_t m_simpleEvents{iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET + 1U};

    static std::vector<ToBeAttached_t> m_toBeAttached;
    static std::vector<ToBeAttached_t> m_toBeDetached;
    static std::array<SimpleEventClass*, iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> m_triggerCallbackArg;
    static constexpr uint64_t CALLBACK_WAIT_IN_MS = 100U;
    static uint64_t m_triggerCallbackRuntimeInMs;
};
uint64_t ActiveCallSet_test::SimpleEventClass::m_invalidateTriggerId = 0U;
std::array<ActiveCallSet_test::SimpleEventClass*, iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET>
    ActiveCallSet_test::m_triggerCallbackArg;
constexpr uint64_t ActiveCallSet_test::CALLBACK_WAIT_IN_MS;
uint64_t ActiveCallSet_test::m_triggerCallbackRuntimeInMs;
std::vector<ActiveCallSet_test::ToBeAttached_t> ActiveCallSet_test::m_toBeAttached;
std::vector<ActiveCallSet_test::ToBeAttached_t> ActiveCallSet_test::m_toBeDetached;

//////////////////////////////////
// attach / detach
//////////////////////////////////
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
    }
    auto result = m_sut->attachEvent(m_simpleEvents[m_sut->capacity()], ActiveCallSet_test::triggerCallback<0U>);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::ACTIVE_CALL_SET_FULL));
}

TEST_F(ActiveCallSet_test, AttachingWithEnumIfEnoughSpaceAvailableWorks)
{
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                   ActiveCallSet_test::triggerCallback<0U>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, AttachWithEnumTillCapacityIsFullWorks)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_FALSE(m_sut
                         ->attachEvent(m_simpleEvents[i],
                                       ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                       ActiveCallSet_test::triggerCallback<0U>)
                         .has_error());
    }
}

TEST_F(ActiveCallSet_test, AttachWithEnumOneMoreThanCapacityFails)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(
            m_simpleEvents[i], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);
    }
    auto result = m_sut->attachEvent(m_simpleEvents[m_sut->capacity()],
                                     ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                     ActiveCallSet_test::triggerCallback<0U>);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::ACTIVE_CALL_SET_FULL));
}

TEST_F(ActiveCallSet_test, DetachMakesSpaceForAnotherAttachWithEventEnum)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(
            m_simpleEvents[i], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);
    }

    m_sut->detachEvent(m_simpleEvents[0U], ActiveCallSet_test::SimpleEvent::Hypnotoad);
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[m_sut->capacity()],
                                   ActiveCallSet_test::SimpleEvent::Hypnotoad,
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
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[m_sut->capacity()],
                                   ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                   ActiveCallSet_test::triggerCallback<0U>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, AttachingEventWithoutEventTypeLeadsToAttachedTriggerHandle)
{
    m_sut->attachEvent(m_simpleEvents[0U], ActiveCallSet_test::triggerCallback<0U>);
    EXPECT_TRUE(m_simpleEvents[0U].m_handleHypnotoad.isValid());
}

TEST_F(ActiveCallSet_test, AttachingEventWithEventTypeLeadsToAttachedTriggerHandle)
{
    m_sut->attachEvent(m_simpleEvents[0U],
                       ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                       ActiveCallSet_test::triggerCallback<0U>);
    EXPECT_TRUE(m_simpleEvents[0U].m_handleStoepsel.isValid());
}

TEST_F(ActiveCallSet_test, AttachingSameEventWithEventEnumTwiceFails)
{
    m_sut->attachEvent(m_simpleEvents[0U],
                       ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                       ActiveCallSet_test::triggerCallback<0U>);

    auto result = m_sut->attachEvent(m_simpleEvents[0U],
                                     ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                                     ActiveCallSet_test::triggerCallback<0U>);
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
    m_sut->attachEvent(
        m_simpleEvents[0U], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);

    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                                   ActiveCallSet_test::triggerCallback<0U>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, DetachingSameClassWithDifferentEventEnumChangesNothing)
{
    m_sut->attachEvent(
        m_simpleEvents[0U], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);

    m_sut->detachEvent(m_simpleEvents[0U], ActiveCallSet_test::SimpleEvent::StoepselBachelorParty);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, DetachingDifferentClassWithSameEventEnumChangesNothing)
{
    m_sut->attachEvent(
        m_simpleEvents[0U], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0U>);

    m_sut->detachEvent(m_simpleEvents[1U], ActiveCallSet_test::SimpleEvent::Hypnotoad);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, AttachingTillCapacityFilledSetsUpTriggerHandle)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0U>);
    }

    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_TRUE(m_simpleEvents[i].m_handleHypnotoad.isValid());
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

///////////////////////////////////
// calling callbacks
///////////////////////////////////
TIMING_TEST_F(ActiveCallSet_test, CallbackIsCalledAfterNotify, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == &fuu);
});

TIMING_TEST_F(ActiveCallSet_test, CallbackIsCalledOnlyOnceWhenTriggered, Repeat(5), [&] {
    SimpleEventClass fuu1;
    SimpleEventClass fuu2;
    m_sut->attachEvent(
        fuu1, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->attachEvent(
        fuu2, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<1U>);

    fuu1.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0U] = nullptr;
    fuu2.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == nullptr);
    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[1U] == &fuu2);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerWhileInCallbackLeadsToAnotherCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0U] = nullptr;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == &fuu);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerWhileInCallbackLeadsToAnotherCallbackOnce, Repeat(5), [&] {
    SimpleEventClass fuu;
    SimpleEventClass bar;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->attachEvent(
        bar, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<1U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0U] = nullptr;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    m_triggerCallbackArg[0U] = nullptr;

    bar.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(4 * CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == nullptr);
    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[1U] == &bar);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerMultipleTimesWhileInCallbackLeadsToAnotherCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    m_triggerCallbackArg[0U] = nullptr;
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS * 2U));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == &fuu);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerMultipleTimesWhileInCallbackLeadsToAnotherCallbackOnce, Repeat(5), [&] {
    SimpleEventClass fuu;
    SimpleEventClass bar;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);
    m_sut->attachEvent(
        bar, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<1U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0U] = nullptr;
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    // trigger bar and fuu should not be triggered again
    m_triggerCallbackArg[0U] = nullptr;
    bar.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(4U * CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == nullptr);
    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[1U] == &bar);
});

TIMING_TEST_F(ActiveCallSet_test, NoTriggerLeadsToNoCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0U>);

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == nullptr);
});

template <uint64_t N>
struct AttachEvent
{
    template <typename EventType>
    static void doIt(ActiveCallSet_test::ActiveCallSetMock& sut,
                     std::vector<ActiveCallSet_test::SimpleEventClass>& events,
                     const EventType event)
    {
        EXPECT_THAT(sut.attachEvent(events[N], event, ActiveCallSet_test::triggerCallback<N>).has_error(), Eq(false));
        AttachEvent<N - 1U>::doIt(sut, events, event);
    }
};

template <>
struct AttachEvent<0U>
{
    template <typename EventType>
    static void doIt(ActiveCallSet_test::ActiveCallSetMock& sut,
                     std::vector<ActiveCallSet_test::SimpleEventClass>& events,
                     const EventType event)
    {
        EXPECT_THAT(sut.attachEvent(events[0U], event, ActiveCallSet_test::triggerCallback<0U>).has_error(), Eq(false));
    }
};

TIMING_TEST_F(ActiveCallSet_test, TriggeringAllEventsCallsAllCallbacks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U>::doIt(
        *m_sut, events, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    // we triggered events[0] with a long runtime to safely trigger all events again
    // while the callback is still running. to verify that events[0] is called again
    // we reset the m_triggerCallbackArg[0] to nullptr (which is again set by the
    // event[0] callback) and set the runtime of the callbacks to zero
    m_triggerCallbackArg[0U] = nullptr;
    m_triggerCallbackRuntimeInMs = 0U;

    for (auto& e : events)
        e.triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[i] == &events[i]);
    }
});

TIMING_TEST_F(ActiveCallSet_test, TriggeringAllEventsCallsAllCallbacksOnce, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1>::doIt(
        *m_sut, events, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    m_triggerCallbackRuntimeInMs = 0U;

    for (auto& e : events)
        e.triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    for (auto& t : m_triggerCallbackArg)
    {
        t = nullptr;
    }

    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0] == &events[0U]);
    for (uint64_t i = 1U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[i] == nullptr);
    }
});

//////////////////////////////////
// concurrent attach / detach
//////////////////////////////////
TIMING_TEST_F(ActiveCallSet_test, AttachingWhileCallbackIsRunningWorks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    m_sut->attachEvent(events[1U], SimpleEvent::StoepselBachelorParty, triggerCallback<1U>);
    events[1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS * 2U));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[1U] == &events[1U]);
});

TIMING_TEST_F(ActiveCallSet_test, AttachingMultipleWhileCallbackIsRunningWorks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    m_sut->attachEvent(events[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U],
                       SimpleEvent::StoepselBachelorParty,
                       triggerCallback<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 2U>::doIt(
        *m_sut, events, SimpleEvent::StoepselBachelorParty);

    m_triggerCallbackRuntimeInMs = 0U;
    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        events[i].triggerStoepsel();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[i] == &events[i]);
    }
});

TIMING_TEST_F(ActiveCallSet_test, DetachingWhileCallbackIsRunningWorks, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    m_triggerCallbackArg[0U] = nullptr;
    m_sut->detachEvent(events[0U], SimpleEvent::StoepselBachelorParty);
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == nullptr);
});

TIMING_TEST_F(ActiveCallSet_test, DetachingWhileCallbackIsRunningBlocksDetach, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, triggerCallback<0U>);
    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
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
    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
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

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    m_triggerCallbackRuntimeInMs = 0U;
    for (auto& e : events)
    {
        m_sut->detachEvent(e, SimpleEvent::StoepselBachelorParty);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    for (auto& t : m_triggerCallbackArg)
    {
        t = nullptr;
    }
    for (auto& e : events)
    {
        e.triggerStoepsel();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[i] == nullptr);
    }
});

TIMING_TEST_F(ActiveCallSet_test, AttachingDetachingRunsIndependentOfCallback, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_sut->attachEvent(events[iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U],
                       SimpleEvent::StoepselBachelorParty,
                       triggerCallback<iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U>);
    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
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
// attach / detach in callbacks
//////////////////////////////////
TIMING_TEST_F(ActiveCallSet_test, DetachingSelfInCallbackWorks, Repeat(5), [&] {
    m_toBeDetached.clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_toBeDetached.push_back({&events[0U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, detachCallback);

    events[0U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_sut->size() == 0U);
});

TIMING_TEST_F(ActiveCallSet_test, DetachingNonSelfEventInCallbackWorks, Repeat(5), [&] {
    m_toBeDetached.clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_toBeDetached.push_back({&events[1U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, detachCallback);
    m_sut->attachEvent(events[1U], SimpleEvent::StoepselBachelorParty, triggerCallback<1U>);

    events[0U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_sut->size() == 1U);
});

TIMING_TEST_F(ActiveCallSet_test, DetachedCallbacksAreNotBeingCalledWhenTriggeredBefore, Repeat(5), [&] {
    m_toBeDetached.clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_toBeDetached.push_back({&events[1U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, notifyAndThenDetachCallback);
    m_sut->attachEvent(events[1U], SimpleEvent::StoepselBachelorParty, triggerCallback<1U>);

    m_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    m_triggerCallbackArg[1U] = nullptr;
    m_triggerCallbackRuntimeInMs = 0U;

    events[0U].triggerStoepsel();
    events[1U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[1U] == nullptr);
});

TIMING_TEST_F(ActiveCallSet_test, AttachingInCallbackWorks, Repeat(1), [&] {
    m_toBeAttached.clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
    m_toBeAttached.push_back({&events[1U], &*m_sut});
    m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, attachCallback);

    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    events[1U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0U] == &events[1U]);
});
