// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"

using namespace ::testing;

class MemoryManager_test : public Test
{
  public:
    void SetUp()
    {
        rawMemory = malloc(rawMemorySize);
        allocator = new iox::posix::Allocator(rawMemory, rawMemorySize);
        sut = new iox::mepoo::MemoryManager();
    };
    void TearDown()
    {
        delete sut;
        delete allocator;
        free(rawMemory);
    };

    uint32_t adjustedChunkSize(uint32_t chunkSize)
    {
        // internally, the chunks are adjusted to the additional management information
        return chunkSize + sizeof(iox::mepoo::ChunkHeader);
    }

    iox::posix::Allocator* allocator;
    void* rawMemory;
    size_t rawMemorySize = 1000000;

    iox::mepoo::MemoryManager* sut;
    iox::mepoo::MePooConfig mempoolconf;
};


TEST_F(MemoryManager_test, addMemPoolWrongOrderAtLastElement)
{
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({128, 10});
    mempoolconf.addMemPool({256, 10});
    mempoolconf.addMemPool({64, 10});

    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, allocator, allocator); }, ".*");
}

TEST_F(MemoryManager_test, getMempoolChunkSizeForPayloadSize)
{
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({64, 10});
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    EXPECT_THAT(sut->getMempoolChunkSizeForPayloadSize(50), Eq(adjustedChunkSize(64u)));
}

TEST_F(MemoryManager_test, getChunkSizeForWrongSampleSize)
{
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({64, 10});
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    EXPECT_THAT(sut->getMempoolChunkSizeForPayloadSize(129), Eq(0u));
}

TEST_F(MemoryManager_test, wrongcallConfigureMemoryManager)
{
    mempoolconf.addMemPool({32, 10});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    EXPECT_THAT(sut->getNumberOfMemPools(), Eq(1u));
    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, allocator, allocator); }, ".*");
}

TEST_F(MemoryManager_test, getNumberOfMemPools)
{
    EXPECT_THAT(sut->getNumberOfMemPools(), Eq(0u));
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({64, 10});
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    EXPECT_THAT(sut->getNumberOfMemPools(), Eq(3u));
}

TEST_F(MemoryManager_test, getChunkWithNoMemPool)
{
    EXPECT_DEATH({ sut->getChunk(15); }, ".*");
}

TEST_F(MemoryManager_test, getTooLargeChunk)
{
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({64, 10});
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    EXPECT_DEATH({ sut->getChunk(200); }, ".*");
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolSingleChunk)
{
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    auto bla = sut->getChunk(50);
    EXPECT_THAT(bla, Eq(true));
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolAllChunks)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({128, ChunkCount});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(50));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolToMuchChunks)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({128, ChunkCount});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(128));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }
    EXPECT_THAT(sut->getChunk(128), Eq(false));
}


TEST_F(MemoryManager_test, freeChunkSingleMemPoolFullToEmptyToFull)
{
    constexpr uint32_t ChunkCount{100};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;
        mempoolconf.addMemPool({128, ChunkCount});
        sut->configureMemoryManager(mempoolconf, allocator, allocator);
        for (size_t i = 0; i < ChunkCount; i++)
        {
            chunkStore.push_back(sut->getChunk(128));
            EXPECT_THAT(chunkStore.back(), Eq(true));
        }

        EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0));

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(128));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolSingleChunk)
{
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({64, 10});
    mempoolconf.addMemPool({128, 10});
    mempoolconf.addMemPool({256, 10});

    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    auto bla = sut->getChunk(32);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(64);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(128);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(256);
    EXPECT_THAT(bla, Eq(true));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolAllChunks)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({32, ChunkCount});
    mempoolconf.addMemPool({64, ChunkCount});
    mempoolconf.addMemPool({128, ChunkCount});
    mempoolconf.addMemPool({256, ChunkCount});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(32));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(64));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(128));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(256));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(ChunkCount));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolTooMuchChunks)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({32, ChunkCount});
    mempoolconf.addMemPool({64, ChunkCount});
    mempoolconf.addMemPool({128, ChunkCount});
    mempoolconf.addMemPool({256, ChunkCount});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(32));
        chunkStore.push_back(sut->getChunk(64));
        chunkStore.push_back(sut->getChunk(128));
        chunkStore.push_back(sut->getChunk(256));
    }

    EXPECT_THAT(sut->getChunk(32), Eq(false));
    EXPECT_THAT(sut->getChunk(64), Eq(false));
    EXPECT_THAT(sut->getChunk(128), Eq(false));
    EXPECT_THAT(sut->getChunk(256), Eq(false));
}

TEST_F(MemoryManager_test, freeChunkMultiMemPoolFullToEmptyToFull)
{
    constexpr uint32_t ChunkCount{100};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;

        mempoolconf.addMemPool({32, ChunkCount});
        mempoolconf.addMemPool({64, ChunkCount});
        mempoolconf.addMemPool({128, ChunkCount});
        mempoolconf.addMemPool({256, ChunkCount});
        sut->configureMemoryManager(mempoolconf, allocator, allocator);

        for (size_t i = 0; i < ChunkCount; i++)
        {
            chunkStore.push_back(sut->getChunk(32));
            chunkStore.push_back(sut->getChunk(64));
            chunkStore.push_back(sut->getChunk(128));
            chunkStore.push_back(sut->getChunk(256));
        }

        EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
        EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(ChunkCount));
        EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(ChunkCount));
        EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(ChunkCount));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0u));

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(32));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(64));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(128));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(256));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(ChunkCount));
}

TEST_F(MemoryManager_test, getChunkWithSizeZeroShouldFail)
{
    EXPECT_DEATH({ sut->getChunk(0); }, ".*");
}

TEST_F(MemoryManager_test, addMemPoolWithChunkCountZeroShouldFail)
{
    mempoolconf.addMemPool({32, 0});
    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, allocator, allocator); }, ".*");
}
