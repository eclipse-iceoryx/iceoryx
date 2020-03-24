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

#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/variant_queue.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;

class ChunkDistributor_test : public Test
{
  public:
    SharedChunk allocateChunk(uint32_t value)
    {
        ChunkManagement* chunkMgmt = static_cast<ChunkManagement*>(chunkMgmtPool.getChunk());
        auto chunk = mempool.getChunk();
        ChunkHeader* chunkHeader = new (chunk) ChunkHeader();
        new (chunkMgmt) ChunkManagement{chunkHeader, &mempool, &chunkMgmtPool};
        *static_cast<uint32_t*>(chunkHeader->payload()) = value;
        return SharedChunk(chunkMgmt);
    }
    uint32_t getSharedChunkValue(const SharedChunk& chunk)
    {
        return *static_cast<uint32_t*>(chunk.getPayload());
    }

    static constexpr size_t MEGABYTE = 1 << 20;
    static constexpr size_t MEMORY_SIZE = 1 * MEGABYTE;
    char memory[MEMORY_SIZE];
    iox::posix::Allocator allocator{memory, MEMORY_SIZE};
    MemPool mempool{128, 20, &allocator, &allocator};
    MemPool chunkMgmtPool{128, 20, &allocator, &allocator};


    void SetUp(){};
    void TearDown(){};

    std::shared_ptr<ChunkQueueData> getChunkQueueData()
    {
        return std::make_shared<ChunkQueueData>(VariantQueueTypes::SoFi_SingleProducerSingleConsumer);
    }

    std::shared_ptr<ChunkDistributorData> getChunkDistributorData()
    {
        return std::make_shared<ChunkDistributorData>();
    }
};

TEST_F(ChunkDistributor_test, AddingNonAddedQueueWorks)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    EXPECT_THAT(sut.addQueue(queueData.get()), Eq(true));
}

TEST_F(ChunkDistributor_test, AddingNullptrQueueDoesNotWork)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    EXPECT_THAT(sut.addQueue(nullptr), Eq(false));
}

TEST_F(ChunkDistributor_test, AddingQueueTwiceDoesWork)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());
    EXPECT_THAT(sut.addQueue(queueData.get()), Eq(true));
}

TEST_F(ChunkDistributor_test, NewChunkDistributorHasNoQueues)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TEST_F(ChunkDistributor_test, AfterAddingQueueChunkDistributorHasQueues)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());
    EXPECT_THAT(sut.hasStoredQueues(), Eq(true));
}

TEST_F(ChunkDistributor_test, AfterAddingQueueFailureNoQueuesAreStored)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    sut.addQueue(nullptr);
}

TEST_F(ChunkDistributor_test, RemovingExistingQueueWorks)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());
    sut.removeQueue(queueData.get());

    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TEST_F(ChunkDistributor_test, RemovingNonExistingQueueChangesNothing)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());

    auto queueData2 = getChunkQueueData();
    sut.removeQueue(queueData2.get());

    EXPECT_THAT(sut.hasStoredQueues(), Eq(true));
}

TEST_F(ChunkDistributor_test, RemoveAllQueuesWhenContainingOne)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());
    sut.removeAllQueues();

    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TEST_F(ChunkDistributor_test, RemoveAllQueuesWhenContainingMultipleQueues)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());
    auto queueData2 = getChunkQueueData();
    sut.addQueue(queueData2.get());
    sut.removeAllQueues();

    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TEST_F(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueue)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());

    auto chunk = allocateChunk(4451);
    sut.deliverToAllStoredQueues(chunk);

    ChunkQueue queue(queueData.get());
    auto result = queue.pop();

    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(4451));
}

TEST_F(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueueDeliversOneChunk)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());

    auto chunk = allocateChunk(4451);
    sut.deliverToAllStoredQueues(chunk);

    ChunkQueue queue(queueData.get());
    EXPECT_THAT(queue.size(), Eq(1));
    EXPECT_THAT(sut.getHistorySize(), Eq(1));
}

TEST_F(ChunkDistributor_test, DeliverToAllStoredQueuesWithDuplicatedQueueDeliversOneChunk)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());
    sut.addQueue(queueData.get());
    sut.addQueue(queueData.get());

    auto chunk = allocateChunk(4451);
    sut.deliverToAllStoredQueues(chunk);

    ChunkQueue queue(queueData.get());
    EXPECT_THAT(queue.size(), Eq(1));
    EXPECT_THAT(sut.getHistorySize(), Eq(1));
}


TEST_F(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueueMultipleChunks)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());

    auto limit = 10;
    for (auto i = 0; i < limit; ++i)
        sut.deliverToAllStoredQueues(allocateChunk(i * 123));

    ChunkQueue queue(queueData.get());
    for (auto i = 0; i < limit; ++i)
    {
        auto result = queue.pop();

        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(getSharedChunkValue(*result), Eq(i * 123));
    }
}

TEST_F(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueueDeliverMultipleChunks)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());

    auto limit = 10;
    for (auto i = 0; i < limit; ++i)
        sut.deliverToAllStoredQueues(allocateChunk(i * 123));

    ChunkQueue queue(queueData.get());
    EXPECT_THAT(queue.size(), Eq(limit));
    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TEST_F(ChunkDistributor_test, DeliverToAllStoredQueuesWithMultipleQueues)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto limit = 10;
    std::vector<std::shared_ptr<ChunkQueueData>> queueData;
    for (auto i = 0; i < limit; ++i)
    {
        queueData.emplace_back(getChunkQueueData());
        sut.addQueue(queueData.back().get());
    }

    auto chunk = allocateChunk(24451);
    sut.deliverToAllStoredQueues(chunk);

    for (auto i = 0; i < limit; ++i)
    {
        ChunkQueue queue(queueData[i].get());
        auto result = queue.pop();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(getSharedChunkValue(*result), Eq(24451));
    }
    EXPECT_THAT(sut.getHistorySize(), Eq(1));
}

TEST_F(ChunkDistributor_test, DeliverToAllStoredQueuesWithMultipleQueuesMultipleChunks)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto limit = 10;
    std::vector<std::shared_ptr<ChunkQueueData>> queueData;
    for (auto i = 0; i < limit; ++i)
    {
        queueData.emplace_back(getChunkQueueData());
        sut.addQueue(queueData.back().get());
    }

    for (auto i = 0; i < limit; ++i)
        sut.deliverToAllStoredQueues(allocateChunk(i * 34));

    for (auto i = 0; i < limit; ++i)
    {
        for (auto k = 0; k < limit; ++k)
        {
            ChunkQueue queue(queueData[i].get());
            auto result = queue.pop();
            ASSERT_THAT(result.has_value(), Eq(true));
            EXPECT_THAT(getSharedChunkValue(*result), Eq(k * 34));
        }
    }
    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TEST_F(ChunkDistributor_test, AddToHistoryWithoutQueues)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());
    auto limit = 8;
    for (auto i = 0; i < limit; ++i)
        sut.deliverToAllStoredQueues(allocateChunk(34));

    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TEST_F(ChunkDistributor_test, HistoryEmptyWhenCreated)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());
    EXPECT_THAT(sut.getHistorySize(), Eq(0));
}

TEST_F(ChunkDistributor_test, HistoryEmptyAfterClear)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());
    auto limit = 8;
    for (auto i = 0; i < limit; ++i)
        sut.deliverToAllStoredQueues(allocateChunk(34));
    sut.clearHistory();

    EXPECT_THAT(sut.getHistorySize(), Eq(0));
}

TEST_F(ChunkDistributor_test, addToHistoryWithoutDelivery)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());
    auto limit = 7;
    for (auto i = 0; i < limit; ++i)
        sut.addToHistoryWithoutDelivery(allocateChunk(34));

    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TEST_F(ChunkDistributor_test, DeliverToQueueDirectlyWhenNotAdded)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();

    auto chunk = allocateChunk(4451);
    sut.deliverToQueue(queueData.get(), chunk);

    ChunkQueue queue(queueData.get());
    auto result = queue.pop();

    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(4451));
}

TEST_F(ChunkDistributor_test, DeliverToQueueDirectlyWhenAdded)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());

    auto chunk = allocateChunk(451);
    sut.deliverToQueue(queueData.get(), chunk);

    ChunkQueue queue(queueData.get());
    auto result = queue.pop();

    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(451));
}

TEST_F(ChunkDistributor_test, DeliverToQueueDirectlyWhenNotAddedDoesNotChangeHistory)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();

    auto chunk = allocateChunk(4451);
    sut.deliverToQueue(queueData.get(), chunk);

    EXPECT_THAT(sut.getHistorySize(), Eq(0));
}

TEST_F(ChunkDistributor_test, DeliverToQueueDirectlyWhenAddedDoesNotChangeHistory)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    auto queueData = getChunkQueueData();
    sut.addQueue(queueData.get());

    auto chunk = allocateChunk(4451);
    sut.deliverToQueue(queueData.get(), chunk);

    EXPECT_THAT(sut.getHistorySize(), Eq(0));
}

TEST_F(ChunkDistributor_test, DeliverHistoryOnAddWithLessThanAvailable)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    sut.deliverToAllStoredQueues(allocateChunk(1));
    sut.deliverToAllStoredQueues(allocateChunk(2));
    sut.deliverToAllStoredQueues(allocateChunk(3));

    EXPECT_THAT(sut.getHistorySize(), Eq(3));

    // add a queue with a requested history of one must deliver the latest sample
    auto queueData = getChunkQueueData();
    ChunkQueue queue(queueData.get());
    sut.addQueue(queueData.get(), 1);

    EXPECT_THAT(queue.size(), Eq(1));
    auto result = queue.pop();

    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(3));
}

TEST_F(ChunkDistributor_test, DeliverHistoryOnAddWithExactAvailable)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    sut.deliverToAllStoredQueues(allocateChunk(1));
    sut.deliverToAllStoredQueues(allocateChunk(2));
    sut.deliverToAllStoredQueues(allocateChunk(3));

    EXPECT_THAT(sut.getHistorySize(), Eq(3));

    // add a queue with a requested history of 3 must deliver all three in the order oldest to newest
    auto queueData = getChunkQueueData();
    ChunkQueue queue(queueData.get());
    sut.addQueue(queueData.get(), 3);

    EXPECT_THAT(queue.size(), Eq(3));
    auto result = queue.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(1));
    result = queue.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(2));
    result = queue.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(3));
}

TEST_F(ChunkDistributor_test, DeliverHistoryOnAddWithMoreThanAvailable)
{
    auto sutData = getChunkDistributorData();
    ChunkDistributor sut(sutData.get());

    sut.deliverToAllStoredQueues(allocateChunk(1));
    sut.deliverToAllStoredQueues(allocateChunk(2));
    sut.deliverToAllStoredQueues(allocateChunk(3));

    EXPECT_THAT(sut.getHistorySize(), Eq(3));

    // add a queue with a requested history of 5 must deliver only the three available in the order oldest to newest
    auto queueData = getChunkQueueData();
    ChunkQueue queue(queueData.get());
    sut.addQueue(queueData.get(), 5);

    EXPECT_THAT(queue.size(), Eq(3));
    auto result = queue.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(1));
    result = queue.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(2));
    result = queue.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(getSharedChunkValue(*result), Eq(3));
}