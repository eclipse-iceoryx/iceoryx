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

        void invalidateTrigger(const uint64_t id)
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

        void disableEvent(const SimpleEvent event)
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

        void disableEvent()
        {
            m_handleHypnotoad.reset();
        }

        void triggerStoepsel()
        {
            m_hasTriggered.store(true);
            m_handleStoepsel.trigger();
        }

        void resetTrigger()
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
    static void triggerCallback(ActiveCallSet_test::SimpleEventClass* const event)
    {
        ActiveCallSet_test::m_triggerCallbackArg[N] = event;
        std::this_thread::sleep_for(std::chrono::milliseconds(m_triggerCallbackRuntimeInMs));
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
    };

    void TearDown(){};

    using eventVector_t = iox::cxx::vector<SimpleEventClass, iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET + 1>;
    eventVector_t m_simpleEvents{iox::MAX_NUMBER_OF_EVENTS_PER_WAITSET + 1};

    static std::array<SimpleEventClass*, iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> m_triggerCallbackArg;
    static constexpr uint64_t CALLBACK_WAIT_IN_MS = 10U;
    static uint64_t m_triggerCallbackRuntimeInMs;
};
uint64_t ActiveCallSet_test::SimpleEventClass::m_invalidateTriggerId = 0U;
std::array<ActiveCallSet_test::SimpleEventClass*, iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET>
    ActiveCallSet_test::m_triggerCallbackArg;
constexpr uint64_t ActiveCallSet_test::CALLBACK_WAIT_IN_MS;
uint64_t ActiveCallSet_test::m_triggerCallbackRuntimeInMs;

//////////////////////////////////
// attach / detach test collection
//////////////////////////////////

TEST_F(ActiveCallSet_test, IsEmptyWhenConstructed)
{
    EXPECT_THAT(m_sut->size(), Eq(0U));
}

TEST_F(ActiveCallSet_test, AttachingWithoutEnumIfEnoughSpaceAvailableWorks)
{
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0], ActiveCallSet_test::triggerCallback<0>).has_error());
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, AttachWithoutEnumTillCapacityIsFullWorks)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0>).has_error());
    }
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity()));
}

TEST_F(ActiveCallSet_test, DetachDecreasesSize)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0>);
    }
    m_sut->detachEvent(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1));
}

TEST_F(ActiveCallSet_test, AttachWithoutEnumOneMoreThanCapacityFails)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0>).has_error();
    }
    auto result = m_sut->attachEvent(m_simpleEvents[m_sut->capacity()], ActiveCallSet_test::triggerCallback<0>);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::ACTIVE_CALL_SET_FULL));
}

TEST_F(ActiveCallSet_test, AttachingWithEnumIfEnoughSpaceAvailableWorks)
{
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0],
                                   ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                   ActiveCallSet_test::triggerCallback<0>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, AttachWithEnumTillCapacityIsFullWorks)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_FALSE(m_sut
                         ->attachEvent(m_simpleEvents[i],
                                       ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                       ActiveCallSet_test::triggerCallback<0>)
                         .has_error());
    }
}

TEST_F(ActiveCallSet_test, AttachWithEnumOneMoreThanCapacityFails)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut
            ->attachEvent(
                m_simpleEvents[i], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0>)
            .has_error();
    }
    auto result = m_sut->attachEvent(m_simpleEvents[m_sut->capacity()],
                                     ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                     ActiveCallSet_test::triggerCallback<0>);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::ACTIVE_CALL_SET_FULL));
}

TEST_F(ActiveCallSet_test, DetachMakesSpaceForAnotherAttachWithEventEnum)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut
            ->attachEvent(
                m_simpleEvents[i], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0>)
            .has_error();
    }

    m_sut->detachEvent(m_simpleEvents[0], ActiveCallSet_test::SimpleEvent::Hypnotoad);
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[m_sut->capacity()],
                                   ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                   ActiveCallSet_test::triggerCallback<0>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, DetachMakesSpaceForAnotherAttachWithoutEventEnum)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0>).has_error();
    }

    m_sut->detachEvent(m_simpleEvents[0]);
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[m_sut->capacity()],
                                   ActiveCallSet_test::SimpleEvent::Hypnotoad,
                                   ActiveCallSet_test::triggerCallback<0>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, AttachingEventWithoutEventTypeLeadsToAttachedTriggerHandle)
{
    m_sut->attachEvent(m_simpleEvents[0], ActiveCallSet_test::triggerCallback<0>);
    EXPECT_TRUE(m_simpleEvents[0].m_handleHypnotoad.isValid());
}

TEST_F(ActiveCallSet_test, AttachingEventWithEventTypeLeadsToAttachedTriggerHandle)
{
    m_sut->attachEvent(m_simpleEvents[0],
                       ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                       ActiveCallSet_test::triggerCallback<0>);
    EXPECT_TRUE(m_simpleEvents[0].m_handleStoepsel.isValid());
}

TEST_F(ActiveCallSet_test, AttachingSameEventWithEventEnumTwiceFails)
{
    m_sut->attachEvent(m_simpleEvents[0],
                       ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                       ActiveCallSet_test::triggerCallback<0>);

    auto result = m_sut->attachEvent(m_simpleEvents[0],
                                     ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                                     ActiveCallSet_test::triggerCallback<0>);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(ActiveCallSet_test, AttachingSameEventWithoutEventEnumTwiceFails)
{
    m_sut->attachEvent(m_simpleEvents[0], ActiveCallSet_test::triggerCallback<0>);

    auto result = m_sut->attachEvent(m_simpleEvents[0], ActiveCallSet_test::triggerCallback<0>);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ActiveCallSetError::EVENT_ALREADY_ATTACHED));
}

TEST_F(ActiveCallSet_test, AttachingSameClassWithTwoDifferentEventsWorks)
{
    m_sut->attachEvent(
        m_simpleEvents[0], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0>);

    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0],
                                   ActiveCallSet_test::SimpleEvent::StoepselBachelorParty,
                                   ActiveCallSet_test::triggerCallback<0>)
                     .has_error());
}

TEST_F(ActiveCallSet_test, DetachingSameClassWithDifferentEventEnumChangesNothing)
{
    m_sut->attachEvent(
        m_simpleEvents[0], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0>);

    m_sut->detachEvent(m_simpleEvents[0], ActiveCallSet_test::SimpleEvent::StoepselBachelorParty);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, DetachingDifferentClassWithSameEventEnumChangesNothing)
{
    m_sut->attachEvent(
        m_simpleEvents[0], ActiveCallSet_test::SimpleEvent::Hypnotoad, ActiveCallSet_test::triggerCallback<0>);

    m_sut->detachEvent(m_simpleEvents[1], ActiveCallSet_test::SimpleEvent::Hypnotoad);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(ActiveCallSet_test, AttachingTillCapacityFilledSetsUpTriggerHandle)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0>).has_error();
    }

    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_TRUE(m_simpleEvents[i].m_handleHypnotoad.isValid());
    }
}

TEST_F(ActiveCallSet_test, DTorDetachesAllAttachedEvents)
{
    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        m_sut->attachEvent(m_simpleEvents[i], ActiveCallSet_test::triggerCallback<0>).has_error();
    }

    m_sut.reset();

    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_FALSE(m_simpleEvents[i].m_handleHypnotoad.isValid());
    }
}

TEST_F(ActiveCallSet_test, AttachedEventDTorDetachesItself)
{
    {
        SimpleEventClass fuu;
        m_sut->attachEvent(fuu, ActiveCallSet_test::triggerCallback<0>);
    }

    EXPECT_THAT(m_sut->size(), Eq(0U));
}

//////////////////////////////////
// concurrent attach / detach
//////////////////////////////////


// -- goes out of scope while callback is running


///////////////////////////////////
// calling callbacks
///////////////////////////////////
TIMING_TEST_F(ActiveCallSet_test, CallbackIsCalledAfterNotify, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0>);

    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0] == &fuu);
});

TIMING_TEST_F(ActiveCallSet_test, CallbackIsCalledOnlyOnceWhenTriggered, Repeat(5), [&] {
    SimpleEventClass fuu1;
    SimpleEventClass fuu2;
    m_sut->attachEvent(
        fuu1, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0>);
    m_sut->attachEvent(
        fuu2, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<1>);

    fuu1.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0] = nullptr;
    fuu2.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0] == nullptr);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerWhileInCallbackLeadsToAnotherOneTimeCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0>);

    m_triggerCallbackRuntimeInMs = 3 * CALLBACK_WAIT_IN_MS / 2;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0] = nullptr;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0] == &fuu);
});

TIMING_TEST_F(ActiveCallSet_test, TriggerMultipleTimesWhileInCallbackLeadsToAnotherOneTimeCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0>);

    m_triggerCallbackRuntimeInMs = 3 * CALLBACK_WAIT_IN_MS / 2;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0] = nullptr;
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    m_triggerCallbackArg[0] = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS * 2));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0] == nullptr);
});

TIMING_TEST_F(ActiveCallSet_test, NoTriggerLeadsToNoCallback, Repeat(5), [&] {
    SimpleEventClass fuu;
    m_sut->attachEvent(
        fuu, ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<0>);

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0] == nullptr);
});

template <uint64_t N>
struct AttachEvent;

template <>
struct AttachEvent<0>
{
};

TIMING_TEST_F(ActiveCallSet_test, TriggeringAllEventsCallsAllCallbacksOnce, Repeat(5), [&] {
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
        m_sut->attachEvent(
            events[i], ActiveCallSet_test::SimpleEvent::StoepselBachelorParty, ActiveCallSet_test::triggerCallback<i>);

    m_triggerCallbackRuntimeInMs = 3 * CALLBACK_WAIT_IN_MS / 2;
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2));

    TIMING_TEST_EXPECT_TRUE(m_triggerCallbackArg[0] == nullptr);
});

