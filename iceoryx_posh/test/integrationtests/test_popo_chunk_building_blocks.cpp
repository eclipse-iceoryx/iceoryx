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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "test.hpp"

#include <stdlib.h>
#include <thread>

using namespace ::testing;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;
using ::testing::Return;
using namespace std::chrono_literals;

struct DummySample
{
    uint64_t dummy{42};
};

/// @todo Make it a typed test?
class ChunkBuildingBlocks_IntegrationTest : public Test
{
  public:
    ChunkBuildingBlocks_IntegrationTest()
    {
        m_mempoolconf.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_mempoolconf.addMemPool({BIG_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }
    virtual ~ChunkBuildingBlocks_IntegrationTest()
    {
    }

    void SetUp()
    {
        /// @todo connect all four sut's
        // ChunkSender
        // ChunkSenderData
        // ChunkReceiver
        // ChunkReceiverData
    }


    void TearDown()
    {
    }


    std::thread chunkSenderThread{([&] {
        /// @todo in a loop
        // std::this_thread::sleep_for((rand() % 100)ms);
        // auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        // EXPECT_FALSE(chunk.has_error());
        // EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
        // m_chunkSender.send(chunk);
        // EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
        // std::this_thread::sleep_for((rand() % 100)ms);
    })};

    std::thread chunkDistributorThread{([&] {
        /// @todo in a loop
        // std::this_thread::sleep_for((rand() % 100)ms);
        // deliverToAllStoredQueues(); // multi threaded policy
        // random sleep between 0-1s?
        // std::this_thread::sleep_for((rand() % 100)ms);
    })};

    std::thread ChunkReceiverThread{([&] {
        /// @todo in a loop
        // std::this_thread::sleep_for((rand() % 100)ms);
        // get();
        // cast to SampleType
        // is the counter running correct?
        // std::this_thread::sleep_for((rand() % 100)ms);
    })};

    static constexpr size_t MEMORY_SIZE = 1024 * 1024;
    uint8_t m_memory[1024 * 1024];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = 20;
    static constexpr uint32_t SMALL_CHUNK = 128;
    static constexpr uint32_t BIG_CHUNK = 256;
    static constexpr uint64_t HISTORY_CAPACITY = 4;
    static constexpr uint32_t MAX_NUMBER_QUEUES = 128;

    iox::posix::Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;

    using ChunkDistributorData_t = iox::popo::ChunkDistributorData<MAX_NUMBER_QUEUES, iox::popo::ThreadSafePolicy>;
    iox::popo::ChunkSenderData<ChunkDistributorData_t> m_chunkSenderData{&m_memoryManager, 0}; // must be 0 for test
    iox::popo::ChunkSenderData<ChunkDistributorData_t> m_chunkSenderDataWithHistory{&m_memoryManager, HISTORY_CAPACITY};

    using ChunkDistributor_t = iox::popo::ChunkDistributor<MAX_NUMBER_QUEUES, iox::popo::ThreadSafePolicy>;
    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSender{&m_chunkSenderData};
    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSenderWithHistory{&m_chunkSenderDataWithHistory};
};

TEST_F(ChunkBuildingBlocks_IntegrationTest, SendWithoutConnection)
{
}

TEST_F(ChunkBuildingBlocks_IntegrationTest, SendAndReceive)
{
}
