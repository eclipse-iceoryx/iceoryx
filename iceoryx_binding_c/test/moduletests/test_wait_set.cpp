// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "mocks/wait_set_mock.hpp"

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

namespace
{
using namespace ::testing;

namespace
{
class iox_ws_test : public Test
{
  public:
    void SetUp() override
    {
        m_callbackOrigin = nullptr;
        m_contextData = nullptr;

        for (uint64_t i = 0U; i <= MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            m_portDataVector.emplace_back(TEST_SERVICE_DESCRIPTION,
                                          "someAppName",
                                          iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
                                          m_subscriberOptions);
            m_subscriberVector.emplace_back();
            m_subscriberVector[i].m_portData = &m_portDataVector[i];
        }

        for (uint64_t i = 0U; i <= MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            m_userTrigger.emplace_back(iox_user_trigger_init(&m_userTriggerStorage[i]));
        }
    }

    void TearDown() override
    {
        delete m_sut;

        for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
        {
            iox_user_trigger_deinit(m_userTrigger[i]);
        }
    }

    std::unique_ptr<PoshRuntimeMock> runtimeMock = PoshRuntimeMock::create("rudi_ruessel");
    const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};
    iox::popo::SubscriberOptions m_subscriberOptions{MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY, 0U};
    cxx::vector<iox::popo::SubscriberPortData, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1U> m_portDataVector;
    cxx::vector<cpp2c_Subscriber, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1U> m_subscriberVector;

    ConditionVariableData m_condVar{"Horscht"};
    WaitSetMock* m_sut = new WaitSetMock{m_condVar};

    iox_user_trigger_storage_t m_userTriggerStorage[MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1];
    cxx::vector<iox_user_trigger_t, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1> m_userTrigger;

    iox_notification_info_t m_eventInfoStorage[MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET];
    uint64_t m_missedElements = 0U;
    uint64_t m_numberOfTriggeredConditions = 0U;
    timespec m_timeout{0, 0};

    iox::mepoo::MemoryManager memoryManager;
    ClientPortData clientPortData{{"ServiceA", "InstanceA", "EventA"}, "rudi_ruessel", ClientOptions(), &memoryManager};
    iox_client_storage_t clientStorage;

    ServerPortData serverPortData{
        {"ServiceA", "InstanceA", "EventA"}, "hypnotoad_loves_iceoryx", ServerOptions(), &memoryManager};
    iox_server_storage_t serverStorage;

    static void* m_callbackOrigin;
    static void* m_contextData;
};

void* iox_ws_test::m_callbackOrigin = nullptr;
void* iox_ws_test::m_contextData = nullptr;

void subscriberCallback(iox_sub_t subscriber)
{
    iox_ws_test::m_callbackOrigin = subscriber;
}

void subscriberCallbackWithContextData(iox_sub_t subscriber, void* const contextData)
{
    iox_ws_test::m_callbackOrigin = subscriber;
    iox_ws_test::m_contextData = contextData;
}

void userTriggerCallback(iox::popo::UserTrigger* userTrigger)
{
    iox_ws_test::m_callbackOrigin = userTrigger;
}

void userTriggerCallbackWithContextData(iox::popo::UserTrigger* userTrigger, void* const contextData)
{
    iox_ws_test::m_callbackOrigin = userTrigger;
    iox_ws_test::m_contextData = contextData;
}

void clientCallback(iox::popo::UntypedClient* client)
{
    iox_ws_test::m_callbackOrigin = client;
}

void clientCallbackWithContextData(iox::popo::UntypedClient* client, void* const contextData)
{
    iox_ws_test::m_callbackOrigin = client;
    iox_ws_test::m_contextData = contextData;
}

void serverCallback(iox::popo::UntypedServer* client)
{
    iox_ws_test::m_callbackOrigin = client;
}

void serverCallbackWithContextData(iox::popo::UntypedServer* client, void* const contextData)
{
    iox_ws_test::m_callbackOrigin = client;
    iox_ws_test::m_contextData = contextData;
}
} // namespace

/// @todo iox-#1106 will be enabled when worked on this issue
TEST_F(iox_ws_test, DISABLED_InitWaitSetWithNullptrForStorageReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0f6b413-de1f-441f-916e-aa158fbfdde3");
    EXPECT_EQ(iox_ws_init(nullptr), nullptr);
}

TEST_F(iox_ws_test, CapacityIsCorrect)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab5c64d3-0f74-4aa5-8e8d-8419c3ad71ed");
    EXPECT_EQ(iox_ws_capacity(m_sut), MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET);
}

TEST_F(iox_ws_test, SizeIsZeroWhenConstructed)
{
    ::testing::Test::RecordProperty("TEST_ID", "64bf992a-0089-43ba-a3ef-bfd411843b27");
    EXPECT_EQ(iox_ws_size(m_sut), 0U);
}

TEST_F(iox_ws_test, SizeIsOneWhenOneUserTriggerIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "3fd08c7e-7eda-4586-a737-e803ca3ba995");
    EXPECT_EQ(iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    EXPECT_EQ(iox_ws_size(m_sut), 1U);
}

TEST_F(iox_ws_test, SizeIsOneWhenOneSubscriberStateIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "401d63e6-0708-49a2-b8b1-f8447d82e660");
    constexpr uint64_t customId = 123U;
    EXPECT_EQ(iox_ws_attach_subscriber_state(
                  m_sut, &m_subscriberVector[0U], SubscriberState_HAS_DATA, customId, subscriberCallback),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    EXPECT_EQ(iox_ws_size(m_sut), 1U);
}

TEST_F(iox_ws_test, SizeIsOneWhenOneUserTriggerWithNullptrCallBackIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bb92cc8-a3cf-4ac8-8df8-6047ea1228c2");
    EXPECT_EQ(iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, NULL),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    EXPECT_EQ(iox_ws_size(m_sut), 1U);
}

TEST_F(iox_ws_test, SizeIsOneWhenOneSubscriberStateWithNullptrCallBackIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd04bf28-3ae5-4903-b46b-f09da31b46ce");
    constexpr uint64_t customId = 123U;
    EXPECT_EQ(iox_ws_attach_subscriber_state(m_sut, &m_subscriberVector[0U], SubscriberState_HAS_DATA, customId, NULL),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    EXPECT_EQ(iox_ws_size(m_sut), 1U);
}

TEST_F(iox_ws_test, SizeIsOneWhenOneSubscriberEventIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "2becf5a5-f6ba-4e65-abfb-8880b1f0789d");
    constexpr uint64_t customId = 123U;
    EXPECT_EQ(iox_ws_attach_subscriber_event(
                  m_sut, &m_subscriberVector[0U], SubscriberEvent_DATA_RECEIVED, customId, subscriberCallback),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    EXPECT_EQ(iox_ws_size(m_sut), 1U);
}

TEST_F(iox_ws_test, SizeIsOneWhenOneSubscriberEventWithNullptrCallBackIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "b6aafb13-8da0-4058-bcbf-d84d4dc39aa5");
    constexpr uint64_t customId = 123U;
    EXPECT_EQ(
        iox_ws_attach_subscriber_event(m_sut, &m_subscriberVector[0U], SubscriberEvent_DATA_RECEIVED, customId, NULL),
        iox_WaitSetResult::WaitSetResult_SUCCESS);
    EXPECT_EQ(iox_ws_size(m_sut), 1U);
}

TEST_F(iox_ws_test, AttachingMoreUserTriggerThanCapacityAvailableFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c696900b-7ef0-4f32-8666-7340b52cef1e");
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        EXPECT_EQ(iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback),
                  iox_WaitSetResult::WaitSetResult_SUCCESS);
        EXPECT_EQ(iox_ws_size(m_sut), i + 1U);
    }
    EXPECT_EQ(iox_ws_attach_user_trigger_event(
                  m_sut, m_userTrigger[MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET], 0U, userTriggerCallback),
              iox_WaitSetResult::WaitSetResult_WAIT_SET_FULL);
}

TEST_F(iox_ws_test, AttachingMoreSubscriberStatesThanCapacityAvailableFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "f69eb949-255a-483d-bcde-9af45ec67ab8");
    constexpr uint64_t customId = 123U;
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        EXPECT_EQ(iox_ws_attach_subscriber_state(
                      m_sut, &m_subscriberVector[i], SubscriberState_HAS_DATA, customId, subscriberCallback),
                  iox_WaitSetResult::WaitSetResult_SUCCESS);
        EXPECT_EQ(iox_ws_size(m_sut), i + 1U);
    }
    EXPECT_EQ(iox_ws_attach_subscriber_state(m_sut,
                                             &m_subscriberVector[MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET],
                                             SubscriberState_HAS_DATA,
                                             customId,
                                             subscriberCallback),
              iox_WaitSetResult::WaitSetResult_WAIT_SET_FULL);
}

TEST_F(iox_ws_test, AttachingMoreSubscriberEventsThanCapacityAvailableFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "555dd52c-222a-40da-8311-15046db418eb");
    constexpr uint64_t customId = 123U;
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        EXPECT_EQ(iox_ws_attach_subscriber_event(
                      m_sut, &m_subscriberVector[i], SubscriberEvent_DATA_RECEIVED, customId, subscriberCallback),
                  iox_WaitSetResult::WaitSetResult_SUCCESS);
        EXPECT_EQ(iox_ws_size(m_sut), i + 1U);
    }
    EXPECT_EQ(iox_ws_attach_subscriber_event(m_sut,
                                             &m_subscriberVector[MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET],
                                             SubscriberEvent_DATA_RECEIVED,
                                             customId,
                                             subscriberCallback),
              iox_WaitSetResult::WaitSetResult_WAIT_SET_FULL);
}

TEST_F(iox_ws_test, SizeDecreasesWhenAttachedUserTriggerIsDeinitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d1dccce-0712-4242-b6c9-54060d4827f6");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);
    iox_ws_detach_user_trigger_event(m_sut, m_userTrigger[0U]);
    EXPECT_EQ(iox_ws_size(m_sut), 0U);
}

TEST_F(iox_ws_test, SizeDecreasesWhenAttachedSubscriberStateIsDeinitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "a985dee5-7e4f-43f5-b6f3-7f8a2a500202");
    constexpr uint64_t customId = 123U;
    EXPECT_EQ(iox_ws_attach_subscriber_state(
                  m_sut, &m_subscriberVector[0U], SubscriberState_HAS_DATA, customId, subscriberCallback),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    iox_ws_detach_subscriber_state(m_sut, &m_subscriberVector[0U], SubscriberState_HAS_DATA);
    EXPECT_EQ(iox_ws_size(m_sut), 0U);
}

TEST_F(iox_ws_test, SizeDecreasesWhenAttachedSubscriberEventIsDeinitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "f332215a-d2b6-49c9-a2bd-ba71c7ee3612");
    constexpr uint64_t customId = 123U;
    EXPECT_EQ(iox_ws_attach_subscriber_event(
                  m_sut, &m_subscriberVector[0U], SubscriberEvent_DATA_RECEIVED, customId, subscriberCallback),
              iox_WaitSetResult::WaitSetResult_SUCCESS);
    iox_ws_detach_subscriber_event(m_sut, &m_subscriberVector[0U], SubscriberEvent_DATA_RECEIVED);
    EXPECT_EQ(iox_ws_size(m_sut), 0U);
}

TEST_F(iox_ws_test, NumberOfTriggeredConditionsIsOneWhenOneWasTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1fbf9fd-fbae-439d-94f5-41e5d9756fd2");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);
    iox_user_trigger_trigger(m_userTrigger[0U]);

    EXPECT_EQ(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements), 1U);
}

TEST_F(iox_ws_test, NumberOfTriggeredConditionsIsCorrectWhenMultipleWereTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "da1f3eaa-fbdd-4ab1-844e-ba48ba6989f9");
    for (uint64_t i = 0U; i < 10U; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements), 10U);
}

TEST_F(iox_ws_test, NumberOfTriggeredConditionsIsCorrectWhenAllWereTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "74a629b1-bd15-4acb-8ae0-4ee926c594a8");
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
              MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET);
}

TEST_F(iox_ws_test, SingleTriggerCaseWaitReturnsCorrectTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd35162d-a076-43b3-bc3b-fcc574c6b5cf");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0], 5678U, userTriggerCallback);
    iox_user_trigger_trigger(m_userTrigger[0U]);

    iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);

    iox_notification_info_t eventInfo = m_eventInfoStorage[0U];

    EXPECT_EQ(iox_notification_info_get_notification_id(eventInfo), 5678U);
    EXPECT_TRUE(iox_notification_info_does_originate_from_user_trigger(eventInfo, m_userTrigger[0U]));
}

TEST_F(iox_ws_test, MultiTriggerCaseWaitReturnsCorrectTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "c3bfe540-950b-4f70-bb8b-7f7c1500de29");
    for (uint64_t i = 0U; i < 8; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 1337U + i, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);

    for (uint64_t i = 0U; i < 8; ++i)
    {
        iox_notification_info_t eventInfo = m_eventInfoStorage[i];
        EXPECT_EQ(iox_notification_info_get_notification_id(eventInfo), 1337U + i);
        EXPECT_TRUE(iox_notification_info_does_originate_from_user_trigger(eventInfo, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, MaxTriggerCaseWaitReturnsCorrectTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "a76d77ee-cc02-4532-b792-209794200bf8");
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 42U * i + 1U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);

    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        iox_notification_info_t eventInfo = m_eventInfoStorage[i];
        EXPECT_EQ(iox_notification_info_get_notification_id(eventInfo), 42U * i + 1U);
        EXPECT_TRUE(iox_notification_info_does_originate_from_user_trigger(eventInfo, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, TimedWaitNumberOfTriggeredConditionsIsOneWhenOneWasTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "6648bcce-acfc-4e2c-b2ed-1e8ad3284a51");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);
    iox_user_trigger_trigger(m_userTrigger[0U]);

    EXPECT_EQ(iox_ws_timed_wait(
                  m_sut, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
              1U);
}

TEST_F(iox_ws_test, TimedWaitNumberOfTriggeredConditionsIsCorrectWhenMultipleWereTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "3630ed3f-3cbe-4724-b802-c45556e5d7ba");
    for (uint64_t i = 0U; i < 10U; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(iox_ws_timed_wait(
                  m_sut, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
              10U);
}

TEST_F(iox_ws_test, TimedWaitNumberOfTriggeredConditionsIsCorrectWhenAllWereTriggered)
{
    ::testing::Test::RecordProperty("TEST_ID", "028f2b58-42b7-4300-8da9-b1ed036a51d8");
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    EXPECT_EQ(iox_ws_timed_wait(
                  m_sut, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
              MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET);
}

TEST_F(iox_ws_test, SingleTriggerCaseTimedWaitReturnsCorrectTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "6fae144f-056b-4ac6-a849-3cd47135e2db");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 5678U, userTriggerCallback);
    iox_user_trigger_trigger(m_userTrigger[0U]);

    iox_ws_timed_wait(m_sut, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);

    iox_notification_info_t eventInfo = m_eventInfoStorage[0U];

    EXPECT_EQ(iox_notification_info_get_notification_id(eventInfo), 5678U);
    EXPECT_TRUE(iox_notification_info_does_originate_from_user_trigger(eventInfo, m_userTrigger[0U]));
}

TEST_F(iox_ws_test, MultiTriggerCaseTimedWaitReturnsCorrectTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1e8811e-117d-4a73-a46f-7ecbb26b0bf5");
    for (uint64_t i = 0U; i < 8U; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 1337U + i, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_timed_wait(m_sut, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);

    for (uint64_t i = 0U; i < 8U; ++i)
    {
        iox_notification_info_t eventInfo = m_eventInfoStorage[i];
        EXPECT_EQ(iox_notification_info_get_notification_id(eventInfo), 1337U + i);
        EXPECT_TRUE(iox_notification_info_does_originate_from_user_trigger(eventInfo, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, MaxTriggerCaseTimedWaitReturnsCorrectTrigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "343429f9-acba-498f-8b9b-20379960daf6");
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 42U * i + 1U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_timed_wait(m_sut, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);

    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        iox_notification_info_t eventInfo = m_eventInfoStorage[i];
        EXPECT_EQ(iox_notification_info_get_notification_id(eventInfo), 42U * i + 1U);
        EXPECT_TRUE(iox_notification_info_does_originate_from_user_trigger(eventInfo, m_userTrigger[i]));
    }
}

TEST_F(iox_ws_test, MissedElementsIsZeroWhenNothingWasMissed)
{
    ::testing::Test::RecordProperty("TEST_ID", "4080a285-1b64-4be2-9a50-909c102f05cd");
    for (uint64_t i = 0U; i < 12U; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);

    EXPECT_EQ(m_missedElements, 0U);
}

TEST_F(iox_ws_test, MissedElementsIsCorrectWhenSomethingWasMissed)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b0fa82f-3358-4faa-b83e-569e71fad362");
    for (uint64_t i = 0U; i < 12U; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(m_sut, m_eventInfoStorage, 8U, &m_missedElements);

    EXPECT_EQ(m_missedElements, 4U);
}

TEST_F(iox_ws_test, MissedElementsIsCorrectWhenAllWereMissed)
{
    ::testing::Test::RecordProperty("TEST_ID", "502a351f-3388-40a2-bf77-96c019b986f1");
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET; ++i)
    {
        iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[i], 0U, userTriggerCallback);
        iox_user_trigger_trigger(m_userTrigger[i]);
    }

    iox_ws_wait(m_sut, m_eventInfoStorage, 0U, &m_missedElements);

    EXPECT_EQ(m_missedElements, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET);
}

TIMING_TEST_F(iox_ws_test, WaitIsBlockingTillTriggered, Repeat(5), [&] {
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);

    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        iox_ws_wait(m_sut, NULL, 0U, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_user_trigger_trigger(m_userTrigger[0U]);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
});

TIMING_TEST_F(iox_ws_test, WaitIsNonBlockingAfterMarkForDestruction, Repeat(5), [&] {
    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        iox_ws_wait(m_sut, NULL, 0U, &m_missedElements);
        iox_ws_wait(m_sut, NULL, 0U, &m_missedElements);
        iox_ws_wait(m_sut, NULL, 0U, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_ws_mark_for_destruction(m_sut);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
});


TIMING_TEST_F(iox_ws_test, TimedWaitIsBlockingTillTriggered, Repeat(5), [&] {
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);

    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        iox_ws_timed_wait(m_sut, {1000, 1000}, NULL, 0U, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_user_trigger_trigger(m_userTrigger[0U]);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
});

TIMING_TEST_F(iox_ws_test, TimedWaitIsNonBlockingAfterMarkForDestruction, Repeat(5), [&] {
    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        iox_ws_timed_wait(m_sut, {1000, 1000}, NULL, 0U, &m_missedElements);
        iox_ws_timed_wait(m_sut, {1000, 1000}, NULL, 0U, &m_missedElements);
        iox_ws_timed_wait(m_sut, {1000, 1000}, NULL, 0U, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_ws_mark_for_destruction(m_sut);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
});

TIMING_TEST_F(iox_ws_test, TimedWaitBlocksTillTimeout, Repeat(5), [&] {
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);

    std::atomic_bool waitWasCalled{false};
    std::thread t([&] {
        constexpr long hundredMsInNanoSeconds = 100000000L;
        iox_ws_timed_wait(m_sut, {0, hundredMsInNanoSeconds}, NULL, 0U, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(75));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());
    std::this_thread::sleep_for(std::chrono::milliseconds(75));
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());

    t.join();
});

TEST_F(iox_ws_test, SubscriberEventCallbackIsCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "431a93cb-a3ac-4ec8-9f7d-1739cd8bb748");
    iox_ws_attach_subscriber_event(
        m_sut, &m_subscriberVector[0], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, 0, &subscriberCallback);

    m_subscriberVector[0].m_trigger.trigger();
    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(&m_subscriberVector[0]));
}

TEST_F(iox_ws_test, NullptrSubscriberEventCallbackIsCalledWithoutError)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a1ad4d9-cfdb-4e3c-b3b9-82a42e8f2e31");
    iox_ws_attach_subscriber_event(
        m_sut, &m_subscriberVector[0], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, 0, NULL);

    m_subscriberVector[0].m_trigger.trigger();
    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, nullptr);
}

TEST_F(iox_ws_test, SubscriberEventCallbackWithContextDataIsCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ae20a03-cd0a-4e42-bfa0-83ef77dc5ea3");
    uint64_t someContextData = 0U;
    iox_ws_attach_subscriber_event_with_context_data(m_sut,
                                                     &m_subscriberVector[0],
                                                     iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                     0,
                                                     &subscriberCallbackWithContextData,
                                                     &someContextData);

    m_subscriberVector[0].m_trigger.trigger();
    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(&m_subscriberVector[0]));
    EXPECT_THAT(m_contextData, Eq(&someContextData));
}

TEST_F(iox_ws_test, SubscriberStateCallbackIsCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c4443a6-c3f8-441e-baad-ee7058eadbb6");
    iox_ws_attach_subscriber_state(
        m_sut, &m_subscriberVector[0], iox_SubscriberState::SubscriberState_HAS_DATA, 0, &subscriberCallback);

    m_subscriberVector[0].m_portData->m_chunkReceiverData.m_queue.push(iox::mepoo::ShmSafeUnmanagedChunk());
    m_subscriberVector[0].m_trigger.trigger();
    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(&m_subscriberVector[0]));
}

TEST_F(iox_ws_test, SubscriberStateCallbackWithContextDataIsCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cb801d8-6ae3-40d9-ac0c-92bcc65b29b5");
    uint64_t someContextData = 0U;
    iox_ws_attach_subscriber_state_with_context_data(m_sut,
                                                     &m_subscriberVector[0],
                                                     iox_SubscriberState::SubscriberState_HAS_DATA,
                                                     0,
                                                     &subscriberCallbackWithContextData,
                                                     &someContextData);

    m_subscriberVector[0].m_portData->m_chunkReceiverData.m_queue.push(iox::mepoo::ShmSafeUnmanagedChunk());
    m_subscriberVector[0].m_trigger.trigger();
    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(&m_subscriberVector[0]));
    EXPECT_THAT(m_contextData, Eq(&someContextData));
}


TEST_F(iox_ws_test, UserTriggerCallbackIsCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0e72c64-1da7-48c3-8677-79cc0c441b8a");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0], 0, &userTriggerCallback);
    iox_user_trigger_trigger(m_userTrigger[0]);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(m_userTrigger[0]));
}

TEST_F(iox_ws_test, NullptrUserTriggerCallbackIsCalledWithoutError)
{
    ::testing::Test::RecordProperty("TEST_ID", "dcaa2891-b5b3-4c6e-9344-74cef54a6520");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0], 0, NULL);
    iox_user_trigger_trigger(m_userTrigger[0]);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, nullptr);
}

TEST_F(iox_ws_test, UserTriggerCallbackWithContextDataIsCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "901ceeb3-8c3c-4007-a980-f8928044fa83");
    uint64_t someContextData = 0U;
    iox_ws_attach_user_trigger_event_with_context_data(
        m_sut, m_userTrigger[0], 0, &userTriggerCallbackWithContextData, &someContextData);
    iox_user_trigger_trigger(m_userTrigger[0]);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, 1U, &m_missedElements), Eq(1U));
    EXPECT_EQ(m_missedElements, 0U);

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(m_userTrigger[0]));
    EXPECT_THAT(m_contextData, Eq(&someContextData));
}

TEST_F(iox_ws_test, AttachingClientEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8024ff87-166a-4d4c-8cc9-c1f277d30247");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
}

void notifyClient(ClientPortData& portData)
{
    portData.m_connectRequested.store(true);
    portData.m_connectionState = iox::ConnectionState::CONNECTED;
    iox::popo::ChunkQueuePusher<ClientChunkQueueData_t> pusher{&portData.m_chunkReceiverData};
    pusher.push(iox::mepoo::SharedChunk());
    EXPECT_FALSE(portData.m_chunkReceiverData.m_conditionVariableDataPtr->m_semaphore.post().has_error());
}

TEST_F(iox_ws_test, NotifyingClientEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d460351-f1ee-4538-94ee-40a59e82e877");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED, 13137, &clientCallback);

    notifyClient(clientPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(13137));
    EXPECT_THAT(iox_notification_info_get_client_origin(m_eventInfoStorage[0]), Eq(client));
    EXPECT_TRUE(iox_notification_info_does_originate_from_client(m_eventInfoStorage[0], client));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(client)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED);
}

TEST_F(iox_ws_test, NotifyingClientEventWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f1017b4-5edf-44aa-80ab-a4e438816082");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_client_event_with_context_data(
        m_sut, client, ClientEvent_RESPONSE_RECEIVED, 89123, &clientCallbackWithContextData, &clientStorage);

    notifyClient(clientPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_client_origin(m_eventInfoStorage[0]), Eq(client));
    EXPECT_TRUE(iox_notification_info_does_originate_from_client(m_eventInfoStorage[0], client));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(89123));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(client)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&clientStorage)));

    iox_ws_detach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED);
}

TEST_F(iox_ws_test, AttachingClientStateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3eecba2f-07be-4073-8596-a7d2d2966f96");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_client_state(m_sut, client, ClientState_HAS_RESPONSE, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_client_state(m_sut, client, ClientState_HAS_RESPONSE);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
}

TEST_F(iox_ws_test, NotifyingClientStateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9464c3a0-4669-43fe-9edf-0996f744801e");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_client_state(m_sut, client, ClientState_HAS_RESPONSE, 1589123, &clientCallback);

    notifyClient(clientPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(1589123));
    EXPECT_THAT(iox_notification_info_get_client_origin(m_eventInfoStorage[0]), Eq(client));
    EXPECT_TRUE(iox_notification_info_does_originate_from_client(m_eventInfoStorage[0], client));

    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(client)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_client_state(m_sut, client, ClientState_HAS_RESPONSE);
}

TEST_F(iox_ws_test, NotifyingClientStateWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0f25612-dc08-40a9-9b2c-646b2e003e85");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_client_state_with_context_data(
        m_sut, client, ClientState_HAS_RESPONSE, 0, &clientCallbackWithContextData, &clientStorage);

    notifyClient(clientPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_client_origin(m_eventInfoStorage[0]), Eq(client));
    EXPECT_TRUE(iox_notification_info_does_originate_from_client(m_eventInfoStorage[0], client));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(client)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&clientStorage)));

    iox_ws_detach_client_state(m_sut, client, ClientState_HAS_RESPONSE);
}

TEST_F(iox_ws_test, AttachingServerEventWorks)
{
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
}

void notifyServer(ServerPortData& portData)
{
    iox::popo::ChunkQueuePusher<ServerChunkQueueData_t> pusher{&portData.m_chunkReceiverData};
    pusher.push(iox::mepoo::SharedChunk());
    EXPECT_FALSE(portData.m_chunkReceiverData.m_conditionVariableDataPtr->m_semaphore.post().has_error());
}

TEST_F(iox_ws_test, NotifyingServerEventWorks)
{
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED, 1313799, &serverCallback);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(1313799));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED);
}

TEST_F(iox_ws_test, NotifyingServerEventWithContextDataWorks)
{
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_server_event_with_context_data(
        m_sut, server, ServerEvent_REQUEST_RECEIVED, 1319955, &serverCallbackWithContextData, &serverStorage);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(1319955));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&serverStorage)));

    iox_ws_detach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED);
}

TEST_F(iox_ws_test, AttachingServerStateWorks)
{
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_server_state(m_sut, server, ServerState_HAS_REQUEST, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_server_state(m_sut, server, ServerState_HAS_REQUEST);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
}

TEST_F(iox_ws_test, NotifyingServerStateWorks)
{
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_server_state(m_sut, server, ServerState_HAS_REQUEST, 51313799, &serverCallback);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(51313799));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_server_state(m_sut, server, ServerState_HAS_REQUEST);
}

TEST_F(iox_ws_test, NotifyingServerStateWithContextDataWorks)
{
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_server_state_with_context_data(
        m_sut, server, ServerState_HAS_REQUEST, 661319955, &serverCallbackWithContextData, &serverStorage);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(661319955));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&serverStorage)));

    iox_ws_detach_server_state(m_sut, server, ServerState_HAS_REQUEST);
}
} // namespace
