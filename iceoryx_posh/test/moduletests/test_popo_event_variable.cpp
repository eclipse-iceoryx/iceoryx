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

#include "iceoryx_posh/internal/popo/building_blocks/event_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_notifier.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"

#include "test.hpp"
#include "testutils/timing_test.hpp"

#include <thread>

using namespace ::testing;
using namespace iox::popo;

class EventVariable_test : public Test
{
  public:
    using notificationVector = iox::cxx::vector<uint64_t, iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET>;
    EventVariableData m_eventVarData{"Ferdinand"};
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
    EXPECT_THAT(m_eventVarData.m_process.c_str(), StrEq("Ferdinand"));
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
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1;
    EventNotifier sut(m_eventVarData, index);
    sut.notify();
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; i++)
    {
        if (i == index)
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
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET;
    EventNotifier sut(m_eventVarData, index);
    sut.notify();
    for (const auto& notification : m_eventVarData.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

TEST_F(EventVariable_test, GetCorrectNotificationVectorAfterNotifyAndWait)
{
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U;
    EventNotifier notifier(m_eventVarData, index);
    EventListener listener(m_eventVarData);

    notifier.notify();
    const auto& activeNotifications = listener.wait();
    ASSERT_THAT(activeNotifications.size(), Eq(1U));
    EXPECT_THAT(activeNotifications[0], Eq(index));
}

TEST_F(EventVariable_test, GetCorrectNotificationVectorAfterMultipleNotifyAndWait)
{
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 1U;
    EventNotifier notifier1(m_eventVarData, index);
    EventNotifier notifier2(m_eventVarData, 0U);
    EventListener listener(m_eventVarData);

    notifier1.notify();
    notifier2.notify();
    const auto& activeNotifications = listener.wait();
    ASSERT_THAT(activeNotifications.size(), Eq(2U));
    EXPECT_THAT(activeNotifications[0], Eq(0U));
    EXPECT_THAT(activeNotifications[1], Eq(index));
}

TEST_F(EventVariable_test, WaitAndNotifyResultsInCorrectNotificationVector)
{
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 5U;
    EventNotifier notifier(m_eventVarData, index);
    EventListener listener(m_eventVarData);
    notificationVector activeNotifications;

    std::thread waiter([&] {
        activeNotifications = listener.wait();
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(index));
    });

    notifier.notify();
    waiter.join();
}

TIMING_TEST_F(EventVariable_test, WaitBlocks, Repeat(5), [&] {
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 5U;
    EventNotifier notifier(m_eventVarData, index);
    EventListener listener(m_eventVarData);
    notificationVector activeNotifications;
    iox::posix::Semaphore semaphore =
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();
    std::atomic_bool hasWaited{false};

    std::thread waiter([&] {
        semaphore.post();
        activeNotifications = listener.wait();
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(index));
    });

    semaphore.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(false));
    notifier.notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(true));
    waiter.join();
})

TIMING_TEST_F(EventVariable_test, SecondWaitBlocksUntilNewNotification, Repeat(5), [&] {
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET - 2U;
    EventNotifier notifier1(m_eventVarData, index);
    EventNotifier notifier2(m_eventVarData, 0U);
    EventListener listener(m_eventVarData);
    iox::posix::Semaphore semaphore =
        iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();
    std::atomic_bool hasWaited{false};

    notifier1.notify();
    notifier2.notify();
    notificationVector activeNotifications = listener.wait();
    ASSERT_THAT(activeNotifications.size(), Eq(2U));
    EXPECT_THAT(activeNotifications[0], Eq(0U));
    EXPECT_THAT(activeNotifications[1], Eq(index));

    std::thread waiter([&] {
        semaphore.post();
        activeNotifications = listener.wait();
        hasWaited.store(true, std::memory_order_relaxed);
        ASSERT_THAT(activeNotifications.size(), Eq(1U));
        EXPECT_THAT(activeNotifications[0], Eq(index));
    });

    semaphore.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(false));
    notifier1.notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(hasWaited, Eq(true));
    waiter.join();
})

TEST_F(EventVariable_test, ResetTrueEntryResultsInFalse)
{
    uint64_t index = 3U;
    EventNotifier notifier(m_eventVarData, index);
    EventListener listener(m_eventVarData);

    notifier.notify();
    ASSERT_THAT(m_eventVarData.m_activeNotifications[index], Eq(true));

    listener.reset(index);
    EXPECT_THAT(m_eventVarData.m_activeNotifications[index], Eq(false));
}

TEST_F(EventVariable_test, ResetFalseEntryResultsInFalse)
{
    uint64_t index = 0U;
    EventListener listener(m_eventVarData);

    ASSERT_THAT(m_eventVarData.m_activeNotifications[index], Eq(false));
    listener.reset(index);
    EXPECT_THAT(m_eventVarData.m_activeNotifications[index], Eq(false));
}

TEST_F(EventVariable_test, ResetWithTooLargeIndexDoesNotChangeNotificationArray)
{
    uint64_t index = iox::MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET;
    EventListener listener(m_eventVarData);

    listener.reset(index);
    for (const auto& notification : m_eventVarData.m_activeNotifications)
    {
        EXPECT_THAT(notification, Eq(false));
    }
}

