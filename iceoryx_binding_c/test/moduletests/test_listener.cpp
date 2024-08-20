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
#include "iceoryx_posh/internal/popo/ports/client_server_port_types.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::mepoo;
using namespace iox::runtime;
using namespace iox::testing;

extern "C" {
#include "iceoryx_binding_c/client.h"
#include "iceoryx_binding_c/listener.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
}

#include "test.hpp"

#include <thread>

namespace
{
using namespace ::testing;

iox_user_trigger_t g_userTriggerCallbackArgument = nullptr;
iox_sub_t g_subscriberCallbackArgument = nullptr;
iox_client_t g_clientCallbackArgument = nullptr;
iox_server_t g_serverCallbackArgument = nullptr;
iox_service_discovery_t g_serviceDiscoveryCallbackArgument = nullptr;
void* g_contextData = nullptr;

void userTriggerCallback(iox_user_trigger_t userTrigger)
{
    g_userTriggerCallbackArgument = userTrigger;
}

void userTriggerCallbackWithContextData(iox_user_trigger_t userTrigger, void* const contextData)
{
    g_userTriggerCallbackArgument = userTrigger;
    g_contextData = contextData;
}

void subscriberCallback(iox_sub_t subscriber)
{
    g_subscriberCallbackArgument = subscriber;
}

void subscriberCallbackWithContextData(iox_sub_t subscriber, void* const contextData)
{
    g_subscriberCallbackArgument = subscriber;
    g_contextData = contextData;
}

void clientCallback(iox_client_t client)
{
    g_clientCallbackArgument = client;
}

void clientCallbackWithContextData(iox_client_t client, void* const contextData)
{
    g_clientCallbackArgument = client;
    g_contextData = contextData;
}

void serverCallback(iox_server_t server)
{
    g_serverCallbackArgument = server;
}

void serverCallbackWithContextData(iox_server_t server, void* const contextData)
{
    g_serverCallbackArgument = server;
    g_contextData = contextData;
}

void serviceDiscoveryCallback(iox_service_discovery_t serviceDiscovery)
{
    g_serviceDiscoveryCallbackArgument = serviceDiscovery;
}

void serviceDiscoveryCallbackWithContextData(iox_service_discovery_t serviceDiscovery, void* const contextData)
{
    g_serviceDiscoveryCallbackArgument = serviceDiscovery;
    g_contextData = contextData;
}

class iox_listener_test : public Test
{
  public:
    class TestListener : public Listener
    {
      public:
        TestListener(ConditionVariableData& condVar)
            : Listener(condVar)
        {
        }
    };

    void SetUp() override
    {
        g_userTriggerCallbackArgument = nullptr;
        g_subscriberCallbackArgument = nullptr;
        g_clientCallbackArgument = nullptr;
        g_serviceDiscoveryCallbackArgument = nullptr;
        g_contextData = nullptr;

        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);

        m_subscriberPortData.resize(MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U,
                                    TEST_SERVICE_DESCRIPTION,
                                    "myApp",
                                    roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                    iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
                                    subscriberOptions);
        m_subscriber.resize(MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U);
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U; ++i)
        {
            m_userTrigger.emplace_back(iox_user_trigger_init(&m_userTriggerStorage[i]));
            m_subscriber[i].m_portData = &m_subscriberPortData[i];
            m_chunkPusher.emplace_back(&m_subscriberPortData[i].m_chunkReceiverData);
        }
    }

    void Subscribe(cpp2c_Subscriber& subscriber)
    {
        iox_sub_subscribe(&subscriber);

        SubscriberPortSingleProducer(subscriber.m_portData).tryGetCaProMessage();
        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
        SubscriberPortSingleProducer(subscriber.m_portData).dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    }

    void TearDown() override
    {
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U; ++i)
        {
            iox_user_trigger_deinit(m_userTrigger[i]);
        }
    }

    void AttachAllUserTrigger()
    {
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
        {
            EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[i], &userTriggerCallback),
                        Eq(iox_ListenerResult::ListenerResult_SUCCESS));
            EXPECT_THAT(iox_listener_size(&m_sut), Eq(i + 1U));
        }
    }

    void AttachAllSubscriber()
    {
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
        {
            EXPECT_THAT(
                iox_listener_attach_subscriber_event(
                    &m_sut, &m_subscriber[i], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, &subscriberCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
            EXPECT_THAT(iox_listener_size(&m_sut), Eq(i + 1U));
        }
    }

    ConditionVariableData m_condVar{"hypnotoadKnueppeltRetour"};
    TestListener m_sut{m_condVar};
    std::unique_ptr<PoshRuntimeMock> runtimeMock = PoshRuntimeMock::create("long_live_lord_buckethead");

    iox_user_trigger_storage_t m_userTriggerStorage[MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U];
    vector<iox_user_trigger_t, MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1U> m_userTrigger;

    static constexpr uint32_t NUM_CHUNKS_IN_POOL = MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + 2U;
    static constexpr uint64_t CHUNK_SIZE = 128U;
    static constexpr uint64_t MEMORY_SIZE = 1024U * 1024U * 100U;
    uint8_t m_memory[MEMORY_SIZE];
    BumpAllocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolconf;
    MemoryManager m_memoryManager;

    const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};

    iox::popo::SubscriberOptions subscriberOptions{MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY, 0U};

    ServerPortData serverPortData{{"ServiceA", "InstanceA", "EventA"},
                                  "der_wilde_bert",
                                  roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                  ServerOptions(),
                                  &m_memoryManager};
    ClientPortData clientPortData{{"ServiceA", "InstanceA", "EventA"},
                                  "rudi_ruessel",
                                  roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                  ClientOptions(),
                                  &m_memoryManager};
    vector<iox::popo::SubscriberPortData, MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1> m_subscriberPortData;
    vector<cpp2c_Subscriber, MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1> m_subscriber;
    vector<ChunkQueuePusher<SubscriberPortData::ChunkQueueData_t>, MAX_NUMBER_OF_EVENTS_PER_LISTENER + 1> m_chunkPusher;
    static constexpr std::chrono::milliseconds TIMEOUT = std::chrono::milliseconds(100);
};
constexpr std::chrono::milliseconds iox_listener_test::TIMEOUT;

TEST_F(iox_listener_test, InitListenerWithNullptrForStorageReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee5f8898-c178-4546-9bb4-6e3329f1b632");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_init(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, CapacityIsCorrect)
{
    ::testing::Test::RecordProperty("TEST_ID", "0fa5465e-f757-4b04-abc2-ca6f346d66ec");
    EXPECT_THAT(iox_listener_capacity(&m_sut), Eq(MAX_NUMBER_OF_EVENTS_PER_LISTENER));
}

TEST_F(iox_listener_test, SizeIsZeroWhenCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab82bb49-476d-4edc-8d39-25656b1e5ca8");
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
}

TEST_F(iox_listener_test, SizeIsOneWhenOneClassIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "6b5a685d-eb8b-4efa-9e4b-6bc764c156b9");
    EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));
}

TEST_F(iox_listener_test, SizeEqualsCapacityWhenMaximumIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d1c1fa4-1e47-4b96-99d1-5766ef068a74");
    AttachAllUserTrigger();
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(iox_listener_capacity(&m_sut)));
}

TEST_F(iox_listener_test, SizeDecreasesWhenUserTriggersAreDetached)
{
    ::testing::Test::RecordProperty("TEST_ID", "907f0ea4-6ad9-4744-8f38-b1619fa62cfc");
    AttachAllUserTrigger();

    for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        iox_listener_detach_user_trigger_event(&m_sut, m_userTrigger[i]);
        EXPECT_THAT(iox_listener_size(&m_sut), Eq(iox_listener_capacity(&m_sut) - i - 1U));
    }
}

TEST_F(iox_listener_test, FullListenerReturnsLISTENER_FULLWhenAnotherUserTriggerIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "9392e16c-52e9-4754-b332-12da0022b346");
    AttachAllUserTrigger();

    EXPECT_THAT(iox_listener_attach_user_trigger_event(
                    &m_sut, m_userTrigger[MAX_NUMBER_OF_EVENTS_PER_LISTENER], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_LISTENER_FULL));
}

TEST_F(iox_listener_test, AttachingTheSameUserTriggerTwiceLeadsToEVENT_ALREADY_ATTACHED)
{
    ::testing::Test::RecordProperty("TEST_ID", "f3e25bef-7f69-4cdc-a70c-98f24bccb177");
    EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
    EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_EVENT_ALREADY_ATTACHED));
}

TEST_F(iox_listener_test, AttachingSubscriberEventWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3863617a-8483-4f38-afd4-25436158bc45");
    EXPECT_THAT(iox_listener_attach_subscriber_event(
                    &m_sut, &m_subscriber[0U], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, &subscriberCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
}

TEST_F(iox_listener_test, AttachingSubscriberEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "db39c3ef-1518-4769-942e-642d0f58abdb");
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_subscriber_event(
                &m_sut, &m_subscriber[0U], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_subscriber_event(
                nullptr, &m_subscriber[0U], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, &subscriberCallback);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_subscriber_event(
                &m_sut, nullptr, iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, &subscriberCallback);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, AttachingUserTriggerEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "990e8f3c-36f0-4687-8246-ce8a02f969ae");
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_listener_attach_user_trigger_event(nullptr, m_userTrigger[0U], &userTriggerCallback); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_attach_user_trigger_event(&m_sut, nullptr, &userTriggerCallback); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], nullptr); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, AttachingUserTriggerEventWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b89957be-19fc-4830-810c-50902061e03d");
    int someContextData;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_user_trigger_event_with_context_data(
                nullptr, m_userTrigger[0U], &userTriggerCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_user_trigger_event_with_context_data(
                &m_sut, nullptr, &userTriggerCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_user_trigger_event_with_context_data(
                &m_sut, m_userTrigger[0U], nullptr, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_user_trigger_event_with_context_data(
                &m_sut, m_userTrigger[0U], &userTriggerCallbackWithContextData, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, AttachingSubscriberEventWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "99885218-09f1-4a30-9fb7-287f4b752dd4");
    int someContextData;
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_subscriber_event_with_context_data(&m_sut,
                                                                   &m_subscriber[0U],
                                                                   iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                                   nullptr,
                                                                   &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_subscriber_event_with_context_data(nullptr,
                                                                   &m_subscriber[0U],
                                                                   iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                                   &subscriberCallbackWithContextData,
                                                                   &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_subscriber_event_with_context_data(&m_sut,
                                                                   nullptr,
                                                                   iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                                   &subscriberCallbackWithContextData,
                                                                   &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_subscriber_event_with_context_data(&m_sut,
                                                                   &m_subscriber[0U],
                                                                   iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                                   &subscriberCallbackWithContextData,
                                                                   nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, AttachingSubscriberTillListenerFullWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "11636d73-8850-416e-ac84-357d8bf73477");
    AttachAllSubscriber();
}

TEST_F(iox_listener_test, FullListenerReturnsLISTENER_FULLWhenAnotherSubscriberIsAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec1d34c1-d5bb-4648-845b-4a89e5ef2c74");
    AttachAllSubscriber();
    EXPECT_THAT(iox_listener_attach_subscriber_event(&m_sut,
                                                     &m_subscriber[MAX_NUMBER_OF_EVENTS_PER_LISTENER],
                                                     iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                     &subscriberCallback),
                Eq(iox_ListenerResult::ListenerResult_LISTENER_FULL));
}

TEST_F(iox_listener_test, DetachingSubscriberTillListenerEmptyWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "af90d50f-1b01-46e5-8a82-3c8a5e1d00e8");
    AttachAllSubscriber();
    for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
    {
        iox_listener_detach_subscriber_event(
            &m_sut, &m_subscriber[i], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED);
        EXPECT_THAT(iox_listener_size(&m_sut), Eq(MAX_NUMBER_OF_EVENTS_PER_LISTENER - i - 1U));
    }
}

TEST_F(iox_listener_test, DetachingSubscriberEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1121040-1d32-4e94-a993-5f6885bd0ed0");
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_detach_subscriber_event(
                nullptr, &m_subscriber[0U], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_detach_subscriber_event(&m_sut, nullptr, iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED);
        },
        iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, DetachingUserTriggerEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8ec0c93-1c4e-4e30-a15a-89eeaeb2edda");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_detach_user_trigger_event(nullptr, m_userTrigger[0U]); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_detach_user_trigger_event(&m_sut, nullptr); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, CheckListenerSizeWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "44a7b724-e201-4d4e-ad19-daa5eecc3f2a");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_size(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, CheckListenerCapacityWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b09e9e38-9b4e-4998-8966-6685e15492f9");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_capacity(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_listener_test, AttachingSubscriberEventTwiceFailsWithEVENT_ALREADY_ATTACHED)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d84fd37-b170-4028-99f2-848f41de0b35");
    EXPECT_THAT(iox_listener_attach_subscriber_event(
                    &m_sut, &m_subscriber[0U], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, &subscriberCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
    EXPECT_THAT(iox_listener_attach_subscriber_event(
                    &m_sut, &m_subscriber[0U], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, &subscriberCallback),
                Eq(iox_ListenerResult::ListenerResult_EVENT_ALREADY_ATTACHED));
}

TIMING_TEST_F(iox_listener_test, UserTriggerCallbackIsCalledWhenTriggered, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "9cf3ca3b-fc51-4d64-8871-6e4c3d51ac49");
    EXPECT_THAT(iox_listener_attach_user_trigger_event(&m_sut, m_userTrigger[0U], &userTriggerCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
    iox_user_trigger_trigger(m_userTrigger[0U]);
    std::this_thread::sleep_for(TIMEOUT);
    EXPECT_THAT(g_userTriggerCallbackArgument, Eq(m_userTrigger[0U]));
})

TIMING_TEST_F(iox_listener_test, UserTriggerCallbackWithContextDataIsCalledWhenTriggered, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "55c61dc2-4aa3-4c26-b14a-5c137ad1f20e");
    int someContextData;
    EXPECT_THAT(iox_listener_attach_user_trigger_event_with_context_data(
                    &m_sut, m_userTrigger[0U], &userTriggerCallbackWithContextData, &someContextData),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));
    iox_user_trigger_trigger(m_userTrigger[0U]);
    std::this_thread::sleep_for(TIMEOUT);
    EXPECT_THAT(g_userTriggerCallbackArgument, Eq(m_userTrigger[0U]));
    EXPECT_THAT(g_contextData, Eq(static_cast<void*>(&someContextData)));
})

TIMING_TEST_F(iox_listener_test, SubscriberCallbackIsCalledSampleIsReceived, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "541b118b-4a7a-4ea5-aa5f-8e922dfd4aa0");
    EXPECT_THAT(iox_listener_attach_subscriber_event(
                    &m_sut, &m_subscriber[0U], iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED, &subscriberCallback),
                Eq(iox_ListenerResult::ListenerResult_SUCCESS));

    Subscribe(m_subscriber[0U]);
    constexpr uint64_t USER_PAYLOAD_SIZE{100U};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    m_memoryManager.getChunk(chunkSettings)
        .and_then([&](auto& chunk) { m_chunkPusher[0U].push(chunk); })
        .or_else([](auto& error) { GTEST_FAIL() << "getChunk failed with: " << error; });

    std::this_thread::sleep_for(TIMEOUT);
    EXPECT_THAT(g_subscriberCallbackArgument, Eq(&m_subscriber[0U]));
})

TIMING_TEST_F(iox_listener_test, SubscriberCallbackWithContextDataIsCalledSampleIsReceived, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "a51ff99b-f1df-458d-b3c0-a97ddfacf4ec");
    int someContextData;
    EXPECT_THAT(
        iox_listener_attach_subscriber_event_with_context_data(&m_sut,
                                                               &m_subscriber[0U],
                                                               iox_SubscriberEvent::SubscriberEvent_DATA_RECEIVED,
                                                               &subscriberCallbackWithContextData,
                                                               &someContextData),
        Eq(iox_ListenerResult::ListenerResult_SUCCESS));

    Subscribe(m_subscriber[0U]);
    constexpr uint64_t USER_PAYLOAD_SIZE{100U};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    m_memoryManager.getChunk(chunkSettings)
        .and_then([&](auto& chunk) { m_chunkPusher[0U].push(chunk); })
        .or_else([](auto& error) { GTEST_FAIL() << "getChunk failed with: " << error; });

    std::this_thread::sleep_for(TIMEOUT);
    EXPECT_THAT(g_subscriberCallbackArgument, Eq(&m_subscriber[0U]));
    EXPECT_THAT(g_contextData, Eq(static_cast<void*>(&someContextData)));
})

TEST_F(iox_listener_test, AttachingClientWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d0513caa-78c0-4be4-a140-1468c1c4e6e7");
    iox_client_storage_t clientStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED, &clientCallback);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    iox_listener_detach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));

    iox_client_deinit(client);
}

TEST_F(iox_listener_test, AttachingClientWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a94e0e0a-45df-41d2-bcc6-7fad44364cda");
    iox_client_storage_t clientStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_listener_attach_client_event(nullptr, client, ClientEvent_RESPONSE_RECEIVED, &clientCallback); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_listener_attach_client_event(&m_sut, nullptr, ClientEvent_RESPONSE_RECEIVED, &clientCallback); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_listener_attach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED, nullptr); },
        iox::er::ENFORCE_VIOLATION);
    iox_client_deinit(client);
}

TEST_F(iox_listener_test, AttachingClientWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "99f41b1c-4fa7-4048-ae47-86ad466c30a0");
    iox_client_storage_t clientStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_client_event_with_context_data(
                nullptr, client, ClientEvent_RESPONSE_RECEIVED, &clientCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_client_event_with_context_data(
                &m_sut, nullptr, ClientEvent_RESPONSE_RECEIVED, &clientCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_client_event_with_context_data(
                &m_sut, client, ClientEvent_RESPONSE_RECEIVED, nullptr, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_client_event_with_context_data(
                &m_sut, client, ClientEvent_RESPONSE_RECEIVED, &clientCallbackWithContextData, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
    iox_client_deinit(client);
}

TEST_F(iox_listener_test, DettachingClientWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "decb7c56-d5f0-43a2-852f-825887770c2c");
    iox_client_storage_t clientStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED, &clientCallback);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    iox_listener_detach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED);

    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_detach_client_event(nullptr, client, ClientEvent_RESPONSE_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_detach_client_event(&m_sut, nullptr, ClientEvent_RESPONSE_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
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

TIMING_TEST_F(iox_listener_test, NotifyingClientEventWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "1f857df5-47d9-4116-83fd-acc9df4c3d6e");
    iox_client_storage_t clientStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    iox_listener_attach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED, &clientCallback);

    notifyClient(clientPortData);
    std::this_thread::sleep_for(TIMEOUT);
    TIMING_TEST_EXPECT_TRUE(g_clientCallbackArgument == client);

    iox_listener_detach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED);

    iox_client_deinit(client);
})

TIMING_TEST_F(iox_listener_test, NotifyingClientEventWithContextDataWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "64178bc6-ec8f-4504-aceb-6a32ee568ab8");
    iox_client_storage_t clientStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientPortData));

    iox_client_t client = iox_client_init(&clientStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;

    iox_listener_attach_client_event_with_context_data(
        &m_sut, client, ClientEvent_RESPONSE_RECEIVED, &clientCallbackWithContextData, &someContextData);

    notifyClient(clientPortData);
    std::this_thread::sleep_for(TIMEOUT);
    TIMING_TEST_EXPECT_TRUE(g_clientCallbackArgument == client);
    TIMING_TEST_EXPECT_TRUE(g_contextData == static_cast<void*>(&someContextData));

    iox_listener_detach_client_event(&m_sut, client, ClientEvent_RESPONSE_RECEIVED);

    iox_client_deinit(client);
})

//////////////////////
/// BEGIN server tests
//////////////////////

void notifyServer(ServerPortData& portData)
{
    iox::popo::ChunkQueuePusher<ServerChunkQueueData_t> pusher{&portData.m_chunkReceiverData};
    pusher.push(iox::mepoo::SharedChunk());
    EXPECT_FALSE(portData.m_chunkReceiverData.m_conditionVariableDataPtr->m_semaphore->post().has_error());
}

TEST_F(iox_listener_test, AttachingServerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "80eeda1a-f147-427b-9d2e-1510b96b043e");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED, &serverCallback);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    iox_listener_detach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));

    iox_server_deinit(server);
}

TEST_F(iox_listener_test, AttachingServerWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ee63c00-8028-414c-a0a7-98c7c2c80b68");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_server_event_with_context_data(
        &m_sut, server, ServerEvent_REQUEST_RECEIVED, &serverCallbackWithContextData, &someContextData);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    iox_listener_detach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));

    iox_server_deinit(server);
}

TEST_F(iox_listener_test, AttachingServerEventWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6459238c-3819-4e26-b66d-b250a08c0776");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));

    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_listener_attach_server_event(nullptr, server, ServerEvent_REQUEST_RECEIVED, &serverCallback); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_listener_attach_server_event(&m_sut, nullptr, ServerEvent_REQUEST_RECEIVED, &serverCallback); },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] { iox_listener_attach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED, nullptr); },
        iox::er::ENFORCE_VIOLATION);
    iox_server_deinit(server);
}

TEST_F(iox_listener_test, AttachingServerWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb56e5aa-1bed-49c0-a942-23cff1f1c8d6");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_server_event_with_context_data(
                nullptr, server, ServerEvent_REQUEST_RECEIVED, &serverCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_server_event_with_context_data(
                &m_sut, nullptr, ServerEvent_REQUEST_RECEIVED, &serverCallbackWithContextData, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_server_event_with_context_data(
                &m_sut, server, ServerEvent_REQUEST_RECEIVED, nullptr, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_server_event_with_context_data(
                &m_sut, server, ServerEvent_REQUEST_RECEIVED, &serverCallbackWithContextData, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
    iox_server_deinit(server);
}

TEST_F(iox_listener_test, DettachingListenerServerWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1b107418-e907-456d-a939-94b5c73e9ab7");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED, &serverCallback);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_detach_server_event(nullptr, server, ServerEvent_REQUEST_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_listener_detach_server_event(&m_sut, nullptr, ServerEvent_REQUEST_RECEIVED); },
                             iox::er::ENFORCE_VIOLATION);
    iox_server_deinit(server);
}

TIMING_TEST_F(iox_listener_test, NotifyingServerEventWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "0b8c6951-7682-47d2-9c2d-3d43689af144");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);

    iox_listener_attach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED, &serverCallback);

    notifyServer(serverPortData);
    std::this_thread::sleep_for(TIMEOUT);
    TIMING_TEST_EXPECT_TRUE(g_serverCallbackArgument == server);

    iox_listener_detach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED);

    iox_server_deinit(server);
})

TIMING_TEST_F(iox_listener_test, NotifyingServerEventWithContextDataWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "ae71bd2c-474b-4f39-b2d8-7959d26e7d90");
    iox_server_storage_t serverStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverPortData));

    iox_server_t server = iox_server_init(&serverStorage, "ServiceA", "InstanceA", "EventA", nullptr);
    uint64_t someContextData = 0U;

    iox_listener_attach_server_event_with_context_data(
        &m_sut, server, ServerEvent_REQUEST_RECEIVED, &serverCallbackWithContextData, &someContextData);

    notifyServer(serverPortData);
    std::this_thread::sleep_for(TIMEOUT);
    TIMING_TEST_EXPECT_TRUE(g_serverCallbackArgument == server);
    TIMING_TEST_EXPECT_TRUE(g_contextData == static_cast<void*>(&someContextData));

    iox_listener_detach_server_event(&m_sut, server, ServerEvent_REQUEST_RECEIVED);

    iox_server_deinit(server);
})

//////////////////////
/// END server tests
//////////////////////

TEST_F(iox_listener_test, AttachingServiceDiscoveryWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "75fd4e6f-ee2f-4e28-a2d8-8a0f01dbd91c");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_subscriberPortData[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_service_discovery_event(
        &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, &serviceDiscoveryCallback);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    iox_listener_detach_service_discovery_event(
        &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));

    iox_service_discovery_deinit(serviceDiscovery);
}

TEST_F(iox_listener_test, AttachingServiceDiscoveryWithContextDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d7cbe60-bda1-4191-b2d5-d67c47312a48");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_subscriberPortData[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);
    uint64_t someContextData = 0U;

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_service_discovery_event_with_context_data(&m_sut,
                                                                  serviceDiscovery,
                                                                  ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED,
                                                                  &serviceDiscoveryCallbackWithContextData,
                                                                  &someContextData);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    iox_listener_detach_service_discovery_event(
        &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));

    iox_service_discovery_deinit(serviceDiscovery);
}

TEST_F(iox_listener_test, AttachingServiceDiscoveryWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6015de0d-6197-4f53-b9c2-f7f8be9f4b7e");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_subscriberPortData[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_service_discovery_event(
                nullptr, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, &serviceDiscoveryCallback);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_service_discovery_event(
                &m_sut, nullptr, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, &serviceDiscoveryCallback);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_service_discovery_event(
                &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, nullptr);
        },
        iox::er::ENFORCE_VIOLATION);
    iox_service_discovery_deinit(serviceDiscovery);
}

TEST_F(iox_listener_test, AttachingServiceDiscoveryWithContextDataWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f3d6be8-df3c-40a5-ac3d-b88189afbd30");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_subscriberPortData[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);
    uint64_t someContextData = 0U;

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_service_discovery_event_with_context_data(&m_sut,
                                                                  serviceDiscovery,
                                                                  ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED,
                                                                  &serviceDiscoveryCallbackWithContextData,
                                                                  &someContextData);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_service_discovery_event_with_context_data(
                nullptr,
                serviceDiscovery,
                ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED,
                &serviceDiscoveryCallbackWithContextData,
                &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_service_discovery_event_with_context_data(
                &m_sut,
                nullptr,
                ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED,
                &serviceDiscoveryCallbackWithContextData,
                &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_service_discovery_event_with_context_data(
                &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, nullptr, &someContextData);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_attach_service_discovery_event_with_context_data(
                &m_sut,
                serviceDiscovery,
                ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED,
                &serviceDiscoveryCallbackWithContextData,
                nullptr);
        },
        iox::er::ENFORCE_VIOLATION);

    iox_service_discovery_deinit(serviceDiscovery);
}

TEST_F(iox_listener_test, DettachingListenerServiceDiscoveryWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb746406-bb83-4ddb-b943-d8f986369ab1");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_subscriberPortData[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);

    EXPECT_THAT(iox_listener_size(&m_sut), Eq(0U));
    iox_listener_attach_service_discovery_event(
        &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, &serviceDiscoveryCallback);
    EXPECT_THAT(iox_listener_size(&m_sut), Eq(1U));

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_detach_service_discovery_event(
                nullptr, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);
        },
        iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            iox_listener_detach_service_discovery_event(
                &m_sut, nullptr, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);
        },
        iox::er::ENFORCE_VIOLATION);
    iox_service_discovery_deinit(serviceDiscovery);
}

void notifyServiceDiscovery(SubscriberPortData& portData)
{
    ConditionNotifier(*portData.m_chunkReceiverData.m_conditionVariableDataPtr.get(), 0).notify();
}

TIMING_TEST_F(iox_listener_test, NotifyingServiceDiscoveryEventWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "538a50bc-60c8-4485-b70e-59d0c53f618b");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_subscriberPortData[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);

    iox_listener_attach_service_discovery_event(
        &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED, &serviceDiscoveryCallback);

    notifyServiceDiscovery(m_subscriberPortData[0]);
    std::this_thread::sleep_for(TIMEOUT);
    TIMING_TEST_EXPECT_TRUE(g_serviceDiscoveryCallbackArgument == serviceDiscovery);

    iox_listener_detach_service_discovery_event(
        &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);

    iox_service_discovery_deinit(serviceDiscovery);
})

TIMING_TEST_F(iox_listener_test, NotifyingServiceDiscoveryEventWithContextDataWorks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "257c27a5-95c6-489d-919f-125471b399e8");
    iox_service_discovery_storage_t serviceDiscoveryStorage;
    EXPECT_CALL(*runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&m_subscriberPortData[0]));

    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&serviceDiscoveryStorage);
    uint64_t someContextData = 0U;

    iox_listener_attach_service_discovery_event_with_context_data(&m_sut,
                                                                  serviceDiscovery,
                                                                  ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED,
                                                                  &serviceDiscoveryCallbackWithContextData,
                                                                  &someContextData);

    notifyServiceDiscovery(m_subscriberPortData[0]);
    std::this_thread::sleep_for(TIMEOUT);
    TIMING_TEST_EXPECT_TRUE(g_serviceDiscoveryCallbackArgument == serviceDiscovery);
    TIMING_TEST_EXPECT_TRUE(g_contextData == static_cast<void*>(&someContextData));

    iox_listener_detach_service_discovery_event(
        &m_sut, serviceDiscovery, ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED);

    iox_service_discovery_deinit(serviceDiscovery);
})

} // namespace
