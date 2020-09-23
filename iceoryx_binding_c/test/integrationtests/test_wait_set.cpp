// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "mocks/wait_set_mock.hpp"
#include "testutils/timing_test.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/guard_condition.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

#include <atomic>

using namespace ::testing;

class iox_ws_test : public Test
{
  public:
    void SetUp() override
    {
        m_guardCond = iox_guard_cond_init(&m_guardCondStorage);
    }

    void TearDown() override
    {
        delete m_sut;

        iox_guard_cond_deinit(m_guardCond);
        for (auto s : m_subscriber)
        {
            delete s;
        }
    }

    iox_sub_t CreateSubscriber()
    {
        const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};

        cpp2c_Subscriber* subscriber = new cpp2c_Subscriber();
        subscriber->m_portData = new SubscriberPortData{
            TEST_SERVICE_DESCRIPTION, "myApp", iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

        m_subscriber.emplace_back(subscriber);
        return subscriber;
    }

    void RemoveSubscriber(iox_sub_t subscriber)
    {
        delete subscriber->m_portData;
        delete subscriber;
    }

    ConditionVariableData m_condVar;
    iox_ws_storage_t m_sutStorage;

    iox_guard_cond_storage_t m_guardCondStorage;
    iox_guard_cond_t m_guardCond;

    WaitSetMock* m_sut = new WaitSetMock{&m_condVar};
    std::vector<iox_sub_t> m_subscriber;
};

TEST_F(iox_ws_test, AttachSingleConditionIsSuccessful)
{
    iox_sub_t subscriber = CreateSubscriber();
    EXPECT_THAT(iox_ws_attach_condition(m_sut, subscriber), Eq(WaitSetResult_SUCCESS));
}

TEST_F(iox_ws_test, AttachSingleConditionTwiceResultsInFailure)
{
    iox_sub_t subscriber = CreateSubscriber();
    iox_ws_attach_condition(m_sut, subscriber);

    EXPECT_THAT(iox_ws_attach_condition(m_sut, subscriber), Eq(WaitSetResult_CONDITION_VARIABLE_ALREADY_SET));
}

TEST_F(iox_ws_test, DetachAttachedConditionIsSuccessful)
{
    iox_sub_t subscriber = CreateSubscriber();
    iox_ws_attach_condition(m_sut, subscriber);

    EXPECT_TRUE(iox_ws_detach_condition(m_sut, subscriber));
}

TEST_F(iox_ws_test, DetachNotAttachedConditionFails)
{
    iox_sub_t subscriber = CreateSubscriber();

    EXPECT_FALSE(iox_ws_detach_condition(m_sut, subscriber));
}

TEST_F(iox_ws_test, DetachFailsAfterAllConditionsAreDetached)
{
    iox_sub_t subscriber = CreateSubscriber();
    iox_ws_attach_condition(m_sut, subscriber);
    iox_ws_detach_all_conditions(m_sut);

    EXPECT_FALSE(iox_ws_detach_condition(m_sut, subscriber));
}

TEST_F(iox_ws_test, AttachConditionsSucceedsAfterAllConditionsAreDetached)
{
    iox_sub_t subscriber = CreateSubscriber();
    iox_ws_attach_condition(m_sut, subscriber);
    iox_ws_detach_all_conditions(m_sut);

    EXPECT_THAT(iox_ws_attach_condition(m_sut, subscriber), Eq(WaitSetResult_SUCCESS));
}

TIMING_TEST_F(iox_ws_test, TimedWaitBlocksTillTriggered, Repeat(5), [&] {
    std::atomic_bool waitSetNotified{false};

    iox_ws_attach_condition(m_sut, (iox_cond_t)m_guardCond);

    std::thread t([&] {
        struct timespec timeout;
        timeout.tv_sec = 10;
        timeout.tv_nsec = 0;

        uint64_t missedElements;
        iox_ws_timed_wait(m_sut, timeout, NULL, 0, &missedElements);
        waitSetNotified.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    TIMING_TEST_EXPECT_FALSE(waitSetNotified.load());
    iox_guard_cond_trigger(m_guardCond);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_TRUE(waitSetNotified.load());

    t.join();
});

TIMING_TEST_F(iox_ws_test, TimedWaitWritesEmptyConditionArrayWhenNotTriggered, Repeat(5), [&] {
    iox_ws_attach_condition(m_sut, (iox_cond_t)m_guardCond);

    constexpr uint64_t numberOfConditions = 10U;
    iox_cond_t conditions[numberOfConditions];
    uint64_t conditionArraySize;
    uint64_t missedElements;

    std::thread t([&] {
        struct timespec timeout;
        timeout.tv_sec = 0;
        timeout.tv_nsec = 1000;

        conditionArraySize = iox_ws_timed_wait(m_sut, timeout, conditions, numberOfConditions, &missedElements);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    t.join();

    TIMING_TEST_EXPECT_TRUE(conditionArraySize == 0U);
    TIMING_TEST_EXPECT_TRUE(missedElements == 0U);
});

TIMING_TEST_F(iox_ws_test, TimedWaitWritesConditionIntoArrayWhenTriggered, Repeat(5), [&] {
    iox_ws_attach_condition(m_sut, (iox_cond_t)m_guardCond);

    constexpr uint64_t numberOfConditions = 10U;
    iox_cond_t conditions[numberOfConditions];
    uint64_t conditionArraySize;
    uint64_t missedElements;

    std::thread t([&] {
        struct timespec timeout;
        timeout.tv_sec = 10;
        timeout.tv_nsec = 0;

        conditionArraySize = iox_ws_timed_wait(m_sut, timeout, conditions, numberOfConditions, &missedElements);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    iox_guard_cond_trigger(m_guardCond);

    t.join();

    TIMING_TEST_ASSERT_TRUE(conditionArraySize == 1U);
    TIMING_TEST_EXPECT_TRUE((void*)conditions[0] == (void*)m_guardCond);
});

TIMING_TEST_F(iox_ws_test, TimedWaitWritesMissedElementsIntoArrayWhenTriggered, Repeat(5), [&] {
    iox_ws_attach_condition(m_sut, (iox_cond_t)m_guardCond);

    uint64_t missedElements;

    std::thread t([&] {
        struct timespec timeout;
        timeout.tv_sec = 10;
        timeout.tv_nsec = 0;

        iox_ws_timed_wait(m_sut, timeout, NULL, 0, &missedElements);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    iox_guard_cond_trigger(m_guardCond);

    t.join();

    TIMING_TEST_EXPECT_TRUE(missedElements == 1U);
});

TIMING_TEST_F(iox_ws_test, WaitBlocksTillTriggered, Repeat(5), [&] {
    std::atomic_bool waitSetNotified{false};

    iox_ws_attach_condition(m_sut, (iox_cond_t)m_guardCond);

    std::thread t([&] {
        uint64_t missedElements;
        iox_ws_wait(m_sut, NULL, 0, &missedElements);
        waitSetNotified.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    TIMING_TEST_EXPECT_FALSE(waitSetNotified.load());
    iox_guard_cond_trigger(m_guardCond);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_TRUE(waitSetNotified.load());

    t.join();
});

TIMING_TEST_F(iox_ws_test, WaitWritesConditionIntoArrayWhenTriggered, Repeat(5), [&] {
    iox_ws_attach_condition(m_sut, (iox_cond_t)m_guardCond);

    constexpr uint64_t numberOfConditions = 10U;
    iox_cond_t conditions[numberOfConditions];
    uint64_t conditionArraySize;
    uint64_t missedElements;

    std::thread t([&] { conditionArraySize = iox_ws_wait(m_sut, conditions, numberOfConditions, &missedElements); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    iox_guard_cond_trigger(m_guardCond);

    t.join();

    TIMING_TEST_ASSERT_TRUE(conditionArraySize == 1U);
    TIMING_TEST_EXPECT_TRUE((void*)conditions[0] == (void*)m_guardCond);
});

TIMING_TEST_F(iox_ws_test, WaitWritesMissedElementsIntoArrayWhenTriggered, Repeat(5), [&] {
    iox_ws_attach_condition(m_sut, (iox_cond_t)m_guardCond);

    uint64_t missedElements;

    std::thread t([&] { iox_ws_wait(m_sut, NULL, 0, &missedElements); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    iox_guard_cond_trigger(m_guardCond);

    t.join();

    TIMING_TEST_EXPECT_TRUE(missedElements == 1U);
});

