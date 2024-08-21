// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iox/atomic.hpp"
#include "iox/deadline_timer.hpp"

#include "test.hpp"
#include <thread>

namespace
{
using namespace iox;
using namespace iox::popo;
using namespace ::testing;

class TriggerHandle_test : public Test
{
  public:
    void resetCallback(const uint64_t id)
    {
        m_resetCallbackId = id;
    }

    void SetUp()
    {
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    uint64_t m_resetCallbackId = 0U;
    ConditionVariableData m_condVar{"Horscht"};
    TriggerHandle_test* m_self = this;

    Watchdog m_watchdog{units::Duration::fromSeconds(2U)};
    TriggerHandle m_sut{m_condVar, {*this, &TriggerHandle_test::resetCallback}, 12U};
};


TEST_F(TriggerHandle_test, IsValidWhenConditionVariableIsNotNull)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b2ece7a-bd79-45c9-85e6-7868f82bd0f4");
    EXPECT_TRUE(m_sut.isValid());
    EXPECT_TRUE(m_sut);
}

TEST_F(TriggerHandle_test, DefaultCTorConstructsInvalidHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "9910af97-c47a-4048-8c06-13402e02b1ee");
    TriggerHandle sut2;

    EXPECT_FALSE(sut2.isValid());
    EXPECT_THAT(sut2.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
    EXPECT_FALSE(sut2);
}

TEST_F(TriggerHandle_test, InvalidateCreatesInvalidTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "3fdd92d9-d598-443c-b71e-708cf12b874c");
    m_sut.invalidate();

    EXPECT_FALSE(m_sut.isValid());
    EXPECT_FALSE(m_sut);
    EXPECT_THAT(m_sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(TriggerHandle_test, ResetCreatesInvalidTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d977b12-75ce-4374-88bc-d7e814db4fad");
    m_sut.reset();

    EXPECT_FALSE(m_sut.isValid());
    EXPECT_FALSE(m_sut);
    EXPECT_THAT(m_sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(TriggerHandle_test, ResetCallsResetCallbackWhenHandleIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f4b5803-de4a-42f5-afc2-7953a211158d");
    m_sut.reset();
    EXPECT_EQ(m_resetCallbackId, 12U);
    EXPECT_THAT(m_sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(TriggerHandle_test, ResetDoesNotCallResetCallbackWhenHandleIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "900c0fb5-81a7-46dd-bf5f-1df6072bcb53");
    m_sut.invalidate();
    m_sut.reset();
    EXPECT_EQ(m_resetCallbackId, 0U);
    EXPECT_THAT(m_sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(TriggerHandle_test, getConditionVariableDataReturnsCorrectVar)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5722115-2624-4f19-992e-52afb98b9dd5");
    EXPECT_EQ(m_sut.getConditionVariableData(), &m_condVar);
}

TEST_F(TriggerHandle_test, getUniqueIdReturnsCorrectId)
{
    ::testing::Test::RecordProperty("TEST_ID", "856e38a0-c5eb-461c-9445-efb5b21cc5db");
    TriggerHandle sut2{m_condVar, {*m_self, &TriggerHandle_test::resetCallback}, 8912U};
    EXPECT_EQ(sut2.getUniqueId(), 8912U);
}

TEST_F(TriggerHandle_test, triggerNotifiesConditionVariable)
{
    ::testing::Test::RecordProperty("TEST_ID", "11e752c8-d473-4bfd-b973-869c3b2d9fbc");

    iox::concurrent::Atomic<int> stage{0};

    std::thread t([&] {
        stage.store(1);
        ConditionListener(m_condVar).wait();
        stage.store(2);
    });

    // the watchdog prevents an infinite loop in case 'stage' is never set to '1'
    while (stage.load() < 1)
    {
        std::this_thread::yield();
    }

    iox::deadline_timer timeout{200_ms};
    EXPECT_THAT(stage.load(), Eq(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_THAT(stage.load(), Eq(1));
    m_sut.trigger();
    t.join();

    EXPECT_FALSE(timeout.hasExpired());
}

TEST_F(TriggerHandle_test, wasTriggeredReturnsFalseAfterCreation)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a40fead-103f-4f3a-a4db-4b8aacae1b57");
    EXPECT_FALSE(m_sut.wasTriggered());
}

TEST_F(TriggerHandle_test, wasTriggeredReturnsFalseWhenHandleIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "46f36789-894a-4aba-b17e-d844e4cb90ee");
    m_sut.reset();
    EXPECT_FALSE(m_sut.wasTriggered());
}

TEST_F(TriggerHandle_test, wasTriggeredReturnsTrueAfterItWasTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "afc27579-011b-4879-82e4-0377a3bfb68d");
    m_sut.trigger();
    EXPECT_TRUE(m_sut.wasTriggered());
}

TEST_F(TriggerHandle_test, wasTriggeredReturnsFalseAfterItWasTriggeredAndTheListenerResetIt)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d44c016-4abd-4897-bd7a-0a9ce41ffa1f");
    m_sut.trigger();
    ConditionListener(m_condVar).timedWait(units::Duration::fromSeconds(0U));
    EXPECT_FALSE(m_sut.wasTriggered());
}

} // namespace
