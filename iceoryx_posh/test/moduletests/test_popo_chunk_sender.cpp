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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender_data.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;

struct DummySample
{
    uint64_t dummy{42};
};

class ChunkSender_test : public Test
{
  protected:
    ChunkSender_test()
    {
        m_mempoolconf.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_mempoolconf.addMemPool({BIG_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~ChunkSender_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    static constexpr size_t MEMORY_SIZE = 1024 * 1024;
    uint8_t m_memory[MEMORY_SIZE];
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = 20;
    static constexpr uint32_t SMALL_CHUNK = 128;
    static constexpr uint32_t BIG_CHUNK = 256;
    static constexpr uint64_t HISTORY_CAPACITY = 4;
    static constexpr uint32_t MAX_NUMBER_QUEUES = 128;

    iox::posix::Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;

    iox::popo::ChunkQueueData m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

    using ChunkDistributorData_t =
        iox::popo::ChunkDistributorData<MAX_NUMBER_QUEUES, iox::popo::ThreadSafePolicy, iox::popo::ChunkQueuePusher>;
    iox::popo::ChunkSenderData<ChunkDistributorData_t> m_chunkSenderData{&m_memoryManager, 0}; // must be 0 for test
    iox::popo::ChunkSenderData<ChunkDistributorData_t> m_chunkSenderDataWithHistory{&m_memoryManager, HISTORY_CAPACITY};

    using ChunkDistributor_t = iox::popo::ChunkDistributor<ChunkDistributorData_t>;
    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSender{&m_chunkSenderData};
    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSenderWithHistory{&m_chunkSenderDataWithHistory};
};

TEST_F(ChunkSender_test, allocate_OneChunk)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, allocate_MultipleChunks)
{
    auto chunk1 = m_chunkSender.allocate(sizeof(DummySample));
    auto chunk2 = m_chunkSender.allocate(sizeof(DummySample));

    EXPECT_FALSE(chunk1.has_error());
    EXPECT_FALSE(chunk2.has_error());
    if (!chunk1.has_error() && !chunk2.has_error())
    {
        // must be different chunks
        EXPECT_THAT(*chunk1, Ne(*chunk2));
    }
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2u));
}

TEST_F(ChunkSender_test, allocate_Overflow)
{
    std::vector<iox::mepoo::ChunkHeader*> chunks;

    // allocate chunks until MAX_CHUNKS_ALLOCATE_PER_SENDER level
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        if (!chunk.has_error())
        {
            chunks.push_back(*chunk);
        }
    }

    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        EXPECT_THAT(chunks[i], Ne(nullptr));
    }
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    // Allocate one more sample for overflow
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_TRUE(chunk.has_error());
    EXPECT_THAT(chunk.get_error(), Eq(iox::popo::ChunkSenderError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));
}

TEST_F(ChunkSender_test, freeChunk)
{
    std::vector<iox::mepoo::ChunkHeader*> chunks;

    // allocate chunks until MAX_CHUNKS_ALLOCATE_PER_SENDER level
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        if (!chunk.has_error())
        {
            chunks.push_back(*chunk);
        }
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    // free them all
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        m_chunkSender.free(chunks[i]);
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(ChunkSender_test, freeInvalidChunk)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler([&errorHandlerCalled](
        const iox::Error, const std::function<void()>, const iox::ErrorLevel) { errorHandlerCalled = true; });

    auto myCrazyChunk = std::make_shared<iox::mepoo::ChunkHeader>();
    m_chunkSender.free(myCrazyChunk.get());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, sendWithoutReceiver)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    if (!chunk.has_error())
    {
        auto sample = *chunk;
        m_chunkSender.send(sample);
        // chunk is still used because last chunk is stored
        EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    }
}

TEST_F(ChunkSender_test, sendMultipleWithoutReceiverAndAlwaysLast)
{
    for (size_t i = 0; i < 100; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());
        auto lastChunk = m_chunkSender.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(lastChunk.has_value());
            // We get the last chunk again
            EXPECT_TRUE(*chunk == *lastChunk);
            EXPECT_TRUE((*chunk)->payload() == (*lastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(lastChunk.has_value());
        }
        auto sample = (*chunk)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*chunk);
    }

    // Exactly one chunk is used because last chunk is stored
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, sendMultipleWithoutReceiverWithHistoryNoLastReuse)
{
    for (size_t i = 0; i < 10 * HISTORY_CAPACITY; i++)
    {
        auto chunk = m_chunkSenderWithHistory.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());
        auto lastChunk = m_chunkSenderWithHistory.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(lastChunk.has_value());
            // We don't get the last chunk again
            EXPECT_FALSE(*chunk == *lastChunk);
            EXPECT_FALSE((*chunk)->payload() == (*lastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(lastChunk.has_value());
        }
        auto sample = (*chunk)->payload();
        new (sample) DummySample();
        m_chunkSenderWithHistory.send(*chunk);
    }

    // Used chunks == history size
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(HISTORY_CAPACITY));
}

TEST_F(ChunkSender_test, sendOneWithReceiver)
{
    m_chunkSender.addQueue(&m_chunkQueueData);

    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    if (!chunk.has_error())
    {
        auto sample = (*chunk)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*chunk);

        // consume the sample
        {
            iox::popo::ChunkQueuePopper myQueue(&m_chunkQueueData);
            EXPECT_FALSE(myQueue.empty());
            auto popRet = myQueue.pop();
            EXPECT_TRUE(popRet.has_value());
            auto dummySample = *reinterpret_cast<DummySample*>(popRet->getPayload());
            EXPECT_THAT(dummySample.dummy, Eq(42));
        }
    }
}

TEST_F(ChunkSender_test, sendMultipleWithReceiver)
{
    m_chunkSender.addQueue(&m_chunkQueueData);
    iox::popo::ChunkQueuePopper checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL < checkQueue.capacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());

        if (!chunk.has_error())
        {
            auto sample = (*chunk)->payload();
            new (sample) DummySample();
            static_cast<DummySample*>(sample)->dummy = i;
            m_chunkSender.send(*chunk);
        }
    }

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        iox::popo::ChunkQueuePopper myQueue(&m_chunkQueueData);
        EXPECT_FALSE(myQueue.empty());
        auto popRet = myQueue.pop();
        EXPECT_TRUE(popRet.has_value());
        auto dummySample = *reinterpret_cast<DummySample*>(popRet->getPayload());
        EXPECT_THAT(dummySample.dummy, Eq(i));
        EXPECT_THAT(popRet->getChunkHeader()->m_info.m_sequenceNumber, Eq(i));
    }
}

TEST_F(ChunkSender_test, sendMultipleWithReceiverExternalSequenceNumber)
{
    m_chunkSender.addQueue(&m_chunkQueueData);
    iox::popo::ChunkQueuePopper checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL < checkQueue.capacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());

        if (!chunk.has_error())
        {
            (*chunk)->m_info.m_externalSequenceNumber_bl = true;
            (*chunk)->m_info.m_sequenceNumber = i;
            m_chunkSender.send(*chunk);
        }
    }

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        iox::popo::ChunkQueuePopper myQueue(&m_chunkQueueData);
        EXPECT_FALSE(myQueue.empty());
        auto popRet = myQueue.pop();
        EXPECT_TRUE(popRet.has_value());
        EXPECT_THAT(popRet->getChunkHeader()->m_info.m_sequenceNumber, Eq(i));
    }
}


TEST_F(ChunkSender_test, sendTillRunningOutOfChunks)
{
    m_chunkSender.addQueue(&m_chunkQueueData);
    iox::popo::ChunkQueuePopper checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL < checkQueue.capacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());

        if (!chunk.has_error())
        {
            (*chunk)->m_info.m_externalSequenceNumber_bl = true;
            (*chunk)->m_info.m_sequenceNumber = i;
            auto sample = (*chunk)->payload();
            new (sample) DummySample();
            static_cast<DummySample*>(sample)->dummy = i;
            m_chunkSender.send(*chunk);
        }
    }

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler([&errorHandlerCalled](
        const iox::Error, const std::function<void()>, const iox::ErrorLevel) { errorHandlerCalled = true; });

    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_TRUE(chunk.has_error());
    EXPECT_THAT(chunk.get_error(), Eq(iox::popo::ChunkSenderError::RUNNING_OUT_OF_CHUNKS));
}

TEST_F(ChunkSender_test, sendInvalidChunk)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler([&errorHandlerCalled](
        const iox::Error, const std::function<void()>, const iox::ErrorLevel) { errorHandlerCalled = true; });

    auto myCrazyChunk = std::make_shared<iox::mepoo::ChunkHeader>();
    m_chunkSender.send(myCrazyChunk.get());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, pushToHistory)
{
    for (size_t i = 0; i < 10 * HISTORY_CAPACITY; i++)
    {
        auto chunk = m_chunkSenderWithHistory.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());
        m_chunkSenderWithHistory.pushToHistory(*chunk);
    }

    // Used chunks == history size
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(HISTORY_CAPACITY));
}

TEST_F(ChunkSender_test, pushInvalidChunkToHistory)
{
    auto chunk = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler([&errorHandlerCalled](
        const iox::Error, const std::function<void()>, const iox::ErrorLevel) { errorHandlerCalled = true; });

    auto myCrazyChunk = std::make_shared<iox::mepoo::ChunkHeader>();
    m_chunkSender.pushToHistory(myCrazyChunk.get());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, sendMultipleWithReceiverNoLastReuse)
{
    m_chunkSender.addQueue(&m_chunkQueueData);

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());
        auto lastChunk = m_chunkSender.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(lastChunk.has_value());
            // No last chunk for us :-(
            EXPECT_FALSE(*chunk == *lastChunk);
            EXPECT_FALSE((*chunk)->payload() == (*lastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(lastChunk.has_value());
        }
        auto sample = (*chunk)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*chunk);
    }

    // All Chunks used now
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, NUM_CHUNKS_IN_POOL);
}

TEST_F(ChunkSender_test, sendMultipleWithReceiverLastReuseBecauseAlreadyConsumed)
{
    m_chunkSender.addQueue(&m_chunkQueueData);

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto chunk = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(chunk.has_error());
        auto lastChunk = m_chunkSender.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(lastChunk.has_value());
            // We get the last chunk again
            EXPECT_TRUE(*chunk == *lastChunk);
            EXPECT_TRUE((*chunk)->payload() == (*lastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(lastChunk.has_value());
        }
        auto sample = (*chunk)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*chunk);

        iox::popo::ChunkQueuePopper myQueue(&m_chunkQueueData);
        EXPECT_FALSE(myQueue.empty());
        auto popRet = myQueue.pop();
        EXPECT_TRUE(popRet.has_value());
    }

    // All consumed but the lastChunk
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, 1);
}

TEST_F(ChunkSender_test, ReuseLastIfSmaller)
{
    auto chunk = m_chunkSender.allocate(BIG_CHUNK);
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1u));

    auto chunkHeader = *chunk;
    m_chunkSender.send(chunkHeader);

    auto chunkSmaller = m_chunkSender.allocate(SMALL_CHUNK);
    EXPECT_FALSE(chunkSmaller.has_error());

    // no small chunk used as big one is recycled
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1u));

    auto lastChunk = m_chunkSender.getLast();
    EXPECT_TRUE(lastChunk.has_value());
    // We get the last chunk again
    EXPECT_TRUE(*chunkSmaller == *lastChunk);
    EXPECT_TRUE((*chunkSmaller)->payload() == (*lastChunk)->payload());
}

TEST_F(ChunkSender_test, NoReuseOfLastIfBigger)
{
    auto chunk = m_chunkSender.allocate(SMALL_CHUNK);
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto chunkHeader = *chunk;
    m_chunkSender.send(chunkHeader);

    auto chunkBigger = m_chunkSender.allocate(BIG_CHUNK);
    EXPECT_FALSE(chunkBigger.has_error());

    // no reuse, we hav a small and a big chunk in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1u));

    auto lastChunk = m_chunkSender.getLast();
    EXPECT_TRUE(lastChunk.has_value());
    // not the last chunk
    EXPECT_FALSE(*chunkBigger == *lastChunk);
    EXPECT_FALSE((*chunkBigger)->payload() == (*lastChunk)->payload());
}

TEST_F(ChunkSender_test, ReuseOfLastIfBiggerButFitsInChunk)
{
    auto chunk = m_chunkSender.allocate(SMALL_CHUNK - 10);
    EXPECT_FALSE(chunk.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto chunkHeader = *chunk;
    m_chunkSender.send(chunkHeader);

    auto chunkBigger = m_chunkSender.allocate(SMALL_CHUNK);
    EXPECT_FALSE(chunkBigger.has_error());

    // reuse as it still fits in the small chunk
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(0u));

    auto lastChunk = m_chunkSender.getLast();
    EXPECT_TRUE(lastChunk.has_value());
    // not the last chunk
    EXPECT_TRUE(*chunkBigger == *lastChunk);
    EXPECT_TRUE((*chunkBigger)->payload() == (*lastChunk)->payload());
}

TEST_F(ChunkSender_test, Cleanup)
{
    EXPECT_TRUE((HISTORY_CAPACITY + iox::MAX_CHUNKS_ALLOCATE_PER_SENDER) <= NUM_CHUNKS_IN_POOL);

    for (size_t i = 0; i < HISTORY_CAPACITY; i++)
    {
        auto chunk = m_chunkSenderWithHistory.allocate(SMALL_CHUNK);
        EXPECT_FALSE(chunk.has_error());
        m_chunkSenderWithHistory.send(*chunk);
    }

    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        auto chunk = m_chunkSenderWithHistory.allocate(SMALL_CHUNK);
        EXPECT_FALSE(chunk.has_error());
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks,
                Eq(HISTORY_CAPACITY + iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    m_chunkSenderWithHistory.releaseAll();

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}