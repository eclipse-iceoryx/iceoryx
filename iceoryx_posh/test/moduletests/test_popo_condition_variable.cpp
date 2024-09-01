// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_notifier.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iox/algorithm.hpp"
#include "iox/atomic.hpp"
#include "test.hpp"

#include <memory>
#include <thread>
#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::popo;
using namespace iox::units::duration_literals;

class ConditionVariable_test : public Test
{
  public:
    using NotificationVector_t = ConditionListener::NotificationVector_t;
    using Type_t = iox::BestFittingType_t<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>;
    const iox::RuntimeName_t m_runtimeName{"Ferdinand"};
    const iox::units::Duration m_timeToWait = 2_s;
    const iox::units::Duration m_timingTestTime = 100_ms;

    ConditionVariableData m_condVarData{m_runtimeName};
    ConditionListener m_waiter{m_condVarData};
    ConditionNotifier m_signaler{m_condVarData, 0U};
    iox::vector<ConditionNotifier, iox::MAX_NUMBER_OF_NOTIFIERS> m_notifiers;

    void SetUp() override
    {
        for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_NOTIFIERS; ++i)
        {
            m_notifiers.emplace_back(m_condVarData, i);
        }
        m_watchdog.watchAndActOnFailure([&] { std::terminate(); });
    }

    Watchdog m_watchdog{m_timeToWait};
};

TEST_F(ConditionVariable_test, ConditionListenerIsNeitherCopyNorMovable)
{
    ::testing::Test::RecordProperty("TEST_ID", "2105fbcf-ed66-4042-aae3-46c2bb82a63c");
    EXPECT_FALSE(std::is_copy_constructible<ConditionListener>::value);
    EXPECT_FALSE(std::is_move_constructible<ConditionListener>::value);
    EXPECT_FALSE(std::is_copy_assignable<ConditionListener>::value);
    EXPECT_FALSE(std::is_move_assignable<ConditionListener>::value);
}

TEST_F(ConditionVariable_test, ConditionNotifierIsNeitherCopyNorMovable)
{
    ::testing::Test::RecordProperty("TEST_ID", "51b971ea-2fb1-4280-8663-6f86c70ee06a");
    EXPECT_FALSE(std::is_copy_constructible<ConditionNotifier>::value);
    EXPECT_FALSE(std::is_move_constructible<ConditionNotifier>::value);
    EXPECT_FALSE(std::is_copy_assignable<ConditionNotifier>::value);
    EXPECT_FALSE(std::is_move_assignable<ConditionNotifier>::value);
}

TEST_F(ConditionVariable_test, NotifyOnceResultsInBeingTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "372125d2-82b4-4729-bc93-661578af4739");
    m_signaler.notify();
    EXPECT_TRUE(m_waiter.wasNotified());
}

TEST_F(ConditionVariable_test, NoNotifyResultsInNotBeingTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "abe8a485-63d3-486a-b62a-94648b7f7954");
    EXPECT_FALSE(m_waiter.wasNotified());
}

TEST_F(ConditionVariable_test, WaitResetsAllNotificationsInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "ebc9c42a-14e7-471c-a9df-9c5641b5767d");
    m_signaler.notify();
    m_signaler.notify();
    m_signaler.notify();
    m_waiter.wait();

    iox::concurrent::Atomic<bool> isThreadFinished{false};
    std::thread t([&] {
        m_waiter.wait();
        isThreadFinished = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(isThreadFinished.load());
    m_signaler.notify();
    t.join();
    EXPECT_TRUE(isThreadFinished.load());
}

TEST_F(ConditionVariable_test, WaitAndNotifyResultsInImmediateTriggerMultiThreaded)
{
    ::testing::Test::RecordProperty("TEST_ID", "39b40c73-3dcc-4af6-9682-b62816c69854");
    iox::concurrent::Atomic<int> counter{0};
    Barrier isThreadStarted(1U);
    std::thread waiter([&] {
        EXPECT_THAT(counter.load(), Eq(0));
        isThreadStarted.notify();
        m_waiter.wait();
        EXPECT_THAT(counter.load(), Eq(1));
    });
    isThreadStarted.wait();

    counter++;
    m_signaler.notify();
    waiter.join();
}

TEST_F(ConditionVariable_test, AllNotificationsAreFalseAfterConstruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e5f6dbc-84cc-468a-9d64-f5ed88012ebc");
    ConditionVariableData sut;
    for (auto& notification : sut.m_activeNotifications)
    {
        EXPECT_THAT(notification.load(), Eq(false));
    }
}

TEST_F(ConditionVariable_test, CorrectRuntimeNameAfterConstructionWithRuntimeName)
{
    ::testing::Test::RecordProperty("TEST_ID", "acc65071-09ec-40ce-82b4-74964525fabf");
    EXPECT_THAT(m_condVarData.m_runtimeName, Eq(m_runtimeName));
}

TEST_F(ConditionVariable_test, AllNotificationsAreFalseAfterConstructionWithRuntimeName)
{
    ::testing::Test::RecordProperty("TEST_ID", "4825e152-08e3-414e-a34f-d93d048f84b8");
    for (auto& notification : m_condVarData.m_activeNotifications)
    {
        EXPECT_THAT(notification.load(), Eq(false));
    }
}

TEST_F(ConditionVariable_test, NotifyActivatesCorrectIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "2c372bcc-7e91-47c1-8ab9-ccd5be048562");
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    ConditionNotifier sut(m_condVarData, EVENT_INDEX);
    sut.notify();
    for (Type_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER; i++)
    {
        if (i == EVENT_INDEX)
        {
            EXPECT_THAT(m_condVarData.m_activeNotifications[i].load(), Eq(true));
        }
        else
        {
            EXPECT_THAT(m_condVarData.m_activeNotifications[i].load(), Eq(false));
        }
    }
}

TEST_F(ConditionVariable_test, TimedWaitWithZeroTimeoutWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "582f0b1c-c717-410e-8143-61459db672ad");
    ConditionListener sut(m_condVarData);
    EXPECT_TRUE(sut.timedWait(iox::units::Duration::fromSeconds(0)).empty());
}

TEST_F(ConditionVariable_test, TimedWaitWithoutNotificationReturnsEmptyVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "15aaf499-9731-4c53-88f3-88af4983eae0");
    ConditionListener sut(m_condVarData);
    EXPECT_TRUE(sut.timedWait(iox::units::Duration::fromMilliseconds(100)).empty());
}

TEST_F(ConditionVariable_test, TimedWaitReturnsOneNotifiedIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf9ed236-bba9-43cd-84b2-6769d7f47d50");
    ConditionListener sut(m_condVarData);
    ConditionNotifier(m_condVarData, 6U).notify();

    auto indices = sut.timedWait(iox::units::Duration::fromMilliseconds(100));

    ASSERT_THAT(indices.size(), Eq(1U));
    EXPECT_THAT(indices[0U], Eq(6U));
}

TEST_F(ConditionVariable_test, TimedWaitReturnsMultipleNotifiedIndices)
{
    ::testing::Test::RecordProperty("TEST_ID", "771c2c11-effb-435a-9c67-a7d9471fdb6e");
    ConditionListener sut(m_condVarData);
    ConditionNotifier(m_condVarData, 5U).notify();
    ConditionNotifier(m_condVarData, 15U).notify();

    auto indices = sut.timedWait(iox::units::Duration::fromMilliseconds(100));

    ASSERT_THAT(indices.size(), Eq(2U));
    EXPECT_THAT(indices[0U], Eq(5U));
    EXPECT_THAT(indices[1U], Eq(15U));
}

TEST_F(ConditionVariable_test, TimedWaitReturnsAllNotifiedIndices)
{
    ::testing::Test::RecordProperty("TEST_ID", "38ee654b-228a-4462-9614-2901cb5272aa");
    ConditionListener sut(m_condVarData);
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_NOTIFIERS; ++i)
    {
        ConditionNotifier(m_condVarData, i).notify();
    }

    auto indices = sut.timedWait(iox::units::Duration::fromMilliseconds(100));

    ASSERT_THAT(indices.size(), Eq(iox::MAX_NUMBER_OF_NOTIFIERS));
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_NOTIFIERS; ++i)
    {
        EXPECT_THAT(indices[i], Eq(i));
    }
}

TIMING_TEST_F(ConditionVariable_test, TimedWaitBlocksUntilTimeout, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "c755aec9-43c3-4bf4-bec4-5672c76561ef");
    ConditionListener listener(m_condVarData);
    NotificationVector_t activeNotifications;
    iox::concurrent::Atomic<bool> hasWaited{false};

    std::thread waiter([&] {
        activeNotifications = listener.timedWait(m_timingTestTime);
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(0U));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(2 * m_timingTestTime.toMilliseconds() / 3));
    EXPECT_THAT(hasWaited.load(), Eq(false));
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * m_timingTestTime.toMilliseconds() / 3));
    EXPECT_THAT(hasWaited.load(), Eq(true));
    waiter.join();
})

TIMING_TEST_F(ConditionVariable_test, TimedWaitBlocksUntilNotification, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "b2999ddd-d072-4c9f-975e-fc8acc31397d");
    ConditionListener listener(m_condVarData);
    NotificationVector_t activeNotifications;
    iox::concurrent::Atomic<bool> hasWaited{false};

    std::thread waiter([&] {
        activeNotifications = listener.timedWait(m_timingTestTime);
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], 13U);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(m_timingTestTime.toMilliseconds() / 4));
    EXPECT_THAT(hasWaited.load(), Eq(false));
    ConditionNotifier(m_condVarData, 13U).notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(m_timingTestTime.toMilliseconds() / 4));
    EXPECT_THAT(hasWaited.load(), Eq(true));
    waiter.join();
})

TEST_F(ConditionVariable_test, WaitIsNonBlockingAfterDestroyAndReturnsEmptyVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "39bd43c0-c310-4f42-8baa-6873fbbbe705");
    ConditionListener sut(m_condVarData);

    sut.destroy();
    const auto& activeNotifications = sut.wait();

    EXPECT_THAT(activeNotifications.size(), Eq(0U));
}

TEST_F(ConditionVariable_test, WaitIsNonBlockingAfterDestroyAndNotifyAndReturnsEmptyVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "b803fc3e-f3a6-405c-86a0-ecedc06d0c05");
    ConditionListener sut(m_condVarData);
    sut.destroy();

    ConditionNotifier notifier(m_condVarData, 0U);
    notifier.notify();

    const auto& activeNotifications = sut.wait();
    EXPECT_THAT(activeNotifications.size(), Eq(0U));
}

TEST_F(ConditionVariable_test, DestroyWakesUpWaitWhichReturnsEmptyVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed0e434c-6efd-4218-88a8-9332e33f92fd");
    ConditionListener sut(m_condVarData);

    NotificationVector_t activeNotifications;

    std::thread waiter([&] {
        activeNotifications = sut.wait();
        EXPECT_THAT(activeNotifications.size(), Eq(0U));
    });

    sut.destroy();
    waiter.join();
}

TEST_F(ConditionVariable_test, GetCorrectNotificationVectorAfterNotifyAndWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "41a25c52-a358-4e94-b4a5-f315fb5124cd");
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    ConditionNotifier notifier(m_condVarData, EVENT_INDEX);
    ConditionListener listener(m_condVarData);

    notifier.notify();
    const auto& activeNotifications = listener.wait();

    ASSERT_THAT(activeNotifications.size(), Eq(1U));
    EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
}

TEST_F(ConditionVariable_test, GetCorrectNotificationVectorAfterMultipleNotifyAndWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b09bb18-e6c7-42cb-bb34-2da0dd26ca06");
    constexpr Type_t FIRST_EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 1U;
    constexpr Type_t SECOND_EVENT_INDEX = 0U;
    ConditionNotifier notifier1(m_condVarData, FIRST_EVENT_INDEX);
    ConditionNotifier notifier2(m_condVarData, SECOND_EVENT_INDEX);
    ConditionListener listener(m_condVarData);

    notifier1.notify();
    notifier2.notify();
    const auto& activeNotifications = listener.wait();

    ASSERT_THAT(activeNotifications.size(), Eq(2U));
    EXPECT_THAT(activeNotifications[0], Eq(SECOND_EVENT_INDEX));
    EXPECT_THAT(activeNotifications[1], Eq(FIRST_EVENT_INDEX));
}

TEST_F(ConditionVariable_test, WaitAndNotifyResultsInCorrectNotificationVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "4cac0ad0-083b-43dd-867e-dd6abb0291e8");
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 5U;
    ConditionNotifier notifier(m_condVarData, EVENT_INDEX);
    ConditionListener listener(m_condVarData);
    NotificationVector_t activeNotifications;

    std::thread waiter([&] {
        activeNotifications = listener.wait();
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
    });

    notifier.notify();
    waiter.join();
}

TIMING_TEST_F(ConditionVariable_test, WaitBlocks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "09d9ad43-ba97-4331-9a6b-ca22d2dbddb8");
    constexpr Type_t EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 5U;
    ConditionNotifier notifier(m_condVarData, EVENT_INDEX);
    ConditionListener listener(m_condVarData);
    NotificationVector_t activeNotifications;
    Barrier isThreadStarted(1U);
    iox::concurrent::Atomic<bool> hasWaited{false};

    std::thread waiter([&] {
        isThreadStarted.notify();
        activeNotifications = listener.wait();
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(EVENT_INDEX));
    });

    isThreadStarted.wait();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited.load(), Eq(false));
    notifier.notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited.load(), Eq(true));
    waiter.join();
})

TIMING_TEST_F(ConditionVariable_test, SecondWaitBlocksUntilNewNotification, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "dcbd55ee-e401-42cb-bbf2-a266058c2e76");
    constexpr Type_t FIRST_EVENT_INDEX = iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER - 2U;
    constexpr Type_t SECOND_EVENT_INDEX = 0U;
    ConditionNotifier notifier1(m_condVarData, FIRST_EVENT_INDEX);
    ConditionNotifier notifier2(m_condVarData, SECOND_EVENT_INDEX);
    ConditionListener listener(m_condVarData);
    iox::concurrent::Atomic<bool> hasWaited{false};

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

    Barrier isThreadStarted(1U);
    std::thread waiter([&] {
        isThreadStarted.notify();
        activeNotifications = listener.wait();
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(FIRST_EVENT_INDEX));
        for (const auto& notification : m_condVarData.m_activeNotifications)
        {
            EXPECT_THAT(notification.load(), Eq(false));
        }
    });

    isThreadStarted.wait();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited.load(), Eq(false));
    notifier1.notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited.load(), Eq(true));
    waiter.join();
})

void waitReturnsSortedListWhenTriggeredInOrder(ConditionVariable_test& test,
                                               const iox::function_ref<ConditionListener::NotificationVector_t()> wait)
{
    for (uint64_t i = 0U; i < test.m_notifiers.size(); ++i)
    {
        test.m_notifiers[i].notify();
    }

    auto notifications = wait();
    ASSERT_THAT(notifications.size(), Eq(test.m_notifiers.size()));
    for (uint64_t i = 0U; i < test.m_notifiers.size(); ++i)
    {
        EXPECT_THAT(notifications[i], Eq(i));
    }
}

TEST_F(ConditionVariable_test, WaitReturnsSortedListWhenTriggeredInOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "d9cfc71a-3300-41f8-b66f-486bdf5d27bc");
    waitReturnsSortedListWhenTriggeredInOrder(*this, [this] { return m_waiter.wait(); });
}

TEST_F(ConditionVariable_test, TimedWaitReturnsSortedListWhenTriggeredInOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9f875f6-c8ff-4c9c-aafa-78f7c0942bba");
    waitReturnsSortedListWhenTriggeredInOrder(
        *this, [this] { return m_waiter.timedWait(iox::units::Duration::fromSeconds(1)); });
}

void waitReturnsSortedListWhenTriggeredInReverseOrder(
    ConditionVariable_test& test, const iox::function_ref<ConditionListener::NotificationVector_t()> wait)
{
    for (uint64_t i = 0U; i < test.m_notifiers.size(); ++i)
    {
        test.m_notifiers[test.m_notifiers.size() - i - 1U].notify();
    }

    auto notifications = wait();
    ASSERT_THAT(notifications.size(), Eq(test.m_notifiers.size()));
    for (uint64_t i = 0U; i < test.m_notifiers.size(); ++i)
    {
        EXPECT_THAT(notifications[i], Eq(i));
    }
}

TEST_F(ConditionVariable_test, WaitReturnsSortedListWhenTriggeredInReverseOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "a28eb73d-c279-46ed-b6f8-369b10045ea5");
    waitReturnsSortedListWhenTriggeredInReverseOrder(*this, [this] { return m_waiter.wait(); });
}

TEST_F(ConditionVariable_test, TimedWaitReturnsSortedListWhenTriggeredInReverseOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "53050a1c-fb1c-42aa-a376-bfb095bf5f94");
    waitReturnsSortedListWhenTriggeredInReverseOrder(
        *this, [this] { return m_waiter.timedWait(iox::units::Duration::fromSeconds(1)); });
}

} // namespace
