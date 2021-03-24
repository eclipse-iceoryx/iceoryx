// Copyright (c) 2020, 2021 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/popo/trigger.hpp"

#include "test.hpp"
#include <thread>

using namespace iox;
using namespace iox::popo;
using namespace ::testing;

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

    virtual void SetUp()
    {
        internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    Trigger createValidTrigger(const uint64_t eventId = 0U)
    {
        static uint64_t uniqueId = 0U;
        return Trigger(&m_triggerClass,
                       {m_triggerClass, &TriggerClass::hasTriggered},
                       {m_triggerClass, &TriggerClass::resetCall},
                       eventId,
                       TriggerClass::callback,
                       uniqueId++);
    }

    ConditionVariableData m_condVar{"Horscht"};
    TriggerClass m_triggerClass;
};

Trigger_test::TriggerClass* Trigger_test::TriggerClass::m_lastCallbackArgument = nullptr;

TEST_F(Trigger_test, TriggerWithValidOriginIsValid)
{
    Trigger sut = createValidTrigger();

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, MovedConstructedValidTriggerIsValid)
{
    Trigger trigger = createValidTrigger();
    Trigger sut{std::move(trigger)};

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, MovedAssignedValidTriggerIsValid)
{
    Trigger sut = createValidTrigger();
    Trigger trigger = createValidTrigger();
    sut = std::move(trigger);

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, MovedConstructedOriginIsInvalidTriggerAfterMove)
{
    Trigger trigger = createValidTrigger();
    Trigger sut{std::move(trigger)};

    EXPECT_FALSE(trigger.isValid());
    EXPECT_THAT(trigger.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(Trigger_test, MovedAssignedOriginIsInvalidTriggerAfterMove)
{
    Trigger sut = createValidTrigger();
    Trigger trigger = createValidTrigger();
    sut = std::move(trigger);

    EXPECT_FALSE(trigger.isValid());
    EXPECT_THAT(trigger.getUniqueId(), Eq(Trigger::INVALID_TRIGGER_ID));
}

TEST_F(Trigger_test, TriggerWithNullptrOriginIsValid)
{
    const uint64_t eventId = 0U;
    const uint64_t uniqueTriggerId = 0U;
    Trigger sut(static_cast<TriggerClass*>(nullptr),
                {m_triggerClass, &TriggerClass::hasTriggered},
                {m_triggerClass, &TriggerClass::resetCall},
                eventId,
                TriggerClass::callback,
                uniqueTriggerId);

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, TriggerWithInvalidHasTriggeredCallbackCallsErrorHandlerAndIsInvalid)
{
    const uint64_t eventId = 0U;
    const uint64_t uniqueTriggerId = 0U;

    bool hasTerminated = false;
    iox::Error errorType = iox::Error::kNO_ERROR;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            hasTerminated = true;
            errorType = error;
        });

    Trigger sut(&m_triggerClass,
                cxx::ConstMethodCallback<bool>(),
                {m_triggerClass, &TriggerClass::resetCall},
                eventId,
                TriggerClass::callback,
                uniqueTriggerId);

    EXPECT_TRUE(hasTerminated);
    EXPECT_THAT(errorType, Eq(iox::Error::kPOPO__TRIGGER_INVALID_HAS_TRIGGERED_CALLBACK));
    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, TriggerWithEmptyResetCallCallsErrorHandlerAndIsInvalid)
{
    const uint64_t eventId = 0U;
    const uint64_t uniqueTriggerId = 0U;

    bool hasTerminated = false;
    iox::Error errorType = iox::Error::kNO_ERROR;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            hasTerminated = true;
            errorType = error;
        });

    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                cxx::MethodCallback<void, uint64_t>(),
                eventId,
                TriggerClass::callback,
                uniqueTriggerId);

    EXPECT_TRUE(hasTerminated);
    EXPECT_THAT(errorType, Eq(iox::Error::kPOPO__TRIGGER_INVALID_RESET_CALLBACK));
    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, ResetInvalidatesTrigger)
{
    Trigger sut = createValidTrigger();
    sut.reset();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, InvalidateInvalidatesTrigger)
{
    Trigger sut = createValidTrigger();
    sut.invalidate();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, ResetCallsResetcallbackWithCorrectTriggerOrigin)
{
    Trigger sut = createValidTrigger();
    auto uniqueId = sut.getUniqueId();
    sut.reset();

    EXPECT_EQ(m_triggerClass.m_resetCallTriggerArg, uniqueId);
}

TEST_F(Trigger_test, ResetSetsTriggerIdToInvalid)
{
    Trigger sut = createValidTrigger();
    sut.reset();

    EXPECT_EQ(sut.getUniqueId(), Trigger::INVALID_TRIGGER_ID);
}

TEST_F(Trigger_test, TriggerWithEmptyResetInvalidatesTriggerWhenBeingResetted)
{
    const uint64_t eventId = 0U;
    const uint64_t uniqueTriggerId = 0U;

    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {});

    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                cxx::MethodCallback<void, uint64_t>(),
                eventId,
                TriggerClass::callback,
                uniqueTriggerId);

    sut.reset();

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, TriggerCallsHasTriggeredCallback)
{
    Trigger sut = createValidTrigger();

    m_triggerClass.m_hasTriggered = true;
    EXPECT_TRUE(sut.hasTriggered());
    m_triggerClass.m_hasTriggered = false;
    EXPECT_FALSE(sut.hasTriggered());
}

TEST_F(Trigger_test, HasTriggeredCallbackReturnsAlwaysFalseWhenInvalid)
{
    Trigger sut = createValidTrigger();
    m_triggerClass.m_hasTriggered = true;
    sut.reset();

    EXPECT_FALSE(sut.hasTriggered());
}

TEST_F(Trigger_test, UpdateOriginLeadsToDifferentHasTriggeredCallback)
{
    TriggerClass secondTriggerClass;
    Trigger sut = createValidTrigger();

    sut.updateOrigin(&secondTriggerClass);

    secondTriggerClass.m_hasTriggered = false;
    EXPECT_FALSE(sut.hasTriggered());
    secondTriggerClass.m_hasTriggered = true;
    EXPECT_TRUE(sut.hasTriggered());
}

TEST_F(Trigger_test, UpdateOriginDoesNotUpdateHasTriggeredIfItsNotOriginatingFromOrigin)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 891U;
    const uint64_t uniqueTriggerId = 0U;
    TriggerClass secondTriggerClass, thirdTriggerClass;
    Trigger sut(&m_triggerClass,
                {thirdTriggerClass, &TriggerClass::hasTriggered},
                {m_triggerClass, &TriggerClass::resetCall},
                USER_DEFINED_EVENT_ID,
                TriggerClass::callback,
                uniqueTriggerId);

    sut.updateOrigin(&secondTriggerClass);

    thirdTriggerClass.m_hasTriggered = false;
    EXPECT_FALSE(sut.hasTriggered());
    thirdTriggerClass.m_hasTriggered = true;
    EXPECT_TRUE(sut.hasTriggered());
}

TEST_F(Trigger_test, UpdateOriginLeadsToDifferentResetCallback)
{
    Trigger sut = createValidTrigger();
    TriggerClass secondTriggerClass;

    sut.updateOrigin(&secondTriggerClass);
    auto uniqueId = sut.getUniqueId();
    sut.reset();

    EXPECT_EQ(secondTriggerClass.m_resetCallTriggerArg, uniqueId);
}

TEST_F(Trigger_test, UpdateOriginDoesNotUpdateResetIfItsNotOriginatingFromOrigin)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 892U;
    const uint64_t uniqueTriggerId = 0U;
    TriggerClass secondTriggerClass, thirdTriggerClass;
    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                {thirdTriggerClass, &TriggerClass::resetCall},
                USER_DEFINED_EVENT_ID,
                TriggerClass::callback,
                uniqueTriggerId);

    sut.updateOrigin(&secondTriggerClass);
    auto uniqueId = sut.getUniqueId();
    sut.reset();

    EXPECT_EQ(thirdTriggerClass.m_resetCallTriggerArg, uniqueId);
}

TEST_F(Trigger_test, UpdateOriginUpdatesOriginOfEventInfo)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 893U;
    const uint64_t uniqueTriggerId = 0U;
    TriggerClass secondTriggerClass;
    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                {m_triggerClass, &TriggerClass::resetCall},
                USER_DEFINED_EVENT_ID,
                TriggerClass::callback,
                uniqueTriggerId);

    sut.updateOrigin(&secondTriggerClass);
    EXPECT_TRUE(sut.getEventInfo().doesOriginateFrom(&secondTriggerClass));
}

TEST_F(Trigger_test, TriggerIsLogicalEqualToItsOriginAndHasTriggeredCallback)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 894U;
    const uint64_t uniqueTriggerId = 0U;
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 TriggerClass::callback,
                 uniqueTriggerId);

    EXPECT_TRUE(sut1.isLogicalEqualTo(&m_triggerClass, {m_triggerClass, &TriggerClass::hasTriggered}));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualIfHasTriggeredCallbackDiffers)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    const uint64_t uniqueTriggerId1 = 0U;
    TriggerClass secondTriggerClass;
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 TriggerClass::callback,
                 uniqueTriggerId1);

    EXPECT_FALSE(sut1.isLogicalEqualTo(&m_triggerClass, {secondTriggerClass, &TriggerClass::hasTriggered}));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualIfOriginDiffers)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    const uint64_t uniqueTriggerId1 = 0U;
    TriggerClass secondTriggerClass;
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 TriggerClass::callback,
                 uniqueTriggerId1);

    EXPECT_FALSE(sut1.isLogicalEqualTo(&secondTriggerClass, {m_triggerClass, &TriggerClass::hasTriggered}));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualIfOriginAndHasTriggeredCallbackDiffers)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    const uint64_t uniqueTriggerId1 = 0U;
    TriggerClass secondTriggerClass;
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 TriggerClass::callback,
                 uniqueTriggerId1);

    EXPECT_FALSE(sut1.isLogicalEqualTo(&secondTriggerClass, {secondTriggerClass, &TriggerClass::hasTriggered}));
}

TEST_F(Trigger_test, TriggerIsNotLogicalEqualWhenInvalid)
{
    constexpr uint64_t USER_DEFINED_EVENT_ID = 4896U;
    const uint64_t uniqueTriggerId1 = 0U;
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 USER_DEFINED_EVENT_ID,
                 TriggerClass::callback,
                 uniqueTriggerId1);
    sut1.invalidate();

    EXPECT_FALSE(sut1.isLogicalEqualTo(&m_triggerClass, {m_triggerClass, &TriggerClass::hasTriggered}));
}
