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

#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iox/atomic.hpp"
#include "iox/optional.hpp"
#include "iox/smart_lock.hpp"
#include "iox/unnamed_semaphore.hpp"
#include "iox/vector.hpp"
#include "test.hpp"

#include <array>
#include <chrono>
#include <memory>
#include <thread>

namespace
{
using namespace ::testing;

using namespace iox::popo;
using namespace iox::units::duration_literals;

namespace
{
enum SimpleEvent : iox::popo::EventEnumIdentifier
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
        m_handleStoepsel.trigger();
    }
    void triggerNoEventType() noexcept
    {
        m_handleNoEventEnum.trigger();
    }

    iox::popo::TriggerHandle m_handleHypnotoad;
    iox::popo::TriggerHandle m_handleStoepsel;
    iox::popo::TriggerHandle m_handleNoEventEnum;
};

class TestListener : public Listener
{
  public:
    TestListener(ConditionVariableData& data) noexcept
        : Listener(data)
    {
    }
};

struct EventAndSutPair_t
{
    SimpleEventClass* object;
    TestListener* sut;
};

struct TriggerSourceAndCount
{
    iox::concurrent::Atomic<SimpleEventClass*> m_source{nullptr};
    iox::concurrent::Atomic<uint64_t> m_count{0U};
};

iox::concurrent::smart_lock<std::vector<EventAndSutPair_t>> g_toBeAttached;
iox::concurrent::smart_lock<std::vector<EventAndSutPair_t>> g_toBeDetached;
std::array<TriggerSourceAndCount, iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER> g_triggerCallbackArg;
uint64_t g_triggerCallbackRuntimeInMs = 0U;
iox::optional<iox::UnnamedSemaphore> g_callbackBlocker;

class Listener_test : public Test
{
  public:
    template <uint64_t N>
    static void triggerCallback(SimpleEventClass* const event) noexcept
    {
        g_triggerCallbackArg[N].m_source = event;
        ++g_triggerCallbackArg[N].m_count;

        if (g_callbackBlocker)
        {
            IOX_DISCARD_RESULT(g_callbackBlocker->wait());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(g_triggerCallbackRuntimeInMs));
    }

    static void triggerCallbackWithUserType(SimpleEventClass* const event, uint64_t* userType) noexcept
    {
        g_triggerCallbackArg[0].m_source = event;
        ++(*userType);
    }

    static void attachCallback(SimpleEventClass* const) noexcept
    {
        for (auto& e : g_toBeAttached.get_copy())
        {
            ASSERT_FALSE(e.sut
                             ->attachEvent(*e.object,
                                           SimpleEvent::StoepselBachelorParty,
                                           createNotificationCallback(triggerCallback<0U>))
                             .has_error());
        }
    }

    static void detachCallback(SimpleEventClass* const) noexcept
    {
        for (auto& e : g_toBeDetached.get_copy())
        {
            e.sut->detachEvent(*e.object, SimpleEvent::StoepselBachelorParty);
        }
    }

    static void notifyAndThenDetachStoepselCallback(SimpleEventClass* const) noexcept
    {
        for (auto& e : g_toBeDetached.get_copy())
        {
            e.object->triggerStoepsel();
            e.sut->detachEvent(*e.object, SimpleEvent::StoepselBachelorParty);
        }
    }


    void SetUp() override
    {
        g_callbackBlocker.reset();
        for (auto& e : g_triggerCallbackArg)
        {
            e.m_source = nullptr;
            e.m_count = 0U;
        }
        m_sut.emplace(m_condVarData);
        g_invalidateTriggerId = 0U;
        g_triggerCallbackRuntimeInMs = 0U;
        g_toBeAttached->clear();
        g_toBeDetached->clear();
    };

    void activateTriggerCallbackBlocker() noexcept
    {
        iox::UnnamedSemaphoreBuilder()
            .initialValue(0U)
            .isInterProcessCapable(false)
            .create(g_callbackBlocker)
            .expect("Unable to create callback blocker semaphore");
    }

    void unblockTriggerCallback(const uint64_t numberOfUnblocks) noexcept
    {
        for (uint64_t i = 0U; i < numberOfUnblocks; ++i)
        {
            ASSERT_TRUE(static_cast<bool>(g_callbackBlocker));
            IOX_DISCARD_RESULT(g_callbackBlocker->post());
        }
    }

    void fillUpWithSimpleEvents()
    {
        for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
        {
            EXPECT_FALSE(
                m_sut->attachEvent(m_simpleEvents[i], createNotificationCallback(Listener_test::triggerCallback<0U>))
                    .has_error());
            EXPECT_THAT(m_sut->size(), Eq(i + 1U));
        }
    }
    bool fillUpWithSimpleEventsWithEnum(const SimpleEvent eventType)
    {
        for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
        {
            bool hasError = m_sut
                                ->attachEvent(m_simpleEvents[i],
                                              eventType,
                                              createNotificationCallback(Listener_test::triggerCallback<0U>))
                                .has_error();
            EXPECT_FALSE(hasError);
            if (hasError)
            {
                return false;
            }

            EXPECT_THAT(m_sut->size(), Eq(i + 1U));
        }
        return true;
    }

    void TearDown() override
    {
    }

    static constexpr uint64_t OVERFLOW_TEST_APPENDIX = 1U;
    using eventArray_t = SimpleEventClass[iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER + OVERFLOW_TEST_APPENDIX];
    eventArray_t m_simpleEvents;
    ConditionVariableData m_condVarData{"Maulbeerblättle"};
    iox::optional<TestListener> m_sut;

    const iox::units::Duration m_fatalTimeout = 2_s;
    Watchdog m_watchdog{m_fatalTimeout};
    static constexpr uint64_t CALLBACK_WAIT_IN_MS = 100U;
};

constexpr uint64_t Listener_test::CALLBACK_WAIT_IN_MS;
constexpr uint64_t Listener_test::OVERFLOW_TEST_APPENDIX;

template <uint64_t N>
struct AttachEvent
{
    template <typename EventType>
    static void doIt(TestListener& sut, std::vector<SimpleEventClass>& events, const EventType event)
    {
        EXPECT_THAT(sut.attachEvent(events[N], event, createNotificationCallback(Listener_test::triggerCallback<N>))
                        .has_error(),
                    Eq(false));
        AttachEvent<N - 1U>::doIt(sut, events, event);
    }
};

template <>
struct AttachEvent<0U>
{
    template <typename EventType>
    static void doIt(TestListener& sut, std::vector<SimpleEventClass>& events, const EventType event)
    {
        EXPECT_THAT(sut.attachEvent(events[0U], event, createNotificationCallback(Listener_test::triggerCallback<0U>))
                        .has_error(),
                    Eq(false));
    }
};
} // namespace


//////////////////////////////////
// BEGIN attach / detach
//////////////////////////////////
TEST_F(Listener_test, CapacityIsEqualToMAX_NUMBER_OF_EVENTS_PER_LISTENER)
{
    ::testing::Test::RecordProperty("TEST_ID", "1139a55e-d48c-4076-8616-be71e6b69c0a");
    EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER));
}

TEST_F(Listener_test, IsEmptyWhenConstructed)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d0e1023-a8bf-42b5-a03e-c3ea764fd934");
    EXPECT_THAT(m_sut->size(), Eq(0U));
}

TEST_F(Listener_test, AttachingWithoutEnumIfEnoughSpaceAvailableWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "42f0fdf5-9218-4f50-927a-8bcad4e7065f");
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0U], createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(Listener_test, AttachWithoutEnumTillCapacityIsFullWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "da15361c-cb03-45c7-a8ea-feff753d2d0f");
    fillUpWithSimpleEvents();
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity()));
}

TEST_F(Listener_test, DetachDecreasesSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "f35060a7-88f6-4df2-8a77-a27bb1214252");
    fillUpWithSimpleEvents();
    m_sut->detachEvent(m_simpleEvents[0U]);
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1U));
}

TEST_F(Listener_test, AttachWithoutEnumOneMoreThanCapacityFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "e51317c2-4007-4531-9666-2edbe02c9da3");
    fillUpWithSimpleEvents();
    auto result = m_sut->attachEvent(m_simpleEvents[m_sut->capacity()],
                                     createNotificationCallback(Listener_test::triggerCallback<0U>));

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(ListenerError::LISTENER_FULL));
}

TEST_F(Listener_test, AttachingWithEnumIfEnoughSpaceAvailableWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b41bdfd6-ef04-47cd-9bf3-dfbf6424aea8");
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::Hypnotoad,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(Listener_test, AttachWithEnumTillCapacityIsFullWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "05eea314-75f0-40c8-ac97-81094556c974");
    EXPECT_TRUE(fillUpWithSimpleEventsWithEnum(SimpleEvent::Hypnotoad));
}

TEST_F(Listener_test, AttachWithEnumOneMoreThanCapacityFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "56c0e83d-fa47-4952-b8d7-24b044381668");
    fillUpWithSimpleEventsWithEnum(SimpleEvent::Hypnotoad);
    auto result = m_sut->attachEvent(m_simpleEvents[m_sut->capacity()],
                                     SimpleEvent::Hypnotoad,
                                     createNotificationCallback(Listener_test::triggerCallback<0U>));

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(ListenerError::LISTENER_FULL));
}

TEST_F(Listener_test, DetachMakesSpaceForAnotherAttachWithEventEnum)
{
    ::testing::Test::RecordProperty("TEST_ID", "66b44835-f9af-404d-86be-cda891785180");
    fillUpWithSimpleEventsWithEnum(SimpleEvent::Hypnotoad);

    m_sut->detachEvent(m_simpleEvents[0U], SimpleEvent::Hypnotoad);
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[m_sut->capacity()],
                                   SimpleEvent::Hypnotoad,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
}

TEST_F(Listener_test, DetachMakesSpaceForAnotherAttachWithoutEventEnum)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2632526-7d9c-48cc-8fb8-5579ef1f9bad");
    fillUpWithSimpleEvents();

    m_sut->detachEvent(m_simpleEvents[0U]);
    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[m_sut->capacity()],
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
}

TEST_F(Listener_test, AttachingEventWithoutEventTypeLeadsToAttachedNoEventEnumTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "02963e32-50d5-48b1-a938-8064598cc589");
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0U], createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    EXPECT_TRUE(m_simpleEvents[0U].m_handleNoEventEnum.isValid());
}

TEST_F(Listener_test, AttachingEventWithEventTypeLeadsToAttachedTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "841ca69d-c092-49f9-8c5a-27f83e7c451e");
    ASSERT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    EXPECT_TRUE(m_simpleEvents[0U].m_handleStoepsel.isValid());
}

TEST_F(Listener_test, OverridingAlreadyAttachedEventWithEnumFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "333cd1de-76d5-4b96-9dca-f8a25c928a5b");
    ASSERT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    auto result = m_sut->attachEvent(m_simpleEvents[0U],
                                     SimpleEvent::StoepselBachelorParty,
                                     createNotificationCallback(Listener_test::triggerCallback<0U>));
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(ListenerError::EVENT_ALREADY_ATTACHED));
}

TEST_F(Listener_test, OverridingAlreadyAttachedEventWithoutEnumFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "33129abd-fb5c-48d7-a6ad-75b68161aa24");
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0U], createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    auto result =
        m_sut->attachEvent(m_simpleEvents[0U], createNotificationCallback(Listener_test::triggerCallback<0U>));
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(ListenerError::EVENT_ALREADY_ATTACHED));
}

TEST_F(Listener_test, AttachingSameClassWithTwoDifferentEventsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "83ee5edb-e426-4c76-a181-1613c3039151");
    ASSERT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::Hypnotoad,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    EXPECT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
}

TEST_F(Listener_test, AttachingNullptrCallbackFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "d15379e9-9628-4a2e-b80a-e4836c8ec8db");
    auto empty_callback = createNotificationCallback(attachCallback);
    empty_callback.m_callback = nullptr;
    empty_callback.m_contextData = nullptr;

    auto result = m_sut->attachEvent(m_simpleEvents[0U], empty_callback);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(ListenerError::EMPTY_EVENT_CALLBACK));
}

TEST_F(Listener_test, AttachingNullptrCallbackWithEventFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "eba903e2-c7c1-46b2-97c3-0e05c58caca5");
    auto empty_callback = createNotificationCallback(attachCallback);
    empty_callback.m_callback = nullptr;
    empty_callback.m_contextData = nullptr;

    auto result = m_sut->attachEvent(m_simpleEvents[0U], SimpleEvent::StoepselBachelorParty, empty_callback);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(ListenerError::EMPTY_EVENT_CALLBACK));
}

TEST_F(Listener_test, DetachingSameClassWithDifferentEventEnumChangesNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f97ac76-f7d4-4353-94df-8b3ec9fefcc7");
    ASSERT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::Hypnotoad,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    m_sut->detachEvent(m_simpleEvents[0U], SimpleEvent::StoepselBachelorParty);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(Listener_test, DetachingDifferentClassWithSameEventEnumChangesNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "64760cc1-7d65-4c2c-a6f1-83db62c2b18a");
    ASSERT_FALSE(m_sut
                     ->attachEvent(m_simpleEvents[0U],
                                   SimpleEvent::Hypnotoad,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    m_sut->detachEvent(m_simpleEvents[1U], SimpleEvent::Hypnotoad);
    EXPECT_THAT(m_sut->size(), Eq(1U));
}

TEST_F(Listener_test, AttachingWithoutEnumTillCapacityFilledSetsUpNoEventEnumTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "9dd3c485-c129-4d57-8318-50c6bec17888");
    fillUpWithSimpleEvents();

    for (uint64_t i = 0U; i < m_sut->capacity(); ++i)
    {
        EXPECT_TRUE(m_simpleEvents[i].m_handleNoEventEnum.isValid());
    }
}

TEST_F(Listener_test, DTorDetachesAllAttachedEventsWithoutEnum)
{
    ::testing::Test::RecordProperty("TEST_ID", "4204bd01-86d1-4213-a020-5d58e4173d70");
    fillUpWithSimpleEvents();

    auto capacity = m_sut->capacity();
    m_sut.reset();

    for (uint64_t i = 0U; i < capacity; ++i)
    {
        EXPECT_FALSE(m_simpleEvents[i].m_handleNoEventEnum.isValid());
    }
}

TEST_F(Listener_test, DTorDetachesAllAttachedEventsWithEnum)
{
    ::testing::Test::RecordProperty("TEST_ID", "1bd485f3-a634-473a-8f6b-fd344de10a85");
    fillUpWithSimpleEventsWithEnum(SimpleEvent::Hypnotoad);

    auto capacity = m_sut->capacity();
    m_sut.reset();

    for (uint64_t i = 0U; i < capacity; ++i)
    {
        EXPECT_FALSE(m_simpleEvents[i].m_handleHypnotoad.isValid());
    }
}

TEST_F(Listener_test, AttachedEventDTorDetachesItself)
{
    ::testing::Test::RecordProperty("TEST_ID", "198f5c98-5d65-4c16-8201-86d47f1d23e6");
    {
        SimpleEventClass fuu;
        ASSERT_FALSE(
            m_sut->attachEvent(fuu, createNotificationCallback(Listener_test::triggerCallback<0U>)).has_error());
    }

    EXPECT_THAT(m_sut->size(), Eq(0U));
}

TEST_F(Listener_test, AttachingSimpleEventWithoutEnumSetsNoEventEnumTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a2e5c6f-7b60-47e4-8b29-321584857c57");
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut->attachEvent(fuu, createNotificationCallback(Listener_test::triggerCallback<0U>)).has_error());

    EXPECT_TRUE(static_cast<bool>(fuu.m_handleNoEventEnum));
}

TEST_F(Listener_test, DetachingSimpleEventResetsTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0d30f2b-0ad2-46b8-acc1-04416019c50e");
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut->attachEvent(fuu, createNotificationCallback(Listener_test::triggerCallback<0U>)).has_error());
    m_sut->detachEvent(fuu);

    EXPECT_FALSE(static_cast<bool>(fuu.m_handleNoEventEnum));
}

TEST_F(Listener_test, AttachingEventWithEnumSetsTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "350d25da-eb06-48a9-9b47-e23f7dafba53");
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    EXPECT_TRUE(static_cast<bool>(fuu.m_handleStoepsel));
}

TEST_F(Listener_test, DetachingEventWithEnumResetsTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "bc51566f-8b10-490b-bba1-a14865d7d07b");
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    m_sut->detachEvent(fuu, SimpleEvent::StoepselBachelorParty);

    EXPECT_FALSE(static_cast<bool>(fuu.m_handleStoepsel));
}

TEST_F(Listener_test, DetachingNonAttachedEventResetsNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "32deeace-1857-4a52-b3e7-c915037d9950");
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    m_sut->detachEvent(fuu, SimpleEvent::Hypnotoad);

    EXPECT_TRUE(static_cast<bool>(fuu.m_handleStoepsel));
}

///////////////////////////////////
// END
///////////////////////////////////

///////////////////////////////////
// BEGIN calling callbacks
///////////////////////////////////
TIMING_TEST_F(Listener_test, CallbackIsCalledAfterNotify, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "a283a326-52c7-4d39-9241-0770384892ec");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 1U);
})

TIMING_TEST_F(Listener_test, CallbackWithEventAndUserTypeIsCalledAfterNotify, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "6df97139-8c2e-42b1-bd9a-8770c295bf2e");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    uint64_t userType = 0U;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallbackWithUserType, userType))
                     .has_error());

    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu);
    TIMING_TEST_EXPECT_TRUE(userType == 1U);
})

TIMING_TEST_F(Listener_test, CallbackWithUserTypeIsCalledAfterNotify, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "98ffc91c-cf17-4331-8b20-a84121090119");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    uint64_t userType = 0U;
    ASSERT_FALSE(
        m_sut->attachEvent(fuu, createNotificationCallback(Listener_test::triggerCallbackWithUserType, userType))
            .has_error());

    fuu.triggerNoEventType();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu);
    TIMING_TEST_EXPECT_TRUE(userType == 1U);
})

TIMING_TEST_F(Listener_test, CallbackIsCalledOnlyOnceWhenTriggered, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "ad1470d7-a683-4089-a548-93616278c772");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu1;
    SimpleEventClass fuu2;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu1,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu2,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<1U>))
                     .has_error());

    fuu1.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));
    fuu2.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu1);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 1U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source.load() == &fuu2);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count.load() == 1U);
})

TIMING_TEST_F(Listener_test, TriggerWhileInCallbackLeadsToAnotherCallback, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "b29c6689-35cf-4d2f-b719-5c928bf5e870");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    constexpr uint64_t NUMBER_OF_TRIGGER_UNBLOCKS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    fuu.triggerStoepsel();
    m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    unblockTriggerCallback(NUMBER_OF_TRIGGER_UNBLOCKS);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 2U);
})

TIMING_TEST_F(Listener_test, TriggerWhileInCallbackLeadsToAnotherCallbackOnce, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "1dea4bbc-6f11-434c-845d-d2ae1104707e");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    SimpleEventClass bar;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    ASSERT_FALSE(m_sut
                     ->attachEvent(bar,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<1U>))
                     .has_error());

    constexpr uint64_t NUMBER_OF_TRIGGER_UNBLOCKS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    fuu.triggerStoepsel();
    bar.triggerStoepsel();
    m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    unblockTriggerCallback(NUMBER_OF_TRIGGER_UNBLOCKS);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 2U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source.load() == &bar);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count.load() == 1U);
})

TIMING_TEST_F(Listener_test, TriggerMultipleTimesWhileInCallbackLeadsToAnotherCallback, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "9e1e5a70-e2e9-4ee9-85f9-f52cf475cb61");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    constexpr uint64_t NUMBER_OF_RETRIGGERS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (uint64_t i = 0U; i < NUMBER_OF_RETRIGGERS; ++i)
    {
        fuu.triggerStoepsel();
    }
    m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    unblockTriggerCallback(NUMBER_OF_RETRIGGERS);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 2U);
})

TIMING_TEST_F(Listener_test, TriggerMultipleTimesWhileInCallbackLeadsToAnotherCallbackOnce, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "51163b5c-01c9-426b-9f59-3fac62a2f10c");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    SimpleEventClass bar;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());
    ASSERT_FALSE(m_sut
                     ->attachEvent(bar,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<1U>))
                     .has_error());

    constexpr uint64_t NUMBER_OF_RETRIGGERS = 10U;

    activateTriggerCallbackBlocker();
    fuu.triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (uint64_t i = 0U; i < NUMBER_OF_RETRIGGERS; ++i)
    {
        fuu.triggerStoepsel();
    }
    bar.triggerStoepsel();
    m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    unblockTriggerCallback(NUMBER_OF_RETRIGGERS + 1U);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &fuu);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 2U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source.load() == &bar);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count.load() == 1U);
})

TIMING_TEST_F(Listener_test, NoTriggerLeadsToNoCallback, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "6c11cc3f-d251-4e10-9c8e-cdc18c257227");
    m_sut.emplace(m_condVarData);
    SimpleEventClass fuu;
    ASSERT_FALSE(m_sut
                     ->attachEvent(fuu,
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(Listener_test::triggerCallback<0U>))
                     .has_error());

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == nullptr);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 0U);
})

TIMING_TEST_F(Listener_test, TriggeringAllEventsCallsAllCallbacks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "9c0d0be4-7fdf-4b2d-8e40-992a975aa6cc");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U>::doIt(*m_sut, events, SimpleEvent::StoepselBachelorParty);

    activateTriggerCallbackBlocker();
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (auto& e : events)
    {
        e.triggerStoepsel();
    }

    // 10 times more callback runs allowed to allow potential overtriggering
    m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    unblockTriggerCallback(10U * iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_source.load() == &events[0U]);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_count.load() == 2U);
    for (size_t i = 1U; i < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source.load() == &events[i]);
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_count.load() == 1U);
    }
})

TIMING_TEST_F(Listener_test, TriggeringAllEventsCallsAllCallbacksOnce, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "6840b748-adcc-40c3-ac39-24870540a58f");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1>::doIt(*m_sut, events, SimpleEvent::StoepselBachelorParty);

    activateTriggerCallbackBlocker();
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    for (auto& e : events)
    {
        e.triggerStoepsel();
    }

    // 10 times more callback runs allowed to allow potential overtriggering
    m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    unblockTriggerCallback(10U * iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_source.load() == &events[0U]);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0].m_count.load() == 3U);
    for (size_t i = 1U; i < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source.load() == &events[i]);
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_count.load() == 1U);
    }
})
//////////////////////////////////
// END
//////////////////////////////////

//////////////////////////////////
// BEGIN concurrent attach / detach
//////////////////////////////////
TIMING_TEST_F(Listener_test, AttachingWhileCallbackIsRunningWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "35edee3a-ff25-4b2e-a804-363ca56b4481");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);

    ASSERT_FALSE(m_sut
                     ->attachEvent(events[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(triggerCallback<0U>))
                     .has_error());

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    ASSERT_FALSE(m_sut
                     ->attachEvent(events[1U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(triggerCallback<1U>))
                     .has_error());
    events[1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS * 2U));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source.load() == &events[1U]);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count.load() == 1U);
})

TIMING_TEST_F(Listener_test, AttachingMultipleWhileCallbackIsRunningWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "9ff36b35-0396-4f57-831a-dfc9dd0fdd46");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);

    ASSERT_FALSE(
        m_sut
            ->attachEvent(events[iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U],
                          SimpleEvent::StoepselBachelorParty,
                          createNotificationCallback(triggerCallback<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U>))
            .has_error());

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 2U>::doIt(*m_sut, events, SimpleEvent::StoepselBachelorParty);

    g_triggerCallbackRuntimeInMs = 0U;
    for (size_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        events[i].triggerStoepsel();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    for (size_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source.load() == &events[i]);
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_count.load() == 1U);
    }
})

TIMING_TEST_F(Listener_test, DetachingWhileCallbackIsRunningWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "f93b89e9-4bc9-432c-9808-cdb6f46ebff2");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);

    ASSERT_FALSE(m_sut
                     ->attachEvent(events[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(triggerCallback<0U>))
                     .has_error());

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    g_triggerCallbackArg[0U].m_source = nullptr;
    m_sut->detachEvent(events[0U], SimpleEvent::StoepselBachelorParty);
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 1U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == nullptr);
})

TIMING_TEST_F(Listener_test, DetachingWhileCallbackIsRunningBlocksDetach, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "0a2b5a79-1ff6-4171-9318-52c593828cfb");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    ASSERT_FALSE(m_sut
                     ->attachEvent(events[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(triggerCallback<0U>))
                     .has_error());
    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 4U));

    auto begin = std::chrono::system_clock::now();
    m_sut->detachEvent(events[0U], SimpleEvent::StoepselBachelorParty);
    auto end = std::chrono::system_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    TIMING_TEST_EXPECT_TRUE(static_cast<uint64_t>(elapsed.count()) > CALLBACK_WAIT_IN_MS / 2U);
})

TIMING_TEST_F(Listener_test, EventDestructorBlocksWhenCallbackIsRunning, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "0a7fc0d7-aa69-4812-837a-f1497401d3b6");
    m_sut.emplace(m_condVarData);
    SimpleEventClass* event = new SimpleEventClass();
    ASSERT_FALSE(
        m_sut->attachEvent(*event, SimpleEvent::StoepselBachelorParty, createNotificationCallback(triggerCallback<0U>))
            .has_error());
    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    event->triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 4U));

    auto begin = std::chrono::system_clock::now();
    delete event;
    auto end = std::chrono::system_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    TIMING_TEST_EXPECT_TRUE(static_cast<uint64_t>(elapsed.count()) > CALLBACK_WAIT_IN_MS / 2U);
})


TIMING_TEST_F(Listener_test, DetachingMultipleWhileCallbackIsRunningWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "81111c14-84a2-4f38-a77d-03b95e3f3f8c");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U>::doIt(*m_sut, events, SimpleEvent::StoepselBachelorParty);

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

    for (size_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[i].m_source.load() == nullptr);
    }
})

TIMING_TEST_F(Listener_test, AttachingDetachingRunsIndependentOfCallback, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "f5a15344-a2d7-44dd-9a12-a65b70976f46");
    m_sut.emplace(m_condVarData);
    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    ASSERT_FALSE(
        m_sut
            ->attachEvent(events[iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U],
                          SimpleEvent::StoepselBachelorParty,
                          createNotificationCallback(triggerCallback<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U>))
            .has_error());
    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 2U>::doIt(*m_sut, events, SimpleEvent::StoepselBachelorParty);

    for (auto& e : events)
    {
        m_sut->detachEvent(e, SimpleEvent::StoepselBachelorParty);
    }

    // EXPECT_* (assert step) is inside of doIt call. We expect that every event can
    // be attached
    AttachEvent<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 2U>::doIt(*m_sut, events, SimpleEvent::StoepselBachelorParty);
})
//////////////////////////////////
// END
//////////////////////////////////

//////////////////////////////////
// BEGIN attach / detach in callbacks
//////////////////////////////////
TIMING_TEST_F(Listener_test, DetachingSelfInCallbackWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "ac677d64-b444-4155-9879-7b674597be1e");
    m_sut.emplace(m_condVarData);
    g_toBeDetached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    g_toBeDetached->push_back({&events[0U], &*m_sut});
    ASSERT_FALSE(
        m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, createNotificationCallback(detachCallback))
            .has_error());

    events[0U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_sut->size() == 0U);
})

TIMING_TEST_F(Listener_test, DetachingNonSelfEventInCallbackWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "78d646d0-9899-4563-91cf-3740221fbca1");
    m_sut.emplace(m_condVarData);
    g_toBeDetached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    g_toBeDetached->push_back({&events[1U], &*m_sut});
    ASSERT_FALSE(
        m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, createNotificationCallback(detachCallback))
            .has_error());
    ASSERT_FALSE(m_sut
                     ->attachEvent(events[1U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(triggerCallback<1U>))
                     .has_error());

    events[0U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(m_sut->size() == 1U);
})

TIMING_TEST_F(Listener_test, DetachedCallbacksAreNotBeingCalledWhenTriggeredBefore, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "7581bb34-802e-4241-a42e-e72c990936ab");
    // idea of the test is that an event which was detached but is technically still attached
    // since the detach blocks cannot be retriggered again so that the callback is called again.
    // once detach is called either the callback is currently running and detach is blocked or
    // the callback is removed and can never be called again.
    //
    // To test this we attach two events. events[0] detaches events[1] in his callback.
    // events[1] is triggered and the callback has a certain runtime so that we make sure that the callback is
    // running while we retrigger events[0] and events[1].
    // Now events[0] remove events[1] before its trigger callback is executed and therefore the
    // callback is not allowed to be called even so that the trigger came before the detach occurred
    m_sut.emplace(m_condVarData);
    g_toBeDetached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    g_toBeDetached->push_back({&events[1U], &*m_sut});
    ASSERT_FALSE(m_sut
                     ->attachEvent(events[0U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(notifyAndThenDetachStoepselCallback))
                     .has_error());
    ASSERT_FALSE(m_sut
                     ->attachEvent(events[1U],
                                   SimpleEvent::StoepselBachelorParty,
                                   createNotificationCallback(triggerCallback<1U>))
                     .has_error());

    g_triggerCallbackRuntimeInMs = 3U * CALLBACK_WAIT_IN_MS / 2U;
    events[1U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    g_triggerCallbackArg[1U].m_source = nullptr;
    g_triggerCallbackRuntimeInMs = 0U;

    events[1U].triggerStoepsel();
    events[0U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == nullptr);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_source.load() == nullptr);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 0U);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[1U].m_count.load() == 1U);
})

TIMING_TEST_F(Listener_test, AttachingInCallbackWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "58734e7f-2d3f-49ac-9f37-8052016def8c");
    m_sut.emplace(m_condVarData);
    g_toBeAttached->clear();

    std::vector<SimpleEventClass> events(iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    g_toBeAttached->push_back({&events[1U], &*m_sut});
    ASSERT_FALSE(
        m_sut->attachEvent(events[0U], SimpleEvent::StoepselBachelorParty, createNotificationCallback(attachCallback))
            .has_error());

    events[0U].triggerStoepsel();
    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));
    events[1U].triggerStoepsel();

    std::this_thread::sleep_for(std::chrono::milliseconds(CALLBACK_WAIT_IN_MS / 2U));

    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_source.load() == &events[1U]);
    TIMING_TEST_EXPECT_TRUE(g_triggerCallbackArg[0U].m_count.load() == 1U);
})
//////////////////////////////////
// END
//////////////////////////////////

} // namespace
