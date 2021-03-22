// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/condition_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_notifier.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "test.hpp"
#include "testutils/timing_test.hpp"
#include "testutils/watch_dog.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <type_traits>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::units::duration_literals;

class ConditionVariable_test : public Test
{
  public:
    using NotificationVector_t = ConditionListener::NotificationVector_t;
    using Type_t = iox::cxx::BestFittingType_t<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>;
    const iox::ProcessName_t m_process{"Ferdinand"};
    const iox::units::Duration m_timeToWait = 2_s;

    ConditionVariableData m_condVarData{m_process};
    ConditionListener m_waiter{m_condVarData};
    ConditionNotifier m_signaler{m_condVarData, 0U};

    iox::posix::Semaphore m_syncSemaphore =
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();

    void SetUp() override{};
    void TearDown() override
    {
        // Reset condition variable
        m_waiter.resetSemaphore();
    };
};

TEST_F(ConditionVariable_test, ConditionListenerIsNeitherCopyNorMovable)
{
    EXPECT_FALSE(std::is_copy_constructible<ConditionListener>::value);
    EXPECT_FALSE(std::is_move_constructible<ConditionListener>::value);
    EXPECT_FALSE(std::is_copy_assignable<ConditionListener>::value);
    EXPECT_FALSE(std::is_move_assignable<ConditionListener>::value);
}

TEST_F(ConditionVariable_test, ConditionNotifierIsNeitherCopyNorMovable)
{
    EXPECT_FALSE(std::is_copy_constructible<ConditionNotifier>::value);
    EXPECT_FALSE(std::is_move_constructible<ConditionNotifier>::value);
    EXPECT_FALSE(std::is_copy_assignable<ConditionNotifier>::value);
    EXPECT_FALSE(std::is_move_assignable<ConditionNotifier>::value);
}

TEST_F(ConditionVariable_test, TimedWaitWithInvalidTimeResultsInFailure)
{
    EXPECT_FALSE(m_waiter.timedWait(0_ms));
}

TEST_F(ConditionVariable_test, NoNotifyResultsInTimeoutSingleThreaded)
{
    EXPECT_FALSE(m_waiter.timedWait(10_ms));
}

TEST_F(ConditionVariable_test, NotifyOnceResultsInNoTimeoutSingleThreaded)
{
    m_signaler.notify();
    EXPECT_TRUE(m_waiter.timedWait(10_ms));
}

TEST_F(ConditionVariable_test, NotifyOnceResultsInBeingTriggered)
{
    m_signaler.notify();
    EXPECT_TRUE(m_waiter.wasNotified());
}

TEST_F(ConditionVariable_test, NoNotifyResultsInNotBeingTriggered)
{
    EXPECT_FALSE(m_waiter.wasNotified());
}

TEST_F(ConditionVariable_test, WasTriggerCallDoesNotChangeTheState)
{
    m_signaler.notify();
    m_waiter.wasNotified();
    EXPECT_TRUE(m_waiter.timedWait(10_ms));
}

TEST_F(ConditionVariable_test, NotifyOnceResultsInNoWaitSingleThreaded)
{
    m_signaler.notify();
    m_waiter.wait();
    // We expect that the next line is reached
    EXPECT_TRUE(true);
}

TEST_F(ConditionVariable_test, NotifyTwiceResultsInNoWaitSingleThreaded)
{
    m_signaler.notify();
    m_signaler.notify();
    m_waiter.wait();
    m_waiter.wait();
    // We expect that the next line is reached
    EXPECT_TRUE(true);
}

TEST_F(ConditionVariable_test, WaitAndNotifyResultsInImmediateTriggerMultiThreaded)
{
    std::atomic<int> counter{0};
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        m_waiter.wait();
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notify();
    waiter.join();
}

TEST_F(ConditionVariable_test, ResetResultsInBlockingWaitMultiThreaded)
{
    std::atomic<int> counter{0};
    m_signaler.notify();
    m_waiter.resetSemaphore();
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        m_waiter.wait();
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notify();
    waiter.join();
}

TEST_F(ConditionVariable_test, ResetWithoutNotifiyResultsInBlockingWaitMultiThreaded)
{
    std::atomic<int> counter{0};
    m_waiter.resetSemaphore();
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        m_waiter.wait();
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notify();
    waiter.join();
}

TEST_F(ConditionVariable_test, NotifyWhileWaitingResultsNoTimeoutMultiThreaded)
{
    std::atomic<int> counter{0};
    std::thread waiter([&] {
        EXPECT_THAT(counter, Eq(0));
        m_syncSemaphore.post();
        EXPECT_TRUE(m_waiter.timedWait(10_ms));
        EXPECT_THAT(counter, Eq(1));
    });
    m_syncSemaphore.wait();
    counter++;
    m_signaler.notify();
    waiter.join();
}

TEST_F(ConditionVariable_test, AllNotificationsAreFalseAfterConstruction)
{
    ConditionVariableData sut;
    for (auto& notification : sut.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

TEST_F(ConditionVariable_test, CorrectProcessNameAfterConstructionWithProcessName)
{
    EXPECT_THAT(m_condVarData.m_process.c_str(), StrEq(m_process));
}

TEST_F(ConditionVariable_test, AllNotificationsAreFalseAfterConstructionWithProcessName)
{
    for (auto& notification : m_condVarData.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

TEST_F(ConditionVariable_test, NotifyActivatesCorrectIndex)
{
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    ConditionNotifier sut(m_condVarData, EVENT_INDEX);
    sut.notify();
    for (Type_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; i++)
    {
        if (i == EVENT_INDEX)
        {
            EXPECT_THAT(m_condVarData.m_activeNotifications[i], Eq(true));
        }
        else
        {
            EXPECT_THAT(m_condVarData.m_activeNotifications[i], Eq(false));
        }
    }
}

TEST_F(ConditionVariable_test, WaitIsNonBlockingAfterDestroyAndReturnsEmptyVector)
{
    ConditionListener sut(m_condVarData);
    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { std::terminate(); });

    sut.destroy();
    const auto& activeNotifications = sut.waitForNotifications();

    EXPECT_THAT(activeNotifications.size(), Eq(0U));
}

TEST_F(ConditionVariable_test, WaitIsNonBlockingAfterDestroyAndNotifyAndReturnsEmptyVector)
{
    ConditionListener sut(m_condVarData);
    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { std::terminate(); });
    sut.destroy();

    ConditionNotifier notifier(m_condVarData, 0U);
    notifier.notify();

    const auto& activeNotifications = sut.waitForNotifications();
    EXPECT_THAT(activeNotifications.size(), Eq(0U));
}

TEST_F(ConditionVariable_test, DestroyWakesUpWaitWhichReturnsEmptyVector)
{
    ConditionListener sut(m_condVarData);

    NotificationVector_t activeNotifications;

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([] { std::terminate(); });

    std::thread waiter([&] {
        activeNotifications = sut.waitForNotifications();
        EXPECT_THAT(activeNotifications.size(), Eq(0U));
    });

    sut.destroy();
    waiter.join();
}

TEST_F(ConditionVariable_test, GetCorrectNotificationVectorAfterNotifyAndWait)
{
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    ConditionNotifier notifier(m_condVarData, EVENT_INDEX);
    ConditionListener listener(m_condVarData);

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    notifier.notify();
    const auto& activeNotifications = listener.waitForNotifications();

    ASSERT_THAT(activeNotifications.size(), Eq(1U));
    EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
}

TEST_F(ConditionVariable_test, GetCorrectNotificationVectorAfterMultipleNotifyAndWait)
{
    constexpr Type_t FIRST_EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    constexpr Type_t SECOND_EVENT_INDEX = 0U;
    ConditionNotifier notifier1(m_condVarData, FIRST_EVENT_INDEX);
    ConditionNotifier notifier2(m_condVarData, SECOND_EVENT_INDEX);
    ConditionListener listener(m_condVarData);

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    notifier1.notify();
    notifier2.notify();
    const auto& activeNotifications = listener.waitForNotifications();

    ASSERT_THAT(activeNotifications.size(), Eq(2U));
    EXPECT_THAT(activeNotifications[0], Eq(SECOND_EVENT_INDEX));
    EXPECT_THAT(activeNotifications[1], Eq(FIRST_EVENT_INDEX));
}

TEST_F(ConditionVariable_test, WaitAndNotifyResultsInCorrectNotificationVector)
{
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 5U;
    ConditionNotifier notifier(m_condVarData, EVENT_INDEX);
    ConditionListener listener(m_condVarData);
    NotificationVector_t activeNotifications;

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    std::thread waiter([&] {
        activeNotifications = listener.waitForNotifications();
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
    });

    notifier.notify();
    waiter.join();
}

TIMING_TEST_F(ConditionVariable_test, WaitBlocks, Repeat(5), [&] {
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 5U;
    ConditionNotifier notifier(m_condVarData, EVENT_INDEX);
    ConditionListener listener(m_condVarData);
    NotificationVector_t activeNotifications;
    iox::posix::Semaphore threadSetupSemaphore =
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();
    std::atomic_bool hasWaited{false};

    Watchdog watchdog(m_timeToWait);
    watchdog.watchAndActOnFailure([&] { listener.destroy(); });

    std::thread waiter([&] {
        threadSetupSemaphore.post();
        activeNotifications = listener.waitForNotifications();
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

TIMING_TEST_F(ConditionVariable_test, SecondWaitBlocksUntilNewNotification, Repeat(5), [&] {
    constexpr Type_t FIRST_EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 2U;
    constexpr Type_t SECOND_EVENT_INDEX = 0U;
    ConditionNotifier notifier1(m_condVarData, FIRST_EVENT_INDEX);
    ConditionNotifier notifier2(m_condVarData, SECOND_EVENT_INDEX);
    ConditionListener listener(m_condVarData);
    iox::posix::Semaphore threadSetupSemaphore =
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();
    std::atomic_bool hasWaited{false};

    Watchdog watchdogFirstWait(m_timeToWait);
    watchdogFirstWait.watchAndActOnFailure([&] { listener.destroy(); });

    notifier1.notify();
    notifier2.notify();
    NotificationVector_t activeNotifications = listener.waitForNotifications();

    ASSERT_THAT(activeNotifications.size(), Eq(2U));
    EXPECT_THAT(activeNotifications[0], Eq(SECOND_EVENT_INDEX));
    EXPECT_THAT(activeNotifications[1], Eq(FIRST_EVENT_INDEX));

    Watchdog watchdogSecondWait(m_timeToWait);
    watchdogSecondWait.watchAndActOnFailure([&] { listener.destroy(); });

    std::thread waiter([&] {
        threadSetupSemaphore.post();
        activeNotifications = listener.waitForNotifications();
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(FIRST_EVENT_INDEX));
        for (const auto& notification : m_condVarData.m_activeNotifications)
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

