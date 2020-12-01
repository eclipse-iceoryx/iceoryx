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
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "mocks/wait_set_mock.hpp"
#include "testutils/timing_test.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

#include <atomic>
#include <thread>

using namespace ::testing;

class iox_ws_test : public Test
{
  public:
    void SetUp() override
    {
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
        {
            m_userTrigger.emplace_back(iox_user_trigger_init(&m_userTriggerStorage[i]));
        }
    }

    void TearDown() override
    {
        delete m_sut;

        for (uint64_t i = 0U; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
        {
            iox_user_trigger_deinit(m_userTrigger[i]);
        }
    }

    ConditionVariableData m_condVar;
    WaitSetMock* m_sut = new WaitSetMock{&m_condVar};

    iox_user_trigger_storage_t m_userTriggerStorage[MAX_NUMBER_OF_TRIGGERS_PER_WAITSET + 1];
    cxx::vector<iox_user_trigger_t, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET + 1> m_userTrigger;

    iox_trigger_state_storage_t m_triggerStateStorage[MAX_NUMBER_OF_TRIGGERS_PER_WAITSET];
    uint64_t m_missedElements = 0U;
    uint64_t m_numberOfTriggeredConditions = 0U;
    timespec m_timeout{0, 0};
};

TEST_F(iox_ws_test, CapacityIsCorrect)
{
    EXPECT_EQ(iox_ws_trigger_capacity(m_sut), MAX_NUMBER_OF_TRIGGERS_PER_WAITSET);
}

TEST_F(iox_ws_test, SizeIsZeroWhenConstructed)
{
    EXPECT_EQ(iox_ws_size(m_sut), 0);
}

TEST_F(iox_ws_test, SizeIsOneWhenOneClassIsAttached)
{
    EXPECT_EQ(iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 0, NULL),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    EXPECT_EQ(iox_ws_size(m_sut), 1);
}

TEST_F(iox_ws_test, SizeEqualsCapacityWhenMaximumIsAttached)
{
    for (uint64_t i = 0; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        EXPECT_EQ(iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL),
                  iox_WaitSetResult::WaitSetResult_SUCCESS);
    }
    EXPECT_EQ(iox_ws_size(m_sut), iox_ws_trigger_capacity(m_sut));
}

TEST_F(iox_ws_test, SizeDecreasesWhenAttachedObjectIsDeinitialized)
{
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 0, NULL);
    iox_user_trigger_detach(m_userTrigger[0]);
    EXPECT_EQ(iox_ws_size(m_sut), 0);
}

TEST_F(iox_ws_test, NumberOfTriggeredConditionsIsOneWhenOneWasTriggered)
{
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 0, NULL);
    iox_user_trigger_trigger(m_userTrigger[0]);

    EXPECT_EQ(
        iox_ws_wait(
            m_sut, (iox_trigger_state_t)m_triggerStateStorage, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET, &m_missedElements),
        1);
}

TEST_F(iox_ws_test, NumberOfTriggeredConditionsIsCorrectWhenMultipleWereTriggered)
{
    for (uint64_t i = 0; i < 10; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(
        iox_ws_wait(
            m_sut, (iox_trigger_state_t)m_triggerStateStorage, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET, &m_missedElements),
        10);
}

TEST_F(iox_ws_test, NumberOfTriggeredConditionsIsCorrectWhenAllWereTriggered)
{
    for (uint64_t i = 0; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(
        iox_ws_wait(
            m_sut, (iox_trigger_state_t)m_triggerStateStorage, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET, &m_missedElements),
        MAX_NUMBER_OF_TRIGGERS_PER_WAITSET);
}

TEST_F(iox_ws_test, SingleTriggerCaseWaitReturnsCorrectTrigger)
{
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 5678, NULL);
    iox_user_trigger_trigger(m_userTrigger[0]);

    iox_ws_wait(
        m_sut, (iox_trigger_state_t)m_triggerStateStorage, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET, &m_missedElements);

    iox_trigger_state_t triggerState = (iox_trigger_state_t)&m_triggerStateStorage[0];

    EXPECT_EQ(iox_trigger_state_get_trigger_id(triggerState), 5678);
    EXPECT_TRUE(iox_trigger_state_does_originate_from_user_trigger(triggerState, m_userTrigger[0]));
}

TEST_F(iox_ws_test, MultiTriggerCaseWaitReturnsCorrectTrigger)
{
    for (uint64_t i = 0U; i < 8; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 1337 + i, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(
        m_sut, (iox_trigger_state_t)m_triggerStateStorage, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET, &m_missedElements);

    for (uint64_t i = 0U; i < 8; ++i)
    {
        iox_trigger_state_t triggerState = (iox_trigger_state_t)&m_triggerStateStorage[i];
        EXPECT_EQ(iox_trigger_state_get_trigger_id(triggerState), 1337 + i);
        EXPECT_TRUE(iox_trigger_state_does_originate_from_user_trigger(triggerState, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, MaxTriggerCaseWaitReturnsCorrectTrigger)
{
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 42 * i + 1, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(
        m_sut, (iox_trigger_state_t)m_triggerStateStorage, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET, &m_missedElements);

    for (uint64_t i = 0U; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        iox_trigger_state_t triggerState = (iox_trigger_state_t)&m_triggerStateStorage[i];
        EXPECT_EQ(iox_trigger_state_get_trigger_id(triggerState), 42 * i + 1);
        EXPECT_TRUE(iox_trigger_state_does_originate_from_user_trigger(triggerState, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, TimedWaitNumberOfTriggeredConditionsIsOneWhenOneWasTriggered)
{
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 0, NULL);
    iox_user_trigger_trigger(m_userTrigger[0]);

    EXPECT_EQ(iox_ws_timed_wait(m_sut,
                                m_timeout,
                                (iox_trigger_state_t)m_triggerStateStorage,
                                MAX_NUMBER_OF_TRIGGERS_PER_WAITSET,
                                &m_missedElements),
              1);
}

TEST_F(iox_ws_test, TimedWaitNumberOfTriggeredConditionsIsCorrectWhenMultipleWereTriggered)
{
    for (uint64_t i = 0; i < 10; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(iox_ws_timed_wait(m_sut,
                                m_timeout,
                                (iox_trigger_state_t)m_triggerStateStorage,
                                MAX_NUMBER_OF_TRIGGERS_PER_WAITSET,
                                &m_missedElements),
              10);
}

TEST_F(iox_ws_test, TimedWaitNumberOfTriggeredConditionsIsCorrectWhenAllWereTriggered)
{
    for (uint64_t i = 0; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(iox_ws_timed_wait(m_sut,
                                m_timeout,
                                (iox_trigger_state_t)m_triggerStateStorage,
                                MAX_NUMBER_OF_TRIGGERS_PER_WAITSET,
                                &m_missedElements),
              MAX_NUMBER_OF_TRIGGERS_PER_WAITSET);
}

TEST_F(iox_ws_test, SingleTriggerCaseTimedWaitReturnsCorrectTrigger)
{
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 5678, NULL);
    iox_user_trigger_trigger(m_userTrigger[0]);

    iox_ws_timed_wait(m_sut,
                      m_timeout,
                      (iox_trigger_state_t)m_triggerStateStorage,
                      MAX_NUMBER_OF_TRIGGERS_PER_WAITSET,
                      &m_missedElements);

    iox_trigger_state_t triggerState = (iox_trigger_state_t)&m_triggerStateStorage[0];

    EXPECT_EQ(iox_trigger_state_get_trigger_id(triggerState), 5678);
    EXPECT_TRUE(iox_trigger_state_does_originate_from_user_trigger(triggerState, m_userTrigger[0]));
}

TEST_F(iox_ws_test, MultiTriggerCaseTimedWaitReturnsCorrectTrigger)
{
    for (uint64_t i = 0U; i < 8; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 1337 + i, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_timed_wait(m_sut,
                      m_timeout,
                      (iox_trigger_state_t)m_triggerStateStorage,
                      MAX_NUMBER_OF_TRIGGERS_PER_WAITSET,
                      &m_missedElements);

    for (uint64_t i = 0U; i < 8; ++i)
    {
        iox_trigger_state_t triggerState = (iox_trigger_state_t)&m_triggerStateStorage[i];
        EXPECT_EQ(iox_trigger_state_get_trigger_id(triggerState), 1337 + i);
        EXPECT_TRUE(iox_trigger_state_does_originate_from_user_trigger(triggerState, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, MaxTriggerCaseTimedWaitReturnsCorrectTrigger)
{
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 42 * i + 1, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_timed_wait(m_sut,
                      m_timeout,
                      (iox_trigger_state_t)m_triggerStateStorage,
                      MAX_NUMBER_OF_TRIGGERS_PER_WAITSET,
                      &m_missedElements);

    for (uint64_t i = 0U; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        iox_trigger_state_t triggerState = (iox_trigger_state_t)&m_triggerStateStorage[i];
        EXPECT_EQ(iox_trigger_state_get_trigger_id(triggerState), 42 * i + 1);
        EXPECT_TRUE(iox_trigger_state_does_originate_from_user_trigger(triggerState, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, MissedElementsIsZeroWhenNothingWasMissed)
{
    for (uint64_t i = 0; i < 12; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(
        m_sut, (iox_trigger_state_t)m_triggerStateStorage, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET, &m_missedElements);

    EXPECT_EQ(m_missedElements, 0U);
}

TEST_F(iox_ws_test, MissedElementsIsCorrectWhenSomethingWasMissed)
{
    for (uint64_t i = 0; i < 12; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(m_sut, (iox_trigger_state_t)m_triggerStateStorage, 8, &m_missedElements);

    EXPECT_EQ(m_missedElements, 4U);
}

TEST_F(iox_ws_test, MissedElementsIsCorrectWhenAllWereMissed)
{
    for (uint64_t i = 0; i < MAX_NUMBER_OF_TRIGGERS_PER_WAITSET; ++i)
    {
        iox_user_trigger_attach_to_waitset(m_userTrigger[i], m_sut, 0, NULL);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(m_sut, (iox_trigger_state_t)m_triggerStateStorage, 0, &m_missedElements);

    EXPECT_EQ(m_missedElements, MAX_NUMBER_OF_TRIGGERS_PER_WAITSET);
}

TIMING_TEST_F(iox_ws_test, WaitIsBlockingTillTriggered, Repeat(5), [&] {
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 0, NULL);

    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        iox_ws_wait(m_sut, NULL, 0, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_user_trigger_trigger(m_userTrigger[0]);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
});

TIMING_TEST_F(iox_ws_test, TimedWaitIsBlockingTillTriggered, Repeat(5), [&] {
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 0, NULL);

    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        iox_ws_timed_wait(m_sut, {1000, 1000}, NULL, 0, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_user_trigger_trigger(m_userTrigger[0]);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
});

TIMING_TEST_F(iox_ws_test, TimedWaitBlocksTillTimeout, Repeat(5), [&] {
    iox_user_trigger_attach_to_waitset(m_userTrigger[0], m_sut, 0, NULL);

    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        constexpr long hundredMsInNanoSeconds = 100000000;
        iox_ws_timed_wait(m_sut, {0, hundredMsInNanoSeconds}, NULL, 0, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(75));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());
    std::this_thread::sleep_for(std::chrono::milliseconds(75));
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());

    t.join();
});

