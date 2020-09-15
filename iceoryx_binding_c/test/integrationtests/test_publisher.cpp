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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::cxx;
using namespace iox::posix;

extern "C" {
#include "iceoryx_binding_c/publisher.h"
}

#include "test.hpp"

using namespace ::testing;

class c_iox_pub_test : public Test
{
  protected:
    struct DummySample
    {
        uint64_t dummy{42};
    };

    c_iox_pub_test()
    {
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~c_iox_pub_test()
    {
    }

    void SetUp()
    {
        ::testing::internal::CaptureStderr();
    }

    void TearDown()
    {
        std::string output = ::testing::internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    void Subscribe(popo::PublisherPortData* ptr)
    {
        PublisherPortUser userPort(ptr);
        PublisherPortRouDi roudiPort(ptr);

        roudiPort.tryGetCaProMessage();
        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                              iox::capro::ServiceDescription("a", "b", "c"));
        caproMessage.m_chunkQueueData = &m_chunkQueueData;
        auto maybeCaProMessage = roudiPort.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    }

    void Unsubscribe(popo::PublisherPortData* ptr)
    {
        PublisherPortRouDi roudiPort(ptr);

        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                              iox::capro::ServiceDescription("a", "b", "c"));
        caproMessage.m_chunkQueueData = &m_chunkQueueData;
        auto maybeCaProMessage = roudiPort.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    }

    static constexpr size_t MEMORY_SIZE = 1024 * 1024;
    uint8_t m_memory[MEMORY_SIZE];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = 20;
    static constexpr uint32_t CHUNK_SIZE = 128;

    using ChunkQueueData_t = popo::ChunkQueueData<DefaultChunkQueueConfig, popo::ThreadSafePolicy>;
    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

    GenericRAII m_uniqueRouDiId{[] { popo::internal::setUniqueRouDiId(0); },
                                [] { popo::internal::unsetUniqueRouDiId(); }};

    Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolconf;
    MemoryManager m_memoryManager;

    // publisher port w/o history
    PublisherPortData m_publisherPortData{ServiceDescription("a", "b", "c"), "myApp", &m_memoryManager};

    // publisher port w/ history
    PublisherPortData m_publisherPortDataHistory{
        capro::ServiceDescription("x", "y", "z"), "myApp", &m_memoryManager, MAX_PUBLISHER_HISTORY};
};

TEST_F(c_iox_pub_test, initialStateIsNotOffered)
{
    EXPECT_FALSE(iox_pub_is_offered(&m_publisherPortData));
}

TEST_F(c_iox_pub_test, is_offeredAfterOffer)
{
    iox_pub_offer(&m_publisherPortData);
    EXPECT_TRUE(iox_pub_is_offered(&m_publisherPortData));
}

TEST_F(c_iox_pub_test, isNotOfferedAfterStopOffer)
{
    iox_pub_offer(&m_publisherPortData);
    iox_pub_stop_offer(&m_publisherPortData);
    EXPECT_FALSE(iox_pub_is_offered(&m_publisherPortData));
}

TEST_F(c_iox_pub_test, initialStateIsNoSubscribers)
{
    EXPECT_FALSE(iox_pub_has_subscribers(&m_publisherPortData));
}

TEST_F(c_iox_pub_test, has_subscribersAfterSubscription)
{
    iox_pub_offer(&m_publisherPortData);
    this->Subscribe(&m_publisherPortData);
    EXPECT_TRUE(iox_pub_has_subscribers(&m_publisherPortData));
}

TEST_F(c_iox_pub_test, noSubscribersAfterUnsubscribe)
{
    iox_pub_offer(&m_publisherPortData);
    this->Subscribe(&m_publisherPortData);
    this->Unsubscribe(&m_publisherPortData);
    EXPECT_FALSE(iox_pub_has_subscribers(&m_publisherPortData));
}

TEST_F(c_iox_pub_test, allocateChunkForOneChunkIsSuccessful)
{
    void* chunk = nullptr;
    EXPECT_EQ(AllocationResult_SUCCESS, iox_pub_allocate_chunk(&m_publisherPortData, &chunk, sizeof(DummySample)));
}

TEST_F(c_iox_pub_test, allocate_chunkFailsWhenHoldingToManyChunksInParallel)
{
    void* chunk = nullptr;
    for (int i = 0; i < 8 /* ///@todo actually it should be MAX_CHUNKS_HELD_PER_RECEIVER but it does not work*/; ++i)
    {
        EXPECT_EQ(AllocationResult_SUCCESS, iox_pub_allocate_chunk(&m_publisherPortData, &chunk, 100));
    }

    EXPECT_EQ(AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL,
              iox_pub_allocate_chunk(&m_publisherPortData, &chunk, 100));
}

TEST_F(c_iox_pub_test, allocate_chunkFailsWhenOutOfChunks)
{
    std::vector<SharedChunk> chunkBucket;
    while (true)
    {
        auto sharedChunk = m_memoryManager.getChunk(100);
        if (sharedChunk)
            chunkBucket.emplace_back(sharedChunk);
        else
            break;
    }

    void* chunk = nullptr;
    EXPECT_EQ(AllocationResult_RUNNING_OUT_OF_CHUNKS, iox_pub_allocate_chunk(&m_publisherPortData, &chunk, 100));
}

TEST_F(c_iox_pub_test, allocatingChunkAcquiresMemory)
{
    void* chunk = nullptr;
    iox_pub_allocate_chunk(&m_publisherPortData, &chunk, 100);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(c_iox_pub_test, freeingAnAllocatedChunkReleasesTheMemory)
{
    void* chunk = nullptr;
    iox_pub_allocate_chunk(&m_publisherPortData, &chunk, 100);
    iox_pub_free_chunk(&m_publisherPortData, chunk);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(c_iox_pub_test, noLastChunkWhenNothingSent)
{
    EXPECT_EQ(iox_pub_try_get_previous_chunk(&m_publisherPortData), nullptr);
}

TEST_F(c_iox_pub_test, lastChunkAvailableAfterSend)
{
    void* chunk = nullptr;
    iox_pub_allocate_chunk(&m_publisherPortData, &chunk, 100);
    iox_pub_send_chunk(&m_publisherPortData, chunk);

    const void* lastChunk = iox_pub_try_get_previous_chunk(&m_publisherPortData);

    EXPECT_EQ(chunk, lastChunk);
}

TEST_F(c_iox_pub_test, sendDeliversChunk)
{
    void* chunk = nullptr;
    iox_pub_offer(&m_publisherPortData);
    this->Subscribe(&m_publisherPortData);
    iox_pub_allocate_chunk(&m_publisherPortData, &chunk, 100);
    static_cast<DummySample*>(chunk)->dummy = 4711;
    iox_pub_send_chunk(&m_publisherPortData, chunk);

    iox::popo::ChunkQueuePopper<ChunkQueueData_t> m_chunkQueuePopper(&m_chunkQueueData);
    auto maybeSharedChunk = m_chunkQueuePopper.tryPop();

    ASSERT_TRUE(maybeSharedChunk.has_value());
    EXPECT_TRUE(*maybeSharedChunk == chunk);
    EXPECT_TRUE(static_cast<DummySample*>(maybeSharedChunk->getPayload())->dummy == 4711);
}
