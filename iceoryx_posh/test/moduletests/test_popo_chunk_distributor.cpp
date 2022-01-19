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

#include "iceoryx_hoofs/cxx/variant_queue.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "test.hpp"

#include <memory>

namespace
{
using namespace ::testing;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;

using ChunkDistributorTestSubjects = Types<ThreadSafePolicy, SingleThreadedPolicy>;

TYPED_TEST_SUITE(ChunkDistributor_test, ChunkDistributorTestSubjects);


template <typename PolicyType>
class ChunkDistributor_test : public Test
{
  public:
    SharedChunk allocateChunk(uint32_t value)
    {
        ChunkManagement* chunkMgmt = static_cast<ChunkManagement*>(chunkMgmtPool.getChunk());
        auto chunk = mempool.getChunk();

        auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
        EXPECT_FALSE(chunkSettingsResult.has_error());
        if (chunkSettingsResult.has_error())
        {
            return nullptr;
        }
        auto& chunkSettings = chunkSettingsResult.value();

        ChunkHeader* chunkHeader = new (chunk) ChunkHeader(mempool.getChunkSize(), chunkSettings);
        new (chunkMgmt) ChunkManagement{chunkHeader, &mempool, &chunkMgmtPool};
        *static_cast<uint32_t*>(chunkHeader->userPayload()) = value;
        return SharedChunk(chunkMgmt);
    }
    uint32_t getSharedChunkValue(const SharedChunk& chunk)
    {
        return *static_cast<uint32_t*>(chunk.getUserPayload());
    }

    static constexpr uint32_t USER_PAYLOAD_SIZE{128U};
    static constexpr size_t MEGABYTE = 1U << 20U;
    static constexpr size_t MEMORY_SIZE = 1U * MEGABYTE;
    const uint64_t HISTORY_SIZE = 16U;
    static constexpr uint32_t MAX_NUMBER_QUEUES = 128U;
    char memory[MEMORY_SIZE];
    iox::posix::Allocator allocator{memory, MEMORY_SIZE};
    MemPool mempool{sizeof(ChunkHeader) + USER_PAYLOAD_SIZE, 20U, allocator, allocator};
    MemPool chunkMgmtPool{128U, 20U, allocator, allocator};

    struct ChunkDistributorConfig
    {
        static constexpr uint32_t MAX_QUEUES = MAX_NUMBER_QUEUES;
        static constexpr uint64_t MAX_HISTORY_CAPACITY = iox::MAX_PUBLISHER_HISTORY;
    };

    struct ChunkQueueConfig
    {
        static constexpr uint64_t MAX_QUEUE_CAPACITY = MAX_NUMBER_QUEUES;
    };

    using ChunkQueueData_t = ChunkQueueData<ChunkQueueConfig, PolicyType>;
    using ChunkDistributorData_t =
        ChunkDistributorData<ChunkDistributorConfig, PolicyType, ChunkQueuePusher<ChunkQueueData_t>>;
    using ChunkDistributor_t = ChunkDistributor<ChunkDistributorData_t>;

    void SetUp(){};
    void TearDown(){};

    std::shared_ptr<ChunkQueueData_t>
    getChunkQueueData(const QueueFullPolicy policy = QueueFullPolicy::DISCARD_OLDEST_DATA,
                      const VariantQueueTypes queueType = VariantQueueTypes::SoFi_SingleProducerSingleConsumer)
    {
        return std::make_shared<ChunkQueueData_t>(policy, queueType);
    }

    std::shared_ptr<ChunkDistributorData_t>
    getChunkDistributorData(const SubscriberTooSlowPolicy policy = SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA)
    {
        return std::make_shared<ChunkDistributorData_t>(policy, HISTORY_SIZE);
    }

    static constexpr int64_t TIMEOUT_IN_MS = 100;
};
template <typename PolicyType>
constexpr int64_t ChunkDistributor_test<PolicyType>::TIMEOUT_IN_MS;

TYPED_TEST(ChunkDistributor_test, AddingNullptrQueueDoesNotWork)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa7eaa9e-c337-45dc-945a-d097b8916eaa");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    EXPECT_DEATH(IOX_DISCARD_RESULT(sut.tryAddQueue(nullptr)), ".*");
}

TYPED_TEST(ChunkDistributor_test, NewChunkDistributorHasNoQueues)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0a7cec8-ac5f-43c9-b627-0485bc26bbe7");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TYPED_TEST(ChunkDistributor_test, AfterAddingQueueChunkDistributorHasQueues)
{
    ::testing::Test::RecordProperty("TEST_ID", "c7c4683a-b58f-4932-ad82-edc1404da67c");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    auto ret = sut.tryAddQueue(queueData.get());
    EXPECT_FALSE(ret.has_error());
    EXPECT_THAT(sut.hasStoredQueues(), Eq(true));
}

TYPED_TEST(ChunkDistributor_test, QueueOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "581c8cec-b2c5-426c-bf32-eff1f84d59ff");
    std::vector<std::shared_ptr<typename TestFixture::ChunkQueueData_t>> queueVector;
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::PoshError, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0; i < this->MAX_NUMBER_QUEUES; ++i)
    {
        auto queueData = this->getChunkQueueData();
        ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());
        queueVector.push_back(queueData);
    }

    EXPECT_FALSE(errorHandlerCalled);

    auto queueData = this->getChunkQueueData();
    auto ret = sut.tryAddQueue(queueData.get());
    EXPECT_TRUE(ret.has_error());
    EXPECT_THAT(ret.get_error(), Eq(iox::popo::ChunkDistributorError::QUEUE_CONTAINER_OVERFLOW));
    EXPECT_TRUE(errorHandlerCalled);
}

TYPED_TEST(ChunkDistributor_test, RemovingExistingQueueWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc3876ae-24f9-4a3b-b98b-1f6e862bbb6e");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());
    auto ret = sut.tryRemoveQueue(queueData.get());
    EXPECT_FALSE(ret.has_error());
    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TYPED_TEST(ChunkDistributor_test, RemovingNonExistingQueueChangesNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "6245045a-68b5-43c2-a026-e023a23d94c7");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto queueData2 = this->getChunkQueueData();
    auto ret = sut.tryRemoveQueue(queueData2.get());
    EXPECT_TRUE(ret.has_error());
    EXPECT_THAT(ret.get_error(), Eq(iox::popo::ChunkDistributorError::QUEUE_NOT_IN_CONTAINER));
    EXPECT_THAT(sut.hasStoredQueues(), Eq(true));
}

TYPED_TEST(ChunkDistributor_test, RemoveAllQueuesWhenContainingOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "9dac0648-7a7f-4689-bfbd-977d91bb65a4");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());
    sut.removeAllQueues();

    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TYPED_TEST(ChunkDistributor_test, RemoveAllQueuesWhenContainingMultipleQueues)
{
    ::testing::Test::RecordProperty("TEST_ID", "1dc5f2ee-f8d9-48d7-a5f0-e70fe48178fb");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());
    auto queueData2 = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData2.get()).has_error());
    sut.removeAllQueues();

    EXPECT_THAT(sut.hasStoredQueues(), Eq(false));
}

TYPED_TEST(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueue)
{
    ::testing::Test::RecordProperty("TEST_ID", "5bc10e0a-d67b-4123-887c-a50dc16cf680");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto chunk = this->allocateChunk(4451);
    sut.deliverToAllStoredQueues(chunk);

    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    auto maybeSharedChunk = queue.tryPop();

    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(4451u));
}

TYPED_TEST(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueueDeliversOneChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "93a1f4af-b2e4-48a3-b4a4-56a98d1bb66e");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto chunk = this->allocateChunk(4451);
    sut.deliverToAllStoredQueues(chunk);

    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    EXPECT_THAT(queue.size(), Eq(1u));
    EXPECT_THAT(sut.getHistorySize(), Eq(1u));
}

TYPED_TEST(ChunkDistributor_test, DeliverToAllStoredQueuesWithDuplicatedQueueDeliversOneChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f361075-9296-45ac-b355-541e8cc248b2");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto chunk = this->allocateChunk(4451);
    sut.deliverToAllStoredQueues(chunk);

    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    EXPECT_THAT(queue.size(), Eq(1u));
    EXPECT_THAT(sut.getHistorySize(), Eq(1u));
}


TYPED_TEST(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueueMultipleChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a140a2e4-6450-42fe-a8e7-368d0ae795ae");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto limit = 10;
    for (auto i = 0; i < limit; ++i)
        sut.deliverToAllStoredQueues(this->allocateChunk(i * 123));

    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    for (auto i = 0; i < limit; ++i)
    {
        auto maybeSharedChunk = queue.tryPop();

        ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
        EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(i * 123u));
    }
}

TYPED_TEST(ChunkDistributor_test, DeliverToAllStoredQueuesWithOneQueueDeliverMultipleChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b7472f6e-209d-4243-9f0a-04628a76b385");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto limit = 10u;
    for (auto i = 0u; i < limit; ++i)
        sut.deliverToAllStoredQueues(this->allocateChunk(i * 123));

    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    EXPECT_THAT(queue.size(), Eq(limit));
    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TYPED_TEST(ChunkDistributor_test, DeliverToAllStoredQueuesWithMultipleQueues)
{
    ::testing::Test::RecordProperty("TEST_ID", "67669bb8-9413-4419-8bb4-8f362d457744");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto limit = 10;
    std::vector<std::shared_ptr<typename TestFixture::ChunkQueueData_t>> queueData;
    for (auto i = 0; i < limit; ++i)
    {
        queueData.emplace_back(this->getChunkQueueData());
        ASSERT_FALSE(sut.tryAddQueue(queueData.back().get()).has_error());
    }

    auto chunk = this->allocateChunk(24451);
    sut.deliverToAllStoredQueues(chunk);

    for (auto i = 0; i < limit; ++i)
    {
        ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData[i].get());
        auto maybeSharedChunk = queue.tryPop();
        ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
        EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(24451u));
    }
    EXPECT_THAT(sut.getHistorySize(), Eq(1u));
}

TYPED_TEST(ChunkDistributor_test, DeliverToAllStoredQueuesWithMultipleQueuesMultipleChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6930af8f-ab92-44ea-928b-239d45eed807");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto limit = 10u;
    std::vector<std::shared_ptr<typename TestFixture::ChunkQueueData_t>> queueData;
    for (auto i = 0u; i < limit; ++i)
    {
        queueData.emplace_back(this->getChunkQueueData());
        ASSERT_FALSE(sut.tryAddQueue(queueData.back().get()).has_error());
    }

    for (auto i = 0u; i < limit; ++i)
        sut.deliverToAllStoredQueues(this->allocateChunk(i * 34));

    for (auto i = 0u; i < limit; ++i)
    {
        for (auto k = 0u; k < limit; ++k)
        {
            ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData[i].get());
            auto maybeSharedChunk = queue.tryPop();
            ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
            EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(k * 34u));
        }
    }
    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TYPED_TEST(ChunkDistributor_test, AddToHistoryWithoutQueues)
{
    ::testing::Test::RecordProperty("TEST_ID", "1ed709b1-9129-454b-8440-50463ba1c02e");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());
    auto limit = 8u;
    for (auto i = 0u; i < limit; ++i)
        sut.deliverToAllStoredQueues(this->allocateChunk(34));

    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TYPED_TEST(ChunkDistributor_test, HistoryEmptyWhenCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2df42d8-84a6-481b-811f-f8349693682c");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());
    EXPECT_THAT(sut.getHistorySize(), Eq(0u));
}

TYPED_TEST(ChunkDistributor_test, HistoryEmptyAfterClear)
{
    ::testing::Test::RecordProperty("TEST_ID", "87a59dd5-8f85-4166-a296-74eed443290b");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());
    auto limit = 8;
    for (auto i = 0; i < limit; ++i)
        sut.deliverToAllStoredQueues(this->allocateChunk(34));
    sut.clearHistory();

    EXPECT_THAT(sut.getHistorySize(), Eq(0u));
}

TYPED_TEST(ChunkDistributor_test, addToHistoryWithoutDelivery)
{
    ::testing::Test::RecordProperty("TEST_ID", "a01ca23c-c9c8-4695-84b3-0bfc936c96b4");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());
    auto limit = 7u;
    for (auto i = 0u; i < limit; ++i)
        sut.addToHistoryWithoutDelivery(this->allocateChunk(34));

    EXPECT_THAT(sut.getHistorySize(), Eq(limit));
}

TYPED_TEST(ChunkDistributor_test, DeliverToQueueDirectlyWhenNotAdded)
{
    ::testing::Test::RecordProperty("TEST_ID", "168e0415-68fa-4a5c-902b-f0ff29b55dbf");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();

    auto chunk = this->allocateChunk(4451);
    sut.deliverToQueue(queueData.get(), chunk);

    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    auto maybeSharedChunk = queue.tryPop();

    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(4451u));
}

TYPED_TEST(ChunkDistributor_test, DeliverToQueueDirectlyWhenAdded)
{
    ::testing::Test::RecordProperty("TEST_ID", "4edaf3f4-5285-4a9d-b31d-8822f1e46b70");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto chunk = this->allocateChunk(451);
    sut.deliverToQueue(queueData.get(), chunk);

    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    auto maybeSharedChunk = queue.tryPop();

    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(451u));
}

TYPED_TEST(ChunkDistributor_test, DeliverToQueueDirectlyWhenNotAddedDoesNotChangeHistory)
{
    ::testing::Test::RecordProperty("TEST_ID", "539e71d3-3ec5-4aeb-bcac-aa406e0e5828");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();

    auto chunk = this->allocateChunk(4451);
    sut.deliverToQueue(queueData.get(), chunk);

    EXPECT_THAT(sut.getHistorySize(), Eq(0u));
}

TYPED_TEST(ChunkDistributor_test, DeliverToQueueDirectlyWhenAddedDoesNotChangeHistory)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f594607-215e-4db5-bdae-433c185dbbcd");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData = this->getChunkQueueData();
    ASSERT_FALSE(sut.tryAddQueue(queueData.get()).has_error());

    auto chunk = this->allocateChunk(4451);
    sut.deliverToQueue(queueData.get(), chunk);

    EXPECT_THAT(sut.getHistorySize(), Eq(0u));
}

TYPED_TEST(ChunkDistributor_test, DeliverHistoryOnAddWithLessThanAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "faff7ece-c84b-4455-bb12-c83792056e98");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    sut.deliverToAllStoredQueues(this->allocateChunk(1));
    sut.deliverToAllStoredQueues(this->allocateChunk(2));
    sut.deliverToAllStoredQueues(this->allocateChunk(3));

    EXPECT_THAT(sut.getHistorySize(), Eq(3u));

    // add a queue with a requested history of one must deliver the latest sample
    auto queueData = this->getChunkQueueData();
    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    ASSERT_FALSE(sut.tryAddQueue(queueData.get(), 1).has_error());

    EXPECT_THAT(queue.size(), Eq(1u));
    auto maybeSharedChunk = queue.tryPop();

    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(3u));
}

TYPED_TEST(ChunkDistributor_test, DeliverHistoryOnAddWithExactAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "884f4041-f63d-47b7-a6d3-0a84360a3862");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    sut.deliverToAllStoredQueues(this->allocateChunk(1));
    sut.deliverToAllStoredQueues(this->allocateChunk(2));
    sut.deliverToAllStoredQueues(this->allocateChunk(3));

    EXPECT_THAT(sut.getHistorySize(), Eq(3u));

    // add a queue with a requested history of 3 must deliver all three in the order oldest to newest
    auto queueData = this->getChunkQueueData();
    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    ASSERT_FALSE(sut.tryAddQueue(queueData.get(), 3).has_error());

    EXPECT_THAT(queue.size(), Eq(3u));
    auto maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(1u));
    maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(2u));
    maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(3u));
}

TYPED_TEST(ChunkDistributor_test, DeliverHistoryOnAddWithMoreThanAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "64f48f9a-f100-4944-a855-24317a36a97e");
    auto sutData = this->getChunkDistributorData();
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    sut.deliverToAllStoredQueues(this->allocateChunk(1));
    sut.deliverToAllStoredQueues(this->allocateChunk(2));
    sut.deliverToAllStoredQueues(this->allocateChunk(3));

    EXPECT_THAT(sut.getHistorySize(), Eq(3u));

    // add a queue with a requested history of 5 must deliver only the three available in the order oldest to newest
    auto queueData = this->getChunkQueueData();
    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    ASSERT_FALSE(sut.tryAddQueue(queueData.get(), 5).has_error());

    EXPECT_THAT(queue.size(), Eq(3u));
    auto maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(1u));
    maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(2u));
    maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(3u));
}

TYPED_TEST(ChunkDistributor_test, DeliverToSingleQueueBlocksWhenOptionsAreSetToBlocking)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0500dec-bbd8-4958-9545-a14ef68108a1");
    auto sutData = this->getChunkDistributorData(SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER);
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    auto queueData =
        this->getChunkQueueData(QueueFullPolicy::BLOCK_PUBLISHER, VariantQueueTypes::FiFo_MultiProducerSingleConsumer);
    ChunkQueuePopper<typename TestFixture::ChunkQueueData_t> queue(queueData.get());
    queue.setCapacity(1U);

    ASSERT_FALSE(sut.tryAddQueue(queueData.get(), 0U).has_error());
    sut.deliverToAllStoredQueues(this->allocateChunk(155U));

    auto threadSyncSemaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U);
    std::atomic_bool wasChunkDelivered{false};
    std::thread t1([&] {
        ASSERT_FALSE(threadSyncSemaphore->post().has_error());
        sut.deliverToAllStoredQueues(this->allocateChunk(152U));
        wasChunkDelivered = true;
    });

    ASSERT_FALSE(threadSyncSemaphore->wait().has_error());
    std::this_thread::sleep_for(std::chrono::milliseconds(this->TIMEOUT_IN_MS));
    EXPECT_THAT(wasChunkDelivered.load(), Eq(false));

    auto maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(155U));

    t1.join(); // join needs to be before the load to ensure the wasChunkDelivered store happens before the read
    EXPECT_THAT(wasChunkDelivered.load(), Eq(true));

    maybeSharedChunk = queue.tryPop();
    ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
    EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(152U));
}

TYPED_TEST(ChunkDistributor_test, MultipleBlockingQueuesWillBeFilledWhenThereBecomesSpaceAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "8168749d-8472-4999-83b0-5b36a77b04ed");
    auto sutData = this->getChunkDistributorData(SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER);
    typename TestFixture::ChunkDistributor_t sut(sutData.get());

    std::vector<std::shared_ptr<typename TestFixture::ChunkQueueData_t>> queueDatas;
    std::vector<ChunkQueuePopper<typename TestFixture::ChunkQueueData_t>> queues;

    constexpr uint64_t NUMBER_OF_QUEUES = 4U;

    for (uint64_t i = 0U; i < NUMBER_OF_QUEUES; ++i)
    {
        queueDatas.emplace_back(this->getChunkQueueData(QueueFullPolicy::BLOCK_PUBLISHER,
                                                        VariantQueueTypes::FiFo_MultiProducerSingleConsumer));
        queues.emplace_back(queueDatas.back().get());
        queues.back().setCapacity(1U);
        ASSERT_FALSE(sut.tryAddQueue(queueDatas.back().get(), 0U).has_error());
    }

    sut.deliverToAllStoredQueues(this->allocateChunk(425U));

    auto threadSyncSemaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U);
    std::atomic_bool wasChunkDelivered{false};
    std::thread t1([&] {
        ASSERT_FALSE(threadSyncSemaphore->post().has_error());
        sut.deliverToAllStoredQueues(this->allocateChunk(1152U));
        wasChunkDelivered = true;
    });

    ASSERT_FALSE(threadSyncSemaphore->wait().has_error());
    std::this_thread::sleep_for(std::chrono::milliseconds(this->TIMEOUT_IN_MS));
    EXPECT_THAT(wasChunkDelivered.load(), Eq(false));

    for (uint64_t i = 0U; i < NUMBER_OF_QUEUES; ++i)
    {
        auto maybeSharedChunk = queues[i].tryPop();
        ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
        EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(425U));

        if (i + 1U == NUMBER_OF_QUEUES)
        {
            // join needs to be before the load to ensure the wasChunkDelivered store happens before the read
            t1.join();
            EXPECT_THAT(wasChunkDelivered.load(), Eq(true));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(this->TIMEOUT_IN_MS));
            EXPECT_THAT(wasChunkDelivered.load(), Eq(false));
        }

        maybeSharedChunk = queues[i].tryPop();
        ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
        EXPECT_THAT(this->getSharedChunkValue(*maybeSharedChunk), Eq(1152U));
    }
}

} // namespace
