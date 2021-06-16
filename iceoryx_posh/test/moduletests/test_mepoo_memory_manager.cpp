// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using iox::mepoo::ChunkHeader;
using iox::mepoo::ChunkSettings;
using UserPayloadOffset_t = iox::mepoo::ChunkHeader::UserPayloadOffset_t;

class MemoryManager_test : public Test
{
  public:
    void SetUp() override
    {
        rawMemory = malloc(rawMemorySize);
        allocator = new iox::posix::Allocator(rawMemory, rawMemorySize);
        sut = new iox::mepoo::MemoryManager();
    };
    void TearDown() override
    {
        delete sut;
        delete allocator;
        free(rawMemory);
    };

    static constexpr uint32_t CHUNK_SIZE_32{32U};
    static constexpr uint32_t CHUNK_SIZE_64{64U};
    static constexpr uint32_t CHUNK_SIZE_128{128};
    static constexpr uint32_t CHUNK_SIZE_256{256U};

    iox::posix::Allocator* allocator;
    void* rawMemory;
    size_t rawMemorySize = 1000000;

    iox::mepoo::MemoryManager* sut;
    iox::mepoo::MePooConfig mempoolconf;

    const iox::mepoo::ChunkSettings chunkSettings_32{
        iox::mepoo::ChunkSettings::create(32U, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT).value()};
    const iox::mepoo::ChunkSettings chunkSettings_64{
        iox::mepoo::ChunkSettings::create(64U, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT).value()};
    const iox::mepoo::ChunkSettings chunkSettings_128{
        iox::mepoo::ChunkSettings::create(128U, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT).value()};
    const iox::mepoo::ChunkSettings chunkSettings_256{
        iox::mepoo::ChunkSettings::create(256, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT).value()};
};

TEST_F(MemoryManager_test, AddingMempoolNotInTheIncreasingOrderReturnsError)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::FATAL);
        });

    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_CONFIG_MUST_BE_ORDERED_BY_INCREASING_SIZE);
}

TEST_F(MemoryManager_test, WrongCallOfConfigureMemoryManagerReturnsError)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::FATAL);
        });

    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

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

TEST_F(MemoryManager_test, GetNumberOfMemPoolsMethodReturnsTheNumberOfMemPools)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});

    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    EXPECT_EQ(sut->getNumberOfMemPools(), 3U);
}

TEST_F(MemoryManager_test, GetChunkMethodWithNoMemPoolInMemConfigReturnsError)
{
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::SEVERE);
        });

    constexpr uint32_t USER_PAYLOAD_SIZE{15U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    auto bla = sut->getChunk(chunkSettings);
    EXPECT_THAT(bla, Eq(false));

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_GETCHUNK_CHUNK_WITHOUT_MEMPOOL);
}

TEST_F(MemoryManager_test, GetChunkMethodWithChunkSizeGreaterThanAvailableChunkSizeInMemPoolConfigReturnsError)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::SEVERE);
        });

    constexpr uint32_t USER_PAYLOAD_SIZE{200U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    auto bla = sut->getChunk(chunkSettings);
    EXPECT_THAT(bla, Eq(false));

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE);
}

TEST_F(MemoryManager_test, GetChunkMethodWhenNoFreeChunksInMemPoolConfigReturnsError)
{
    constexpr uint32_t CHUNK_COUNT{1U};
    constexpr uint32_t PAYLOAD_SIZE{100U};
    std::vector<iox::mepoo::SharedChunk> chunkStore;
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    auto chunkSettingsResult = ChunkSettings::create(PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();
    chunkStore.push_back(sut->getChunk(chunkSettings));

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_EQ(errorLevel, iox::ErrorLevel::MODERATE);
        });


    auto bla = sut->getChunk(chunkSettings);
    EXPECT_THAT(bla, Eq(false));

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_EQ(detectedError.value(), iox::Error::kMEPOO__MEMPOOL_GETCHUNK_POOL_IS_RUNNING_OUT_OF_CHUNKS);
}

TEST_F(MemoryManager_test, VerifyGetChunkMethodWhenTheRequestedChunkIsAvailableInMemPoolConfig)
{
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    constexpr uint32_t USER_PAYLOAD_SIZE{50U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    auto receivedChunk = sut->getChunk(chunkSettings);

    EXPECT_TRUE(receivedChunk);
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolAllChunks)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    constexpr uint32_t USER_PAYLOAD_SIZE{50U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0U; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(chunkSettings));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolToMuchChunks)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0U; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(chunkSettings_128));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }
    EXPECT_THAT(sut->getChunk(chunkSettings_128), Eq(false));
}

TEST_F(MemoryManager_test, freeChunkSingleMemPoolFullToEmptyToFull)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;
        mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
        sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
        for (size_t i = 0; i < CHUNK_COUNT; i++)
        {
            chunkStore.push_back(sut->getChunk(chunkSettings_128));
            EXPECT_THAT(chunkStore.back(), Eq(true));
        }

        EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
    }

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, 0U);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0U; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(chunkSettings_128));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolSingleChunk)
{
    constexpr uint32_t CHUNK_COUNT{10U};

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});

    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto bla = sut->getChunk(chunkSettings_32);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(chunkSettings_64);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(chunkSettings_128);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(chunkSettings_256);
    EXPECT_THAT(bla, Eq(true));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolAllChunks)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(chunkSettings_32));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(chunkSettings_64));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(chunkSettings_128));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(chunkSettings_256));
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

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(chunkSettings_32));
        chunkStore.push_back(sut->getChunk(chunkSettings_64));
        chunkStore.push_back(sut->getChunk(chunkSettings_128));
        chunkStore.push_back(sut->getChunk(chunkSettings_256));
    }

    EXPECT_THAT(sut->getChunk(chunkSettings_32), Eq(false));
    EXPECT_THAT(sut->getChunk(chunkSettings_64), Eq(false));
    EXPECT_THAT(sut->getChunk(chunkSettings_128), Eq(false));
    EXPECT_THAT(sut->getChunk(chunkSettings_256), Eq(false));
}

TEST_F(MemoryManager_test, emptyMemPoolDoesNotResultInAcquiringChunksFromOtherMemPools)
{
    constexpr uint32_t CHUNK_COUNT{100};

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(chunkSettings_64));
    }

    EXPECT_THAT(sut->getChunk(chunkSettings_64), Eq(false));

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));
}

TEST_F(MemoryManager_test, freeChunkMultiMemPoolFullToEmptyToFull)
{
    constexpr uint32_t CHUNK_COUNT{100U};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;

        mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
        mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
        mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
        mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
        sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

        for (size_t i = 0; i < CHUNK_COUNT; i++)
        {
            chunkStore.push_back(sut->getChunk(chunkSettings_32));
            chunkStore.push_back(sut->getChunk(chunkSettings_64));
            chunkStore.push_back(sut->getChunk(chunkSettings_128));
            chunkStore.push_back(sut->getChunk(chunkSettings_256));
        }

        EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < CHUNK_COUNT; i++)
    {
        chunkStore.push_back(sut->getChunk(chunkSettings_32));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(chunkSettings_64));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(chunkSettings_128));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(chunkSettings_256));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
}

TEST_F(MemoryManager_test, getChunkWithUserPayloadSizeZeroShouldNotFail)
{
    constexpr uint32_t USER_PAYLOAD_SIZE{0U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    mempoolconf.addMemPool({32, 10});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    EXPECT_THAT(sut->getChunk(chunkSettings), Eq(true));
}

TEST_F(MemoryManager_test, addMemPoolWithChunkCountZeroShouldFail)
{
    mempoolconf.addMemPool({32, 0});
    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, *allocator, *allocator); }, ".*");
}

} // namespace