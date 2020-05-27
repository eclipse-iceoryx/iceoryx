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
    uint64_t dummy{42};
};

static constexpr size_t MEMORY_SIZE = 1024 * 1024;
uint8_t m_memory[MEMORY_SIZE];
static constexpr uint32_t NUM_CHUNKS_IN_POOL = 500;
static constexpr uint32_t ITERATIONS = 10000;
static constexpr uint32_t SMALL_CHUNK = 128;
static constexpr uint32_t MAX_NUMBER_QUEUES = 128;

using ChunkDistributorData_t = iox::popo::ChunkDistributorData<MAX_NUMBER_QUEUES, iox::popo::ThreadSafePolicy>;
using ChunkDistributor_t = iox::popo::ChunkDistributor<ChunkDistributorData_t>;

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
                .on_success([&](iox::mepoo::ChunkHeader* chunkHeader) {
                    auto sample = chunkHeader->payload();
                    new (sample) DummySample();
                    static_cast<DummySample*>(sample)->dummy = i;
                    m_chunkSender.send(chunkHeader);

                    /// @todo for debugging only, to be removed
                    chunkHeaderPointerPublisherVector.push_back(sample);

                    m_sendCounter++;
                })
                .on_error([]() {
                    // Errors shall never occur
                    FAIL();
                });

            /// Add some jitter to make thread breathe
            std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));
        }
    }

    void forward()
    {
        uint64_t forwardCounter{0};

        while (m_run)
        {
            m_popper.pop().and_then([&](SharedChunk& chunk) {
                auto dummySample = *reinterpret_cast<DummySample*>(chunk.getPayload());
                // Check if monotonically increasing
                EXPECT_THAT(dummySample.dummy, Eq(forwardCounter));

                /// @todo for debugging only, to be removed
                chunkHeaderPointerForwardingVector.push_back(chunk.getPayload());

                m_chunkDistributor.deliverToAllStoredQueues(chunk);
                forwardCounter++;
            });
            /// Add some jitter to make thread breathe
            std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));
        }
    }

    void subscribe()
    {
        bool finished{false};

        while ((m_receiveCounter < ITERATIONS) && !finished)
        {
            m_chunkReceiver.get()
                .on_success([&](iox::cxx::optional<const iox::mepoo::ChunkHeader*>& maybeChunkHeader) {
                    if (maybeChunkHeader.has_value())
                    {
                        auto chunkHeader = maybeChunkHeader.value();
                        auto dummySample = *reinterpret_cast<DummySample*>(chunkHeader->payload());
                        // Check if monotonically increasing
                        EXPECT_THAT(dummySample.dummy, Eq(m_receiveCounter));

                        /// @todo for debugging only, to be removed
                        chunkHeaderPointerSubscriberVector.push_back(chunkHeader->payload());

                        m_receiveCounter++;
                        /// Add some jitter to make thread breathe
                        std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));
                        m_chunkReceiver.release(chunkHeader);
                    }
                    else if (m_run == false)
                    {
                        finished = true;
                    }
                })
                .on_error([]() {
                    // Errors shall never occur
                    FAIL();
                });
        }
    }

    uint64_t m_sendCounter{0};
    uint64_t m_receiveCounter{0};
    std::atomic<bool> m_run{true};

    // Memory objects
    Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MePooConfig m_mempoolConfig;
    MemoryManager m_memoryManager;

    // Objects used by publishing thread
    ChunkSenderData<ChunkDistributorData_t> m_chunkSenderData{&m_memoryManager};
    ChunkSender<ChunkDistributor_t> m_chunkSender{&m_chunkSenderData};

    // Objects used by forwarding thread
    ChunkDistributorData_t m_chunkDistributorData;
    ChunkDistributor_t m_chunkDistributor{&m_chunkDistributorData};
    ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkQueuePopper m_popper{&m_chunkQueueData};

    // Objects used by subscribing thread
    ChunkReceiverData m_chunkReceiverData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkReceiver m_chunkReceiver{&m_chunkReceiverData};

    /// @todo for debugging only, to be removed
    std::vector<void*> chunkHeaderPointerPublisherVector;
    std::vector<void*> chunkHeaderPointerForwardingVector;
    std::vector<void*> chunkHeaderPointerSubscriberVector;
};

TEST_F(ChunkBuildingBlocks_IntegrationTest, TwoHopsThreeThreads)
{
    std::thread subscribingThread(&ChunkBuildingBlocks_IntegrationTest::subscribe, this);
    std::thread forwardingThread(&ChunkBuildingBlocks_IntegrationTest::forward, this);
    std::thread publishingThread(&ChunkBuildingBlocks_IntegrationTest::publish, this);

    if (publishingThread.joinable())
    {
        publishingThread.join();
    }

    // Signal the other threads we're done
    m_run = false;

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    if (forwardingThread.joinable())
    {
        forwardingThread.join();
    }

    EXPECT_EQ(m_sendCounter, m_receiveCounter);
}
