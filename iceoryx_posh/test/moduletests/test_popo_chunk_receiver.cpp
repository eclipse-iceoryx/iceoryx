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
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
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

class ChunkReceiver_test : public Test
{
  protected:
    ChunkReceiver_test()
    {
        m_mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~ChunkReceiver_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    static constexpr size_t MEGABYTE = 1 << 20;
    static constexpr size_t MEMORY_SIZE = 4 * MEGABYTE;
    std::unique_ptr<char[]> m_memory{new char[MEMORY_SIZE]};
    static constexpr uint32_t NUM_CHUNKS_IN_POOL =
        iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + iox::MAX_SUBSCRIBER_QUEUE_CAPACITY;
    static constexpr uint32_t CHUNK_SIZE = 128;

    iox::posix::Allocator m_memoryAllocator{m_memory.get(), MEMORY_SIZE};
    iox::mepoo::MePooConfig m_mempoolconf;
    iox::mepoo::MemoryManager m_memoryManager;

    using ChunkQueueData_t = iox::popo::ChunkQueueData<iox::DefaultChunkQueueConfig, iox::popo::ThreadSafePolicy>;
    using ChunkReceiverData_t =
        iox::popo::ChunkReceiverData<iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY, ChunkQueueData_t>;
    using ChunkQueuePopper_t = iox::popo::ChunkQueuePopper<ChunkQueueData_t>;

    ChunkReceiverData_t m_chunkReceiverData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::popo::ChunkReceiver<ChunkReceiverData_t> m_chunkReceiver{&m_chunkReceiverData};

    iox::popo::ChunkQueuePusher<ChunkReceiverData_t> m_chunkQueuePusher{&m_chunkReceiverData};
};

TEST_F(ChunkReceiver_test, getNoChunkFromEmptyQueue)
{
    auto maybeChunkHeader = m_chunkReceiver.tryGet();
    EXPECT_FALSE(maybeChunkHeader.has_error());
    EXPECT_FALSE((*maybeChunkHeader).has_value());
}

TEST_F(ChunkReceiver_test, getAndReleaseOneChunk)
{
    {
        // have a scope her to release the shared chunk we allocate
        auto sharedChunk = m_memoryManager.getChunk(sizeof(DummySample));
        EXPECT_TRUE(sharedChunk);
        EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
        auto pushRet = m_chunkQueuePusher.tryPush(sharedChunk);
        EXPECT_FALSE(pushRet.has_error());

        auto maybeChunkHeader = m_chunkReceiver.tryGet();
        EXPECT_FALSE(maybeChunkHeader.has_error());
        EXPECT_TRUE((*maybeChunkHeader).has_value());

        EXPECT_TRUE(sharedChunk.getPayload() == (**maybeChunkHeader)->payload());
        m_chunkReceiver.release(**maybeChunkHeader);
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(ChunkReceiver_test, getAndReleaseMultipleChunks)
{
    std::vector<const iox::mepoo::ChunkHeader*> chunks;

    for (size_t i = 0; i < iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY; i++)
    {
        auto sharedChunk = m_memoryManager.getChunk(sizeof(DummySample));
        EXPECT_TRUE(sharedChunk);
        auto sample = sharedChunk.getPayload();
        new (sample) DummySample();
        static_cast<DummySample*>(sample)->dummy = i;

        auto pushRet = m_chunkQueuePusher.tryPush(sharedChunk);
        EXPECT_FALSE(pushRet.has_error());

        auto maybeChunkHeader = m_chunkReceiver.tryGet();
        EXPECT_FALSE(maybeChunkHeader.has_error());
        EXPECT_TRUE((*maybeChunkHeader).has_value());
        chunks.push_back(**maybeChunkHeader);
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY));

    for (size_t i = 0; i < iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY; i++)
    {
        const auto chunk = chunks.back();
        chunks.pop_back();
        auto dummySample = *reinterpret_cast<DummySample*>(chunk->payload());
        EXPECT_THAT(dummySample.dummy, Eq(iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY - 1 - i));
        m_chunkReceiver.release(chunk);
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}

TEST_F(ChunkReceiver_test, getTooMuchWithoutRelease)
{
    // one more is OK, but we assume that one is released then (aligned with ara::com behavior)
    // therefore MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY+1
    for (size_t i = 0; i < iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + 1; i++)
    {
        auto sharedChunk = m_memoryManager.getChunk(sizeof(DummySample));
        EXPECT_TRUE(sharedChunk);

        auto pushRet = m_chunkQueuePusher.tryPush(sharedChunk);
        EXPECT_FALSE(pushRet.has_error());

        auto maybeChunkHeader = m_chunkReceiver.tryGet();
        EXPECT_FALSE(maybeChunkHeader.has_error());
        EXPECT_TRUE((*maybeChunkHeader).has_value());
    }

    // but now it breaks
    auto sharedChunk = m_memoryManager.getChunk(sizeof(DummySample));
    EXPECT_TRUE(sharedChunk);

    auto pushRet = m_chunkQueuePusher.tryPush(sharedChunk);
    EXPECT_FALSE(pushRet.has_error());

    auto maybeChunkHeader = m_chunkReceiver.tryGet();
    EXPECT_TRUE(maybeChunkHeader.has_error());
    EXPECT_THAT(maybeChunkHeader.get_error(), Eq(iox::popo::ChunkReceiveError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL));
}

TEST_F(ChunkReceiver_test, releaseInvalidChunk)
{
    {
        // have a scope her to release the shared chunk we allocate
        auto sharedChunk = m_memoryManager.getChunk(sizeof(DummySample));
        EXPECT_TRUE(sharedChunk);
        EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
        auto pushRet = m_chunkQueuePusher.tryPush(sharedChunk);
        EXPECT_FALSE(pushRet.has_error());

        auto maybeChunkHeader = m_chunkReceiver.tryGet();
        EXPECT_FALSE(maybeChunkHeader.has_error());
        EXPECT_TRUE((*maybeChunkHeader).has_value());

        EXPECT_TRUE(sharedChunk.getPayload() == (**maybeChunkHeader)->payload());
    }

    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    auto myCrazyChunk = std::make_shared<iox::mepoo::ChunkHeader>();
    m_chunkReceiver.release(myCrazyChunk.get());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1u));
}

TEST_F(ChunkReceiver_test, Cleanup)
{
    for (size_t i = 0; i < iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + iox::MAX_SUBSCRIBER_QUEUE_CAPACITY; i++)
    {
        // MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY on user side and MAX_SUBSCRIBER_QUEUE_CAPACITY in the queue
        auto sharedChunk = m_memoryManager.getChunk(sizeof(DummySample));
        EXPECT_TRUE(sharedChunk);
        auto pushRet = m_chunkQueuePusher.tryPush(sharedChunk);
        EXPECT_FALSE(pushRet.has_error());

        if (i < iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY)
        {
            auto maybeChunkHeader = m_chunkReceiver.tryGet();
            EXPECT_FALSE(maybeChunkHeader.has_error());
            EXPECT_TRUE((*maybeChunkHeader).has_value());
        }
    }

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks,
                Eq(iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY + iox::MAX_SUBSCRIBER_QUEUE_CAPACITY));

    m_chunkReceiver.releaseAll();

    EXPECT_THAT(m_memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0u));
}
