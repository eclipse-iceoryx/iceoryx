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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"
#include "iceoryx_posh/popo/listener.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/listener.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
}

#include "test.hpp"

#include <atomic>
#include <thread>

using namespace ::testing;

namespace
{
iox_user_trigger_t g_userTriggerCallbackArgument = nullptr;

class iox_listener_test : public Test
{
  public:
    class TestListener : public Listener
    {
      public:
        TestListener(EventVariableData& eventVar)
            : Listener(&eventVar)
        {
        }
    };

    void SetUp() override
    {
        g_userTriggerCallbackArgument = nullptr;
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U; ++i)
        {
            m_userTrigger.emplace_back(iox_user_trigger_init(&m_userTriggerStorage[i]));
        }
    }

    void TearDown() override
    {
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U; ++i)
        {
            iox_user_trigger_deinit(m_userTrigger[i]);
        }
    }

    EventVariableData m_eventVar{"hypnotoadKnueppeltRetour"};
    TestListener m_sut{m_eventVar};

    iox_user_trigger_storage_t m_userTriggerStorage[MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1];
    cxx::vector<iox_user_trigger_t, MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1> m_userTrigger;
};

void userTriggerCallback(iox_user_trigger_t userTrigger)
{
    g_userTriggerCallbackArgument = userTrigger;
}
} // namespace

TEST_F(iox_listener_test, CapacityIsCorrect)
{
    EXPECT_THAT(iox_listener_capacity(&m_sut), Eq(MAX_NUMBER_OF_EVENTS_PER_LISTENER));
}

TEST_F(iox_listener_test, SizeIsZeroWhenCreated)
{
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0));
}

TEST_F(iox_listener_test, SizeIsOneWhenOneClassIsAttached)
{
    EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1));
}

TEST_F(iox_listener_test, SizeEqualsCapacityWhenMaximumIsAttached)
{
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[i], &userTriggerCallback),
                    Eq(iox_ListenerResult::ListenerResult_SUCCESS));
        EXPECT_THAT(iox_listener_size(&m_sut), Eq(i + 1U));
    }
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(iox_listener_capacity(&m_sut)));
}

TEST_F(iox_listener_test, SizeDecreasesWhenEventsAreDetached)
{
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[i], &userTriggerCallback),
                    Eq(iox_ListenerResult::ListenerResult_SUCCESS));
        EXPECT_THAT(iox_listener_size(&m_sut), Eq(i + 1U));
    }
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        iox_listener_detach_user_trigger_event(&m_sut, m_userTrigger[i]);
        EXPECT_THAT(iox_listener_size(&m_sut), Eq(iox_listener_capacity(&m_sut) - i - 1U));
    }
}

TEST_F(iox_listener_test, FullListenerReturnsLISTENER_FULLWhenAnotherEventIsAttached)
{
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[i], &userTriggerCallback),
                    Eq(iox_ListenerResult::ListenerResult_SUCCESS));
        EXPECT_THAT(iox_listener_size(&m_sut), Eq(i + 1U));
    }
    EXPECT_THAT(iox_listener_attach_user_trigger_event(
                    &m_sut, m_userTrigger[MAX_NUMBER_OF_EVENTS_PER_LISTENER], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_LISTENER_FULL));
}

TEST_F(iox_listener_test, AttachingTheSameEventTwiceLeadsToEVENT_ALREADY_ATTACHED)
{
    EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
    EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_EVENT_ALREADY_ATTACHED));
}
// test same thing for events with enums
