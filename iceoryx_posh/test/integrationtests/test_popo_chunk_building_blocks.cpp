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

// Memory objects
Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
MePooConfig m_mempoolConfig;
MemoryManager m_memoryManager;

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
        m_chunkSender.addQueue(&m_chunkData);
        m_chunkDistributor.addQueue(&m_chunkReceiverData);
    }
    void TearDown(){};

    void publish()
    {
        for (size_t i = 0; i < ITERATIONS; i++)
        {
            m_chunkSender
                .allocate(sizeof(DummySample))
                /// @todo overload for on_success(TargetType& foo) would be nice?
                .on_success([&](iox::cxx::expected<iox::mepoo::ChunkHeader*, ChunkSenderError>& chunkHeader) {
                    auto sample = chunkHeader.get_value()->payload();
                    new (sample) DummySample();
                    static_cast<DummySample*>(sample)->dummy = i;
                    m_chunkSender.send(chunkHeader.get_value());
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
        while (m_run)
        {
            m_popper.pop().and_then([&](SharedChunk& chunk) { m_chunkDistributor.deliverToAllStoredQueues(chunk); });

            /// Add some jitter to make thread breathe
            std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));
        }
    }

    void subscribe()
    {
        while (m_receiveCounter < ITERATIONS)
        {
            m_chunkReceiver.get()
                .on_success([&](iox::cxx::expected<iox::cxx::optional<const iox::mepoo::ChunkHeader*>,
                                                   ChunkReceiverError>& maybeChunkHeader) {
                    /// @todo overload for and_then(ptr* foo) would be nice?
                    if (maybeChunkHeader.get_value().has_value())
                    {
                        auto chunkHeader = maybeChunkHeader.get_value().value();

                        auto dummySample = *reinterpret_cast<DummySample*>(chunkHeader->payload());

                        m_receiveCounter++;

                        /// Add some jitter to make thread breathe
                        std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));

                        m_chunkReceiver.release(chunkHeader);
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

    // Objects used by publishing thread
    ChunkSenderData<ChunkDistributorData_t> m_chunkSenderData{&m_memoryManager};
    ChunkSender<ChunkDistributor_t> m_chunkSender{&m_chunkSenderData};

    // Objects used by forwarding thread
    ChunkDistributorData_t m_chunkDistributorData;
    ChunkDistributor_t m_chunkDistributor{&m_chunkDistributorData};
    ChunkQueueData m_chunkData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkQueuePopper m_popper{&m_chunkData};

    // Objects used by subscribing thread
    ChunkReceiverData m_chunkReceiverData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkReceiver m_chunkReceiver{&m_chunkReceiverData};
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

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    // Stop the forwarding thread
    m_run = false;

    if (forwardingThread.joinable())
    {
        forwardingThread.join();
    }

    EXPECT_EQ(m_sendCounter, m_receiveCounter);
}
