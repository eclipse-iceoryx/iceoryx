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

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_waiter.hpp"
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
        return Trigger(&m_triggerClass,
                       {m_triggerClass, &TriggerClass::hasTriggered},
                       {m_triggerClass, &TriggerClass::resetCall},
                       eventId,
                       TriggerClass::callback);
    }

    ConditionVariableData m_condVar{"Horscht"};
    TriggerClass m_triggerClass;
};

Trigger_test::TriggerClass* Trigger_test::TriggerClass::m_lastCallbackArgument = nullptr;

TEST_F(Trigger_test, DefaultCTorConstructsEmptyTrigger)
{
    Trigger sut;

    EXPECT_EQ(static_cast<bool>(sut), false);
    EXPECT_EQ(sut.isValid(), false);
    EXPECT_EQ(sut.hasTriggered(), false);
}

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
    Trigger sut;
    Trigger trigger = createValidTrigger();
    sut = std::move(trigger);

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, TriggerWithNullptrOriginIsValid)
{
    uint64_t eventId = 0U;
    Trigger sut(static_cast<TriggerClass*>(nullptr),
                {m_triggerClass, &TriggerClass::hasTriggered},
                {m_triggerClass, &TriggerClass::resetCall},
                eventId,
                TriggerClass::callback);

    EXPECT_TRUE(sut.isValid());
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, TriggerWithInvalidHasTriggeredCallbackIsInvalid)
{
    uint64_t eventId = 0U;
    Trigger sut(&m_triggerClass,
                cxx::ConstMethodCallback<bool>(),
                {m_triggerClass, &TriggerClass::resetCall},
                eventId,
                TriggerClass::callback);

    EXPECT_FALSE(sut.isValid());
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(Trigger_test, TriggerWithEmptyResetCallIsValid)
{
    uint64_t eventId = 0U;
    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                cxx::MethodCallback<void, uint64_t>(),
                eventId,
                TriggerClass::callback);

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
    sut.reset();

    EXPECT_EQ(m_triggerClass.m_resetCallTriggerArg, sut.getUniqueId());
}

TEST_F(Trigger_test, TriggerWithEmptyResetInvalidatesTriggerWhenBeingResetted)
{
    uint64_t eventId = 0U;
    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                cxx::MethodCallback<void, uint64_t>(),
                eventId,
                TriggerClass::callback);

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
    TriggerClass secondTriggerClass, thirdTriggerClass;
    Trigger sut(&m_triggerClass,
                {thirdTriggerClass, &TriggerClass::hasTriggered},
                {m_triggerClass, &TriggerClass::resetCall},
                891U,
                TriggerClass::callback);

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
    sut.reset();

    EXPECT_EQ(secondTriggerClass.m_resetCallTriggerArg, sut.getUniqueId());
}

TEST_F(Trigger_test, UpdateOriginDoesNotUpdateResetIfItsNotOriginatingFromOrigin)
{
    TriggerClass secondTriggerClass, thirdTriggerClass;
    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                {thirdTriggerClass, &TriggerClass::resetCall},
                891U,
                TriggerClass::callback);

    sut.updateOrigin(&secondTriggerClass);
    sut.reset();

    EXPECT_EQ(thirdTriggerClass.m_resetCallTriggerArg, sut.getUniqueId());
}

TEST_F(Trigger_test, UpdateOriginUpdatesOriginOfEventInfo)
{
    TriggerClass secondTriggerClass;
    Trigger sut(&m_triggerClass,
                {m_triggerClass, &TriggerClass::hasTriggered},
                {m_triggerClass, &TriggerClass::resetCall},
                891U,
                TriggerClass::callback);

    sut.updateOrigin(&secondTriggerClass);
    EXPECT_TRUE(sut.getEventInfo().doesOriginateFrom(&secondTriggerClass));
}

/// Two triggers are equal when they have the same:
///   - origin
///   - hasTriggeredCallback
TEST_F(Trigger_test, TriggerIsLogicalEqualToItself)
{
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 8911U,
                 TriggerClass::callback);

    EXPECT_TRUE(sut1.isLogicalEqualTo(sut1));
}

TEST_F(Trigger_test, TwoTriggersAreLogicalEqualIfRequirementsAreFullfilled)
{
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 891U,
                 TriggerClass::callback);

    Trigger sut2(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 891U,
                 TriggerClass::callback);


    EXPECT_TRUE(sut1.isLogicalEqualTo(sut2));
    EXPECT_TRUE(sut2.isLogicalEqualTo(sut1));
}

TEST_F(Trigger_test, TwoTriggersAreNotLogicalEqualIfTriggerIdDiffers)
{
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 2891U,
                 TriggerClass::callback);

    Trigger sut2(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 3891U,
                 TriggerClass::callback);


    EXPECT_FALSE(sut1.isLogicalEqualTo(sut2));
    EXPECT_FALSE(sut2.isLogicalEqualTo(sut1));
}

TEST_F(Trigger_test, TwoTriggersAreNotLogicalEqualIfHasTriggeredCallbackDiffers)
{
    TriggerClass secondTriggerClass;
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 4891U,
                 TriggerClass::callback);

    Trigger sut2(&m_triggerClass,
                 {secondTriggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 4891U,
                 TriggerClass::callback);


    EXPECT_FALSE(sut1.isLogicalEqualTo(sut2));
    EXPECT_FALSE(sut2.isLogicalEqualTo(sut1));
}

TEST_F(Trigger_test, TwoTriggersAreNotLogicalEqualIfOriginDiffers)
{
    TriggerClass secondTriggerClass;
    Trigger sut1(&m_triggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 4891U,
                 TriggerClass::callback);

    Trigger sut2(&secondTriggerClass,
                 {m_triggerClass, &TriggerClass::hasTriggered},
                 {m_triggerClass, &TriggerClass::resetCall},
                 4891U,
                 TriggerClass::callback);


    EXPECT_FALSE(sut1.isLogicalEqualTo(sut2));
    EXPECT_FALSE(sut2.isLogicalEqualTo(sut1));
}
