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

#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/bump_allocator.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::popo;
using namespace iox::mepoo;
using namespace iox::units::duration_literals;

using iox::UniqueId;

class ChunkQueue_testBase
{
  public:
    SharedChunk allocateChunk()
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
        return SharedChunk(chunkMgmt);
    }

    static constexpr uint32_t USER_PAYLOAD_SIZE{128U};
    static constexpr size_t MEGABYTE = 1U << 20U;
    static constexpr size_t MEMORY_SIZE = 4U * MEGABYTE;
    std::unique_ptr<char[]> memory{new char[MEMORY_SIZE]};
    iox::BumpAllocator allocator{memory.get(), MEMORY_SIZE};
    MemPool mempool{
        sizeof(ChunkHeader) + USER_PAYLOAD_SIZE, 2U * iox::MAX_SUBSCRIBER_QUEUE_CAPACITY, allocator, allocator};
    MemPool chunkMgmtPool{128U, 2U * iox::MAX_SUBSCRIBER_QUEUE_CAPACITY, allocator, allocator};

    static constexpr uint32_t RESIZED_CAPACITY{5U};
};

template <typename PolicyType, iox::popo::VariantQueueTypes VariantQueueType>
struct TypeDefinitions
{
    using PolicyType_t = PolicyType;
    static const iox::popo::VariantQueueTypes variantQueueType{VariantQueueType};
};

using ChunkQueueSubjects =
    Types<TypeDefinitions<ThreadSafePolicy, iox::popo::VariantQueueTypes::FiFo_SingleProducerSingleConsumer>,
          TypeDefinitions<ThreadSafePolicy, iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer>,
          TypeDefinitions<SingleThreadedPolicy, iox::popo::VariantQueueTypes::FiFo_SingleProducerSingleConsumer>,
          TypeDefinitions<SingleThreadedPolicy, iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer>>;

TYPED_TEST_SUITE(ChunkQueue_test, ChunkQueueSubjects, );

template <typename TestTypes>
class ChunkQueue_test : public Test, public ChunkQueue_testBase
{
  public:
    void SetUp() override{};
    void TearDown() override{};

    using ChunkQueueData_t = ChunkQueueData<iox::DefaultChunkQueueConfig, typename TestTypes::PolicyType_t>;

    iox::popo::VariantQueueTypes m_variantQueueType{TestTypes::variantQueueType};
    ChunkQueueData_t m_chunkData{QueueFullPolicy::DISCARD_OLDEST_DATA, m_variantQueueType};
    ChunkQueuePopper<ChunkQueueData_t> m_popper{&m_chunkData};
    ChunkQueuePusher<ChunkQueueData_t> m_pusher{&m_chunkData};
};

TYPED_TEST(ChunkQueue_test, InitialEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "589beb23-5ec7-4ca1-863a-ea0a06920502");
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, InitialConditionVariableAttached)
{
    ::testing::Test::RecordProperty("TEST_ID", "7e6116fa-9bbd-4d63-bfd1-ad49edb7a24e");
    EXPECT_THAT(this->m_popper.isConditionVariableSet(), Eq(false));
}

TYPED_TEST(ChunkQueue_test, UniqueIdIsMonotonicallyIncreasing)
{
    ::testing::Test::RecordProperty("TEST_ID", "e984db40-b43c-4a32-8fda-9618a1c1eecd");
    using ChunkQueueData_t = typename ChunkQueue_test<TypeParam>::ChunkQueueData_t;

    ChunkQueueData_t m_chunkData1{QueueFullPolicy::DISCARD_OLDEST_DATA, this->m_variantQueueType};
    {
        ChunkQueueData_t m_chunkData2{QueueFullPolicy::DISCARD_OLDEST_DATA,
                                      iox::popo::VariantQueueTypes::FiFo_SingleProducerSingleConsumer};
        EXPECT_THAT(static_cast<UniqueId::value_type>(m_chunkData2.m_uniqueId),
                    static_cast<UniqueId::value_type>(m_chunkData1.m_uniqueId) + 1);
    }
    ChunkQueueData_t m_chunkData3{QueueFullPolicy::DISCARD_OLDEST_DATA,
                                  iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    EXPECT_THAT(static_cast<UniqueId::value_type>(m_chunkData3.m_uniqueId),
                static_cast<UniqueId::value_type>(m_chunkData1.m_uniqueId) + 2);
}

TYPED_TEST(ChunkQueue_test, PushOneChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "b73a7167-33f6-4ad3-af1d-71d4ee7feb75");
    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);
    EXPECT_THAT(this->m_popper.empty(), Eq(false));
    /// @note size not implemented on FIFO
    if (this->m_variantQueueType != iox::popo::VariantQueueTypes::FiFo_SingleProducerSingleConsumer)
    {
        EXPECT_THAT(this->m_popper.size(), Eq(1U));
    }
}

TYPED_TEST(ChunkQueue_test, PopOneChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fac3e28-5d2a-4321-a176-c7b7a58a93c7");
    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);

    EXPECT_THAT(this->m_popper.tryPop().has_value(), Eq(true));
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
    /// @note size not implemented on FIFO
    if (this->m_variantQueueType != iox::popo::VariantQueueTypes::FiFo_SingleProducerSingleConsumer)
    {
        EXPECT_THAT(this->m_popper.size(), Eq(0U));
    }
}

TYPED_TEST(ChunkQueue_test, PushedChunksMustBePoppedInTheSameOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "6cbc1535-aea1-4d85-ae0a-a20e4fce3032");
    constexpr int32_t NUMBER_CHUNKS{5};
    for (int i = 0; i < NUMBER_CHUNKS; ++i)
    {
        auto chunk = this->allocateChunk();
        *reinterpret_cast<int32_t*>(chunk.getUserPayload()) = i;
        this->m_pusher.push(chunk);
    }

    for (int i = 0; i < NUMBER_CHUNKS; ++i)
    {
        auto maybeSharedChunk = this->m_popper.tryPop();
        ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
        auto data = *reinterpret_cast<int32_t*>((*maybeSharedChunk).getUserPayload());
        EXPECT_THAT(data, Eq(i));
    }
}

TYPED_TEST(ChunkQueue_test, PopChunkWithIncompatibleChunkHeaderCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "597f1da3-6f64-4254-9e41-0c4776746a14");
    auto chunk = this->allocateChunk();
    // this is currently the only possibility to test an invalid CHUNK_HEADER_VERSION
    auto chunkHeaderAddress = reinterpret_cast<uint64_t>(chunk.getChunkHeader());
    auto chunkHeaderVersionAddress = chunkHeaderAddress + sizeof(uint32_t);
    auto chunkHeaderVersionPointer = reinterpret_cast<uint8_t*>(chunkHeaderVersionAddress);
    *chunkHeaderVersionPointer = std::numeric_limits<uint8_t>::max();

    this->m_pusher.push(chunk);

    EXPECT_FALSE(this->m_popper.tryPop().has_value());
    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__CHUNK_QUEUE_POPPER_CHUNK_WITH_INCOMPATIBLE_CHUNK_HEADER_VERSION);
}

TYPED_TEST(ChunkQueue_test, ClearOnEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "9923de92-5c69-4b79-9f3b-793e790d07f3");
    this->m_popper.clear();
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, ClearWithData)
{
    ::testing::Test::RecordProperty("TEST_ID", "797f2fba-1734-4d16-be3b-46c0bf2ead8c");
    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);
    this->m_popper.clear();
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, AttachConditionVariable)
{
    ::testing::Test::RecordProperty("TEST_ID", "5893ac55-bc8d-47b6-baa7-1282dbf4c849");
    ConditionVariableData condVar("Horscht");

    this->m_popper.setConditionVariable(condVar, 0U);

    EXPECT_THAT(this->m_popper.isConditionVariableSet(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, PushAndNotifyConditionVariable)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf87457d-fb2d-4d65-a8f2-9b4457e50b51");
    ConditionVariableData condVar("Horscht");
    ConditionListener condVarWaiter{condVar};

    this->m_popper.setConditionVariable(condVar, 0U);

    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);

    EXPECT_THAT(condVarWaiter.timedWait(1_ns).empty(), Eq(false));
    EXPECT_THAT(condVarWaiter.timedWait(1_ns).empty(), Eq(true)); // shouldn't trigger a second time
}

TYPED_TEST(ChunkQueue_test, AttachSecondConditionVariable)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e55346f-62e1-44bb-bfe8-cef929935edf");
    ConditionVariableData condVar1("Horscht");
    ConditionVariableData condVar2("Schnuppi");
    ConditionListener condVarWaiter1{condVar1};
    ConditionListener condVarWaiter2{condVar2};

    this->m_popper.setConditionVariable(condVar1, 0U);
    this->m_popper.setConditionVariable(condVar2, 1U);

    EXPECT_THAT(condVarWaiter1.timedWait(1_ns).empty(), Eq(true));
    EXPECT_THAT(condVarWaiter2.timedWait(1_ns).empty(), Eq(true));

    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);

    EXPECT_THAT(condVarWaiter1.timedWait(1_ms).empty(), Eq(true));
    EXPECT_THAT(condVarWaiter2.timedWait(1_ms).empty(), Eq(false));
}

/// @note this could be changed to a parameterized ChunkQueueSaturatingFIFO_test when there are more FIFOs available
using ChunkQueueFiFoTestSubjects = Types<ThreadSafePolicy, SingleThreadedPolicy>;

TYPED_TEST_SUITE(ChunkQueueFiFo_test, ChunkQueueFiFoTestSubjects, );

template <typename PolicyType>
class ChunkQueueFiFo_test : public Test, public ChunkQueue_testBase
{
  public:
    void SetUp() override{};
    void TearDown() override{};

    using ChunkQueueData_t = ChunkQueueData<iox::DefaultChunkQueueConfig, PolicyType>;

    ChunkQueueData_t m_chunkData{QueueFullPolicy::DISCARD_OLDEST_DATA,
                                 iox::popo::VariantQueueTypes::FiFo_SingleProducerSingleConsumer};
    ChunkQueuePopper<ChunkQueueData_t> m_popper{&m_chunkData};
    ChunkQueuePusher<ChunkQueueData_t> m_pusher{&m_chunkData};
};

TYPED_TEST(ChunkQueueFiFo_test, InitialSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "f5bf5cc0-0822-47a0-bbe2-76bbc9b12592");
    EXPECT_THAT(this->m_popper.size(), Eq(0U));
}

TYPED_TEST(ChunkQueueFiFo_test, Capacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "080fb9a1-7266-45a5-9c5c-e12a189b4b64");
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(iox::MAX_SUBSCRIBER_QUEUE_CAPACITY));
}

/// @note API currently not supported
TYPED_TEST(ChunkQueueFiFo_test, SetCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2e1c144-bb55-4423-b62b-bfd8d5a64927");
    GTEST_SKIP() << "@todo iox-#615 API currently not supported";
    this->m_popper.setCapacity(this->RESIZED_CAPACITY);
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(this->RESIZED_CAPACITY));
}

TYPED_TEST(ChunkQueueFiFo_test, PushFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "04378227-277b-486b-9acd-f9ada80f4b4e");
    for (auto i = 0U; i < iox::MAX_SUBSCRIBER_QUEUE_CAPACITY; ++i)
    {
        auto chunk = this->allocateChunk();
        EXPECT_TRUE(this->m_pusher.push(chunk));
    }

    {
        auto chunk = this->allocateChunk();
        EXPECT_FALSE(this->m_pusher.push(chunk));
    }

    // get all the chunks in the queue
    while (this->m_popper.tryPop().has_value())
    {
    }

    // all chunks must be released
    EXPECT_THAT(this->mempool.getUsedChunks(), Eq(0U));
}

/// @note this could be changed to a parameterized ChunkQueueOverflowingFIFO_test when there are more FIFOs available
using ChunkQueueSoFiSubjects = Types<ThreadSafePolicy, SingleThreadedPolicy>;

TYPED_TEST_SUITE(ChunkQueueSoFi_test, ChunkQueueSoFiSubjects, );

template <typename PolicyType>
class ChunkQueueSoFi_test : public Test, public ChunkQueue_testBase
{
  public:
    void SetUp() override{};
    void TearDown() override{};

    using ChunkQueueData_t = ChunkQueueData<iox::DefaultChunkQueueConfig, PolicyType>;

    ChunkQueueData_t m_chunkData{QueueFullPolicy::DISCARD_OLDEST_DATA,
                                 iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkQueuePopper<ChunkQueueData_t> m_popper{&m_chunkData};
    ChunkQueuePusher<ChunkQueueData_t> m_pusher{&m_chunkData};
};

TYPED_TEST(ChunkQueueSoFi_test, InitialSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "2521ae60-85bb-45df-9dd0-5159f9dc90ce");
    EXPECT_THAT(this->m_popper.size(), Eq(0U));
}

TYPED_TEST(ChunkQueueSoFi_test, Capacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "093a05f5-fa09-4e6a-9b32-e3a716c16464");
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(iox::MAX_SUBSCRIBER_QUEUE_CAPACITY));
}


TYPED_TEST(ChunkQueueSoFi_test, SetCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "e15eaaf0-2858-4f17-999a-4a2e03a86e99");
    this->m_popper.setCapacity(this->RESIZED_CAPACITY);
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(this->RESIZED_CAPACITY));
}

TYPED_TEST(ChunkQueueSoFi_test, PushFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "74fca742-d53a-4503-ab9a-95733f94844a");
    for (auto i = 0u; i < iox::MAX_SUBSCRIBER_QUEUE_CAPACITY; ++i)
    {
        auto chunk = this->allocateChunk();
        EXPECT_TRUE(this->m_pusher.push(chunk));
    }

    for (auto i = 0U; i < iox::MAX_SUBSCRIBER_QUEUE_CAPACITY; ++i)
    {
        auto chunk = this->allocateChunk();
        EXPECT_FALSE(this->m_pusher.push(chunk));
    }

    // get all the chunks in the queue
    while (this->m_popper.tryPop().has_value())
    {
    }

    // all chunks must be released
    EXPECT_THAT(this->mempool.getUsedChunks(), Eq(0U));
}


TYPED_TEST(ChunkQueueSoFi_test, InitialNoLostChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf7298e9-fd5b-4b7e-8688-37580713050f");
    EXPECT_FALSE(this->m_popper.hasLostChunks());
}

TYPED_TEST(ChunkQueueSoFi_test, IndicateALostChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "adec2b96-2589-4974-8877-4dd1b46603c6");
    this->m_pusher.lostAChunk();

    EXPECT_TRUE(this->m_popper.hasLostChunks());
}

TYPED_TEST(ChunkQueueSoFi_test, LostChunkInfoIsResetAfterRead)
{
    ::testing::Test::RecordProperty("TEST_ID", "a739477d-1b27-46da-8682-cc52d2c05bfd");
    this->m_pusher.lostAChunk();
    this->m_popper.hasLostChunks();

    EXPECT_FALSE(this->m_popper.hasLostChunks());
}

} // namespace
