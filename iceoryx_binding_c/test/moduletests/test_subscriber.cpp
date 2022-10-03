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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"
#include "mocks/wait_set_mock.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/wait_set.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox::cxx;
using namespace iox::mepoo;
using namespace iox::posix;

class iox_sub_test : public Test
{
  public:
    const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};

  protected:
    iox_sub_test()
    {
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);
        m_subscriber->m_portData = &m_portPtr;
    }

    void SetUp()
    {
        m_triggerCallbackLatestArgument = nullptr;
    }

    void TearDown()
    {
    }

    void Subscribe(SubscriberPortData* ptr)
    {
        iox_sub_subscribe(m_sut);

        SubscriberPortSingleProducer(ptr).tryGetCaProMessage();
        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
        SubscriberPortSingleProducer(ptr).dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    }

    static void triggerCallback(iox_sub_t sub)
    {
        m_triggerCallbackLatestArgument = sub;
    }

    iox::mepoo::SharedChunk getChunkFromMemoryManager()
    {
        constexpr uint32_t USER_PAYLOAD_SIZE{100U};

        auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
        iox::cxx::Ensures(!chunkSettingsResult.has_error());
        auto& chunkSettings = chunkSettingsResult.value();

        auto getChunkResult = m_memoryManager.getChunk(chunkSettings);
        iox::cxx::Ensures(!getChunkResult.has_error());
        return getChunkResult.value();
    }

    static iox_sub_t m_triggerCallbackLatestArgument;
    static constexpr size_t MEMORY_SIZE = 1024 * 1024 * 100;
    uint8_t m_memory[MEMORY_SIZE];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + 2U;
    static constexpr uint32_t CHUNK_SIZE = 128U;

    Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolconf;
    MemoryManager m_memoryManager;

    iox::popo::SubscriberOptions subscriberOptions{MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY, 0U};
    iox::popo::SubscriberPortData m_portPtr{TEST_SERVICE_DESCRIPTION,
                                            "myApp",
                                            iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
                                            subscriberOptions};
    ChunkQueuePusher<SubscriberPortData::ChunkQueueData_t> m_chunkPusher{&m_portPtr.m_chunkReceiverData};
    std::unique_ptr<cpp2c_Subscriber> m_subscriber{new cpp2c_Subscriber};
    iox_sub_t m_sut = m_subscriber.get();

    ConditionVariableData m_condVar{"myApp"};
    std::unique_ptr<WaitSetMock> m_waitSet{new WaitSetMock(m_condVar)};
};

iox_sub_t iox_sub_test::m_triggerCallbackLatestArgument = nullptr;

TEST_F(iox_sub_test, initSubscriberWithNullptrForStorageReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "2732d00b-5e31-4b31-9170-3dff0d1b693e");
    iox_sub_options_t options;
    iox_sub_options_init(&options);

    EXPECT_EQ(iox_sub_init(nullptr, "all", "glory", "hypnotoad", &options), nullptr);
}

// this crashes if the fixture is used, therefore a test without a fixture
TEST(iox_sub_test_DeathTest, initSubscriberWithNotInitializedPublisherOptionsTerminates)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a33309e-fe21-45f6-815a-eebe0136c572");
    iox_sub_options_t options;
    iox_sub_storage_t storage;

    EXPECT_DEATH({ iox_sub_init(&storage, "a", "b", "c", &options); }, ".*");
}

TEST_F(iox_sub_test, initSubscriberWithDefaultOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "40eaa006-4781-46cd-bde3-40fa7d572f29");
    iox::roudi::RouDiEnvironment roudiEnv;

    iox_runtime_init("hypnotoad");

    iox_sub_options_t options;
    iox_sub_options_init(&options);
    iox_sub_storage_t storage;

    auto sut = iox_sub_init(&storage, "a", "b", "c", &options);
    EXPECT_THAT(sut, Ne(nullptr));
    iox_sub_deinit(sut);
}

TEST_F(iox_sub_test, initialStateNotSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "766cb6ed-02a9-44aa-9cdf-9803d3821b97");
    EXPECT_EQ(iox_sub_get_subscription_state(m_sut), SubscribeState_NOT_SUBSCRIBED);
}

TEST_F(iox_sub_test, offerLeadsToSubscibeRequestedState)
{
    ::testing::Test::RecordProperty("TEST_ID", "4259d716-ba7b-413a-8c2f-d61f81ca9a16");
    iox_sub_subscribe(m_sut);

    SubscriberPortSingleProducer(&m_portPtr).tryGetCaProMessage();

    EXPECT_EQ(iox_sub_get_subscription_state(m_sut), SubscribeState_SUBSCRIBE_REQUESTED);
}

TEST_F(iox_sub_test, NACKResponseLeadsToSubscribeWaitForOfferState)
{
    ::testing::Test::RecordProperty("TEST_ID", "25233413-4ec2-4fad-ac9f-8bde1b83e26f");
    iox_sub_subscribe(m_sut);

    SubscriberPortSingleProducer(&m_portPtr).tryGetCaProMessage();
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK, TEST_SERVICE_DESCRIPTION);
    SubscriberPortSingleProducer(&m_portPtr).dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_EQ(iox_sub_get_subscription_state(m_sut), SubscribeState_WAIT_FOR_OFFER);
}

TEST_F(iox_sub_test, ACKResponseLeadsToSubscribedState)
{
    ::testing::Test::RecordProperty("TEST_ID", "c34b4fb2-4f90-463e-a849-ab4b239af224");
    iox_sub_subscribe(m_sut);

    SubscriberPortSingleProducer(&m_portPtr).tryGetCaProMessage();
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
    SubscriberPortSingleProducer(&m_portPtr).dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_EQ(iox_sub_get_subscription_state(m_sut), SubscribeState_SUBSCRIBED);
}

TEST_F(iox_sub_test, UnsubscribeLeadsToUnscribeRequestedState)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d57dbac-b658-436f-b1ce-37cd318723f6");
    iox_sub_subscribe(m_sut);

    SubscriberPortSingleProducer(&m_portPtr).tryGetCaProMessage();
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
    SubscriberPortSingleProducer(&m_portPtr).dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    iox_sub_unsubscribe(m_sut);

    SubscriberPortSingleProducer(&m_portPtr).tryGetCaProMessage();

    EXPECT_EQ(iox_sub_get_subscription_state(m_sut), SubscribeState_UNSUBSCRIBE_REQUESTED);
}

TEST_F(iox_sub_test, initialStateNoChunksAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "99974a57-6a0d-41eb-87fa-cabedef4f83d");
    const void* chunk = nullptr;
    EXPECT_EQ(iox_sub_take_chunk(m_sut, &chunk), ChunkReceiveResult_NO_CHUNK_AVAILABLE);
}

TEST_F(iox_sub_test, receiveChunkWhenThereIsOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "b42a8093-10a0-4fdb-9a46-4235a08850a3");
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(getChunkFromMemoryManager());

    const void* chunk = nullptr;
    EXPECT_EQ(iox_sub_take_chunk(m_sut, &chunk), ChunkReceiveResult_SUCCESS);
}

TEST_F(iox_sub_test, receiveChunkWithContent)
{
    ::testing::Test::RecordProperty("TEST_ID", "34cbca17-60b9-4f55-b576-87b82f14d764");
    this->Subscribe(&m_portPtr);
    struct data_t
    {
        int value;
    };

    auto sharedChunk = getChunkFromMemoryManager();
    static_cast<data_t*>(sharedChunk.getUserPayload())->value = 1234;
    m_chunkPusher.push(sharedChunk);

    const void* chunk = nullptr;

    ASSERT_EQ(iox_sub_take_chunk(m_sut, &chunk), ChunkReceiveResult_SUCCESS);
    EXPECT_THAT(static_cast<const data_t*>(chunk)->value, Eq(1234));
}

TEST_F(iox_sub_test, constChunkHeaderCanBeObtainedFromChunkAfterTake)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5d2462e-933d-426f-a5d8-40ac9a0d1773");
    this->Subscribe(&m_portPtr);
    auto sharedChunk = getChunkFromMemoryManager();
    m_chunkPusher.push(sharedChunk);

    const void* chunk = nullptr;

    ASSERT_EQ(iox_sub_take_chunk(m_sut, &chunk), ChunkReceiveResult_SUCCESS);
    auto chunkHeader = iox_chunk_header_from_user_payload_const(chunk);
    ASSERT_NE(chunkHeader, nullptr);
    auto userPayloadFromRoundTrip = iox_chunk_header_to_user_payload_const(chunkHeader);
    EXPECT_EQ(userPayloadFromRoundTrip, chunk);
}

TEST_F(iox_sub_test, receiveChunkWhenToManyChunksAreHold)
{
    ::testing::Test::RecordProperty("TEST_ID", "ce2a7a6a-e170-4bc3-b7c0-d50088e2997c");
    this->Subscribe(&m_portPtr);
    const void* chunk = nullptr;
    for (uint64_t i = 0U; i < MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + 1U; ++i)
    {
        m_chunkPusher.push(getChunkFromMemoryManager());
        iox_sub_take_chunk(m_sut, &chunk);
    }

    m_chunkPusher.push(getChunkFromMemoryManager());
    EXPECT_EQ(iox_sub_take_chunk(m_sut, &chunk), ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL);
}

TEST_F(iox_sub_test, releaseChunkWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "53619897-cad8-4377-a877-4ec6971308fa");
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(getChunkFromMemoryManager());

    const void* chunk = nullptr;
    iox_sub_take_chunk(m_sut, &chunk);

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
    iox_sub_release_chunk(m_sut, chunk);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

TEST_F(iox_sub_test, releaseChunkQueuedChunksWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e32b17f-7454-40fe-bf97-f742249fb7de");
    this->Subscribe(&m_portPtr);
    for (uint64_t i = 0U; i < MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY; ++i)
    {
        m_chunkPusher.push(getChunkFromMemoryManager());
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY));
    iox_sub_release_queued_chunks(m_sut);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

TEST_F(iox_sub_test, initialStateHasNewChunksFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "ced8e035-7748-4322-bf2f-517c5d1468d7");
    EXPECT_FALSE(iox_sub_has_chunks(m_sut));
}

TEST_F(iox_sub_test, receivingChunkLeadsToHasNewChunksTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "966ee341-9809-4cde-9bc0-e460f1b7e367");
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(getChunkFromMemoryManager());

    EXPECT_TRUE(iox_sub_has_chunks(m_sut));
}

TEST_F(iox_sub_test, initialStateHasNoLostChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bea8341-92bd-4135-8b3a-c3393780209f");
    EXPECT_FALSE(iox_sub_has_lost_chunks(m_sut));
}

TEST_F(iox_sub_test, sendingTooMuchLeadsToOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "1192619e-c8c6-4da3-adb8-bdf7a0c2db92");
    this->Subscribe(&m_portPtr);
    for (uint64_t i = 0U; i < DefaultChunkQueueConfig::MAX_QUEUE_CAPACITY; ++i)
    {
        EXPECT_TRUE(m_chunkPusher.push(getChunkFromMemoryManager()));
    }
    EXPECT_FALSE(m_chunkPusher.push(getChunkFromMemoryManager()));
    m_chunkPusher.lostAChunk();

    EXPECT_TRUE(iox_sub_has_lost_chunks(m_sut));
}

TEST_F(iox_sub_test, attachingToWaitSetWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "84480013-15d8-44ac-a371-52c79cb861de");
    EXPECT_EQ(iox_ws_attach_subscriber_state(m_waitSet.get(), m_sut, SubscriberState_HAS_DATA, 0U, triggerCallback),
              WaitSetResult_SUCCESS);
    EXPECT_EQ(m_waitSet->size(), 1U);
}

TEST_F(iox_sub_test, attachingToAnotherWaitsetCleansupAtOriginalWaitset)
{
    ::testing::Test::RecordProperty("TEST_ID", "cfc150f3-aa6d-41a1-a68e-7b21711ef5c3");
    WaitSetMock m_waitSet2{m_condVar};
    iox_ws_attach_subscriber_state(m_waitSet.get(), m_sut, SubscriberState_HAS_DATA, 0U, triggerCallback);

    EXPECT_EQ(iox_ws_attach_subscriber_state(&m_waitSet2, m_sut, SubscriberState_HAS_DATA, 0U, triggerCallback),
              WaitSetResult_SUCCESS);
    EXPECT_EQ(m_waitSet->size(), 0U);
    EXPECT_EQ(m_waitSet2.size(), 1U);
}

TEST_F(iox_sub_test, detachingFromWaitSetWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "07a62461-e7ec-4db0-a847-b52ef333701f");
    iox_ws_attach_subscriber_state(m_waitSet.get(), m_sut, SubscriberState_HAS_DATA, 0U, triggerCallback);
    iox_ws_detach_subscriber_state(m_waitSet.get(), m_sut, SubscriberState_HAS_DATA);
    EXPECT_EQ(m_waitSet->size(), 0U);
}

TEST_F(iox_sub_test, hasDataTriggersWaitSetWithCorrectNotificationId)
{
    ::testing::Test::RecordProperty("TEST_ID", "f67bb084-ebe3-4480-8efc-80a902f4fe87");
    iox_ws_attach_subscriber_state(m_waitSet.get(), m_sut, SubscriberState_HAS_DATA, 587U, triggerCallback);
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(getChunkFromMemoryManager());

    auto triggerVector = m_waitSet->wait();

    ASSERT_EQ(triggerVector.size(), 1U);
    EXPECT_EQ(triggerVector[0]->getNotificationId(), 587U);
}

TEST_F(iox_sub_test, hasDataTriggersWaitSetWithCorrectCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d31e5f3-a052-42fb-a521-83a9dd624dbe");
    iox_ws_attach_subscriber_state(m_waitSet.get(), m_sut, SubscriberState_HAS_DATA, 0U, iox_sub_test::triggerCallback);
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(getChunkFromMemoryManager());

    auto triggerVector = m_waitSet->wait();

    ASSERT_EQ(triggerVector.size(), 1U);
    (*triggerVector[0U])();
    EXPECT_EQ(m_triggerCallbackLatestArgument, m_sut);
}

TEST_F(iox_sub_test, deinitSubscriberDetachesTriggerFromWaitSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "93e350fb-5430-43ff-982b-b43c6ae9b890");
    iox::roudi::RouDiEnvironment roudiEnv;
    iox_runtime_init("hypnotoad");

    iox_sub_storage_t storage;
    auto subscriber = iox_sub_init(&storage, "foo", "bar", "baz", nullptr);
    subscriber->m_portData = &m_portPtr;

    iox_ws_attach_subscriber_state(
        m_waitSet.get(), subscriber, SubscriberState_HAS_DATA, 0U, iox_sub_test::triggerCallback);

    iox_sub_deinit(subscriber);

    EXPECT_EQ(m_waitSet->size(), 0U);
}

TEST_F(iox_sub_test, correctServiceDescriptionReturned)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbd568af-49c3-445f-a852-ac0e5d249627");
    auto serviceDescription = iox_sub_get_service_description(m_sut);

    EXPECT_THAT(std::string(serviceDescription.serviceString), Eq("a"));
    EXPECT_THAT(std::string(serviceDescription.instanceString), Eq("b"));
    EXPECT_THAT(std::string(serviceDescription.eventString), Eq("c"));
}

TEST(iox_sub_options_test, subscriberOptionsAreInitializedCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef45f9da-ef2a-4d48-9c05-23cec0837444");
    iox_sub_options_t sut;
    sut.queueCapacity = 37;
    sut.historyRequest = 73;
    sut.nodeName = "Dr.Gonzo";
    sut.subscribeOnCreate = false;
    sut.queueFullPolicy = QueueFullPolicy_BLOCK_PRODUCER;

    SubscriberOptions options;
    // set subscribeOnCreate to the opposite of the expected default to check if it gets overwritten to default
    sut.subscribeOnCreate = (options.subscribeOnCreate == false) ? true : false;

    iox_sub_options_init(&sut);
    EXPECT_EQ(sut.queueCapacity, options.queueCapacity);
    EXPECT_EQ(sut.historyRequest, options.historyRequest);
    EXPECT_EQ(sut.nodeName, nullptr);
    EXPECT_EQ(sut.subscribeOnCreate, options.subscribeOnCreate);
    EXPECT_EQ(sut.queueFullPolicy, cpp2c::queueFullPolicy(options.queueFullPolicy));
    EXPECT_TRUE(iox_sub_options_is_initialized(&sut));
}

TEST(iox_sub_options_test, subscriberOptionsInitializationCheckReturnsTrueAfterDefaultInit)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f5365c9-e86f-4a7f-8b44-ba7c449cd770");
    iox_sub_options_t sut;
    iox_sub_options_init(&sut);
    EXPECT_TRUE(iox_sub_options_is_initialized(&sut));
}

TEST(iox_sub_options_test, subscriberOptionsInitializationCheckReturnsFalseWithoutDefaultInit)
{
    ::testing::Test::RecordProperty("TEST_ID", "059e83e4-b64b-4104-844f-4f69a44cea20");
    iox_sub_options_t sut;
#if (defined(__GNUC__) && __GNUC__ >= 7 && !defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    EXPECT_FALSE(iox_sub_options_is_initialized(&sut));
#if (defined(__GNUC__) && __GNUC__ >= 7 && !defined(__clang__))
#pragma GCC diagnostic pop
#endif
}

TEST(iox_sub_options_test, subscriberOptionInitializationWithNullptrDoesNotCrash)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c8eeb6e-5681-4551-865b-11b6a599edf5");
    EXPECT_EXIT(
        {
            iox_sub_options_init(nullptr);
            exit(0);
        },
        ::testing::ExitedWithCode(0),
        ".*");
}

} // namespace
