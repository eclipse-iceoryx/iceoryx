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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/mepoo/typed_mem_pool.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_waiter.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::Return;

using namespace iox::popo;
using namespace iox::mepoo;
using namespace iox::units::duration_literals;

class ChunkQueue_testBase
{
  public:
    SharedChunk allocateChunk()
    {
        ChunkManagement* chunkMgmt = static_cast<ChunkManagement*>(chunkMgmtPool.getChunk());
        auto chunk = mempool.getChunk();
        ChunkHeader* chunkHeader = new (chunk) ChunkHeader();
        new (chunkMgmt) ChunkManagement{chunkHeader, &mempool, &chunkMgmtPool};
        return SharedChunk(chunkMgmt);
    }

    static constexpr size_t MEGABYTE = 1 << 20;
    static constexpr size_t MEMORY_SIZE = 4 * MEGABYTE;
    std::unique_ptr<char[]> memory{new char[MEMORY_SIZE]};
    iox::posix::Allocator allocator{memory.get(), MEMORY_SIZE};
    MemPool mempool{128, 2 * iox::MAX_SUBSCRIBER_QUEUE_CAPACITY, &allocator, &allocator};
    MemPool chunkMgmtPool{128, 2 * iox::MAX_SUBSCRIBER_QUEUE_CAPACITY, &allocator, &allocator};

    static constexpr uint32_t RESIZED_CAPACITY{5u};
};

template <typename PolicyType, iox::cxx::VariantQueueTypes VariantQueueType>
struct TypeDefinitions
{
    using PolicyType_t = PolicyType;
    static const iox::cxx::VariantQueueTypes variantQueueType{VariantQueueType};
};

using ChunkQueueSubjects =
    Types<TypeDefinitions<ThreadSafePolicy, iox::cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer>,
          TypeDefinitions<ThreadSafePolicy, iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer>,
          TypeDefinitions<SingleThreadedPolicy, iox::cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer>,
          TypeDefinitions<SingleThreadedPolicy, iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer>>;

/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(ChunkQueue_test, ChunkQueueSubjects);
#pragma GCC diagnostic pop

template <typename TestTypes>
class ChunkQueue_test : public Test, public ChunkQueue_testBase
{
  public:
    void SetUp() override{};
    void TearDown() override{};

    using ChunkQueueData_t = ChunkQueueData<iox::DefaultChunkQueueConfig, typename TestTypes::PolicyType_t>;

    iox::cxx::VariantQueueTypes m_variantQueueType{TestTypes::variantQueueType};
    ChunkQueueData_t m_chunkData{m_variantQueueType};
    ChunkQueuePopper<ChunkQueueData_t> m_popper{&m_chunkData};
    ChunkQueuePusher<ChunkQueueData_t> m_pusher{&m_chunkData};
};

TYPED_TEST(ChunkQueue_test, InitialEmpty)
{
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, InitialConditionVariableAttached)
{
    EXPECT_THAT(this->m_popper.isConditionVariableSet(), Eq(false));
}

TYPED_TEST(ChunkQueue_test, PushOneChunk)
{
    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);
    EXPECT_THAT(this->m_popper.empty(), Eq(false));
    /// @note size not implemented on FIFO
    if (this->m_variantQueueType != iox::cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer)
    {
        EXPECT_THAT(this->m_popper.size(), Eq(1u));
    }
}

TYPED_TEST(ChunkQueue_test, PopOneChunk)
{
    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);

    EXPECT_THAT(this->m_popper.tryPop().has_value(), Eq(true));
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
    /// @note size not implemented on FIFO
    if (this->m_variantQueueType != iox::cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer)
    {
        EXPECT_THAT(this->m_popper.size(), Eq(0u));
    }
}

TYPED_TEST(ChunkQueue_test, PushedChunksMustBePoppedInTheSameOrder)
{
    constexpr int32_t NUMBER_CHUNKS{5};
    for (int i = 0; i < NUMBER_CHUNKS; ++i)
    {
        auto chunk = this->allocateChunk();
        *reinterpret_cast<int32_t*>(chunk.getPayload()) = i;
        this->m_pusher.push(chunk);
    }

    for (int i = 0; i < NUMBER_CHUNKS; ++i)
    {
        auto maybeSharedChunk = this->m_popper.tryPop();
        ASSERT_THAT(maybeSharedChunk.has_value(), Eq(true));
        auto data = *reinterpret_cast<int32_t*>((*maybeSharedChunk).getPayload());
        EXPECT_THAT(data, Eq(i));
    }
}

TYPED_TEST(ChunkQueue_test, ClearOnEmpty)
{
    this->m_popper.clear();
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, ClearWithData)
{
    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);
    this->m_popper.clear();
    EXPECT_THAT(this->m_popper.empty(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, AttachConditionVariable)
{
    ConditionVariableData condVar("Horscht");

    this->m_popper.setConditionVariable(&condVar);

    EXPECT_THAT(this->m_popper.isConditionVariableSet(), Eq(true));
}

TYPED_TEST(ChunkQueue_test, PushAndNotifyConditionVariable)
{
    ConditionVariableData condVar("Horscht");
    ConditionVariableWaiter condVarWaiter{&condVar};

    this->m_popper.setConditionVariable(&condVar);

    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);

    EXPECT_THAT(condVarWaiter.timedWait(1_ns), Eq(true));
    EXPECT_THAT(condVarWaiter.timedWait(1_ns), Eq(false)); // shouldn't trigger a second time
}

TYPED_TEST(ChunkQueue_test, AttachSecondConditionVariable)
{
    ConditionVariableData condVar1("Horscht");
    ConditionVariableData condVar2("Schnuppi");
    ConditionVariableWaiter condVarWaiter1{&condVar1};
    ConditionVariableWaiter condVarWaiter2{&condVar2};

    this->m_popper.setConditionVariable(&condVar1);
    this->m_popper.setConditionVariable(&condVar2);

    EXPECT_THAT(condVarWaiter1.timedWait(1_ns), Eq(false));
    EXPECT_THAT(condVarWaiter2.timedWait(1_ns), Eq(false));

    auto chunk = this->allocateChunk();
    this->m_pusher.push(chunk);

    EXPECT_THAT(condVarWaiter1.timedWait(1_ms), Eq(false));
    EXPECT_THAT(condVarWaiter2.timedWait(1_ms), Eq(true));
}

/// @note this could be changed to a parameterized ChunkQueueSaturatingFIFO_test when there are more FIFOs available
using ChunkQueueFiFoTestSubjects = Types<ThreadSafePolicy, SingleThreadedPolicy>;
/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(ChunkQueueFiFo_test, ChunkQueueFiFoTestSubjects);
#pragma GCC diagnostic pop

template <typename PolicyType>
class ChunkQueueFiFo_test : public Test, public ChunkQueue_testBase
{
  public:
    void SetUp() override{};
    void TearDown() override{};

    using ChunkQueueData_t = ChunkQueueData<iox::DefaultChunkQueueConfig, PolicyType>;

    ChunkQueueData_t m_chunkData{iox::cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer};
    ChunkQueuePopper<ChunkQueueData_t> m_popper{&m_chunkData};
    ChunkQueuePusher<ChunkQueueData_t> m_pusher{&m_chunkData};
};

/// @note API currently not supported
TYPED_TEST(ChunkQueueFiFo_test, DISABLED_InitialSize)
{
    EXPECT_THAT(this->m_popper.size(), Eq(0u));
}

/// @note API currently not supported
TYPED_TEST(ChunkQueueFiFo_test, DISABLED_Capacity)
{
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(iox::MAX_SUBSCRIBER_QUEUE_CAPACITY));
}

/// @note API currently not supported
TYPED_TEST(ChunkQueueFiFo_test, DISABLED_SetCapacity)
{
    this->m_popper.setCapacity(this->RESIZED_CAPACITY);
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(this->RESIZED_CAPACITY));
}

TYPED_TEST(ChunkQueueFiFo_test, PushFull)
{
    for (auto i = 0u; i < iox::MAX_SUBSCRIBER_QUEUE_CAPACITY; ++i)
    {
        auto chunk = this->allocateChunk();
        this->m_pusher.push(chunk);
    }

    {
        auto chunk = this->allocateChunk();
        this->m_pusher.push(chunk);
        EXPECT_TRUE(this->m_popper.hasOverflown());
    }

    // get all the chunks in the queue
    while (this->m_popper.tryPop().has_value())
    {
    }

    // all chunks must be released
    EXPECT_THAT(this->mempool.getUsedChunks(), Eq(0u));
}

/// @note this could be changed to a parameterized ChunkQueueOverflowingFIFO_test when there are more FIFOs available
using ChunkQueueSoFiSubjects = Types<ThreadSafePolicy, SingleThreadedPolicy>;
/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(ChunkQueueSoFi_test, ChunkQueueSoFiSubjects);
#pragma GCC diagnostic pop

template <typename PolicyType>
class ChunkQueueSoFi_test : public Test, public ChunkQueue_testBase
{
  public:
    void SetUp() override{};
    void TearDown() override{};

    using ChunkQueueData_t = ChunkQueueData<iox::DefaultChunkQueueConfig, PolicyType>;

    ChunkQueueData_t m_chunkData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    ChunkQueuePopper<ChunkQueueData_t> m_popper{&m_chunkData};
    ChunkQueuePusher<ChunkQueueData_t> m_pusher{&m_chunkData};
};

TYPED_TEST(ChunkQueueSoFi_test, InitialSize)
{
    EXPECT_THAT(this->m_popper.size(), Eq(0u));
}

TYPED_TEST(ChunkQueueSoFi_test, Capacity)
{
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(iox::MAX_SUBSCRIBER_QUEUE_CAPACITY));
}


TYPED_TEST(ChunkQueueSoFi_test, SetCapacity)
{
    this->m_popper.setCapacity(this->RESIZED_CAPACITY);
    EXPECT_THAT(this->m_popper.getCurrentCapacity(), Eq(this->RESIZED_CAPACITY));
}

TYPED_TEST(ChunkQueueSoFi_test, PushFull)
{
    for (auto i = 0u; i < iox::MAX_SUBSCRIBER_QUEUE_CAPACITY * 2; ++i)
    {
        auto chunk = this->allocateChunk();
        this->m_pusher.push(chunk);
    }

    // this led to an overflow
    EXPECT_TRUE(this->m_popper.hasOverflown());

    // get al the chunks in the queue
    while (this->m_popper.tryPop().has_value())
    {
    }

    // all chunks must be released
    EXPECT_THAT(this->mempool.getUsedChunks(), Eq(0u));
}
