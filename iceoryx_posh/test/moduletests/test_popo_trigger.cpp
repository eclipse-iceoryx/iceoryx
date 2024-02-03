// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/popo/trigger.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "test.hpp"

#include <thread>

namespace
{
using namespace iox;
using namespace iox::popo;
using namespace ::testing;
using namespace ::testing::internal;

class Trigger_test : public Test
{
  public:
    class TriggerClass
    {
      public:
        bool hasTriggered() const
        {
            return m_hasTriggered;
        }

        void resetCall(const uint64_t trigger)
        {
            m_resetCallTriggerArg = trigger;
        }

        static void callback(TriggerClass* const ptr)
        {
            m_lastCallbackArgument = ptr;
        }

        bool m_hasTriggered = false;
        uint64_t m_resetCallTriggerArg = 0U;
        ConditionVariableData* m_condVar = nullptr;
        static TriggerClass* m_lastCallbackArgument;

        const Trigger* m_moveCallTriggerArg = nullptr;
        void* m_moveCallNewOriginArg = nullptr;
    };

    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    Trigger
    createValidStateBasedTrigger(const uint64_t eventId = 0U, const uint64_t type = 0U, const uint64_t typeHash = 0U)
    {
        static uint64_t uniqueId = 0U;
        Trigger trigger(StateBasedTrigger,
                        &m_triggerClass,
                        {m_triggerClass, &TriggerClass::hasTriggered},
                        {m_triggerClass, &TriggerClass::resetCall},
                        eventId,
                        createNotificationCallback(TriggerClass::callback),
                        uniqueId++,
                        type,
                        typeHash);
        EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::STATE_BASED));
        return trigger;
    }

    Trigger
    createValidEventBasedTrigger(const uint64_t eventId = 0U, const uint64_t type = 0U, const uint64_t typeHash = 0U)
    {
        static uint64_t uniqueId = 0U;
        Trigger trigger(EventBasedTrigger,
                        &m_triggerClass,
                        {m_triggerClass, &TriggerClass::resetCall},
                        eventId,
                        createNotificationCallback(TriggerClass::callback),
                        uniqueId++,
                        type,
                        typeHash);
        EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::EVENT_BASED));
        return trigger;
    }

    ConditionVariableData m_condVar{"Horscht"};
    TriggerClass m_triggerClass;
};

Trigger_test::TriggerClass* Trigger_test::TriggerClass::m_lastCallbackArgument = nullptr;

// state based trigger

TEST_F(Trigger_test, TriggerWithValidOriginIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "54b79e70-4b18-4d37-b652-821218a176e0");
    Trigger sut = createValidStateBasedTrigger();

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, MovedConstructedValidTriggerIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "08a65b29-c171-47b3-9de5-29ae6cb3b0b7");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 90001U;
    constexpr uint64_t originTypeHash = 40001U;

    Trigger trigger = createValidStateBasedTrigger(id, originType, originTypeHash);
    Trigger sut{std::move(trigger)};

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::STATE_BASED));
    EXPECT_TRUE(sut.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));

    EXPECT_FALSE(trigger.isValid());
    EXPECT_FALSE(static_cast<bool>(trigger));
    EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(trigger.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, MovedAssignedValidTriggerIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "49205508-79d9-4a54-8d23-96eb70bb8446");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 190001U;
    constexpr uint64_t originTypeHash = 140001U;
    constexpr uint64_t anotherOriginType = 290001U;
    constexpr uint64_t anotherOriginTypeHash = 240001U;

    Trigger sut = createValidStateBasedTrigger(id, originType, originTypeHash);
    Trigger trigger = createValidStateBasedTrigger(id, anotherOriginType, anotherOriginTypeHash);
    sut = std::move(trigger);

    EXPECT_FALSE(trigger.isValid());
    EXPECT_THAT(trigger.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::STATE_BASED));
    EXPECT_TRUE(sut.isLogicalEqualTo(&m_triggerClass, anotherOriginType, anotherOriginTypeHash));

    EXPECT_FALSE(trigger.isValid());
    EXPECT_FALSE(static_cast<bool>(trigger));
    EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(trigger.isLogicalEqualTo(&m_triggerClass, anotherOriginType, anotherOriginTypeHash));
}

TEST_F(Trigger_test, TriggerWithNullptrOriginIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "ade4ce54-1659-4d1d-8784-f8f0edc65fb0");
    constexpr uint64_t eventId = 0U;
    constexpr uint64_t uniqueTriggerId = 0U;
    constexpr uint64_t type = 0U;
    constexpr uint64_t typeHash = 0U;

    Trigger sut(StateBasedTrigger,
                static_cast<TriggerClass*>(nullptr),
                {m_triggerClass, &TriggerClass::hasTriggered},
                {m_triggerClass, &TriggerClass::resetCall},
                eventId,
                createNotificationCallback(TriggerClass::callback),
                uniqueTriggerId,
                type,
                typeHash);

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, ResetInvalidatesTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "3d8c297c-04d1-4ce1-b51e-684666470bc5");
    Trigger sut = createValidStateBasedTrigger();
    sut.reset();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(Trigger_test, InvalidateInvalidatesTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4b87606-acf4-4962-9b68-48650457f2e0");
    Trigger sut = createValidStateBasedTrigger();
    sut.invalidate();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(Trigger_test, ResetCallsResetcallbackWithCorrectTriggerOrigin)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ce18ec9-b6c9-47d9-afec-57be1724737c");
    Trigger sut = createValidStateBasedTrigger();
    auto uniqueId = sut.getUniqueId();
    sut.reset();

    EXPECT_EQ(m_triggerClass.m_resetCallTriggerArg, uniqueId);
}

TEST_F(Trigger_test, ResetSetsTriggerIdToInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed2f8f50-4a6e-4fb9-bd49-d605c87b1cfb");
    Trigger sut = createValidStateBasedTrigger();
    sut.reset();

    EXPECT_EQ(sut.getUniqueId(), Trigger::INVALID_TRIGGER_ID);
}

TEST_F(Trigger_test, TriggerWithEmptyResetInvalidatesTriggerWhenBeingResetted)
{
    ::testing::Test::RecordProperty("TEST_ID", "64b55c52-06cf-4d96-bb17-2121aa6e68e6");
    constexpr uint64_t eventId = 0U;
    constexpr uint64_t uniqueTriggerId = 0U;
    constexpr uint64_t type = 0U;
    constexpr uint64_t typeHash = 0U;

    Trigger sut(
        StateBasedTrigger,
        &m_triggerClass,
        {m_triggerClass, &TriggerClass::hasTriggered},
        [](auto) {},
        eventId,
        createNotificationCallback(TriggerClass::callback),
        uniqueTriggerId,
        type,
        typeHash);

    sut.reset();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));

    IOX_TESTING_EXPECT_OK();
}

TEST_F(Trigger_test, TriggerCallsHasTriggeredCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "7fa3c68f-6b84-41e4-9ab1-b32f98bd8353");
    Trigger sut = createValidStateBasedTrigger();

    m_triggerClass.m_hasTriggered = true;
    EXPECT_TRUE(sut.isStateConditionSatisfied());
    m_triggerClass.m_hasTriggered = false;
    EXPECT_FALSE(sut.isStateConditionSatisfied());
}

TEST_F(Trigger_test, HasTriggeredCallbackReturnsAlwaysFalseWhenInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "3895a8fb-6ffe-4a0f-9490-3058aac28eac");
    Trigger sut = createValidStateBasedTrigger();
    m_triggerClass.m_hasTriggered = true;
    sut.reset();

    EXPECT_FALSE(sut.isStateConditionSatisfied());
}

TEST_F(Trigger_test, TriggerIsLogicalEqualToItself)
{
    ::testing::Test::RecordProperty("TEST_ID", "90e2a167-ac32-4eab-818d-57e0ffadfe42");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 894U;
    constexpr uint64_t uniqueTriggerId = 0U;
    constexpr uint64_t originType = 4123U;
    constexpr uint64_t originTypeHash = 1423123U;
    Trigger sut1(StateBasedTrigger,
                 &m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 createNotificationCallback(TriggerClass::callback),
                 uniqueTriggerId,
                 originType,
                 originTypeHash);

    EXPECT_TRUE(sut1.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualIfOriginTypeDiffers)
{
    ::testing::Test::RecordProperty("TEST_ID", "de5132b2-d750-4ef7-aa76-9826c91e22da");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    constexpr uint64_t uniqueTriggerId1 = 0U;
    constexpr uint64_t originType = 84123U;
    constexpr uint64_t differentOriginType = 23U;
    constexpr uint64_t originTypeHash = 11423123U;
    Trigger sut1(StateBasedTrigger,
                 &m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 createNotificationCallback(TriggerClass::callback),
                 uniqueTriggerId1,
                 originType,
                 originTypeHash);

    EXPECT_FALSE(sut1.isLogicalEqualTo(&m_triggerClass, differentOriginType, originTypeHash));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualIfOriginAndOriginTypeHashDiffers)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a14f657-1053-47bf-9ffb-bbceef1c6403");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    constexpr uint64_t uniqueTriggerId1 = 0U;
    constexpr uint64_t originType = 84U;
    constexpr uint64_t differentOriginTypeHash = 823U;
    constexpr uint64_t originTypeHash = 13U;
    TriggerClass secondTriggerClass;
    Trigger sut1(StateBasedTrigger,
                 &m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 createNotificationCallback(TriggerClass::callback),
                 uniqueTriggerId1,
                 originType,
                 originTypeHash);

    EXPECT_FALSE(sut1.isLogicalEqualTo(&secondTriggerClass, originType, differentOriginTypeHash));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualIfOriginTypeAndOriginTypeHashDiffers)
{
    ::testing::Test::RecordProperty("TEST_ID", "ddfc4457-a2da-494e-b27b-4f590614fda4");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    constexpr uint64_t uniqueTriggerId1 = 0U;
    constexpr uint64_t originType = 584U;
    constexpr uint64_t differentOriginType = 65823U;
    constexpr uint64_t differentOriginTypeHash = 5823U;
    constexpr uint64_t originTypeHash = 513U;

    TriggerClass secondTriggerClass;
    Trigger sut1(StateBasedTrigger,
                 &m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 createNotificationCallback(TriggerClass::callback),
                 uniqueTriggerId1,
                 originType,
                 originTypeHash);

    EXPECT_FALSE(sut1.isLogicalEqualTo(&secondTriggerClass, differentOriginType, differentOriginTypeHash));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualWhenInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "f398a2de-4191-48cd-a991-32e65705653d");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    constexpr uint64_t uniqueTriggerId1 = 0U;
    constexpr uint64_t originType = 584U;
    constexpr uint64_t originTypeHash = 513U;

    Trigger sut1(StateBasedTrigger,
                 &m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 createNotificationCallback(TriggerClass::callback),
                 uniqueTriggerId1,
                 originType,
                 originTypeHash);
    sut1.invalidate();

    EXPECT_FALSE(sut1.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));
}

// event based trigger

TEST_F(Trigger_test, ValidEventBasedTriggerIsValidAndAlwaysTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "89c5a02d-d63e-4c3f-b5e2-dc1cd82c9b4e");
    Trigger sut = createValidEventBasedTrigger();
    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(sut.isStateConditionSatisfied());
    EXPECT_THAT(sut.getUniqueId(), Ne(Trigger::INVALID_TRIGGER_ID));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::EVENT_BASED));
}

TEST_F(Trigger_test, InvalidatedEventBasedTriggerIsNotValidAndNotTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "0326188b-a56d-4ac4-bb48-bdbfa3c89ef4");
    Trigger sut = createValidEventBasedTrigger();
    sut.invalidate();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(sut.isStateConditionSatisfied());
    EXPECT_THAT(sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::INVALID));
}

TEST_F(Trigger_test, ResetEventBasedTriggerIsNotValidAndNotTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "379b7b0d-fc6e-4b54-b9c0-e90f17c4f3b9");
    Trigger sut = createValidEventBasedTrigger();
    sut.reset();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(sut.isStateConditionSatisfied());
    EXPECT_THAT(sut.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::INVALID));
}

TEST_F(Trigger_test, ValidEventBasedTriggerIsLogicalEqualToSameEventOriginAndEmptyHasTriggeredCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "74159b9c-066f-4c5d-a8e3-7a29aae1fa7f");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 4584U;
    constexpr uint64_t originTypeHash = 4513U;
    Trigger sut = createValidEventBasedTrigger(id, originType, originTypeHash);

    EXPECT_TRUE(sut.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, ValidEventBasedTriggerIsNotLogicalEqualToDifferentEventOrigin)
{
    ::testing::Test::RecordProperty("TEST_ID", "69142086-6c77-47e8-9d56-09be5a3eefc8");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 458U;
    constexpr uint64_t originTypeHash = 413U;
    Trigger sut = createValidEventBasedTrigger(id, originType, originTypeHash);
    TriggerClass anotherTriggerClass;

    EXPECT_FALSE(sut.isLogicalEqualTo(&anotherTriggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, InvalidEventBasedTriggerIsLogicalEqualToSameEventOrigin)
{
    ::testing::Test::RecordProperty("TEST_ID", "45225f4a-ae2a-4f67-a101-4a7f006b0904");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 4598U;
    constexpr uint64_t originTypeHash = 4883U;
    Trigger sut = createValidEventBasedTrigger(id, originType, originTypeHash);
    sut.invalidate();

    EXPECT_FALSE(sut.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, InvalidEventBasedTriggerIsNotLogicalEqualToDifferentEventOrigin)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d619868-0c74-483a-b68e-e9f91940c229");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 48U;
    constexpr uint64_t originTypeHash = 83U;
    Trigger sut = createValidEventBasedTrigger(id, originType, originTypeHash);
    sut.invalidate();
    TriggerClass anotherTriggerClass;

    EXPECT_FALSE(sut.isLogicalEqualTo(&anotherTriggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, EventBasedMovedConstructedWithValidTriggerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2de4db14-a80d-4b9a-8483-718d9940b508");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 7424598U;
    constexpr uint64_t originTypeHash = 6424883U;
    Trigger trigger = createValidEventBasedTrigger(id, originType, originTypeHash);
    Trigger sut{std::move(trigger)};

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::EVENT_BASED));
    EXPECT_TRUE(sut.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));

    EXPECT_FALSE(trigger.isValid());
    EXPECT_FALSE(static_cast<bool>(trigger));
    EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(trigger.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, EventBasedMovedAssignedWithValidTriggerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "92484233-74eb-4228-8ea0-e996cb3978b4");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 74598U;
    constexpr uint64_t originTypeHash = 243U;
    constexpr uint64_t anotherOriginType = 11174598U;
    constexpr uint64_t anotherOriginTypeHash = 111243U;

    Trigger sut = createValidStateBasedTrigger(id, originType, originTypeHash);
    Trigger trigger = createValidEventBasedTrigger(id, anotherOriginType, anotherOriginTypeHash);
    sut = std::move(trigger);

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::EVENT_BASED));
    EXPECT_TRUE(sut.isLogicalEqualTo(&m_triggerClass, anotherOriginType, anotherOriginTypeHash));

    EXPECT_FALSE(trigger.isValid());
    EXPECT_FALSE(static_cast<bool>(trigger));
    EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(trigger.isLogicalEqualTo(&m_triggerClass, anotherOriginType, anotherOriginTypeHash));
}

TEST_F(Trigger_test, EventBasedMovedConstructedWithInvalidTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8259fd0-1dde-482a-a485-097a7819e4e5");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 997458U;
    constexpr uint64_t originTypeHash = 99243U;
    Trigger trigger = createValidEventBasedTrigger(id, originType, originTypeHash);
    Trigger trigger1 = std::move(trigger);
    Trigger sut{std::move(trigger)};

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(sut.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));

    EXPECT_FALSE(trigger.isValid());
    EXPECT_FALSE(static_cast<bool>(trigger));
    EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(trigger.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));
}

TEST_F(Trigger_test, EventBasedMovedAssignedWithInvalidTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "526ae232-4078-42c8-9488-6dfacd0af79c");
    constexpr uint64_t id = 0U;
    constexpr uint64_t originType = 740598U;
    constexpr uint64_t originTypeHash = 20043U;
    constexpr uint64_t anotherOriginType = 111074598U;
    constexpr uint64_t anotherOriginTypeHash = 10011243U;
    Trigger sut = createValidStateBasedTrigger(id, originType, originTypeHash);
    Trigger trigger = createValidEventBasedTrigger(id, anotherOriginType, anotherOriginTypeHash);
    Trigger trigger1 = std::move(trigger);
    sut = std::move(trigger);

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_THAT(sut.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(sut.isLogicalEqualTo(&m_triggerClass, originType, originTypeHash));

    EXPECT_FALSE(trigger.isValid());
    EXPECT_FALSE(static_cast<bool>(trigger));
    EXPECT_THAT(trigger.getTriggerType(), Eq(TriggerType::INVALID));
    EXPECT_FALSE(trigger.isLogicalEqualTo(&m_triggerClass, anotherOriginType, anotherOriginTypeHash));
}

} // namespace
