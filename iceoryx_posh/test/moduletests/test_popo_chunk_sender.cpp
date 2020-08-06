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

    struct ChunkDistributorConfig
    {
        static constexpr uint32_t MAX_QUEUES = MAX_NUMBER_QUEUES;
        static constexpr uint64_t MAX_HISTORY_CAPACITY = iox::MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR;
    };

    struct ChunkQueueConfig
    {
        static constexpr uint32_t MAX_QUEUE_CAPACITY = NUM_CHUNKS_IN_POOL;
    };

    using ChunkQueueData_t = iox::popo::ChunkQueueData<ChunkQueueConfig>;
    using ChunkDistributorData_t = iox::popo::ChunkDistributorData<ChunkDistributorConfig,
                                                                   iox::popo::ThreadSafePolicy,
                                                                   iox::popo::ChunkQueuePusher<ChunkQueueData_t>>;
    using ChunkDistributor_t = iox::popo::ChunkDistributor<ChunkDistributorData_t>;

    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

    iox::popo::ChunkSenderData<iox::MAX_CHUNKS_ALLOCATE_PER_SENDER, ChunkDistributorData_t> m_chunkSenderData{
        &m_memoryManager, 0}; // must be 0 for test
    iox::popo::ChunkSenderData<iox::MAX_CHUNKS_ALLOCATE_PER_SENDER, ChunkDistributorData_t>
        m_chunkSenderDataWithHistory{&m_memoryManager, HISTORY_CAPACITY};

    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSender{&m_chunkSenderData};
    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSenderWithHistory{&m_chunkSenderDataWithHistory};
};

TEST_F(ChunkSender_test, allocate_OneChunk)
{
    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
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
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        if (!maybeChunkHeader.has_error())
        {
            chunks.push_back(*maybeChunkHeader);
        }
    }

    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        EXPECT_THAT(chunks[i], Ne(nullptr));
    }
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    // Allocate one more sample for overflow
    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_TRUE(maybeChunkHeader.has_error());
    EXPECT_THAT(maybeChunkHeader.get_error(), Eq(iox::popo::AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));
}

TEST_F(ChunkSender_test, freeChunk)
{
    std::vector<iox::mepoo::ChunkHeader*> chunks;

    // allocate chunks until MAX_CHUNKS_ALLOCATE_PER_SENDER level
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        if (!maybeChunkHeader.has_error())
        {
            chunks.push_back(*maybeChunkHeader);
        }
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    // release them all
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        m_chunkSender.release(chunks[i]);
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(ChunkSender_test, freeInvalidChunk)
{
    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    auto myCrazyChunk = std::make_shared<iox::mepoo::ChunkHeader>();
    m_chunkSender.release(myCrazyChunk.get());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, sendWithoutReceiver)
{
    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    if (!maybeChunkHeader.has_error())
    {
        auto sample = *maybeChunkHeader;
        m_chunkSender.send(sample);
        // chunk is still used because last chunk is stored
        EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    }
}

TEST_F(ChunkSender_test, sendMultipleWithoutReceiverAndAlwaysLast)
{
    for (size_t i = 0; i < 100; i++)
    {
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSender.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(maybeLastChunk.has_value());
            // We get the last chunk again
            EXPECT_TRUE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_TRUE((*maybeChunkHeader)->payload() == (*maybeLastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);
    }

    // Exactly one chunk is used because last chunk is stored
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, sendMultipleWithoutReceiverWithHistoryNoLastReuse)
{
    for (size_t i = 0; i < 10 * HISTORY_CAPACITY; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSenderWithHistory.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(maybeLastChunk.has_value());
            // We don't get the last chunk again
            EXPECT_FALSE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_FALSE((*maybeChunkHeader)->payload() == (*maybeLastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->payload();
        new (sample) DummySample();
        m_chunkSenderWithHistory.send(*maybeChunkHeader);
    }

    // Used chunks == history size
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(HISTORY_CAPACITY));
}

TEST_F(ChunkSender_test, sendOneWithReceiver)
{
    m_chunkSender.addQueue(&m_chunkQueueData);

    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    if (!maybeChunkHeader.has_error())
    {
        auto sample = (*maybeChunkHeader)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);

        // consume the sample
        {
            iox::popo::ChunkQueuePopper<ChunkQueueData_t> myQueue(&m_chunkQueueData);
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
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL <= checkQueue.getCurrentCapacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());

        if (!maybeChunkHeader.has_error())
        {
            auto sample = (*maybeChunkHeader)->payload();
            new (sample) DummySample();
            static_cast<DummySample*>(sample)->dummy = i;
            m_chunkSender.send(*maybeChunkHeader);
        }
    }

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        iox::popo::ChunkQueuePopper<ChunkQueueData_t> myQueue(&m_chunkQueueData);
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
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL <= checkQueue.getCurrentCapacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());

        if (!maybeChunkHeader.has_error())
        {
            (*maybeChunkHeader)->m_info.m_externalSequenceNumber_bl = true;
            (*maybeChunkHeader)->m_info.m_sequenceNumber = i;
            m_chunkSender.send(*maybeChunkHeader);
        }
    }

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        iox::popo::ChunkQueuePopper<ChunkQueueData_t> myQueue(&m_chunkQueueData);
        EXPECT_FALSE(myQueue.empty());
        auto popRet = myQueue.pop();
        EXPECT_TRUE(popRet.has_value());
        EXPECT_THAT(popRet->getChunkHeader()->m_info.m_sequenceNumber, Eq(i));
    }
}


TEST_F(ChunkSender_test, sendTillRunningOutOfChunks)
{
    m_chunkSender.addQueue(&m_chunkQueueData);
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL <= checkQueue.getCurrentCapacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());

        if (!maybeChunkHeader.has_error())
        {
            (*maybeChunkHeader)->m_info.m_externalSequenceNumber_bl = true;
            (*maybeChunkHeader)->m_info.m_sequenceNumber = i;
            auto sample = (*maybeChunkHeader)->payload();
            new (sample) DummySample();
            static_cast<DummySample*>(sample)->dummy = i;
            m_chunkSender.send(*maybeChunkHeader);
        }
    }

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_TRUE(maybeChunkHeader.has_error());
    EXPECT_THAT(maybeChunkHeader.get_error(), Eq(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS));
}

TEST_F(ChunkSender_test, sendInvalidChunk)
{
    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    auto myCrazyChunk = std::make_shared<iox::mepoo::ChunkHeader>();
    m_chunkSender.send(myCrazyChunk.get());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkSender_test, pushToHistory)
{
    for (size_t i = 0; i < 10 * HISTORY_CAPACITY; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());
        m_chunkSenderWithHistory.pushToHistory(*maybeChunkHeader);
    }

    // Used chunks == history size
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(HISTORY_CAPACITY));
}

TEST_F(ChunkSender_test, pushInvalidChunkToHistory)
{
    auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

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
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSender.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(maybeLastChunk.has_value());
            // No last chunk for us :-(
            EXPECT_FALSE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_FALSE((*maybeChunkHeader)->payload() == (*maybeLastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);
    }

    // All Chunks used now
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, NUM_CHUNKS_IN_POOL);
}

TEST_F(ChunkSender_test, sendMultipleWithReceiverLastReuseBecauseAlreadyConsumed)
{
    m_chunkSender.addQueue(&m_chunkQueueData);

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.allocate(sizeof(DummySample));
        EXPECT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSender.getLast();
        if (i > 0)
        {
            EXPECT_TRUE(maybeLastChunk.has_value());
            // We get the last chunk again
            EXPECT_TRUE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_TRUE((*maybeChunkHeader)->payload() == (*maybeLastChunk)->payload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->payload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);

        iox::popo::ChunkQueuePopper<ChunkQueueData_t> myQueue(&m_chunkQueueData);
        EXPECT_FALSE(myQueue.empty());
        auto popRet = myQueue.pop();
        EXPECT_TRUE(popRet.has_value());
    }

    // All consumed but the lastChunk
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, 1);
}

TEST_F(ChunkSender_test, ReuseLastIfSmaller)
{
    auto maybeChunkHeader = m_chunkSender.allocate(BIG_CHUNK);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1u));

    auto chunkHeader = *maybeChunkHeader;
    m_chunkSender.send(chunkHeader);

    auto chunkSmaller = m_chunkSender.allocate(SMALL_CHUNK);
    EXPECT_FALSE(chunkSmaller.has_error());

    // no small chunk used as big one is recycled
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1u));

    auto maybeLastChunk = m_chunkSender.getLast();
    EXPECT_TRUE(maybeLastChunk.has_value());
    // We get the last chunk again
    EXPECT_TRUE(*chunkSmaller == *maybeLastChunk);
    EXPECT_TRUE((*chunkSmaller)->payload() == (*maybeLastChunk)->payload());
}

TEST_F(ChunkSender_test, NoReuseOfLastIfBigger)
{
    auto maybeChunkHeader = m_chunkSender.allocate(SMALL_CHUNK);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto chunkHeader = *maybeChunkHeader;
    m_chunkSender.send(chunkHeader);

    auto chunkBigger = m_chunkSender.allocate(BIG_CHUNK);
    EXPECT_FALSE(chunkBigger.has_error());

    // no reuse, we hav a small and a big chunk in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1u));

    auto maybeLastChunk = m_chunkSender.getLast();
    EXPECT_TRUE(maybeLastChunk.has_value());
    // not the last chunk
    EXPECT_FALSE(*chunkBigger == *maybeLastChunk);
    EXPECT_FALSE((*chunkBigger)->payload() == (*maybeLastChunk)->payload());
}

TEST_F(ChunkSender_test, ReuseOfLastIfBiggerButFitsInChunk)
{
    auto maybeChunkHeader = m_chunkSender.allocate(SMALL_CHUNK - 10);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));

    auto chunkHeader = *maybeChunkHeader;
    m_chunkSender.send(chunkHeader);

    auto chunkBigger = m_chunkSender.allocate(SMALL_CHUNK);
    EXPECT_FALSE(chunkBigger.has_error());

    // reuse as it still fits in the small chunk
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(0u));

    auto maybeLastChunk = m_chunkSender.getLast();
    EXPECT_TRUE(maybeLastChunk.has_value());
    // not the last chunk
    EXPECT_TRUE(*chunkBigger == *maybeLastChunk);
    EXPECT_TRUE((*chunkBigger)->payload() == (*maybeLastChunk)->payload());
}

TEST_F(ChunkSender_test, Cleanup)
{
    EXPECT_TRUE((HISTORY_CAPACITY + iox::MAX_CHUNKS_ALLOCATE_PER_SENDER) <= NUM_CHUNKS_IN_POOL);

    for (size_t i = 0; i < HISTORY_CAPACITY; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.allocate(SMALL_CHUNK);
        EXPECT_FALSE(maybeChunkHeader.has_error());
        m_chunkSenderWithHistory.send(*maybeChunkHeader);
    }

    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATE_PER_SENDER; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.allocate(SMALL_CHUNK);
        EXPECT_FALSE(maybeChunkHeader.has_error());
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks,
                Eq(HISTORY_CAPACITY + iox::MAX_CHUNKS_ALLOCATE_PER_SENDER));

    m_chunkSenderWithHistory.releaseAll();

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}
