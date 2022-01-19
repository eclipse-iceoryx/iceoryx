// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "test.hpp"

#include <memory>

namespace
{
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
        m_memoryManager.configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);
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

    static constexpr uint32_t USER_PAYLOAD_ALIGNMENT = iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT;
    static constexpr uint32_t USER_HEADER_SIZE = iox::CHUNK_NO_USER_HEADER_SIZE;
    static constexpr uint32_t USER_HEADER_ALIGNMENT = iox::CHUNK_NO_USER_HEADER_ALIGNMENT;

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
    iox::posix::Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;

    struct ChunkDistributorConfig
    {
        static constexpr uint32_t MAX_QUEUES = MAX_NUMBER_QUEUES;
        static constexpr uint64_t MAX_HISTORY_CAPACITY = iox::MAX_PUBLISHER_HISTORY;
    };

    struct ChunkQueueConfig
    {
        static constexpr uint64_t MAX_QUEUE_CAPACITY = NUM_CHUNKS_IN_POOL;
    };

    using ChunkQueueData_t = iox::popo::ChunkQueueData<ChunkQueueConfig, iox::popo::ThreadSafePolicy>;
    using ChunkDistributorData_t = iox::popo::ChunkDistributorData<ChunkDistributorConfig,
                                                                   iox::popo::ThreadSafePolicy,
                                                                   iox::popo::ChunkQueuePusher<ChunkQueueData_t>>;
    using ChunkDistributor_t = iox::popo::ChunkDistributor<ChunkDistributorData_t>;
    using ChunkSenderData_t =
        iox::popo::ChunkSenderData<iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY, ChunkDistributorData_t>;

    ChunkQueueData_t m_chunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkSenderData_t m_chunkSenderData{
        &m_memoryManager, iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA, 0}; // must be 0 for test
    ChunkSenderData_t m_chunkSenderDataWithHistory{
        &m_memoryManager, iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA, HISTORY_CAPACITY};

    iox::popo::ChunkSender<ChunkSenderData_t> m_chunkSender{&m_chunkSenderData};
    iox::popo::ChunkSender<ChunkSenderData_t> m_chunkSenderWithHistory{&m_chunkSenderDataWithHistory};
};

TEST_F(ChunkSender_test, allocate_OneChunkWithoutUserHeaderAndSmallUserPayloadAlignmentResultsInSmallChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c60fd47-6637-4a9f-bf1b-1b5f707a0cdf");
    constexpr uint32_t USER_PAYLOAD_SIZE{SMALL_CHUNK / 2};
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT};
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(ChunkSender_test, allocate_OneChunkWithoutUserHeaderAndLargeUserPayloadAlignmentResultsInLargeChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "a1743fc6-65a2-4218-be6b-b0b8c2e7d1f7");
    constexpr uint32_t USER_PAYLOAD_SIZE{SMALL_CHUNK / 2};
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT{SMALL_CHUNK};
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1U));
}

TEST_F(ChunkSender_test, allocate_OneChunkWithLargeUserHeaderResultsInLargeChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "e13e06e5-3c9a-4ec2-812c-4ea73cd1d4eb");
    constexpr uint32_t LARGE_HEADER_SIZE{SMALL_CHUNK};
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), LARGE_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1U));
}

TEST_F(ChunkSender_test, allocate_ChunkHasOriginIdSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "7e33b20b-93f9-4b53-926b-20295ac73b61");
    iox::UniquePortId uniqueId;
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        uniqueId, sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);

    ASSERT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT((*maybeChunkHeader)->originId(), Eq(uniqueId));
}

TEST_F(ChunkSender_test, allocate_MultipleChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0be0972d-f7d4-4400-bbc9-31767aef2e2b");
    auto chunk1 = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    auto chunk2 = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);

    ASSERT_FALSE(chunk1.has_error());
    ASSERT_FALSE(chunk2.has_error());
    // must be different chunks
    EXPECT_THAT(*chunk1, Ne(*chunk2));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));
}

TEST_F(ChunkSender_test, allocate_Overflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "93e3689a-a1f2-46d2-887d-8c04560161af");
    std::vector<iox::mepoo::ChunkHeader*> chunks;

    // tryAllocate chunks until MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY level
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY; i++)
    {
        auto maybeChunkHeader = m_chunkSender.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        if (!maybeChunkHeader.has_error())
        {
            chunks.push_back(*maybeChunkHeader);
        }
    }

    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY; i++)
    {
        EXPECT_THAT(chunks[i], Ne(nullptr));
    }
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks,
                Eq(iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY));

    // Allocate one more sample for overflow
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_TRUE(maybeChunkHeader.has_error());
    EXPECT_THAT(maybeChunkHeader.get_error(), Eq(iox::popo::AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks,
                Eq(iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY));
}

TEST_F(ChunkSender_test, freeChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4a6eb09-a431-4f38-bd0c-38baf896a639");
    std::vector<iox::mepoo::ChunkHeader*> chunks;

    // tryAllocate chunks until MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY level
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY; i++)
    {
        auto maybeChunkHeader = m_chunkSender.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        if (!maybeChunkHeader.has_error())
        {
            chunks.push_back(*maybeChunkHeader);
        }
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks,
                Eq(iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY));

    // release them all
    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY; i++)
    {
        m_chunkSender.release(chunks[i]);
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

TEST_F(ChunkSender_test, freeInvalidChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d9f448d-f622-44f3-a78b-31b0a7b0d1a8");
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::PoshError, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    ChunkMock<bool> myCrazyChunk;
    m_chunkSender.release(myCrazyChunk.chunkHeader());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(ChunkSender_test, sendWithoutReceiver)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9c56b90-2b9d-4097-a908-8f2282b83e10");
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    if (!maybeChunkHeader.has_error())
    {
        auto sample = *maybeChunkHeader;
        m_chunkSender.send(sample);
        // chunk is still used because last chunk is stored
        EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
    }
}

TEST_F(ChunkSender_test, sendMultipleWithoutReceiverAndAlwaysLast)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5b798e6-5163-460b-a2d6-cce5592d3c04");
    for (size_t i = 0; i < 100; i++)
    {
        auto maybeChunkHeader = m_chunkSender.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        ASSERT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSender.tryGetPreviousChunk();
        if (i > 0)
        {
            ASSERT_TRUE(maybeLastChunk.has_value());
            // We get the last chunk again
            EXPECT_TRUE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_TRUE((*maybeChunkHeader)->userPayload() == (*maybeLastChunk)->userPayload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->userPayload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);
    }

    // Exactly one chunk is used because last chunk is stored
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(ChunkSender_test, sendMultipleWithoutReceiverWithHistoryNoLastReuse)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb782ba8-f457-4f4c-9b3e-c616fa329261");
    for (size_t i = 0; i < 10 * HISTORY_CAPACITY; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        ASSERT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSenderWithHistory.tryGetPreviousChunk();
        if (i > 0)
        {
            ASSERT_TRUE(maybeLastChunk.has_value());
            // We don't get the last chunk again
            EXPECT_FALSE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_FALSE((*maybeChunkHeader)->userPayload() == (*maybeLastChunk)->userPayload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->userPayload();
        new (sample) DummySample();
        m_chunkSenderWithHistory.send(*maybeChunkHeader);
    }

    // Used chunks == history size
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(HISTORY_CAPACITY));
}

TEST_F(ChunkSender_test, sendOneWithReceiver)
{
    ::testing::Test::RecordProperty("TEST_ID", "9279f04c-e37c-4d0f-8217-720afe59f52b");
    ASSERT_FALSE(m_chunkSender.tryAddQueue(&m_chunkQueueData).has_error());

    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    if (!maybeChunkHeader.has_error())
    {
        auto sample = (*maybeChunkHeader)->userPayload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);

        // consume the sample
        {
            iox::popo::ChunkQueuePopper<ChunkQueueData_t> myQueue(&m_chunkQueueData);
            EXPECT_FALSE(myQueue.empty());
            auto popRet = myQueue.tryPop();
            EXPECT_TRUE(popRet.has_value());
            auto dummySample = *reinterpret_cast<DummySample*>(popRet->getUserPayload());
            EXPECT_THAT(dummySample.dummy, Eq(42U));
        }
    }
}

TEST_F(ChunkSender_test, sendMultipleWithReceiver)
{
    ::testing::Test::RecordProperty("TEST_ID", "07e6a360-f5ae-4cd9-9bee-54b3c31c3390");
    ASSERT_FALSE(m_chunkSender.tryAddQueue(&m_chunkQueueData).has_error());
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL <= checkQueue.getCurrentCapacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        EXPECT_FALSE(maybeChunkHeader.has_error());

        if (!maybeChunkHeader.has_error())
        {
            auto sample = (*maybeChunkHeader)->userPayload();
            new (sample) DummySample();
            static_cast<DummySample*>(sample)->dummy = i;
            m_chunkSender.send(*maybeChunkHeader);
        }
    }

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        iox::popo::ChunkQueuePopper<ChunkQueueData_t> myQueue(&m_chunkQueueData);
        EXPECT_FALSE(myQueue.empty());
        auto popRet = myQueue.tryPop();
        EXPECT_TRUE(popRet.has_value());
        auto dummySample = *reinterpret_cast<DummySample*>(popRet->getUserPayload());
        EXPECT_THAT(dummySample.dummy, Eq(i));
        EXPECT_THAT(popRet->getChunkHeader()->sequenceNumber(), Eq(i));
    }
}

TEST_F(ChunkSender_test, sendTillRunningOutOfChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b951495a-e216-43ff-96a0-a530b7a6455b");
    ASSERT_FALSE(m_chunkSender.tryAddQueue(&m_chunkQueueData).has_error());
    iox::popo::ChunkQueuePopper<ChunkQueueData_t> checkQueue(&m_chunkQueueData);
    EXPECT_TRUE(NUM_CHUNKS_IN_POOL <= checkQueue.getCurrentCapacity());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        EXPECT_FALSE(maybeChunkHeader.has_error());

        if (!maybeChunkHeader.has_error())
        {
            auto sample = (*maybeChunkHeader)->userPayload();
            new (sample) DummySample();
            static_cast<DummySample*>(sample)->dummy = i;
            m_chunkSender.send(*maybeChunkHeader);
        }
    }

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::PoshError, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_TRUE(maybeChunkHeader.has_error());
    EXPECT_THAT(maybeChunkHeader.get_error(), Eq(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS));
}

TEST_F(ChunkSender_test, sendInvalidChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "72680d04-71aa-4229-84c3-11ce8442e9b3");
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::PoshError, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    ChunkMock<bool> myCrazyChunk;
    m_chunkSender.send(myCrazyChunk.chunkHeader());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(ChunkSender_test, pushToHistory)
{
    ::testing::Test::RecordProperty("TEST_ID", "5ef98161-c7f9-455b-a9db-8eaa1a6b3342");
    for (size_t i = 0; i < 10 * HISTORY_CAPACITY; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        EXPECT_FALSE(maybeChunkHeader.has_error());
        m_chunkSenderWithHistory.pushToHistory(*maybeChunkHeader);
    }

    // Used chunks == history size
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(HISTORY_CAPACITY));
}

TEST_F(ChunkSender_test, pushInvalidChunkToHistory)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b28c59a-ab22-4e2e-8e1a-4246a7dfc38f");
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::PoshError, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    ChunkMock<bool> myCrazyChunk;
    m_chunkSender.pushToHistory(myCrazyChunk.chunkHeader());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(ChunkSender_test, sendMultipleWithReceiverNoLastReuse)
{
    ::testing::Test::RecordProperty("TEST_ID", "955b4e9d-6c17-45d4-85ca-3a4411e71957");
    ASSERT_FALSE(m_chunkSender.tryAddQueue(&m_chunkQueueData).has_error());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        ASSERT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSender.tryGetPreviousChunk();
        if (i > 0)
        {
            ASSERT_TRUE(maybeLastChunk.has_value());
            // No last chunk for us :-(
            EXPECT_FALSE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_FALSE((*maybeChunkHeader)->userPayload() == (*maybeLastChunk)->userPayload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->userPayload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);
    }

    // All Chunks used now
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, NUM_CHUNKS_IN_POOL);
}

TEST_F(ChunkSender_test, sendMultipleWithReceiverLastReuseBecauseAlreadyConsumed)
{
    ::testing::Test::RecordProperty("TEST_ID", "18ffac1d-1ac9-4318-94f0-1178f3d68a2a");
    ASSERT_FALSE(m_chunkSender.tryAddQueue(&m_chunkQueueData).has_error());

    for (size_t i = 0; i < NUM_CHUNKS_IN_POOL; i++)
    {
        auto maybeChunkHeader = m_chunkSender.tryAllocate(
            iox::UniquePortId(), sizeof(DummySample), alignof(DummySample), USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        ASSERT_FALSE(maybeChunkHeader.has_error());
        auto maybeLastChunk = m_chunkSender.tryGetPreviousChunk();
        if (i > 0)
        {
            ASSERT_TRUE(maybeLastChunk.has_value());
            // We get the last chunk again
            EXPECT_TRUE(*maybeChunkHeader == *maybeLastChunk);
            EXPECT_TRUE((*maybeChunkHeader)->userPayload() == (*maybeLastChunk)->userPayload());
        }
        else
        {
            EXPECT_FALSE(maybeLastChunk.has_value());
        }
        auto sample = (*maybeChunkHeader)->userPayload();
        new (sample) DummySample();
        m_chunkSender.send(*maybeChunkHeader);

        iox::popo::ChunkQueuePopper<ChunkQueueData_t> myQueue(&m_chunkQueueData);
        EXPECT_FALSE(myQueue.empty());
        auto popRet = myQueue.tryPop();
        EXPECT_TRUE(popRet.has_value());
    }

    // All consumed but the lastChunk
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, 1);
}

TEST_F(ChunkSender_test, ReuseLastIfSmaller)
{
    ::testing::Test::RecordProperty("TEST_ID", "cff81129-75aa-45bb-a427-870286fb3ee5");
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), BIG_CHUNK, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1U));

    auto chunkHeader = *maybeChunkHeader;
    m_chunkSender.send(chunkHeader);

    auto chunkSmaller = m_chunkSender.tryAllocate(
        iox::UniquePortId(), SMALL_CHUNK, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(chunkSmaller.has_error());

    // no small chunk used as big one is recycled
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1U));

    auto maybeLastChunk = m_chunkSender.tryGetPreviousChunk();
    ASSERT_TRUE(maybeLastChunk.has_value());
    // We get the last chunk again
    EXPECT_TRUE(*chunkSmaller == *maybeLastChunk);
    EXPECT_TRUE((*chunkSmaller)->userPayload() == (*maybeLastChunk)->userPayload());
}

TEST_F(ChunkSender_test, NoReuseOfLastIfBigger)
{
    ::testing::Test::RecordProperty("TEST_ID", "44eb7a6c-d50e-4915-a458-e401e96c4c6d");
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), SMALL_CHUNK, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    auto chunkHeader = *maybeChunkHeader;
    m_chunkSender.send(chunkHeader);

    auto chunkBigger = m_chunkSender.tryAllocate(
        iox::UniquePortId(), BIG_CHUNK, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(chunkBigger.has_error());

    // no reuse, we hav a small and a big chunk in use
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(1U));

    auto maybeLastChunk = m_chunkSender.tryGetPreviousChunk();
    ASSERT_TRUE(maybeLastChunk.has_value());
    // not the last chunk
    EXPECT_FALSE(*chunkBigger == *maybeLastChunk);
    EXPECT_FALSE((*chunkBigger)->userPayload() == (*maybeLastChunk)->userPayload());
}

TEST_F(ChunkSender_test, ReuseOfLastIfBiggerButFitsInChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "1deadbe1-7877-486a-941e-9d41d03b5aba");
    auto maybeChunkHeader = m_chunkSender.tryAllocate(
        iox::UniquePortId(), SMALL_CHUNK - 10, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(maybeChunkHeader.has_error());
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    auto chunkHeader = *maybeChunkHeader;
    m_chunkSender.send(chunkHeader);

    auto chunkBigger = m_chunkSender.tryAllocate(
        iox::UniquePortId(), SMALL_CHUNK, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(chunkBigger.has_error());

    // reuse as it still fits in the small chunk
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(1).m_usedChunks, Eq(0U));

    auto maybeLastChunk = m_chunkSender.tryGetPreviousChunk();
    ASSERT_TRUE(maybeLastChunk.has_value());
    // not the last chunk
    EXPECT_TRUE(*chunkBigger == *maybeLastChunk);
    EXPECT_TRUE((*chunkBigger)->userPayload() == (*maybeLastChunk)->userPayload());
}

TEST_F(ChunkSender_test, Cleanup)
{
    ::testing::Test::RecordProperty("TEST_ID", "5e5ab921-24bf-45a9-9572-68e444120baa");
    EXPECT_TRUE((HISTORY_CAPACITY + iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY) <= NUM_CHUNKS_IN_POOL);

    for (size_t i = 0; i < HISTORY_CAPACITY; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.tryAllocate(
            iox::UniquePortId(), SMALL_CHUNK, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        EXPECT_FALSE(maybeChunkHeader.has_error());
        m_chunkSenderWithHistory.send(*maybeChunkHeader);
    }

    for (size_t i = 0; i < iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY; i++)
    {
        auto maybeChunkHeader = m_chunkSenderWithHistory.tryAllocate(
            iox::UniquePortId(), SMALL_CHUNK, USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
        EXPECT_FALSE(maybeChunkHeader.has_error());
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks,
                Eq(HISTORY_CAPACITY + iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY));

    m_chunkSenderWithHistory.releaseAll();

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

} // namespace
