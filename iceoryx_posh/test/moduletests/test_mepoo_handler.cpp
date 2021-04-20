// Copyright (c) 2019-2021 by  Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2019-2020 by Apex. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

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
    static constexpr uint32_t CHUNK_SIZE_32{32U};
    static constexpr uint32_t CHUNK_SIZE_64{64U};
    static constexpr uint32_t CHUNK_SIZE_128{128};
    static constexpr uint32_t CHUNK_SIZE_256{256U};
};

TEST_F(MemoryManager_test, AddingMempoolNotInTheIncreasingOrderReturnsError)
{
    constexpr uint32_t chunkCount{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, chunkCount});
    mempoolconf.addMemPool({CHUNK_SIZE_128, chunkCount});
    mempoolconf.addMemPool({CHUNK_SIZE_256, chunkCount});
    mempoolconf.addMemPool({CHUNK_SIZE_64, chunkCount});
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::FATAL);
        });

    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_CONFIG_MUST_BE_ORDERED_BY_INCREASING_SIZE);
}

TEST_F(MemoryManager_test, AddingMempoolAfterGeneratingChunkManagementPoolReturnsError)
{
    constexpr uint32_t chunkCount{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, chunkCount});
    mempoolconf.addMemPool({CHUNK_SIZE_64, chunkCount});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::FATAL);
        });

    mempoolconf.addMemPool({CHUNK_SIZE_128, chunkCount});
    mempoolconf.addMemPool({CHUNK_SIZE_256, chunkCount});

    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_ADDMEMPOOL_AFTER_GENERATECHUNKMANAGEMENTPOOL);
}

TEST_F(MemoryManager_test, GetMempoolInfoMethodForOutOfBoundaryMempoolIndexReturnsZeroForAllMempoolAttributes)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    constexpr uint32_t INVALID_MEMPOOL_INDEX = 2U;
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    iox::mepoo::MemPoolInfo poolInfo = sut->getMemPoolInfo(INVALID_MEMPOOL_INDEX);

    EXPECT_EQ(poolInfo.m_chunkSize, 0U);
    EXPECT_EQ(poolInfo.m_minFreeChunks, 0U);
    EXPECT_EQ(poolInfo.m_numChunks, 0U);
    EXPECT_EQ(poolInfo.m_usedChunks, 0U);
}

TEST_F(MemoryManager_test, WrongCallOfConfigureMemoryManagerResultsInTermination)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    EXPECT_EQ(sut->getNumberOfMemPools(), 1U);

    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, allocator, allocator); }, ".*");
}

TEST_F(MemoryManager_test, GetNumberOfMemPoolsMethodReturnsTheNumberOfMemPools)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});

    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    EXPECT_EQ(sut->getNumberOfMemPools(), 3U);
}

TEST_F(MemoryManager_test, GetChunkMethodWithNoMemPoolInMemConfigReturnsError)
{
    constexpr uint32_t size{15U};
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::SEVERE);
        });

    sut->getChunk(size);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_GETCHUNK_CHUNK_WITHOUT_MEMPOOL);
}

TEST_F(MemoryManager_test, GetChunkMethodWithChunkSizeGreaterThanAvailableChunkSizeInMemPoolConfigReturnsError)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    constexpr uint32_t size{200U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::SEVERE);
        });

    sut->getChunk(size);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE);
}

TEST_F(MemoryManager_test, GetChunkMethodWhenNoFreeChunksInMemPoolConfigReturnsError)
{
    constexpr uint32_t CHUNK_COUNT{1U};
    constexpr uint32_t size{100U};
    std::vector<iox::mepoo::SharedChunk> chunkStore;
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    chunkStore.push_back(sut->getChunk(100U));

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::MODERATE);
        });

    sut->getChunk(size);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_GETCHUNK_POOL_IS_RUNNING_OUT_OF_CHUNKS);
}

TEST_F(MemoryManager_test, VerifyGetChunkMethodWhenTheRequestedChunkIsAvailableInMemPoolConfig)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    constexpr uint32_t size{50U};
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    auto receivedChunk = sut->getChunk(size);

    ASSERT_TRUE(receivedChunk);
    EXPECT_EQ(receivedChunk.releaseWithRelativePtr()->m_mempool->getChunkSize(),
              sut->sizeWithChunkHeaderStruct(CHUNK_SIZE_128));
}

TEST_F(MemoryManager_test, RequiredChunkMemorySizeMethodReturnsTheTotalSizeOfTheChunkMemoryNeeded)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});

    EXPECT_EQ(sut->requiredChunkMemorySize(mempoolconf), sut->sizeWithChunkHeaderStruct(CHUNK_SIZE_128) * CHUNK_COUNT);
}

TEST_F(MemoryManager_test,
       RequiredFullMemorySizeMethodReturnsTheSumOfRequiredChunkMemorySizeAndRequiredManagementMemorySize)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});

    EXPECT_EQ(sut->requiredFullMemorySize(mempoolconf),
              (sut->requiredChunkMemorySize(mempoolconf) + sut->requiredManagementMemorySize(mempoolconf)));
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolAllChunks)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({128U, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0U; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(50U));
        EXPECT_TRUE(chunkStore.back());
    }

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolToMuchChunks)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({128U, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0U; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(128U));
        EXPECT_TRUE(chunkStore.back());
    }
    EXPECT_FALSE(sut->getChunk(128U));
}


TEST_F(MemoryManager_test, freeChunkSingleMemPoolFullToEmptyToFull)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;
        mempoolconf.addMemPool({128U, CHUNK_COUNT});
        sut->configureMemoryManager(mempoolconf, allocator, allocator);
        for (size_t i = 0U; i < CHUNK_COUNT; i++)
        {
            chunkStore.push_back(sut->getChunk(128U));
            EXPECT_TRUE(chunkStore.back());
        }

        EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
    }

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, 0U);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0U; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(128U));
        EXPECT_TRUE(chunkStore.back());
    }

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
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
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({32, CHUNK_COUNT});
    mempoolconf.addMemPool({64, CHUNK_COUNT});
    mempoolconf.addMemPool({128, CHUNK_COUNT});
    mempoolconf.addMemPool({256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < CHUNK_COUNT; i++)
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

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolTooMuchChunks)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({32, CHUNK_COUNT});
    mempoolconf.addMemPool({64, CHUNK_COUNT});
    mempoolconf.addMemPool({128, CHUNK_COUNT});
    mempoolconf.addMemPool({256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < CHUNK_COUNT; i++)
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
    constexpr uint32_t CHUNK_COUNT{100U};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;

        mempoolconf.addMemPool({32, CHUNK_COUNT});
        mempoolconf.addMemPool({64, CHUNK_COUNT});
        mempoolconf.addMemPool({128, CHUNK_COUNT});
        mempoolconf.addMemPool({256, CHUNK_COUNT});
        sut->configureMemoryManager(mempoolconf, allocator, allocator);

        for (size_t i = 0; i < CHUNK_COUNT; i++)
        {
            chunkStore.push_back(sut->getChunk(32));
            chunkStore.push_back(sut->getChunk(64));
            chunkStore.push_back(sut->getChunk(128));
            chunkStore.push_back(sut->getChunk(256));
        }

        EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0u));

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < CHUNK_COUNT; i++)
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

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
}

TEST_F(MemoryManager_test, getChunkWithSizeZeroShouldFail)
{
    EXPECT_DEATH({ sut->getChunk(0U); }, ".*");
}

TEST_F(MemoryManager_test, addMemPoolWithChunkCountZeroShouldFail)
{
    mempoolconf.addMemPool({32U, 0U});
    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, allocator, allocator); }, ".*");
}
