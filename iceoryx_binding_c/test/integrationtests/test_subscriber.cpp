// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::cxx;
using namespace iox::posix;

extern "C" {
#include "iceoryx_binding_c/subscriber.h"
}

#include "test.hpp"

using namespace ::testing;

class binding_c_Subscriber_test : public Test
{
  public:
    const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION{"a", "b", "c"};

  protected:
    binding_c_Subscriber_test()
    {
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~binding_c_Subscriber_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    void Subscribe(SubscriberPortData* ptr)
    {
        uint64_t queueCapacity = MAX_CHUNKS_HELD_PER_RECEIVER;
        Subscriber_subscribe(ptr, queueCapacity);

        SubscriberPortSingleProducer(ptr).getCaProMessage();
        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
        SubscriberPortSingleProducer(ptr).dispatchCaProMessage(caproMessage);
    }

    static constexpr size_t MEMORY_SIZE = 1024 * 1024 * 100;
    uint8_t m_memory[MEMORY_SIZE];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = MAX_CHUNKS_HELD_PER_RECEIVER + 2;
    static constexpr uint32_t CHUNK_SIZE = 128;

    Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolconf;
    MemoryManager m_memoryManager;

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
    iox::popo::SubscriberPortData m_portPtr{
        TEST_SERVICE_DESCRIPTION, "myApp", iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkQueuePusher<SubscriberPortData::ChunkQueueData_t> m_chunkPusher{&m_portPtr.m_chunkReceiverData};
};

TEST_F(binding_c_Subscriber_test, initialStateNotSubscribed)
{
    EXPECT_EQ(Subscriber_getSubscriptionState(&m_portPtr), SubscribeState_NOT_SUBSCRIBED);
}

TEST_F(binding_c_Subscriber_test, offerLeadsToSubscibeRequestedState)
{
    uint64_t queueCapacity = 1u;
    Subscriber_subscribe(&m_portPtr, queueCapacity);

    SubscriberPortSingleProducer(&m_portPtr).getCaProMessage();

    EXPECT_EQ(Subscriber_getSubscriptionState(&m_portPtr), SubscribeState_SUBSCRIBE_REQUESTED);
}

TEST_F(binding_c_Subscriber_test, NACKResponseLeadsToSubscribeWaitForOfferState)
{
    uint64_t queueCapacity = 1u;
    Subscriber_subscribe(&m_portPtr, queueCapacity);

    SubscriberPortSingleProducer(&m_portPtr).getCaProMessage();
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK, TEST_SERVICE_DESCRIPTION);
    SubscriberPortSingleProducer(&m_portPtr).dispatchCaProMessage(caproMessage);

    EXPECT_EQ(Subscriber_getSubscriptionState(&m_portPtr), SubscribeState_WAIT_FOR_OFFER);
}

TEST_F(binding_c_Subscriber_test, ACKResponseLeadsToSubscribedState)
{
    uint64_t queueCapacity = 1u;
    Subscriber_subscribe(&m_portPtr, queueCapacity);

    SubscriberPortSingleProducer(&m_portPtr).getCaProMessage();
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
    SubscriberPortSingleProducer(&m_portPtr).dispatchCaProMessage(caproMessage);

    EXPECT_EQ(Subscriber_getSubscriptionState(&m_portPtr), SubscribeState_SUBSCRIBED);
}

TEST_F(binding_c_Subscriber_test, UnsubscribeLeadsToUnscribeRequestedState)
{
    uint64_t queueCapacity = 1u;
    Subscriber_subscribe(&m_portPtr, queueCapacity);

    SubscriberPortSingleProducer(&m_portPtr).getCaProMessage();
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK, TEST_SERVICE_DESCRIPTION);
    SubscriberPortSingleProducer(&m_portPtr).dispatchCaProMessage(caproMessage);

    Subscriber_unsubscribe(&m_portPtr);

    SubscriberPortSingleProducer(&m_portPtr).getCaProMessage();

    EXPECT_EQ(Subscriber_getSubscriptionState(&m_portPtr), SubscribeState_UNSUBSCRIBE_REQUESTED);
}

TEST_F(binding_c_Subscriber_test, initialStateNoChunksAvailable)
{
    const void* chunk = nullptr;
    EXPECT_EQ(Subscriber_getChunk(&m_portPtr, &chunk), ChunkReceiveError_NO_CHUNK_RECEIVED);
}

TEST_F(binding_c_Subscriber_test, receiveChunkWhenThereIsOne)
{
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(m_memoryManager.getChunk(100));

    const void* chunk = nullptr;
    EXPECT_EQ(Subscriber_getChunk(&m_portPtr, &chunk), ChunkReceiveError_SUCCESS);
}

TEST_F(binding_c_Subscriber_test, DISABLED_receiveChunkWithContent)
{
    this->Subscribe(&m_portPtr);
    struct data_t
    {
        int value;
    };

    auto sharedChunk = m_memoryManager.getChunk(100);
    static_cast<data_t*>(sharedChunk.getPayload())->value = 1234;
    m_chunkPusher.push(m_memoryManager.getChunk(100));

    const void* chunk = nullptr;

    ASSERT_EQ(Subscriber_getChunk(&m_portPtr, &chunk), ChunkReceiveError_SUCCESS);
    EXPECT_THAT(static_cast<const data_t*>(chunk)->value, Eq(1234));
}

TEST_F(binding_c_Subscriber_test, receiveChunkWhenToManyChunksAreHold)
{
    this->Subscribe(&m_portPtr);
    const void* chunk = nullptr;
    for (int i = 0; i < MAX_CHUNKS_HELD_PER_RECEIVER + 1; ++i)
    {
        m_chunkPusher.push(m_memoryManager.getChunk(100));
        Subscriber_getChunk(&m_portPtr, &chunk);
    }

    m_chunkPusher.push(m_memoryManager.getChunk(100));
    EXPECT_EQ(Subscriber_getChunk(&m_portPtr, &chunk), ChunkReceiveError_TOO_MANY_CHUNKS_HELD_IN_PARALLEL);
}

TEST_F(binding_c_Subscriber_test, releaseChunkWorks)
{
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(m_memoryManager.getChunk(100));

    const void* chunk = nullptr;
    Subscriber_getChunk(&m_portPtr, &chunk);

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    Subscriber_releaseChunk(&m_portPtr, chunk);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(binding_c_Subscriber_test, releaseChunkQueuedChunksWorks)
{
    this->Subscribe(&m_portPtr);
    for (int i = 0; i < MAX_CHUNKS_HELD_PER_RECEIVER; ++i)
    {
        m_chunkPusher.push(m_memoryManager.getChunk(100));
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(MAX_CHUNKS_HELD_PER_RECEIVER));
    Subscriber_releaseQueuedChunks(&m_portPtr);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(binding_c_Subscriber_test, initialStateHasNewChunksFalse)
{
    EXPECT_FALSE(Subscriber_hasNewChunks(&m_portPtr));
}

TEST_F(binding_c_Subscriber_test, receivingChunkLeadsToHasNewChunksTrue)
{
    this->Subscribe(&m_portPtr);
    m_chunkPusher.push(m_memoryManager.getChunk(100));

    EXPECT_TRUE(Subscriber_hasNewChunks(&m_portPtr));
}

TEST_F(binding_c_Subscriber_test, initialStateHasNoLostChunks)
{
    EXPECT_FALSE(Subscriber_hasLostChunks(&m_portPtr));
}

TEST_F(binding_c_Subscriber_test, sendingTooMuchLeadsToLostChunks)
{
    this->Subscribe(&m_portPtr);
    for (int i = 0; i < DefaultChunkQueueConfig::MAX_QUEUE_CAPACITY + 1; ++i)
    {
        m_chunkPusher.push(m_memoryManager.getChunk(100));
    }

    EXPECT_TRUE(Subscriber_hasLostChunks(&m_portPtr));
}

