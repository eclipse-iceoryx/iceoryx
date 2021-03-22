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

#include "iceoryx_posh/internal/popo/building_blocks/event_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_notifier.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"

#include "test.hpp"
#include "testutils/timing_test.hpp"
#include "testutils/watch_dog.hpp"

#include <thread>

using namespace ::testing;
using namespace iox::popo;
using namespace iox::units::duration_literals;

class EventVariable_test : public Test
{
  public:
    using Type_t = iox::cxx::BestFittingType_t<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>;
    using NotificationVector_t = EventListener::NotificationVector_t;

    const iox::ProcessName_t m_process{"Ferdinand"};
    EventVariableData m_eventVarData{m_process};

    const iox::units::Duration m_timeToWait = 2_s;
};

TEST_F(EventVariable_test, AllNotificationsAreFalseAfterConstruction)
{
    EventVariableData sut;
    for (auto& notification : sut.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

TEST_F(EventVariable_test, CorrectProcessNameAfterConstructionWithProcessName)
{
    EXPECT_THAT(m_eventVarData.m_process.c_str(), StrEq(m_process));
}

TEST_F(EventVariable_test, AllNotificationsAreFalseAfterConstructionWithProcessName)
{
    for (auto& notification : m_eventVarData.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

TEST_F(EventVariable_test, NotifyActivatesCorrectIndex)
{
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    EventNotifier sut(m_eventVarData, EVENT_INDEX);
    sut.notify();
    for (Type_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; i++)
    {
        if (i == EVENT_INDEX)
        {
            EXPECT_THAT(m_eventVarData.m_activeNotifications[i], Eq(true));
        }
        else
        {
            EXPECT_THAT(m_eventVarData.m_activeNotifications[i], Eq(false));
        }
    }
}

TEST_F(EventVariable_test, NotifyActivatesNoIndexIfIndexIsTooLarge)
{
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER;
    EventNotifier sut(m_eventVarData, EVENT_INDEX);
    sut.notify();
    for (const auto& notification : m_eventVarData.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

TEST_F(EventVariable_test, WaitIsNonBlockingAfterDestroyAndReturnsEmptyVector)
{
    EventListener sut(m_eventVarData);
    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { std::terminate(); });

    sut.destroy();
    const auto& activeNotifications = sut.wait();

    EXPECT_THAT(activeNotifications.size(), Eq(0U));
}

TEST_F(EventVariable_test, WaitIsNonBlockingAfterDestroyAndNotifyAndReturnsEmptyVector)
{
    EventListener sut(m_eventVarData);
    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { std::terminate(); });
    sut.destroy();

    EventNotifier notifier(m_eventVarData, 0U);
    notifier.notify();

    const auto& activeNotifications = sut.wait();
    EXPECT_THAT(activeNotifications.size(), Eq(0U));
}

TEST_F(EventVariable_test, DestroyWakesUpWaitWhichReturnsEmptyVector)
{
    EventListener sut(m_eventVarData);

    NotificationVector_t activeNotifications;

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([] { std::terminate(); });

    std::thread waiter([&] {
        activeNotifications = sut.wait();
        EXPECT_THAT(activeNotifications.size(), Eq(0U));
    });

    sut.destroy();
    waiter.join();
}

TEST_F(EventVariable_test, GetCorrectNotificationVectorAfterNotifyAndWait)
{
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    EventNotifier notifier(m_eventVarData, EVENT_INDEX);
    EventListener listener(m_eventVarData);

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    notifier.notify();
    const auto& activeNotifications = listener.wait();

    ASSERT_THAT(activeNotifications.size(), Eq(1U));
    EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
}

TEST_F(EventVariable_test, GetCorrectNotificationVectorAfterMultipleNotifyAndWait)
{
    constexpr Type_t FIRST_EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    constexpr Type_t SECOND_EVENT_INDEX = 0U;
    EventNotifier notifier1(m_eventVarData, FIRST_EVENT_INDEX);
    EventNotifier notifier2(m_eventVarData, SECOND_EVENT_INDEX);
    EventListener listener(m_eventVarData);

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    notifier1.notify();
    notifier2.notify();
    const auto& activeNotifications = listener.wait();

    ASSERT_THAT(activeNotifications.size(), Eq(2U));
    EXPECT_THAT(activeNotifications[0], Eq(SECOND_EVENT_INDEX));
    EXPECT_THAT(activeNotifications[1], Eq(FIRST_EVENT_INDEX));
}

TEST_F(EventVariable_test, WaitAndNotifyResultsInCorrectNotificationVector)
{
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 5U;
    EventNotifier notifier(m_eventVarData, EVENT_INDEX);
    EventListener listener(m_eventVarData);
    NotificationVector_t activeNotifications;

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    std::thread waiter([&] {
        activeNotifications = listener.wait();
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
    });

    notifier.notify();
    waiter.join();
}

TIMING_TEST_F(EventVariable_test, WaitBlocks, Repeat(5), [&] {
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 5U;
    EventNotifier notifier(m_eventVarData, EVENT_INDEX);
    EventListener listener(m_eventVarData);
    NotificationVector_t activeNotifications;
    iox::posix::Semaphore threadSetupSemaphore =
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();
    std::atomic_bool hasWaited{false};

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    std::thread waiter([&] {
        threadSetupSemaphore.post();
        activeNotifications = listener.wait();
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
    });

    threadSetupSemaphore.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(false));
    notifier.notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(true));
    waiter.join();
})

TIMING_TEST_F(EventVariable_test, SecondWaitBlocksUntilNewNotification, Repeat(5), [&] {
    constexpr Type_t FIRST_EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 2U;
    constexpr Type_t SECOND_EVENT_INDEX = 0U;
    EventNotifier notifier1(m_eventVarData, FIRST_EVENT_INDEX);
    EventNotifier notifier2(m_eventVarData, SECOND_EVENT_INDEX);
    EventListener listener(m_eventVarData);
    iox::posix::Semaphore threadSetupSemaphore =
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();
    std::atomic_bool hasWaited{false};

    Watchdog watchdogFirstWait(m_timeToWait);
    watchdogFirstWait.watchAndActOnFailure([&] { listener.destroy(); });

    notifier1.notify();
    notifier2.notify();
    NotificationVector_t activeNotifications = listener.wait();

    ASSERT_THAT(activeNotifications.size(), Eq(2U));
    EXPECT_THAT(activeNotifications[0], Eq(SECOND_EVENT_INDEX));
    EXPECT_THAT(activeNotifications[1], Eq(FIRST_EVENT_INDEX));

    Watchdog watchdogSecondWait(m_timeToWait);
    watchdogSecondWait.watchAndActOnFailure([&] { listener.destroy(); });

    std::thread waiter([&] {
        threadSetupSemaphore.post();
        activeNotifications = listener.wait();
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(FIRST_EVENT_INDEX));
        for (const auto& notification : m_eventVarData.m_activeNotifications)
        {
            EXPECT_THAT(notification, Eq(false));
        }
    });

    threadSetupSemaphore.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(false));
    notifier1.notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(true));
    waiter.join();
})

