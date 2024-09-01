// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iox/atomic.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "mocks/wait_set_mock.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::runtime;
using namespace iox::testing;

extern "C" {
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

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
                                          roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                          iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
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

        for (auto trigger : m_userTrigger)
        {
            iox_user_trigger_deinit(trigger);
        }
    }

    std::unique_ptr<PoshRuntimeMock> runtimeMock = PoshRuntimeMock::create("rudi_ruessel");
    const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};
    iox::popo::SubscriberOptions m_subscriberOptions{MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY, 0U};
    vector<iox::popo::SubscriberPortData, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1U> m_portDataVector;
    vector<cpp2c_Subscriber, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1U> m_subscriberVector;

    ConditionVariableData m_condVar{"Horscht"};
    WaitSetMock* m_sut = new WaitSetMock{m_condVar};

    iox_user_trigger_storage_t m_userTriggerStorage[MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1];
    vector<iox_user_trigger_t, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET + 1> m_userTrigger;

    iox_notification_info_t m_eventInfoStorage[MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET];
    uint64_t m_missedElements = 0U;
    uint64_t m_numberOfTriggeredConditions = 0U;
    timespec m_timeout{0, 0};

    iox::mepoo::MemoryManager memoryManager;
    ClientPortData clientPortData{{"ServiceA", "InstanceA", "EventA"},
                                  "rudi_ruessel",
                                  roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                  ClientOptions(),
                                  &memoryManager};
    iox_client_storage_t clientStorage;

    ServerPortData serverPortData{{"ServiceA", "InstanceA", "EventA"},
                                  "hypnotoad_loves_iceoryx",
                                  roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                  ServerOptions(),
                                  &memoryManager};
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

void serverCallback(iox::popo::UntypedServer* server)
{
    iox_ws_test::m_callbackOrigin = server;
}

void serverCallbackWithContextData(iox::popo::UntypedServer* server, void* const contextData)
{
    iox_ws_test::m_callbackOrigin = server;
    iox_ws_test::m_contextData = contextData;
}

void serviceDiscoveryCallback(iox_service_discovery_t serviceDiscovery)
{
    iox_ws_test::m_callbackOrigin = serviceDiscovery;
}

void serviceDiscoveryCallbackWithContextData(iox_service_discovery_t serviceDiscovery, void* const contextData)
{
    iox_ws_test::m_callbackOrigin = serviceDiscovery;
    iox_ws_test::m_contextData = contextData;
}

} // namespace

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

TEST_F(iox_ws_test, WaitSetInitWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "46fcbcfe-8f54-4154-8d89-f17811ddce44");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_init(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDeinitWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "981c8d9f-7db1-484f-8301-d39ccc7b2301");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_deinit(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetTimedWaitWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ea969a6-4a0f-41e6-b2d3-532db22bf104");
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_timed_wait(m_sut, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_timed_wait(
                nullptr, m_timeout, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetWaitWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "80fda0a6-4a14-466b-a928-752898dee48d");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_wait(m_sut, NULL, 0U, nullptr); }, iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_wait(nullptr, NULL, 0U, nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetSizeWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f4da6e0-4912-4863-af2f-4c46d9d843fa");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_size(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetCapacityWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1ea6d251-02a4-4a5c-beeb-74a8b70bb7cc");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_capacity(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetMarkForDestructionWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1ca1ea2b-d0e8-4935-ae5c-f47e5f8dc859");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_mark_for_destruction(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachSubscriberStateWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a14d1a2-ac10-4c3c-b7c0-2d1b38e802b2");
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_state(
                nullptr, &m_subscriberVector[0], iox_SubscriberState::SubscriberState_HAS_DATA, 0, &subscriberCallback);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_state(
                m_sut, nullptr, iox_SubscriberState::SubscriberState_HAS_DATA, 0, &subscriberCallback);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachSubscriberStateWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "367e99e5-0288-4f1f-b49f-655980a9a2c4");
    uint64_t someContextData = 0U;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_state_with_context_data(nullptr,
                                                             &m_subscriberVector[0],
                                                             iox_SubscriberState::SubscriberState_HAS_DATA,
                                                             0,
                                                             &subscriberCallbackWithContextData,
                                                             &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_state_with_context_data(m_sut,
                                                             nullptr,
                                                             iox_SubscriberState::SubscriberState_HAS_DATA,
                                                             0,
                                                             &subscriberCallbackWithContextData,
                                                             &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachSubscriberEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "23001644-088f-413b-8f0e-20151638d064");
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_event(nullptr,
                                           &m_subscriberVector[0],
                                           iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                           0,
                                           &subscriberCallback);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_event(
                m_sut, nullptr, iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, 0, &subscriberCallback);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachSubscriberEventWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "0c3f2ce8-9b18-409b-913f-41f7e840df66");
    uint64_t someContextData = 0U;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_event_with_context_data(nullptr,
                                                             &m_subscriberVector[0],
                                                             iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                             0,
                                                             &subscriberCallbackWithContextData,
                                                             &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_subscriber_event_with_context_data(m_sut,
                                                             nullptr,
                                                             iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                             0,
                                                             &subscriberCallbackWithContextData,
                                                             &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachUserTriggerEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6797e1c6-d187-4e42-a2bb-c46efe1536e5");
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_attach_user_trigger_event(nullptr, m_userTrigger[0U], 0U, userTriggerCallback); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_attach_user_trigger_event(m_sut, nullptr, 0U, userTriggerCallback); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachUserTriggerEventWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef97af83-24ea-4734-967a-5dc6ea056b90");
    uint64_t someContextData = 0U;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_user_trigger_event_with_context_data(
                nullptr, m_userTrigger[0], 0, &userTriggerCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_user_trigger_event_with_context_data(
                m_sut, nullptr, 0, &userTriggerCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachSubscriberEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a22da45-f7e6-4873-ae5d-31ea8548db93");
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_detach_subscriber_event(nullptr, &m_subscriberVector[0U], SubscriberEvent_DATA_RECEIVED); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_subscriber_event(m_sut, nullptr, SubscriberEvent_DATA_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachSubscriberStateWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb37cf0f-53df-441c-8a3c-42dc5f2b3182");
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_detach_subscriber_state(nullptr, &m_subscriberVector[0U], SubscriberState_HAS_DATA); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_subscriber_state(m_sut, nullptr, SubscriberState_HAS_DATA); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachUserTriggerEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b34d03e8-4ead-4b84-b4d1-2a7a1a2b2df7");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_user_trigger_event(nullptr, m_userTrigger[0U]); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_user_trigger_event(m_sut, nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachClientEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "7564a392-8720-42b6-a850-b85b363524fd");
    iox_client_t client = iox_client_t();

    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_attach_client_event(nullptr, client, ClientEvent_RESPONSE_RECEIVED, 0, nullptr); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_attach_client_event(m_sut, nullptr, ClientEvent_RESPONSE_RECEIVED, 0, nullptr); },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachClientEventWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e01b407-379e-462d-bdaf-d30894ee4971");
    iox_client_t client = iox_client_t();
    uint64_t someContextData = 0U;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_client_event_with_context_data(nullptr,
                                                         client,
                                                         ClientEvent_RESPONSE_RECEIVED,
                                                         89123,
                                                         &clientCallbackWithContextData,
                                                         &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_client_event_with_context_data(
                m_sut, nullptr, ClientEvent_RESPONSE_RECEIVED, 89123, &clientCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachClientStateWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "255590fc-565e-4cf7-890d-889ea8790439");
    iox_client_t client = iox_client_t();
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_attach_client_state(m_sut, nullptr, ClientState_HAS_RESPONSE, 0, nullptr); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_attach_client_state(nullptr, client, ClientState_HAS_RESPONSE, 0, nullptr); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachClientStateWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0e41734-9f56-4c1c-bf9f-82c50e19a758");
    iox_client_t client = iox_client_t();
    uint64_t someContextData = 0U;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_client_state_with_context_data(
                m_sut, nullptr, ClientState_HAS_RESPONSE, 0, &clientCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_client_state_with_context_data(
                nullptr, client, ClientState_HAS_RESPONSE, 0, &clientCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachClientEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7e243b8-34c4-48e0-8b0a-f988c35835be");
    iox_client_t client = iox_client_t();
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_client_event(nullptr, client, ClientEvent_RESPONSE_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_client_event(m_sut, nullptr, ClientEvent_RESPONSE_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachClientStateWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "cfc25db4-1675-4968-a6fd-eda0a8a9d54a");
    iox_client_t client = iox_client_t();
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_client_state(nullptr, client, ClientState_HAS_RESPONSE); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_client_state(m_sut, nullptr, ClientState_HAS_RESPONSE); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachServerEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "dda23967-4dec-4905-8581-7c126b902b18");
    iox_server_t server = iox_server_t();

    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_attach_server_event(nullptr, server, ServerEvent_REQUEST_RECEIVED, 0, nullptr); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_attach_server_event(m_sut, nullptr, ServerEvent_REQUEST_RECEIVED, 0, nullptr); },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachServerEventWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "91a84993-7a86-4f4f-9f36-8795d100080c");
    iox_server_t server = iox_server_t();
    uint64_t someContextData = 0U;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_server_event_with_context_data(
                nullptr, server, ServerEvent_REQUEST_RECEIVED, 0, serverCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_server_event_with_context_data(
                m_sut, nullptr, ServerEvent_REQUEST_RECEIVED, 0, serverCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachServerStateWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdbe65c0-55a1-44ab-9b5c-e60b31078f5d");
    iox_server_t server = iox_server_t();
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_attach_server_state(nullptr, server, ServerState_HAS_REQUEST, 0, nullptr); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_attach_server_state(m_sut, nullptr, ServerState_HAS_REQUEST, 0, nullptr); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachServerStateWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "70c5f4b1-f9da-4689-8f04-00266d419c5c");
    iox_server_t server = iox_server_t();
    uint64_t someContextData = 0U;
    constexpr uint64_t SOME_EVENT_ID = 912371012314;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_server_state_with_context_data(m_sut,
                                                         server,
                                                         ServerState_HAS_REQUEST,
                                                         SOME_EVENT_ID,
                                                         &serverCallbackWithContextData,
                                                         &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_server_state_with_context_data(m_sut,
                                                         server,
                                                         ServerState_HAS_REQUEST,
                                                         SOME_EVENT_ID,
                                                         &serverCallbackWithContextData,
                                                         &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachServerEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1310f324-abb9-45ce-8ec7-c23fd20a9c20");
    iox_server_t server = iox_server_t();

    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_server_event(nullptr, server, ServerEvent_REQUEST_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_server_event(m_sut, nullptr, ServerEvent_REQUEST_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachServerStateWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "7247d49a-fb2c-4aaa-acf9-ed077a62e7c0");
    iox_server_t server = iox_server_t();

    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_server_state(nullptr, server, ServerState_HAS_REQUEST); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_ws_detach_server_state(m_sut, nullptr, ServerState_HAS_REQUEST); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachServiceDiscoveryEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "117a9521-62e8-4c9c-b797-a015d97f4eef");
    iox_service_discovery_t serviceDiscovery = iox_service_discovery_t();

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_service_discovery_event(
                m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, 0, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_service_discovery_event(
                m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, 0, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetAttachServiceDiscoveryEventWithConextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e19d3a7-6231-408d-a3c0-e378dd754c7d");
    iox_service_discovery_t serviceDiscovery = iox_service_discovery_t();

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_service_discovery_event_with_context_data(
                m_sut, nullptr, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, 0, nullptr, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_attach_service_discovery_event_with_context_data(
                nullptr, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, 0, nullptr, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_ws_test, WaitSetDetachServiceDiscoveryEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c489ea47-6239-4dc7-ba86-c181f034132f");
    iox_service_discovery_t serviceDiscovery = iox_service_discovery_t();

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_ws_detach_service_discovery_event(
                nullptr, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_ws_detach_service_discovery_event(m_sut, nullptr, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED); },
        iox::er::ENFORCE_VIOLATION);
}

TIMING_TEST_F(iox_ws_test, WaitIsBlockingTillTriggered, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "6d8a476d-5bcd-45a5-bbd4-7b3b709ac967");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);

    iox::concurrent::Atomic<bool> waitWasCalled{false};
    std::thread t([&] {
        iox_ws_wait(m_sut, NULL, 0U, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_user_trigger_trigger(m_userTrigger[0U]);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
})

TIMING_TEST_F(iox_ws_test, WaitIsNonBlockingAfterMarkForDestruction, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "4e576665-fda1-4f3c-8588-e9d2cffcb3f4");
    iox::concurrent::Atomic<bool> waitWasCalled{false};
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
})


TIMING_TEST_F(iox_ws_test, TimedWaitIsBlockingTillTriggered, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "e79edc1d-8b8a-4dd0-97ba-e2f41c9c8b31");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);

    iox::concurrent::Atomic<bool> waitWasCalled{false};
    std::thread t([&] {
        iox_ws_timed_wait(m_sut, {1000, 1000}, NULL, 0U, &m_missedElements);
        waitWasCalled.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TIMING_TEST_EXPECT_FALSE(waitWasCalled.load());

    iox_user_trigger_trigger(m_userTrigger[0U]);

    t.join();
    TIMING_TEST_EXPECT_TRUE(waitWasCalled.load());
})

TIMING_TEST_F(iox_ws_test, TimedWaitIsNonBlockingAfterMarkForDestruction, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "a6da4f49-b162-4c70-b0fa-c4ef1f988c57");
    iox::concurrent::Atomic<bool> waitWasCalled{false};
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
})

TIMING_TEST_F(iox_ws_test, TimedWaitBlocksTillTimeout, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "12fbbbc8-80b2-4e7e-af41-1376b2e48f4a");
    iox_ws_attach_user_trigger_event(m_sut, m_userTrigger[0U], 0U, userTriggerCallback);

    iox::concurrent::Atomic<bool> waitWasCalled{false};
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
})

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

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0U));
    iox_ws_attach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1U));

    iox_ws_detach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0U));

    iox_client_deinit(client);
}

void notifyClient(ClientPortData& portData)
{
    portData.m_connectRequested.store(true);
    portData.m_connectionState = iox::ConnectionState::CONNECTED;
    iox::popo::ChunkQueuePusher<ClientChunkQueueData_t> pusher{&portData.m_chunkReceiverData};
    pusher.push(iox::mepoo::SharedChunk());
    EXPECT_FALSE(portData.m_chunkReceiverData.m_conditionVariableDataPtr->m_semaphore->post().has_error());
}

TEST_F(iox_ws_test, NotifyingClientEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d460351-f1ee-4538-94ee-40a59e82e877");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED, 13137, &clientCallback);

    notifyClient(clientPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1U));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(13137));
    EXPECT_THAT(iox_notification_info_get_client_origin(m_eventInfoStorage[0]), Eq(client));
    EXPECT_TRUE(iox_notification_info_does_originate_from_client(m_eventInfoStorage[0], client));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(client)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED);

    iox_client_deinit(client);
}

TEST_F(iox_ws_test, NotifyingClientEventWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f1017b4-5edf-44aa-80ab-a4e438816082");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;
    iox_ws_attach_client_event_with_context_data(
        m_sut, client, ClientEvent_RESPONSE_RECEIVED, 89123, &clientCallbackWithContextData, &someContextData);

    notifyClient(clientPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_client_origin(m_eventInfoStorage[0]), Eq(client));
    EXPECT_TRUE(iox_notification_info_does_originate_from_client(m_eventInfoStorage[0], client));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(89123));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(client)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&someContextData)));

    iox_ws_detach_client_event(m_sut, client, ClientEvent_RESPONSE_RECEIVED);

    iox_client_deinit(client);
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

    iox_client_deinit(client);
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

    iox_client_deinit(client);
}

TEST_F(iox_ws_test, NotifyingClientStateWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0f25612-dc08-40a9-9b2c-646b2e003e85");
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;
    iox_ws_attach_client_state_with_context_data(
        m_sut, client, ClientState_HAS_RESPONSE, 0, &clientCallbackWithContextData, &someContextData);

    notifyClient(clientPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_client_origin(m_eventInfoStorage[0]), Eq(client));
    EXPECT_TRUE(iox_notification_info_does_originate_from_client(m_eventInfoStorage[0], client));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(client)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&someContextData)));

    iox_ws_detach_client_state(m_sut, client, ClientState_HAS_RESPONSE);

    iox_client_deinit(client);
}

//////////////////////
/// BEGIN server tests
//////////////////////

void notifyServer(ServerPortData& portData)
{
    iox::popo::ChunkQueuePusher<ServerChunkQueueData_t> pusher{&portData.m_chunkReceiverData};
    pusher.push(iox::mepoo::SharedChunk());
    EXPECT_FALSE(portData.m_chunkReceiverData.m_conditionVariableDataPtr->m_semaphore->post().has_error());
}

TEST_F(iox_ws_test, AttachingServerEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4e3fe5f-59a4-4cba-851d-77d4951eed72");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));

    iox_server_deinit(server);
}

TEST_F(iox_ws_test, AttachingServerEventWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1254df23-88ff-46fe-ba37-239066599b35");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_server_event_with_context_data(
        m_sut, server, ServerEvent_REQUEST_RECEIVED, 0, serverCallbackWithContextData, &someContextData);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));

    iox_server_deinit(server);
}

TEST_F(iox_ws_test, NotifyingServerEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "003b92c9-3607-4247-8385-3d03085fa574");
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    constexpr uint64_t SOME_EVENT_ID = 1313799;
    iox_ws_attach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED, SOME_EVENT_ID, &serverCallback);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(SOME_EVENT_ID));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED);

    iox_server_deinit(server);
}

TEST_F(iox_ws_test, NotifyingServerEventWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a08df13b-ad50-4753-9a1d-07b306d6f8d2");
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    constexpr uint64_t SOME_EVENT_ID = 5123901293;
    uint64_t someContextData = 0U;
    iox_ws_attach_server_event_with_context_data(
        m_sut, server, ServerEvent_REQUEST_RECEIVED, SOME_EVENT_ID, &serverCallbackWithContextData, &someContextData);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(SOME_EVENT_ID));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&someContextData)));

    iox_ws_detach_server_event(m_sut, server, ServerEvent_REQUEST_RECEIVED);

    iox_server_deinit(server);
}

TEST_F(iox_ws_test, AttachingServerStateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a29c16e-f164-420e-8824-785df24ae8f3");
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_server_state(m_sut, server, ServerState_HAS_REQUEST, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_server_state(m_sut, server, ServerState_HAS_REQUEST);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));

    iox_server_deinit(server);
}

TEST_F(iox_ws_test, NotifyingServerStateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0fc37898-5aa1-416c-ab19-e64a428ad5ba");
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    constexpr uint64_t SOME_EVENT_ID = 9012314;
    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    iox_ws_attach_server_state(m_sut, server, ServerState_HAS_REQUEST, SOME_EVENT_ID, &serverCallback);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(SOME_EVENT_ID));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_server_state(m_sut, server, ServerState_HAS_REQUEST);

    iox_server_deinit(server);
}

TEST_F(iox_ws_test, NotifyingServerStateWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "29905ac6-e146-4bd8-a1d9-a0626ff0ee54");
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    constexpr uint64_t SOME_EVENT_ID = 912371012314;
    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;
    iox_ws_attach_server_state_with_context_data(
        m_sut, server, ServerState_HAS_REQUEST, SOME_EVENT_ID, &serverCallbackWithContextData, &someContextData);

    notifyServer(serverPortData);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(SOME_EVENT_ID));
    EXPECT_THAT(iox_notification_info_get_server_origin(m_eventInfoStorage[0]), Eq(server));
    EXPECT_TRUE(iox_notification_info_does_originate_from_server(m_eventInfoStorage[0], server));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(server)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&someContextData)));

    iox_ws_detach_server_state(m_sut, server, ServerState_HAS_REQUEST);

    iox_server_deinit(server);
}

////////////////////
/// END server tests
////////////////////

TEST_F(iox_ws_test, AttachingServiceDiscoveryEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a8be9cbd-d9b6-45a3-b34f-d58fb864d40d");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_portDataVector[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_service_discovery_event(
        m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, 0, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_service_discovery_event(m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));

    iox_service_discovery_deinit(serviceDiscovery);
}

TEST_F(iox_ws_test, AttachingServiceDiscoveryEventWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "69515627-1590-4616-8502-975cd9256ecf");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_portDataVector[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);

    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));
    iox_ws_attach_service_discovery_event_with_context_data(
        m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, 0, nullptr, nullptr);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(1));

    iox_ws_detach_service_discovery_event(m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);
    EXPECT_THAT(iox_ws_size(m_sut), Eq(0));

    iox_service_discovery_deinit(serviceDiscovery);
}

void notifyServiceDiscovery(SubscriberPortData& portData)
{
    iox::popo::ChunkQueuePusher<SubscriberChunkReceiverData_t> pusher{&portData.m_chunkReceiverData};
    pusher.push(iox::mepoo::SharedChunk());
    EXPECT_FALSE(portData.m_chunkReceiverData.m_conditionVariableDataPtr->m_semaphore->post().has_error());
}

TEST_F(iox_ws_test, NotifyingServiceDiscoveryEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "945dcf94-4679-469f-aa47-1a87d536da72");
    constexpr uint64_t EVENT_ID = 13;
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_portDataVector[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);

    iox_ws_attach_service_discovery_event(
        m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, EVENT_ID, &serviceDiscoveryCallback);

    notifyServiceDiscovery(m_portDataVector[0]);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(EVENT_ID));
    EXPECT_THAT(iox_notification_info_get_service_discovery_origin(m_eventInfoStorage[0]), Eq(serviceDiscovery));
    EXPECT_TRUE(iox_notification_info_does_originate_from_service_discovery(m_eventInfoStorage[0], serviceDiscovery));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(serviceDiscovery)));
    EXPECT_THAT(m_contextData, Eq(nullptr));

    iox_ws_detach_service_discovery_event(m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);

    iox_service_discovery_deinit(serviceDiscovery);
}

TEST_F(iox_ws_test, NotifyingServiceDiscoveryEventWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "510a0351-afeb-4c0f-a4b6-3032f1f3f831");
    constexpr uint64_t EVENT_ID = 31;
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_portDataVector[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);
    uint64_t someContextData = 0U;

    iox_ws_attach_service_discovery_event_with_context_data(m_sut,
                                                            serviceDiscovery,
                                                            ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED,
                                                            EVENT_ID,
                                                            &serviceDiscoveryCallbackWithContextData,
                                                            &someContextData);

    notifyServiceDiscovery(m_portDataVector[0]);

    ASSERT_THAT(iox_ws_wait(m_sut, m_eventInfoStorage, MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET, &m_missedElements),
                Eq(1));
    EXPECT_THAT(iox_notification_info_get_notification_id(m_eventInfoStorage[0]), Eq(EVENT_ID));
    EXPECT_THAT(iox_notification_info_get_service_discovery_origin(m_eventInfoStorage[0]), Eq(serviceDiscovery));
    EXPECT_TRUE(iox_notification_info_does_originate_from_service_discovery(m_eventInfoStorage[0], serviceDiscovery));
    iox_notification_info_call(m_eventInfoStorage[0]);

    EXPECT_THAT(m_callbackOrigin, Eq(static_cast<void*>(serviceDiscovery)));
    EXPECT_THAT(m_contextData, Eq(static_cast<void*>(&someContextData)));

    iox_ws_detach_service_discovery_event(m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);

    iox_service_discovery_deinit(serviceDiscovery);
}

} // namespace
