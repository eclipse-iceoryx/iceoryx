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

class binding_c_PublisherPort_test : public Test
{
  protected:
    struct DummySample
    {
        uint64_t dummy{42};
    };

    binding_c_PublisherPort_test()
    {
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~binding_c_PublisherPort_test()
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

        roudiPort.getCaProMessage();
        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                              iox::capro::ServiceDescription("a", "b", "c"));
        caproMessage.m_chunkQueueData = &m_chunkQueueData;
        auto maybeCaProMessage = roudiPort.dispatchCaProMessage(caproMessage);
    }

    void Unsubscribe(popo::PublisherPortData* ptr)
    {
        PublisherPortRouDi roudiPort(ptr);

        iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                              iox::capro::ServiceDescription("a", "b", "c"));
        caproMessage.m_chunkQueueData = &m_chunkQueueData;
        auto maybeCaProMessage = roudiPort.dispatchCaProMessage(caproMessage);
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
        capro::ServiceDescription("x", "y", "z"), "myApp", &m_memoryManager, MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR};
};

TEST_F(binding_c_PublisherPort_test, initialStateIsNotOffered)
{
    EXPECT_FALSE(Publisher_isOffered(&m_publisherPortData));
}

TEST_F(binding_c_PublisherPort_test, isOfferedAfterOffer)
{
    Publisher_offer(&m_publisherPortData);
    EXPECT_TRUE(Publisher_isOffered(&m_publisherPortData));
}

TEST_F(binding_c_PublisherPort_test, isNotOfferedAfterStopOffer)
{
    Publisher_offer(&m_publisherPortData);
    Publisher_stopOffer(&m_publisherPortData);
    EXPECT_FALSE(Publisher_isOffered(&m_publisherPortData));
}

TEST_F(binding_c_PublisherPort_test, initialStateIsNoSubscribers)
{
    EXPECT_FALSE(Publisher_hasSubscribers(&m_publisherPortData));
}

TEST_F(binding_c_PublisherPort_test, hasSubscribersAfterSubscription)
{
    Publisher_offer(&m_publisherPortData);
    this->Subscribe(&m_publisherPortData);
    EXPECT_TRUE(Publisher_hasSubscribers(&m_publisherPortData));
}

TEST_F(binding_c_PublisherPort_test, noSubscribersAfterUnsubscribe)
{
    Publisher_offer(&m_publisherPortData);
    this->Subscribe(&m_publisherPortData);
    this->Unsubscribe(&m_publisherPortData);
    EXPECT_FALSE(Publisher_hasSubscribers(&m_publisherPortData));
}

TEST_F(binding_c_PublisherPort_test, allocateChunkForOneChunkIsSuccessful)
{
    void* chunk = nullptr;
    EXPECT_EQ(AllocationError_SUCCESS, Publisher_allocateChunk(&m_publisherPortData, &chunk, sizeof(DummySample)));
}

TEST_F(binding_c_PublisherPort_test, allocateChunkFailsWhenHoldingToManyChunksInParallel)
{
    void* chunk = nullptr;
    for (int i = 0; i < 8 /* ///@todo actually it should be MAX_CHUNKS_HELD_PER_RECEIVER but it does not work*/; ++i)
    {
        EXPECT_EQ(AllocationError_SUCCESS, Publisher_allocateChunk(&m_publisherPortData, &chunk, 100));
    }

    EXPECT_EQ(AllocationError_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL,
              Publisher_allocateChunk(&m_publisherPortData, &chunk, 100));
}

TEST_F(binding_c_PublisherPort_test, allocateChunkFailsWhenOutOfChunks)
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
    EXPECT_EQ(AllocationError_RUNNING_OUT_OF_CHUNKS, Publisher_allocateChunk(&m_publisherPortData, &chunk, 100));
}

TEST_F(binding_c_PublisherPort_test, allocatingChunkAcquiresMemory)
{
    void* chunk = nullptr;
    Publisher_allocateChunk(&m_publisherPortData, &chunk, 100);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(binding_c_PublisherPort_test, freeingAnAllocatedChunkReleasesTheMemory)
{
    void* chunk = nullptr;
    Publisher_allocateChunk(&m_publisherPortData, &chunk, 100);
    Publisher_freeChunk(&m_publisherPortData, chunk);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(binding_c_PublisherPort_test, noLastChunkWhenNothingSent)
{
    EXPECT_EQ(Publisher_getLastChunk(&m_publisherPortData), nullptr);
}

TEST_F(binding_c_PublisherPort_test, lastChunkAvailableAfterSend)
{
    void* chunk = nullptr;
    Publisher_allocateChunk(&m_publisherPortData, &chunk, 100);
    Publisher_sendChunk(&m_publisherPortData, chunk);

    const void* lastChunk = Publisher_getLastChunk(&m_publisherPortData);

    EXPECT_EQ(chunk, lastChunk);
}

TEST_F(binding_c_PublisherPort_test, sendDeliversChunk)
{
    void* chunk = nullptr;
    Publisher_offer(&m_publisherPortData);
    this->Subscribe(&m_publisherPortData);
    Publisher_allocateChunk(&m_publisherPortData, &chunk, 100);
    static_cast<DummySample*>(chunk)->dummy = 4711;
    Publisher_sendChunk(&m_publisherPortData, chunk);

    iox::popo::ChunkQueuePopper<ChunkQueueData_t> m_chunkQueuePopper(&m_chunkQueueData);
    auto maybeSharedChunk = m_chunkQueuePopper.pop();

    ASSERT_TRUE(maybeSharedChunk.has_value());
    EXPECT_TRUE(*maybeSharedChunk == chunk);
    EXPECT_TRUE(static_cast<DummySample*>(maybeSharedChunk->getPayload())->dummy == 4711);
}
