// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iox/atomic.hpp"
#include "iox/optional.hpp"
#include "iox/vector.hpp"
#include "test.hpp"

#include <chrono>
#include <memory>
#include <thread>

namespace
{
using namespace ::testing;

using namespace iox::popo;
using namespace iox::popo::detail;
using namespace iox;
using namespace iox::units::duration_literals;

class WaitSetHelper_test : public Test
{
  public:
    using value_t = ConditionListener::NotificationVector_t::value_type;
};

TEST_F(WaitSetHelper_test, MergeTwoDisjunctNonEmptySortedNotificationVectors)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f39641f-de8a-434a-8a50-cd2b66b476da");
    constexpr value_t OFFSET = 37U;
    constexpr value_t VECTOR_SIZE = 10U;
    ConditionListener::NotificationVector_t first;
    ConditionListener::NotificationVector_t second;

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        first.emplace_back(static_cast<value_t>(i + OFFSET));
    }

    for (value_t i = VECTOR_SIZE / 2U; i < VECTOR_SIZE; ++i)
    {
        second.emplace_back(static_cast<value_t>(i + OFFSET));
    }

    auto mergedNotificationVector = uniqueMergeSortedNotificationVector(first, second);
    auto mergedNotificationVectorSwitched = uniqueMergeSortedNotificationVector(second, first);

    ASSERT_THAT(mergedNotificationVector.size(), Eq(VECTOR_SIZE));
    for (value_t i = 0U; i < VECTOR_SIZE; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i + OFFSET));
    }
    EXPECT_TRUE(mergedNotificationVector == mergedNotificationVectorSwitched);
}

TEST_F(WaitSetHelper_test, MergeTwoDisjunctNonEmptySortedNotificationVectorsWithAGap)
{
    ::testing::Test::RecordProperty("TEST_ID", "15d3c063-8bc5-47eb-84a4-35f055a1d82c");
    constexpr value_t OFFSET = 41U;
    constexpr value_t GAP = 13U;
    constexpr value_t VECTOR_SIZE = 10U;
    ConditionListener::NotificationVector_t first;
    ConditionListener::NotificationVector_t second;

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        first.emplace_back(static_cast<value_t>(i + OFFSET));
    }

    for (value_t i = VECTOR_SIZE / 2U; i < VECTOR_SIZE; ++i)
    {
        second.emplace_back(static_cast<value_t>(i + OFFSET + GAP));
    }

    auto mergedNotificationVector = uniqueMergeSortedNotificationVector(first, second);
    auto mergedNotificationVectorSwitched = uniqueMergeSortedNotificationVector(second, first);

    ASSERT_THAT(mergedNotificationVector.size(), Eq(VECTOR_SIZE));
    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i + OFFSET));
    }
    for (value_t i = VECTOR_SIZE / 2U; i < VECTOR_SIZE; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i + OFFSET + GAP));
    }
    EXPECT_TRUE(mergedNotificationVector == mergedNotificationVectorSwitched);
}

TEST_F(WaitSetHelper_test, MergeTwoAlternatingDisjunctNonEmptySortedNotificationVectors)
{
    ::testing::Test::RecordProperty("TEST_ID", "02cc9514-6cfe-4e08-8806-f371561fef41");
    constexpr value_t OFFSET = 73U;
    constexpr value_t VECTOR_SIZE = 10U;
    ConditionListener::NotificationVector_t first;
    ConditionListener::NotificationVector_t second;

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        first.emplace_back(static_cast<value_t>(i * 2 + OFFSET));
    }

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        second.emplace_back(static_cast<value_t>(i * 2 + 1 + OFFSET));
    }

    auto mergedNotificationVector = uniqueMergeSortedNotificationVector(first, second);
    auto mergedNotificationVectorSwitched = uniqueMergeSortedNotificationVector(second, first);

    ASSERT_THAT(mergedNotificationVector.size(), Eq(VECTOR_SIZE));
    for (value_t i = 0; i < VECTOR_SIZE; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i + OFFSET));
    }
    EXPECT_TRUE(mergedNotificationVector == mergedNotificationVectorSwitched);
}

TEST_F(WaitSetHelper_test, MergingIdenticalNotificationVectorResultsInUnchangedNotificationVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "50f05cf2-62fa-49b8-8380-1dd0ac2470ec");
    constexpr value_t OFFSET = 111U;
    constexpr value_t VECTOR_SIZE = 10U;
    ConditionListener::NotificationVector_t someNotificationVector;

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        someNotificationVector.emplace_back(static_cast<value_t>(i * 2 + OFFSET));
    }

    auto mergedNotificationVector = uniqueMergeSortedNotificationVector(someNotificationVector, someNotificationVector);

    ASSERT_THAT(mergedNotificationVector.size(), Eq(VECTOR_SIZE / 2U));
    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i * 2 + OFFSET));
    }
}

TEST_F(WaitSetHelper_test, MergingWithOneEmptyNotificationVectorResultsInUnchangedNotificationVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0a0eb3a-08a3-4898-a8c9-a4f7eff0115c");
    constexpr value_t OFFSET = 123U;
    constexpr value_t VECTOR_SIZE = 10U;
    ConditionListener::NotificationVector_t someNotificationVector;

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        someNotificationVector.emplace_back(static_cast<value_t>(i * 3 + OFFSET));
    }

    auto mergedNotificationVector =
        uniqueMergeSortedNotificationVector(someNotificationVector, ConditionListener::NotificationVector_t());

    ASSERT_THAT(mergedNotificationVector.size(), Eq(5U));
    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i * 3 + OFFSET));
    }
}

TEST_F(WaitSetHelper_test, MergePartiallyOverlappingSortedNotificationVectors)
{
    ::testing::Test::RecordProperty("TEST_ID", "c57dda77-81a5-413f-b54b-e924e67b66a5");
    constexpr value_t VECTOR_SIZE = 10U;
    constexpr value_t MAX_OVERLAPPING_INDEX = 8U;
    constexpr value_t OFFSET = 155U;
    ConditionListener::NotificationVector_t first;
    ConditionListener::NotificationVector_t second;

    for (value_t i = 3U; i < VECTOR_SIZE; ++i)
    {
        first.emplace_back(static_cast<value_t>(i + OFFSET));
    }

    for (value_t i = 0U; i < MAX_OVERLAPPING_INDEX; ++i)
    {
        second.emplace_back(static_cast<value_t>(i + OFFSET));
    }

    auto mergedNotificationVector = uniqueMergeSortedNotificationVector(first, second);
    auto mergedNotificationVectorSwitched = uniqueMergeSortedNotificationVector(second, first);

    ASSERT_THAT(mergedNotificationVector.size(), Eq(VECTOR_SIZE));
    for (value_t i = 0U; i < VECTOR_SIZE; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i + OFFSET));
    }
    EXPECT_TRUE(mergedNotificationVector == mergedNotificationVectorSwitched);
}

TEST_F(WaitSetHelper_test, MergeWithDisjunctOneElementNotificationVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a56b0f9-82d2-4f9a-881f-338dd572a453");
    constexpr value_t OFFSET = 160U;
    constexpr value_t VECTOR_SIZE = 10U;
    ConditionListener::NotificationVector_t first;
    ConditionListener::NotificationVector_t second;

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        first.emplace_back(static_cast<value_t>(i + OFFSET));
    }

    second.emplace_back(static_cast<value_t>(VECTOR_SIZE / 2U + OFFSET));

    auto mergedNotificationVector = uniqueMergeSortedNotificationVector(first, second);
    auto mergedNotificationVectorSwitched = uniqueMergeSortedNotificationVector(second, first);

    ASSERT_THAT(mergedNotificationVector.size(), Eq(VECTOR_SIZE / 2U + 1));
    for (value_t i = 0U; i < VECTOR_SIZE / 2U + 1; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i + OFFSET));
    }
    EXPECT_TRUE(mergedNotificationVector == mergedNotificationVectorSwitched);
}

TEST_F(WaitSetHelper_test, MergeWithOverlappingOneElementNotificationVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "05fb7baf-51e9-4ff9-bb35-8ae4174b0216");
    constexpr value_t OFFSET = 200U;
    constexpr value_t VECTOR_SIZE = 10U;
    ConditionListener::NotificationVector_t first;
    ConditionListener::NotificationVector_t second;

    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        first.emplace_back(static_cast<value_t>(i + OFFSET));
    }

    second.emplace_back(static_cast<value_t>(0 + OFFSET));

    auto mergedNotificationVector = uniqueMergeSortedNotificationVector(first, second);
    auto mergedNotificationVectorSwitched = uniqueMergeSortedNotificationVector(second, first);

    ASSERT_THAT(mergedNotificationVector.size(), Eq(VECTOR_SIZE / 2U));
    for (value_t i = 0U; i < VECTOR_SIZE / 2U; ++i)
    {
        EXPECT_THAT(mergedNotificationVector[i], Eq(i + OFFSET));
    }
    EXPECT_TRUE(mergedNotificationVector == mergedNotificationVectorSwitched);
}


class WaitSetTest : public iox::popo::WaitSet<>
{
  public:
    WaitSetTest(iox::popo::ConditionVariableData& condVarData) noexcept
        : WaitSet(condVarData)
    {
    }
};

enum class SimpleEvent1 : iox::popo::EventEnumIdentifier
{
    EVENT1 = 0,
    EVENT2 = 1,
    INVALID = 2
};

enum class SimpleEvent2 : iox::popo::EventEnumIdentifier
{
    EVENT1 = 0,
    EVENT2 = 1,
    INVALID = 2
};

enum class SimpleState1 : iox::popo::StateEnumIdentifier
{
    STATE1 = 0,
    STATE2 = 1,
    INVALID = 2
};

enum class SimpleState2 : iox::popo::StateEnumIdentifier
{
    STATE1 = 0,
    STATE2 = 1,
    INVALID = 2
};

class WaitSet_test : public Test
{
  public:
    class SimpleEventClass
    {
      public:
        SimpleEventClass() = default;
        SimpleEventClass(const SimpleEventClass&) = delete;
        SimpleEventClass(SimpleEventClass&&) = delete;

        SimpleEventClass& operator=(const SimpleEventClass&) = delete;
        SimpleEventClass& operator=(SimpleEventClass&&) = delete;
        ~SimpleEventClass() = default;

        bool hasEventSet() const noexcept
        {
            return static_cast<bool>(m_eventHandle);
        }

        bool hasStateSet() const noexcept
        {
            return static_cast<bool>(m_stateHandle);
        }

        void enableEvent(iox::popo::TriggerHandle&& handle) noexcept
        {
            m_eventHandle = std::move(handle);
        }

        void enableEvent(iox::popo::TriggerHandle&& handle, const SimpleEvent1 event) noexcept
        {
            m_eventHandle = std::move(handle);
            m_simpleEvent1 = event;
        }

        void enableEvent(iox::popo::TriggerHandle&& handle, const SimpleEvent2 event) noexcept
        {
            m_eventHandle = std::move(handle);
            m_simpleEvent2 = event;
        }

        void enableState(iox::popo::TriggerHandle&& handle) noexcept
        {
            m_stateHandle = std::move(handle);
        }

        void enableState(iox::popo::TriggerHandle&& handle, const SimpleState1 state) noexcept
        {
            m_stateHandle = std::move(handle);
            m_simpleState1 = state;
        }

        void enableState(iox::popo::TriggerHandle&& handle, const SimpleState2 state) noexcept
        {
            m_stateHandle = std::move(handle);
            m_simpleState2 = state;
        }

        void invalidateTrigger(const uint64_t id)
        {
            m_invalidateTriggerId.emplace_back(id);
            m_stateHandle.invalidate();
            m_eventHandle.invalidate();
        }

        iox::popo::WaitSetIsConditionSatisfiedCallback getCallbackForIsStateConditionSatisfied() const noexcept
        {
            return (m_isEventBased) ? iox::nullopt
                                    : iox::popo::WaitSetIsConditionSatisfiedCallback(
                                        iox::in_place, *this, &SimpleEventClass::hasTriggered);
        }

        iox::popo::WaitSetIsConditionSatisfiedCallback
        getCallbackForIsStateConditionSatisfied(SimpleState1 state) const noexcept
        {
            m_simpleState1TriggerCallback = state;
            return (m_isEventBased) ? iox::nullopt
                                    : iox::popo::WaitSetIsConditionSatisfiedCallback(
                                        iox::in_place, *this, &SimpleEventClass::hasTriggered);
        }

        iox::popo::WaitSetIsConditionSatisfiedCallback
        getCallbackForIsStateConditionSatisfied(SimpleState2 state) const noexcept
        {
            m_simpleState2TriggerCallback = state;
            return (m_isEventBased) ? iox::nullopt
                                    : iox::popo::WaitSetIsConditionSatisfiedCallback(
                                        iox::in_place, *this, &SimpleEventClass::hasTriggered);
        }

        bool hasTriggered() const
        {
            if (m_autoResetTrigger)
            {
                return m_hasTriggered.exchange(false);
            }
            return m_hasTriggered.load();
        }

        void disableEvent()
        {
            m_eventHandle.reset();
        }

        void disableState()
        {
            m_stateHandle.reset();
        }

        uint64_t getUniqueStateId() const
        {
            return m_stateHandle.getUniqueId();
        }

        uint64_t getUniqueNotificationId() const
        {
            return m_eventHandle.getUniqueId();
        }

        void trigger()
        {
            m_hasTriggered.store(true);
            m_stateHandle.trigger();
            m_eventHandle.trigger();
        }

        void resetTrigger()
        {
            m_hasTriggered.store(false);
        }

        iox::popo::TriggerHandle m_eventHandle;
        iox::popo::TriggerHandle m_stateHandle;
        mutable iox::concurrent::Atomic<bool> m_hasTriggered{false};
        static std::vector<uint64_t> m_invalidateTriggerId;

        static SimpleEvent1 m_simpleEvent1;
        static SimpleEvent2 m_simpleEvent2;
        static SimpleState1 m_simpleState1;
        static SimpleState2 m_simpleState2;
        static SimpleState1 m_simpleState1TriggerCallback;
        static SimpleState2 m_simpleState2TriggerCallback;

        SimpleEventClass* m_triggerCallbackArgument1 = nullptr;
        SimpleEventClass* m_triggerCallbackArgument2 = nullptr;
        uint64_t* m_contextData1 = nullptr;
        uint64_t* m_contextData2 = nullptr;
        bool m_autoResetTrigger = true;
        bool m_isEventBased = false;
    };

    ConditionVariableData m_condVarData{"Horscht"};
    optional<WaitSetTest> m_sut;

    static void triggerCallback1(WaitSet_test::SimpleEventClass* const waitset)
    {
        waitset->m_triggerCallbackArgument1 = waitset;
    }

    static void triggerCallback2(WaitSet_test::SimpleEventClass* const waitset)
    {
        waitset->m_triggerCallbackArgument2 = waitset;
    }

    static void triggerCallback1WithContextData(WaitSet_test::SimpleEventClass* const waitset,
                                                uint64_t* const contextData)
    {
        waitset->m_triggerCallbackArgument1 = waitset;
        waitset->m_contextData1 = contextData;
    }

    static void triggerCallback2WithContextData(WaitSet_test::SimpleEventClass* const waitset,
                                                uint64_t* const contextData)
    {
        waitset->m_triggerCallbackArgument2 = waitset;
        waitset->m_contextData2 = contextData;
    }

    void SetUp() override
    {
        SimpleEventClass::m_simpleEvent1 = SimpleEvent1::INVALID;
        SimpleEventClass::m_simpleEvent2 = SimpleEvent2::INVALID;
        SimpleEventClass::m_simpleState1 = SimpleState1::INVALID;
        SimpleEventClass::m_simpleState2 = SimpleState2::INVALID;
        SimpleEventClass::m_simpleState1TriggerCallback = SimpleState1::INVALID;
        SimpleEventClass::m_simpleState2TriggerCallback = SimpleState2::INVALID;
        m_sut.emplace(m_condVarData);
        WaitSet_test::SimpleEventClass::m_invalidateTriggerId.clear();
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown() override
    {
    }

    template <uint64_t NotificationInfoVectorCapacity, typename EventOrigin>
    static bool doesNotificationInfoVectorContain(
        const iox::vector<const NotificationInfo*, NotificationInfoVectorCapacity>& eventInfoVector,
        const uint64_t eventId,
        const EventOrigin& origin)
    {
        for (auto& e : eventInfoVector)
        {
            if (e->getNotificationId() == eventId && e->doesOriginateFrom(&origin)
                && e->template getOrigin<EventOrigin>() == &origin)
            {
                return true;
            }
        }
        return false;
    }

    bool attachAllEvents()
    {
        for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            EXPECT_TRUE(m_sut->attachEvent(m_simpleEvents[i], i));
            EXPECT_TRUE(m_simpleEvents[i].hasEventSet());
            EXPECT_FALSE(m_simpleEvents[i].hasStateSet());
            EXPECT_THAT(m_sut->size(), Eq(i + 1U));
            EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
        }

        return m_sut->size() == m_sut->capacity();
    }

    bool attachAllStates()
    {
        for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            EXPECT_TRUE(m_sut->attachState(m_simpleEvents[i], i));
            EXPECT_FALSE(m_simpleEvents[i].hasEventSet());
            EXPECT_TRUE(m_simpleEvents[i].hasStateSet());
            EXPECT_THAT(m_sut->size(), Eq(i + 1U));
            EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
        }

        return m_sut->size() == m_sut->capacity();
    }

    bool attachAllWithEventStateMix()
    {
        for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            if (i % 2 == 0U)
            {
                EXPECT_TRUE(m_sut->attachState(m_simpleEvents[i], i));
                EXPECT_FALSE(m_simpleEvents[i].hasEventSet());
                EXPECT_TRUE(m_simpleEvents[i].hasStateSet());
            }
            else
            {
                EXPECT_TRUE(m_sut->attachEvent(m_simpleEvents[i], i));
                EXPECT_TRUE(m_simpleEvents[i].hasEventSet());
                EXPECT_FALSE(m_simpleEvents[i].hasStateSet());
            }
            EXPECT_THAT(m_sut->size(), Eq(i + 1U));
            EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
        }

        return m_sut->size() == m_sut->capacity();
    }

    bool detachAllEvents()
    {
        for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            m_sut->detachEvent(m_simpleEvents[i]);
            EXPECT_FALSE(m_simpleEvents[i].hasEventSet());
            EXPECT_FALSE(m_simpleEvents[i].hasStateSet());
            EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - i - 1U));
            EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
        }

        return m_sut->size() == 0U;
    }

    bool detachAllStates()
    {
        for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            m_sut->detachState(m_simpleEvents[i]);
            EXPECT_FALSE(m_simpleEvents[i].hasEventSet());
            EXPECT_FALSE(m_simpleEvents[i].hasStateSet());
            EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - i - 1U));
            EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
        }

        return m_sut->size() == 0U;
    }

    bool detachAllWithEventStateMix()
    {
        for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            if (i % 2 == 0U)
            {
                m_sut->detachState(m_simpleEvents[i]);
            }
            else
            {
                m_sut->detachEvent(m_simpleEvents[i]);
            }
            EXPECT_FALSE(m_simpleEvents[i].hasEventSet());
            EXPECT_FALSE(m_simpleEvents[i].hasStateSet());
            EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - i - 1U));
            EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
        }

        return m_sut->size() == 0U;
    }


    const iox::units::Duration m_timeToWait = 2_s;
    Watchdog m_watchdog{m_timeToWait};
    using eventVector_t = iox::vector<SimpleEventClass, iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1>;
    eventVector_t m_simpleEvents{iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1};
};
std::vector<uint64_t> WaitSet_test::SimpleEventClass::m_invalidateTriggerId;
SimpleEvent1 WaitSet_test::SimpleEventClass::m_simpleEvent1 = SimpleEvent1::INVALID;
SimpleEvent2 WaitSet_test::SimpleEventClass::m_simpleEvent2 = SimpleEvent2::INVALID;
SimpleState1 WaitSet_test::SimpleEventClass::m_simpleState1 = SimpleState1::INVALID;
SimpleState2 WaitSet_test::SimpleEventClass::m_simpleState2 = SimpleState2::INVALID;
SimpleState1 WaitSet_test::SimpleEventClass::m_simpleState1TriggerCallback = SimpleState1::INVALID;
SimpleState2 WaitSet_test::SimpleEventClass::m_simpleState2TriggerCallback = SimpleState2::INVALID;

////////////////////////
// BEGIN attach / detach
////////////////////////

TEST_F(WaitSet_test, AttachEventOnceIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "67eb525b-e991-42e0-8dc6-b19640ed095b");
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0]).has_error());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
}

TEST_F(WaitSet_test, AttachMaxEventsIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "13f533a2-fe92-4180-8729-50ff3ab7c53a");
    EXPECT_TRUE(attachAllEvents());
}

TEST_F(WaitSet_test, AttachMoreThanMaxEventsFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6047fc25-6e1c-42d7-89a4-21510f579855");
    EXPECT_TRUE(attachAllEvents());

    EXPECT_TRUE(m_sut->attachEvent(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET]).has_error());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasEventSet());
    EXPECT_THAT(m_sut->size(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
    EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
}

TEST_F(WaitSet_test, AttachStateOnceIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "62b8b76c-a5c0-41ea-ab6b-156b2540917a");
    EXPECT_FALSE(m_sut->attachState(m_simpleEvents[0]).has_error());
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
}

TEST_F(WaitSet_test, AttachMaxStatesIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb0bc6ec-adc3-46a3-b511-cd98cc66c656");
    EXPECT_TRUE(attachAllStates());
}

TEST_F(WaitSet_test, AttachMoreThanMaxStatesFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "f877bbdc-6219-4a3b-9395-7decd2c30715");
    EXPECT_TRUE(attachAllStates());

    EXPECT_TRUE(m_sut->attachState(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET]).has_error());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasEventSet());
    EXPECT_THAT(m_sut->size(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
    EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
}

TEST_F(WaitSet_test, AttachMoreThanMaxFailsWithMixedEventsStates)
{
    ::testing::Test::RecordProperty("TEST_ID", "7616ed90-080c-422a-8a52-798e92f3d097");
    EXPECT_TRUE(attachAllWithEventStateMix());

    EXPECT_TRUE(m_sut->attachEvent(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET]).has_error());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasEventSet());
    EXPECT_THAT(m_sut->size(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
    EXPECT_THAT(m_sut->capacity(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));
}

TEST_F(WaitSet_test, AttachingSameEventTwiceResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "cbbedc2a-68dc-4a77-be61-34ff70d7a3dc");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 0U;
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID).has_error());
    auto result2 = m_sut->attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.error(), Eq(WaitSetError::ALREADY_ATTACHED));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameStateTwiceResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "abe4f5f5-9360-4743-b88a-e741f2706aef");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 0U;
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0], USER_DEFINED_EVENT_ID).has_error());
    auto result2 = m_sut->attachState(m_simpleEvents[0], USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.error(), Eq(WaitSetError::ALREADY_ATTACHED));
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameEventWithNonNullIdTwiceResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "14074c98-6cd1-4152-842f-d769402ea7b4");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 121U;
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID).has_error());
    auto result2 = m_sut->attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.error(), Eq(WaitSetError::ALREADY_ATTACHED));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameStateWithNonNullIdTwiceResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "12f82b42-2357-4f9e-93f5-7433418629e8");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 121U;
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0], USER_DEFINED_EVENT_ID).has_error());
    auto result2 = m_sut->attachState(m_simpleEvents[0], USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.error(), Eq(WaitSetError::ALREADY_ATTACHED));
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameEventWithDifferentIdResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "7dc5a359-025a-48d1-9040-e0ba408e3ed7");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 2101U;
    constexpr uint64_t ANOTHER_USER_DEFINED_EVENT_ID = 9121U;
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0], USER_DEFINED_EVENT_ID).has_error());
    auto result2 = m_sut->attachEvent(m_simpleEvents[0], ANOTHER_USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.error(), Eq(WaitSetError::ALREADY_ATTACHED));
}

TEST_F(WaitSet_test, AttachingSameStateWithDifferentIdResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a5670bd-5160-47be-a9f5-a9029d7566af");
    constexpr uint64_t USER_DEFINED_EVENT_ID = 2101U;
    constexpr uint64_t ANOTHER_USER_DEFINED_EVENT_ID = 9121U;
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0], USER_DEFINED_EVENT_ID).has_error());
    auto result2 = m_sut->attachState(m_simpleEvents[0], ANOTHER_USER_DEFINED_EVENT_ID);

    ASSERT_TRUE(result2.has_error());
    EXPECT_THAT(result2.error(), Eq(WaitSetError::ALREADY_ATTACHED));
}

TEST_F(WaitSet_test, DetachingAttachedEventIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4358d62-2670-4817-bfee-65c45da6405b");
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0]).has_error());
    m_sut->detachEvent(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(0U));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, DetachingAttachedStateIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "fed30b7f-d6b7-46a3-bec6-ccb414318f2e");
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0]).has_error());
    m_sut->detachState(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(0U));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, DetachingAttachedEventTwiceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd76f97e-d221-421a-9884-5f546a39421f");
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0]).has_error());
    m_sut->detachEvent(m_simpleEvents[0]);
    m_sut->detachEvent(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(0U));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, DetachingAttachedStateTwiceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a7ba7bb-6b45-41e5-8032-56608be23d69");
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0]).has_error());
    m_sut->detachState(m_simpleEvents[0]);
    m_sut->detachState(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(0U));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, DetachingMakesSpaceForAnotherEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "98b70d41-2845-46a8-8fc3-b7d05b76765c");
    ASSERT_TRUE(attachAllEvents());

    m_sut->detachEvent(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1U));

    EXPECT_TRUE(m_sut->attachEvent(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET]));
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity()));
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasEventSet());
}

TEST_F(WaitSet_test, DetachingMakesSpaceForAnotherState)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb1abaf9-3be3-4050-9b73-e7b08ed25e10");
    ASSERT_TRUE(attachAllStates());

    m_sut->detachState(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1U));

    EXPECT_TRUE(m_sut->attachState(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET]));
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity()));
    EXPECT_TRUE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasEventSet());
}

TEST_F(WaitSet_test, DetachingMakesSpaceForAnotherAttachmentWithMixedEventsStates)
{
    ::testing::Test::RecordProperty("TEST_ID", "87db3ed1-b949-412a-80f0-4455f17602b8");
    ASSERT_TRUE(attachAllWithEventStateMix());

    m_sut->detachState(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1U));

    EXPECT_TRUE(m_sut->attachState(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET]));
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity()));
    EXPECT_TRUE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET].hasEventSet());
}

TEST_F(WaitSet_test, DetachingAllEventAttachmentsOfFullWaitSetIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab3a1cd7-fd54-48f1-ae0b-a2e596c43d77");
    EXPECT_TRUE(attachAllEvents());
    EXPECT_TRUE(detachAllEvents());
}

TEST_F(WaitSet_test, DetachingAllStateAttachmentsOfFullWaitSetIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "36a006dd-9f7f-442d-88ea-37a79f4193fd");
    EXPECT_TRUE(attachAllStates());
    EXPECT_TRUE(detachAllStates());
}

TEST_F(WaitSet_test, DetachingAllMixedAttachmentsOfFullWaitSetIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "cbaa0de2-908f-478d-936d-d6e16f3bcc87");
    EXPECT_TRUE(attachAllWithEventStateMix());
    EXPECT_TRUE(detachAllWithEventStateMix());
}

TEST_F(WaitSet_test, DetachingAttachedEventWithDetachStateChangesNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ddf3a17-64d7-44ce-a228-2b8d147ca8ab");
    EXPECT_TRUE(m_sut->attachEvent(m_simpleEvents[0]));

    m_sut->detachState(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, DetachingAttachedStateWithDetachEventChangesNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "25c185f1-180a-4caa-86d5-fc02eb5d65d2");
    EXPECT_TRUE(m_sut->attachState(m_simpleEvents[0]));

    m_sut->detachEvent(m_simpleEvents[0]);
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingEventWithEnumIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad4a906f-4f1e-4d85-b2ab-d5e8ad4ce872");
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0], SimpleEvent1::EVENT1).has_error());
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(SimpleEventClass::m_simpleEvent1, Eq(SimpleEvent1::EVENT1));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameEventWithEnumFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "eede8515-f406-4b96-a132-06cec1afe3ba");
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0], SimpleEvent1::EVENT1).has_error());

    auto result = m_sut->attachEvent(m_simpleEvents[0], SimpleEvent1::EVENT1);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(WaitSetError::ALREADY_ATTACHED));
    EXPECT_THAT(SimpleEventClass::m_simpleEvent1, Eq(SimpleEvent1::EVENT1));
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameEventWithDifferentEnumValueSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "809101ea-f2cd-4023-bf56-d1480ccdde4c");
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0], SimpleEvent1::EVENT1).has_error());
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0], SimpleEvent1::EVENT2).has_error());

    // SimpleEvents has only one handler for the attachedEvents, if another is attached the first
    // one is detached, therefore the size == 1
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(SimpleEventClass::m_simpleEvent1, Eq(SimpleEvent1::EVENT2));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameEventWithDifferentEnumTypeSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "0132cb70-afc9-4b80-87d0-692e20715350");
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0], SimpleEvent1::EVENT1).has_error());
    EXPECT_FALSE(m_sut->attachEvent(m_simpleEvents[0], SimpleEvent2::EVENT1).has_error());

    // SimpleEvents has only one handler for the attachedEvents, if another is attached the first
    // one is detached, therefore the size == 1
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(SimpleEventClass::m_simpleEvent2, Eq(SimpleEvent2::EVENT1));
    EXPECT_FALSE(m_simpleEvents[0].hasStateSet());
    EXPECT_TRUE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingStateWithEnumIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "e45cc4ea-26d0-4395-8d04-090ec48dcb30");
    EXPECT_FALSE(m_sut->attachState(m_simpleEvents[0], SimpleState1::STATE1).has_error());
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(SimpleEventClass::m_simpleState1, Eq(SimpleState1::STATE1));
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameStateWithEnumFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "de485c47-195e-490b-8028-97647bed21aa");
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0], SimpleState1::STATE1).has_error());

    auto result = m_sut->attachState(m_simpleEvents[0], SimpleState1::STATE1);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(WaitSetError::ALREADY_ATTACHED));
    EXPECT_THAT(SimpleEventClass::m_simpleState1, Eq(SimpleState1::STATE1));
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameStateWithDifferentEnumValueSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "5fac3148-affb-4cbc-867f-08dd6bf4bbca");
    EXPECT_FALSE(m_sut->attachState(m_simpleEvents[0], SimpleState1::STATE1).has_error());
    EXPECT_FALSE(m_sut->attachState(m_simpleEvents[0], SimpleState1::STATE2).has_error());

    // SimpleEvents has only one handler for the attachedStates, if another is attached the first
    // one is detached, therefore the size == 1
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(SimpleEventClass::m_simpleState1, Eq(SimpleState1::STATE2));
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

TEST_F(WaitSet_test, AttachingSameStateWithDifferentEnumTypeSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ea90219-6fb6-4907-9992-e94304f5fb6c");
    EXPECT_FALSE(m_sut->attachState(m_simpleEvents[0], SimpleState1::STATE1).has_error());
    EXPECT_FALSE(m_sut->attachState(m_simpleEvents[0], SimpleState2::STATE1).has_error());

    // SimpleEvents has only one handler for the attachedEvents, if another is attached the first
    // one is detached, therefore the size == 1
    EXPECT_THAT(m_sut->size(), Eq(1U));
    EXPECT_THAT(SimpleEventClass::m_simpleState2, Eq(SimpleState2::STATE1));
    EXPECT_TRUE(m_simpleEvents[0].hasStateSet());
    EXPECT_FALSE(m_simpleEvents[0].hasEventSet());
}

////////////////////////
// END
////////////////////////

////////////////////////
// BEGIN lifetime
////////////////////////

TEST_F(WaitSet_test, ResetCallbackIsCalledWhenWaitsetGoesOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6505b19-2d92-4bae-a7fe-55b2d7933787");
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0]).has_error());
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[1]).has_error());
    std::vector<uint64_t> uniqueTriggerIds;
    uniqueTriggerIds.emplace_back(m_simpleEvents[0].getUniqueNotificationId());
    uniqueTriggerIds.emplace_back(m_simpleEvents[1].getUniqueStateId());
    m_sut.reset();

    std::sort(uniqueTriggerIds.begin(), uniqueTriggerIds.end());
    std::sort(SimpleEventClass::m_invalidateTriggerId.begin(), SimpleEventClass::m_invalidateTriggerId.end());

    EXPECT_THAT(uniqueTriggerIds, Eq(SimpleEventClass::m_invalidateTriggerId));
}

TEST_F(WaitSet_test, ResetCallbackIsCalledWhenFullWaitsetGoesOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba745725-e0f9-4085-a513-49d5e5f7bb16");
    attachAllWithEventStateMix();
    std::vector<uint64_t> uniqueTriggerIds;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        if (i % 2 == 0U)
        {
            uniqueTriggerIds.emplace_back(m_simpleEvents[i].getUniqueStateId());
        }
        else
        {
            uniqueTriggerIds.emplace_back(m_simpleEvents[i].getUniqueNotificationId());
        }
    }
    m_sut.reset();

    std::sort(uniqueTriggerIds.begin(), uniqueTriggerIds.end());
    std::sort(SimpleEventClass::m_invalidateTriggerId.begin(), SimpleEventClass::m_invalidateTriggerId.end());

    EXPECT_THAT(uniqueTriggerIds, Eq(SimpleEventClass::m_invalidateTriggerId));
}

TEST_F(WaitSet_test, EventAttachmentRemovesItselfFromWaitsetWhenGoingOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "81efdd57-7986-4533-84f5-676aca90ec04");
    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[i], 100U + i).has_error());
    }

    constexpr uint64_t USER_DEFINED_EVENT_ID = 0U;
    optional<SimpleEventClass> temporaryTrigger;
    temporaryTrigger.emplace();
    ASSERT_FALSE(m_sut->attachEvent(*temporaryTrigger, USER_DEFINED_EVENT_ID).has_error());
    // goes out of scope here and creates space again for an additional trigger
    // if this doesn't work we are unable to acquire another trigger since the
    // waitset is already full
    temporaryTrigger.reset();
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1U));
    temporaryTrigger.emplace();

    EXPECT_FALSE(m_sut->attachEvent(*temporaryTrigger, USER_DEFINED_EVENT_ID).has_error());
}

TEST_F(WaitSet_test, StateAttachmentRemovesItselfFromWaitsetWhenGoingOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "06062f01-c814-427d-91ed-f1ed4d2e4b07");
    for (uint64_t i = 0U; i + 1U < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        ASSERT_FALSE(m_sut->attachState(m_simpleEvents[i], 100U + i).has_error());
    }

    constexpr uint64_t USER_DEFINED_EVENT_ID = 0U;
    optional<SimpleEventClass> temporaryTrigger;
    temporaryTrigger.emplace();
    ASSERT_FALSE(m_sut->attachState(*temporaryTrigger, USER_DEFINED_EVENT_ID).has_error());
    // goes out of scope here and creates space again for an additional trigger
    // if this doesn't work we are unable to acquire another trigger since the
    // waitset is already full
    temporaryTrigger.reset();
    EXPECT_THAT(m_sut->size(), Eq(m_sut->capacity() - 1U));
    temporaryTrigger.emplace();

    EXPECT_FALSE(m_sut->attachState(*temporaryTrigger, USER_DEFINED_EVENT_ID).has_error());
}

TEST_F(WaitSet_test, MultipleAttachmentsRemovingThemselfFromWaitsetWhenGoingOutOfScope)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3e7bd84-8af9-49e8-9e22-e28e0ce00bab");
    attachAllWithEventStateMix();

    // here the attachments go out of scope
    m_simpleEvents.clear();

    EXPECT_THAT(m_sut->size(), Eq(0U));
}

TEST_F(WaitSet_test, AttachmentsGoingOutOfScopeReducesSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "f798143f-4bd3-4776-b086-7239cedf2031");
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0]).has_error());
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[1]).has_error());
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[2]).has_error());
    {
        SimpleEventClass simpleEvent1, simpleEvent2;
        ASSERT_FALSE(m_sut->attachEvent(simpleEvent1).has_error());
        ASSERT_FALSE(m_sut->attachEvent(simpleEvent2).has_error());
        EXPECT_EQ(m_sut->size(), 5U);
    }

    EXPECT_EQ(m_sut->size(), 3U);
}

////////////////////////
// END
////////////////////////

////////////////////////
// BEGIN trigger and blocking
////////////////////////
TEST_F(WaitSet_test, WaitBlocksWhenNothingTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "66c4d11f-f330-4629-b74a-faa87440a9a0");
    iox::concurrent::Atomic<bool> doStartWaiting{false};
    iox::concurrent::Atomic<bool> isThreadFinished{false};
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[i], 5U + i).has_error());
    }

    std::thread t([&] {
        doStartWaiting.store(true);
        auto triggerVector = m_sut->wait();
        isThreadFinished.store(true);
    });

    while (!doStartWaiting.load())
        ;


    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(isThreadFinished.load());

    m_simpleEvents[0].trigger();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(isThreadFinished.load());

    t.join();
}

TEST_F(WaitSet_test, TimedWaitReturnsNothingWhenNothingTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf1a8c00-e9c9-43e1-813e-64fd12d4e055");
    iox::vector<expected<TriggerHandle, WaitSetError>*, iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET> trigger;
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[i], 5U + i).has_error());
    }

    auto triggerVector = m_sut->timedWait(10_ms);
    ASSERT_THAT(triggerVector.size(), Eq(0U));
}

void WaitReturnsTheOneTriggeredCondition(WaitSet_test* test,
                                         const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        ASSERT_FALSE(test->m_sut->attachEvent(test->m_simpleEvents[i], 5U + i).has_error());
    }

    test->m_simpleEvents[0].trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(1U));
    EXPECT_THAT(triggerVector[0U]->getNotificationId(), 5U);
    EXPECT_TRUE(triggerVector[0U]->doesOriginateFrom(&test->m_simpleEvents[0]));
    EXPECT_EQ(triggerVector[0U]->getOrigin<WaitSet_test::SimpleEventClass>(), &test->m_simpleEvents[0]);
}

TEST_F(WaitSet_test, WaitReturnsTheOneTriggeredCondition)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e631eb4-c4d6-4b62-a8dc-44500a3f497f");
    WaitReturnsTheOneTriggeredCondition(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsTheOneTriggeredCondition)
{
    ::testing::Test::RecordProperty("TEST_ID", "9312f87a-13ca-494e-81e8-f2100cc56646");
    WaitReturnsTheOneTriggeredCondition(this, [&] { return m_sut->timedWait(10_ms); });
}

void WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered(
    WaitSet_test* test, const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        ASSERT_FALSE(test->m_sut->attachEvent(test->m_simpleEvents[i], 100U + i).has_error());
    }

    for (uint64_t i = 0U; i < 24U; ++i)
    {
        test->m_simpleEvents[i].trigger();
    }

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(24U));

    for (uint64_t i = 0U; i < 24U; ++i)
    {
        EXPECT_TRUE(WaitSet_test::doesNotificationInfoVectorContain(triggerVector, 100U + i, test->m_simpleEvents[i]));
    }
}

TEST_F(WaitSet_test, WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "1292fbcb-df42-47c4-9888-e71f1243f8ef");
    WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsAllTriggeredConditionWhenMultipleAreTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "c651b1ff-3622-4da9-98d9-23376d6863a7");
    WaitReturnsAllTriggeredConditionWhenMultipleAreTriggered(this, [&] { return m_sut->timedWait(10_ms); });
}


void WaitReturnsAllTriggeredConditionWhenAllAreTriggered(
    WaitSet_test* test, const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        ASSERT_FALSE(test->m_sut->attachEvent(test->m_simpleEvents[i], i * 3U + 2U).has_error());
    }

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        test->m_simpleEvents[i].trigger();
    }

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET));

    for (uint64_t i = 0U; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        EXPECT_TRUE(
            WaitSet_test::doesNotificationInfoVectorContain(triggerVector, i * 3U + 2U, test->m_simpleEvents[i]));
    }
}

TEST_F(WaitSet_test, WaitReturnsAllTriggeredConditionWhenAllAreTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5ef3060-e636-4bf7-acc3-47d7de8796c8");
    WaitReturnsAllTriggeredConditionWhenAllAreTriggered(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsAllTriggeredConditionWhenAllAreTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "a1e6d732-1f25-461c-b061-4418e1d25dd9");
    WaitReturnsAllTriggeredConditionWhenAllAreTriggered(this, [&] { return m_sut->timedWait(10_ms); });
}

void WaitReturnsEventTriggersWithOneCorrectCallback(WaitSet_test* test,
                                                    const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    auto result1 = test->m_sut->attachEvent(
        test->m_simpleEvents[0], 1U, createNotificationCallback(WaitSet_test::triggerCallback1));

    ASSERT_THAT(result1.has_error(), Eq(false));

    test->m_simpleEvents[0].trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(1U));

    (*triggerVector[0U])();

    EXPECT_THAT(test->m_simpleEvents[0].m_triggerCallbackArgument1, Eq(&test->m_simpleEvents[0]));
}

TEST_F(WaitSet_test, WaitReturnsEventTriggersWithOneCorrectCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6c1f551-4d1b-4e85-80fc-aa7d135478b0");
    WaitReturnsEventTriggersWithOneCorrectCallback(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsEventTriggersWithTwoCorrectCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "5e1dbc44-db59-479a-ab59-a30486c33b82");
    WaitReturnsEventTriggersWithOneCorrectCallback(this, [&] { return m_sut->timedWait(10_ms); });
}

void WaitReturnsEventTriggersWithTwoCorrectCallbacksWithContextData(
    WaitSet_test* test, const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    uint64_t contextData1 = 0U;
    uint64_t contextData2 = 0U;
    auto result1 = test->m_sut->attachEvent(
        test->m_simpleEvents[0],
        1U,
        createNotificationCallback(WaitSet_test::triggerCallback1WithContextData, contextData1));
    auto result2 = test->m_sut->attachEvent(
        test->m_simpleEvents[1],
        2U,
        createNotificationCallback(WaitSet_test::triggerCallback2WithContextData, contextData2));

    ASSERT_THAT(result1.has_error(), Eq(false));
    ASSERT_THAT(result2.has_error(), Eq(false));

    test->m_simpleEvents[0].trigger();
    test->m_simpleEvents[1].trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(2U));

    (*triggerVector[0U])();
    (*triggerVector[1U])();

    EXPECT_THAT(test->m_simpleEvents[0].m_triggerCallbackArgument1, Eq(&test->m_simpleEvents[0]));
    EXPECT_THAT(test->m_simpleEvents[1].m_triggerCallbackArgument2, Eq(&test->m_simpleEvents[1]));
    EXPECT_THAT(test->m_simpleEvents[0].m_contextData1, Eq(&contextData1));
    EXPECT_THAT(test->m_simpleEvents[1].m_contextData2, Eq(&contextData2));
}

TEST_F(WaitSet_test, WaitReturnsEventTriggersWithTwoCorrectCallbacksWithContextData)
{
    ::testing::Test::RecordProperty("TEST_ID", "6215eb19-9304-40b8-a700-96ac3331338e");
    WaitReturnsEventTriggersWithTwoCorrectCallbacksWithContextData(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsEventTriggersWithTwoCorrectCallbacksWithContextData)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee21de8a-ba45-42aa-9428-5cfd83d66fa2");
    WaitReturnsEventTriggersWithTwoCorrectCallbacksWithContextData(this, [&] { return m_sut->timedWait(10_ms); });
}

void WaitReturnsStateTriggersWithOneCorrectCallback(WaitSet_test* test,
                                                    const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    auto result1 = test->m_sut->attachState(
        test->m_simpleEvents[0], 1U, createNotificationCallback(WaitSet_test::triggerCallback1));

    ASSERT_THAT(result1.has_error(), Eq(false));

    test->m_simpleEvents[0].trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(1U));

    (*triggerVector[0U])();

    EXPECT_THAT(test->m_simpleEvents[0].m_triggerCallbackArgument1, Eq(&test->m_simpleEvents[0]));
}

TEST_F(WaitSet_test, WaitReturnsStateTriggersWithOneCorrectCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "d5a1d84a-0882-494f-a3e8-3781e3adba96");
    WaitReturnsStateTriggersWithOneCorrectCallback(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsStateTriggersWithTwoCorrectCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a087010-496d-4e0e-9352-a8a8dc7a16be");
    WaitReturnsStateTriggersWithOneCorrectCallback(this, [&] { return m_sut->timedWait(10_ms); });
}

void WaitReturnsStateTriggersWithTwoCorrectCallbacksWithContextData(
    WaitSet_test* test, const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    uint64_t contextData1 = 0U;
    uint64_t contextData2 = 0U;
    auto result1 = test->m_sut->attachState(
        test->m_simpleEvents[0],
        1U,
        createNotificationCallback(WaitSet_test::triggerCallback1WithContextData, contextData1));
    auto result2 = test->m_sut->attachState(
        test->m_simpleEvents[1],
        2U,
        createNotificationCallback(WaitSet_test::triggerCallback2WithContextData, contextData2));

    ASSERT_THAT(result1.has_error(), Eq(false));
    ASSERT_THAT(result2.has_error(), Eq(false));

    test->m_simpleEvents[0].trigger();
    test->m_simpleEvents[1].trigger();

    auto triggerVector = waitCall();
    ASSERT_THAT(triggerVector.size(), Eq(2U));

    (*triggerVector[0U])();
    (*triggerVector[1U])();

    EXPECT_THAT(test->m_simpleEvents[0].m_triggerCallbackArgument1, Eq(&test->m_simpleEvents[0]));
    EXPECT_THAT(test->m_simpleEvents[1].m_triggerCallbackArgument2, Eq(&test->m_simpleEvents[1]));
    EXPECT_THAT(test->m_simpleEvents[0].m_contextData1, Eq(&contextData1));
    EXPECT_THAT(test->m_simpleEvents[1].m_contextData2, Eq(&contextData2));
}

TEST_F(WaitSet_test, WaitReturnsStateTriggersWithTwoCorrectCallbacksWithContextData)
{
    ::testing::Test::RecordProperty("TEST_ID", "27fa9ad0-b1a4-45e7-be4a-e620ec083ac3");
    WaitReturnsStateTriggersWithTwoCorrectCallbacksWithContextData(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, TimedWaitReturnsStateTriggersWithTwoCorrectCallbacksWithContextData)
{
    ::testing::Test::RecordProperty("TEST_ID", "03746599-c01f-4d9c-b4bc-68890f3b4cc5");
    WaitReturnsStateTriggersWithTwoCorrectCallbacksWithContextData(this, [&] { return m_sut->timedWait(10_ms); });
}

void NonResetStatesAreReturnedAgain(WaitSet_test* test,
                                    const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    test->attachAllStates();

    test->m_simpleEvents[2].m_autoResetTrigger = false;
    test->m_simpleEvents[2].trigger();

    test->m_simpleEvents[7].m_autoResetTrigger = false;
    test->m_simpleEvents[7].trigger();

    auto eventVector = waitCall();

    // ACT
    eventVector = waitCall();

    ASSERT_THAT(eventVector.size(), Eq(2U));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 2U, test->m_simpleEvents[2]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 7U, test->m_simpleEvents[7]));
}

TEST_F(WaitSet_test, NonResetStatesAreReturnedAgainInTimedWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "ac2001d2-29fa-4c87-a874-6052e7158000");
    NonResetStatesAreReturnedAgain(this, [&] { return m_sut->timedWait(iox::units::Duration::fromMilliseconds(100)); });
}

TEST_F(WaitSet_test, NonResetStatesAreReturnedAgainInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0968c3d-82d1-4f4a-89d3-9495c417de32");
    NonResetStatesAreReturnedAgain(this, [&] { return m_sut->wait(); });
}

void TriggeredEventsAreNotReturnedTwice(WaitSet_test* test,
                                        const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    test->attachAllEvents();

    test->m_simpleEvents[2].trigger();
    test->m_simpleEvents[7].trigger();

    auto eventVector = waitCall();

    // ACT
    test->m_simpleEvents[3].trigger();
    eventVector = waitCall();

    ASSERT_THAT(eventVector.size(), Eq(1U));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 3U, test->m_simpleEvents[3]));
}

TEST_F(WaitSet_test, TriggeredEventsAreNotReturnedTwiceInTimedWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f2ea5d1-ebd0-4d1c-818a-3ee160f09266");
    TriggeredEventsAreNotReturnedTwice(this,
                                       [&] { return m_sut->timedWait(iox::units::Duration::fromMilliseconds(100)); });
}

TEST_F(WaitSet_test, TriggeredEventsAreNotReturnedTwiceInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "627f5612-724b-45d7-86dd-78cb28c7c6e6");
    TriggeredEventsAreNotReturnedTwice(this, [&] { return m_sut->wait(); });
}

void InMixSetupOnlyStateTriggerAreReturnedTwice(WaitSet_test* test,
                                                const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    test->attachAllWithEventStateMix();

    for (auto& event : test->m_simpleEvents)
    {
        event.m_autoResetTrigger = false;
        event.trigger();
    }

    auto eventVector = waitCall();

    // ACT
    eventVector = waitCall();

    ASSERT_THAT(eventVector.size(), Eq(iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET / 2U));
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; i += 2)
    {
        EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, i, test->m_simpleEvents[i]));
    }
}

TEST_F(WaitSet_test, InMixSetupOnlyStateTriggerAreReturnedTwiceInTimedWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ddc13f8-0a50-4446-977b-701606b6e972");
    InMixSetupOnlyStateTriggerAreReturnedTwice(
        this, [&] { return m_sut->timedWait(iox::units::Duration::fromMilliseconds(100)); });
}

TEST_F(WaitSet_test, InMixSetupOnlyStateTriggerAreReturnedTwiceInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf4a8278-3999-46df-8ef0-d26752eecc03");
    InMixSetupOnlyStateTriggerAreReturnedTwice(this, [&] { return m_sut->wait(); });
}

void WhenStateIsNotResetAndEventIsTriggeredBeforeItIsReturnedAgain(
    WaitSet_test* test, const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    test->attachAllWithEventStateMix();

    test->m_simpleEvents[2].m_autoResetTrigger = false;
    test->m_simpleEvents[2].trigger();

    auto eventVector = waitCall();

    test->m_simpleEvents[1].trigger();

    // ACT
    eventVector = waitCall();

    ASSERT_THAT(eventVector.size(), Eq(2));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 1U, test->m_simpleEvents[1]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 2U, test->m_simpleEvents[2]));
}

TEST_F(WaitSet_test, WhenStateIsNotResetAndEventIsTriggeredBeforeItIsReturnedAgainInTimedWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "afce92a2-e6a8-489b-bef6-6c225ce69e41");
    WhenStateIsNotResetAndEventIsTriggeredBeforeItIsReturnedAgain(
        this, [&] { return m_sut->timedWait(iox::units::Duration::fromMilliseconds(100)); });
}

TEST_F(WaitSet_test, WhenStateIsNotResetAndEventIsTriggeredBeforeItIsReturnedAgainInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "903f03d9-14a4-4b93-864d-b4cc9dc2af36");
    WhenStateIsNotResetAndEventIsTriggeredBeforeItIsReturnedAgain(this, [&] { return m_sut->wait(); });
}

void WhenStateIsNotResetAndEventIsTriggeredAfterItIsReturnedAgain(
    WaitSet_test* test, const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    test->attachAllWithEventStateMix();

    test->m_simpleEvents[2].m_autoResetTrigger = false;
    test->m_simpleEvents[2].trigger();

    auto eventVector = waitCall();

    test->m_simpleEvents[3].trigger();

    // ACT
    eventVector = waitCall();

    ASSERT_THAT(eventVector.size(), Eq(2));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 2U, test->m_simpleEvents[2]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 3U, test->m_simpleEvents[3]));
}

TEST_F(WaitSet_test, WhenStateIsNotResetAndEventIsTriggeredAfterItIsReturnedAgainInTimedWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "40ab8422-1b9b-4d0a-837d-2f7317b93c03");
    WhenStateIsNotResetAndEventIsTriggeredAfterItIsReturnedAgain(
        this, [&] { return m_sut->timedWait(iox::units::Duration::fromMilliseconds(100)); });
}

TEST_F(WaitSet_test, WhenStateIsNotResetAndEventIsTriggeredAfterItIsReturnedAgainInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "39050968-20d7-4e10-9d03-ad4c30368497");
    WhenStateIsNotResetAndEventIsTriggeredAfterItIsReturnedAgain(this, [&] { return m_sut->wait(); });
}

void WhenStateIsNotResetAndEventsAreTriggeredItIsReturnedAgain(
    WaitSet_test* test, const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    test->attachAllWithEventStateMix();

    test->m_simpleEvents[2].m_autoResetTrigger = false;
    test->m_simpleEvents[2].trigger();

    test->m_simpleEvents[7].trigger();

    test->m_simpleEvents[12].m_autoResetTrigger = false;
    test->m_simpleEvents[12].trigger();

    auto eventVector = waitCall();

    test->m_simpleEvents[1].trigger();
    test->m_simpleEvents[3].trigger();
    test->m_simpleEvents[6].trigger();
    test->m_simpleEvents[13].trigger();

    // ACT
    eventVector = waitCall();

    ASSERT_THAT(eventVector.size(), Eq(6));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 1U, test->m_simpleEvents[1]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 2U, test->m_simpleEvents[2]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 3U, test->m_simpleEvents[3]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 6U, test->m_simpleEvents[6]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 12U, test->m_simpleEvents[12]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 13U, test->m_simpleEvents[13]));
}

TEST_F(WaitSet_test, WhenStateIsNotResetAndEventsAreTriggeredItIsReturnedAgainInTimedWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "c49a11fc-66fc-46dc-b957-582a315f963a");
    WhenStateIsNotResetAndEventsAreTriggeredItIsReturnedAgain(
        this, [&] { return m_sut->timedWait(iox::units::Duration::fromMilliseconds(100)); });
}

TEST_F(WaitSet_test, WhenStateIsNotResetAndEventsAreTriggeredItIsReturnedAgainInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "39235ee8-1687-4d36-b5b6-3a22931297b8");
    WhenStateIsNotResetAndEventsAreTriggeredItIsReturnedAgain(this, [&] { return m_sut->wait(); });
}

void NotifyingWaitSetTwiceWithSameTriggersWorks(WaitSet_test* test,
                                                const std::function<WaitSet<>::NotificationInfoVector()>& waitCall)
{
    test->attachAllEvents();

    test->m_simpleEvents[2].trigger();
    test->m_simpleEvents[7].trigger();

    auto eventVector = waitCall();

    test->m_simpleEvents[2].trigger();
    test->m_simpleEvents[7].trigger();

    eventVector = waitCall();

    ASSERT_THAT(eventVector.size(), Eq(2));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 2U, test->m_simpleEvents[2]));
    EXPECT_TRUE(test->doesNotificationInfoVectorContain(eventVector, 7U, test->m_simpleEvents[7]));
}

TEST_F(WaitSet_test, NotifyingWaitSetTwiceWithSameTriggersWorksInTimedWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "49365392-7052-44b8-a731-d33853494be9");
    NotifyingWaitSetTwiceWithSameTriggersWorks(
        this, [&] { return m_sut->timedWait(iox::units::Duration::fromMilliseconds(100)); });
}

TEST_F(WaitSet_test, NotifyingWaitSetTwiceWithSameTriggersWorksInWait)
{
    ::testing::Test::RecordProperty("TEST_ID", "1700eb0f-0517-45c2-8e30-7c3ccbd97a12");
    NotifyingWaitSetTwiceWithSameTriggersWorks(this, [&] { return m_sut->wait(); });
}

TEST_F(WaitSet_test, EventBasedTriggerIsReturnedOnlyOnceWhenItsTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "84da1aef-eaef-4a4b-a089-3ccd2f543b5a");
    m_simpleEvents[0].m_isEventBased = true;
    m_simpleEvents[0].m_autoResetTrigger = false;

    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0], 3431).has_error());

    m_simpleEvents[0].trigger();

    auto eventVector = m_sut->wait();
    ASSERT_THAT(eventVector.size(), Eq(1));
    EXPECT_TRUE(doesNotificationInfoVectorContain(eventVector, 3431, m_simpleEvents[0]));

    eventVector = m_sut->timedWait(iox::units::Duration::fromMilliseconds(1));
    EXPECT_TRUE(eventVector.empty());
}

TEST_F(WaitSet_test, MixingEventAndStateBasedTriggerHandlesEventTriggeresWithWaitCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "1df0d6ca-1190-4f5a-bb76-dbb7f155c3fb");
    m_simpleEvents[0].m_autoResetTrigger = false;
    m_simpleEvents[1].m_autoResetTrigger = false;

    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0], 3431).has_error());
    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[1], 8171).has_error());

    m_simpleEvents[0].trigger();
    m_simpleEvents[1].trigger();

    auto eventVector = m_sut->wait();
    ASSERT_THAT(eventVector.size(), Eq(2));
    EXPECT_TRUE(doesNotificationInfoVectorContain(eventVector, 3431, m_simpleEvents[0]));
    EXPECT_TRUE(doesNotificationInfoVectorContain(eventVector, 8171, m_simpleEvents[1]));

    eventVector = m_sut->timedWait(iox::units::Duration::fromMilliseconds(1));
    ASSERT_THAT(eventVector.size(), Eq(1));
    EXPECT_TRUE(doesNotificationInfoVectorContain(eventVector, 8171, m_simpleEvents[1]));
}

TEST_F(WaitSet_test, WaitUnblocksAfterMarkForDestructionCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "a7c0b153-65da-4603-bd82-5f5db5841a2b");
    iox::concurrent::Atomic<bool> doStartWaiting{false};
    iox::concurrent::Atomic<bool> isThreadFinished{false};
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0U], 0U).has_error());

    std::thread t([&] {
        doStartWaiting.store(true);
        auto triggerVector = m_sut->wait();
        triggerVector = m_sut->wait();
        triggerVector = m_sut->wait();
        isThreadFinished.store(true);
    });

    while (!doStartWaiting.load())
        ;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(isThreadFinished.load());

    m_sut->markForDestruction();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(isThreadFinished.load());

    t.join();
}

TEST_F(WaitSet_test, TimedWaitUnblocksAfterMarkForDestructionCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "63573915-bb36-4ece-93be-2adc853582e6");
    iox::concurrent::Atomic<bool> doStartWaiting{false};
    iox::concurrent::Atomic<bool> isThreadFinished{false};
    ASSERT_FALSE(m_sut->attachEvent(m_simpleEvents[0U], 0U).has_error());

    std::thread t([&] {
        doStartWaiting.store(true);
        auto triggerVector = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
        triggerVector = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
        triggerVector = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
        isThreadFinished.store(true);
    });

    while (!doStartWaiting.load())
        ;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(isThreadFinished.load());

    m_sut->markForDestruction();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(isThreadFinished.load());

    t.join();
}

TEST_F(WaitSet_test, WaitSetReturnsIfStateTriggeredBeforeAttachingWithEventType)
{
    ::testing::Test::RecordProperty("TEST_ID", "407dc160-a50c-45b3-84bc-92a5f186fc14");
    m_simpleEvents[0U].m_autoResetTrigger = false;
    m_simpleEvents[0U].trigger();

    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0U], SimpleState1::STATE1).has_error());

    auto triggerVector = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
    ASSERT_THAT(triggerVector.size(), Eq(1U));
    EXPECT_TRUE(triggerVector[0U]->doesOriginateFrom(&m_simpleEvents[0U]));
}

TEST_F(WaitSet_test, WaitSetReturnsIfStateTriggeredBeforeAttachingWithEventId)
{
    ::testing::Test::RecordProperty("TEST_ID", "5753de85-7b34-4024-bd50-938c8885d269");
    m_simpleEvents[0U].m_autoResetTrigger = false;
    m_simpleEvents[0U].trigger();

    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0U], 0U).has_error());

    auto triggerVector = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
    ASSERT_THAT(triggerVector.size(), Eq(1U));
    EXPECT_TRUE(triggerVector[0U]->doesOriginateFrom(&m_simpleEvents[0U]));
}

TEST_F(WaitSet_test, WaitSetReturnsAgainIfStateTriggeredBeforeAttachingWithEventType)
{
    ::testing::Test::RecordProperty("TEST_ID", "c86d2592-e8e5-4acc-b468-0a82be82fe7c");
    m_simpleEvents[0U].m_autoResetTrigger = false;
    m_simpleEvents[0U].trigger();

    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0U], SimpleState1::STATE1).has_error());

    auto triggerVector1 = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
    ASSERT_THAT(triggerVector1.size(), Eq(1U));
    EXPECT_TRUE(triggerVector1[0U]->doesOriginateFrom(&m_simpleEvents[0U]));

    // Waiting for another time should lead to the same result
    auto triggerVector2 = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
    ASSERT_THAT(triggerVector2.size(), Eq(1U));
    EXPECT_TRUE(triggerVector2[0U]->doesOriginateFrom(&m_simpleEvents[0U]));
}

TEST_F(WaitSet_test, WaitSetReturnsAgainIfStateTriggeredBeforeAttachingWithEventId)
{
    ::testing::Test::RecordProperty("TEST_ID", "b07c9e09-f497-4bfa-9403-633d15363f5e");
    m_simpleEvents[0U].m_autoResetTrigger = false;
    m_simpleEvents[0U].trigger();

    ASSERT_FALSE(m_sut->attachState(m_simpleEvents[0U], 0U).has_error());

    auto triggerVector1 = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
    ASSERT_THAT(triggerVector1.size(), Eq(1U));
    EXPECT_TRUE(triggerVector1[0U]->doesOriginateFrom(&m_simpleEvents[0U]));

    // Waiting for another time should lead to the same result
    auto triggerVector2 = m_sut->timedWait(iox::units::Duration::fromSeconds(1337));
    ASSERT_THAT(triggerVector2.size(), Eq(1U));
    EXPECT_TRUE(triggerVector2[0U]->doesOriginateFrom(&m_simpleEvents[0U]));
}


} // namespace
