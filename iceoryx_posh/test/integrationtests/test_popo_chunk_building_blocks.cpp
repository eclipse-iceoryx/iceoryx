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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "test.hpp"

#include <chrono>
#include <stdlib.h>
#include <thread>

using namespace ::testing;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;
using namespace iox::posix;
using ::testing::Return;

struct DummySample
{
    uint64_t m_dummy{42};
};

static constexpr uint32_t NUM_CHUNKS_IN_POOL = 3 * iox::MAX_RECEIVER_QUEUE_CAPACITY;
static constexpr uint32_t SMALL_CHUNK = 128;
static constexpr uint32_t CHUNK_META_INFO_SIZE = 256;
static constexpr size_t MEMORY_SIZE = NUM_CHUNKS_IN_POOL * (SMALL_CHUNK + CHUNK_META_INFO_SIZE);
alignas(64) uint8_t g_memory[MEMORY_SIZE];
static constexpr uint32_t ITERATIONS = 10000;
static constexpr uint32_t MAX_NUMBER_QUEUES = 128;

struct ChunkDistributorConfig
{
    static constexpr uint32_t MAX_QUEUES = MAX_NUMBER_QUEUES;
    static constexpr uint64_t MAX_HISTORY_CAPACITY = iox::MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR;
};

struct ChunkQueueConfig
{
    static constexpr uint32_t MAX_QUEUE_CAPACITY = NUM_CHUNKS_IN_POOL;
};

using ChunkQueueData_t = ChunkQueueData<ChunkQueueConfig>;
using ChunkDistributorData_t =
    ChunkDistributorData<ChunkDistributorConfig, ThreadSafePolicy, ChunkQueuePusher<ChunkQueueData_t>>;
using ChunkDistributor_t = ChunkDistributor<ChunkDistributorData_t>;
using ChunkQueuePopper_t = ChunkQueuePopper<ChunkQueueData_t>;

class ChunkBuildingBlocks_IntegrationTest : public Test
{
  public:
    ChunkBuildingBlocks_IntegrationTest()
    {
        m_mempoolConfig.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolConfig, &m_memoryAllocator, &m_memoryAllocator);
    }
    virtual ~ChunkBuildingBlocks_IntegrationTest()
    {
        /// @note One chunk is on hold due to the fact that chunkSender and chunkDistributor hold last chunk
        EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1));
    }

    void SetUp()
    {
        m_chunkSender.addQueue(&m_chunkQueueData);
        m_chunkDistributor.addQueue(&m_chunkReceiverData);
    }
    void TearDown(){};

    void publish()
    {
        for (size_t i = 0; i < ITERATIONS; i++)
        {
            m_chunkSender.allocate(sizeof(DummySample))
                .and_then([&](iox::mepoo::ChunkHeader* chunkHeader) {
                    auto sample = chunkHeader->payload();
                    new (sample) DummySample();
                    static_cast<DummySample*>(sample)->m_dummy = i;
                    m_chunkSender.send(chunkHeader);
                    m_sendCounter++;
                })
                .or_else([](AllocationError) {
                    // Errors shall never occur
                    FAIL();
                });

            /// Add some jitter to make thread breathe
            std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));
        }
        // Signal the next threads we're done
        m_publisherRun = false;
    }

    void forward()
    {
        uint64_t forwardCounter{0};
        bool finished{false};
        // this is to prevent a race condition on thread shutdown; there must be two consecutive empty pops after the
        // publish thread finished
        bool newChunkReceivedInLastIteration{true};
        while (!finished)
        {
            ASSERT_FALSE(m_popper.hasOverflown());

            m_popper.pop()
                .and_then([&](SharedChunk& chunk) {
                    auto dummySample = *reinterpret_cast<DummySample*>(chunk.getPayload());
                    // Check if monotonically increasing
                    EXPECT_THAT(dummySample.m_dummy, Eq(forwardCounter));
                    m_chunkDistributor.deliverToAllStoredQueues(chunk);
                    forwardCounter++;
                    newChunkReceivedInLastIteration = true;
                })
                .or_else([&] {
                    if (!m_publisherRun.load(std::memory_order_relaxed))
                    {
                        if (newChunkReceivedInLastIteration)
                        {
                            newChunkReceivedInLastIteration = false;
                        }
                        else
                        {
                            finished = true;
                        }
                    }
                });
        }
        // Signal the next threads we're done
        m_forwarderRun = false;
    }

    void subscribe()
    {
        bool finished{false};
        // this is to prevent a race condition on thread shutdown; there must be two consecutive empty pops after the
        // forward thread finished
        bool newChunkReceivedInLastIteration{true};

        while (!finished)
        {
            ASSERT_FALSE(m_chunkReceiver.hasOverflown());

            m_chunkReceiver.get()
                .and_then([&](iox::cxx::optional<const iox::mepoo::ChunkHeader*>& maybeChunkHeader) {
                    if (maybeChunkHeader.has_value())
                    {
                        auto chunkHeader = maybeChunkHeader.value();
                        auto dummySample = *reinterpret_cast<DummySample*>(chunkHeader->payload());
                        // Check if monotonically increasing
                        EXPECT_THAT(dummySample.m_dummy, Eq(m_receiveCounter));
                        m_receiveCounter++;
                        m_chunkReceiver.release(chunkHeader);
                        newChunkReceivedInLastIteration = true;
                    }
                    else if (!m_forwarderRun.load(std::memory_order_relaxed))
                    {
                        if (newChunkReceivedInLastIteration)
                        {
                            newChunkReceivedInLastIteration = false;
                        }
                        else
                        {
                            finished = true;
                        }
                    }
                })
                .or_else([](ChunkReceiveError) {
                    // Errors shall never occur
                    FAIL();
                });
        }
    }

    uint64_t m_sendCounter{0};
    uint64_t m_receiveCounter{0};
    std::atomic<bool> m_publisherRun{true};
    std::atomic<bool> m_forwarderRun{true};

    // Memory objects
    Allocator m_memoryAllocator{g_memory, MEMORY_SIZE};
    MePooConfig m_mempoolConfig;
    MemoryManager m_memoryManager;

    // Objects used by publishing thread
    ChunkSenderData<iox::MAX_CHUNKS_ALLOCATE_PER_SENDER, ChunkDistributorData_t> m_chunkSenderData{&m_memoryManager};
    ChunkSender<ChunkDistributor_t> m_chunkSender{&m_chunkSenderData};

    // Objects used by forwarding thread
    ChunkDistributorData_t m_chunkDistributorData;
    ChunkDistributor_t m_chunkDistributor{&m_chunkDistributorData};
    ChunkQueueData_t m_chunkQueueData{
        iox::cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer}; // SoFi intentionally not used
    ChunkQueuePopper_t m_popper{&m_chunkQueueData};

    // Objects used by subscribing thread
    ChunkReceiverData<iox::MAX_CHUNKS_HELD_PER_RECEIVER, ChunkQueueData_t> m_chunkReceiverData{
        iox::cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer}; // SoFi intentionally not used
    ChunkReceiver<ChunkQueuePopper_t> m_chunkReceiver{&m_chunkReceiverData};
};

TEST_F(ChunkBuildingBlocks_IntegrationTest, TwoHopsThreeThreadsNoSoFi)
{
    std::thread subscribingThread(&ChunkBuildingBlocks_IntegrationTest::subscribe, this);
    std::thread forwardingThread(&ChunkBuildingBlocks_IntegrationTest::forward, this);
    std::thread publishingThread(&ChunkBuildingBlocks_IntegrationTest::publish, this);

    if (publishingThread.joinable())
    {
        publishingThread.join();
    }

    if (forwardingThread.joinable())
    {
        forwardingThread.join();
    }

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    EXPECT_EQ(m_sendCounter, m_receiveCounter);
}
