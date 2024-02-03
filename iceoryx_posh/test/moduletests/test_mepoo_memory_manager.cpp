// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::testing;

using iox::mepoo::ChunkHeader;
using iox::mepoo::ChunkSettings;
using UserPayloadOffset_t = iox::mepoo::ChunkHeader::UserPayloadOffset_t;

class MemoryManager_test : public Test
{
  public:
    void SetUp() override
    {
        rawMemory = malloc(rawMemorySize);
        allocator = new iox::BumpAllocator(rawMemory, rawMemorySize);
        sut = new iox::mepoo::MemoryManager();
    };
    void TearDown() override
    {
        delete sut;
        delete allocator;
        free(rawMemory);
    };

    using ChunkStore = std::vector<iox::mepoo::SharedChunk>;
    ChunkStore getChunksFromSut(const uint32_t numberOfChunks, const iox::mepoo::ChunkSettings& chunkSettings)
    {
        ChunkStore chunkStore;
        for (uint32_t i = 0; i < numberOfChunks; ++i)
        {
            sut->getChunk(chunkSettings)
                .and_then([&](auto& chunk) {
                    EXPECT_TRUE(chunk);
                    chunkStore.push_back(chunk);
                })
                .or_else([](const auto& error) { GTEST_FAIL() << "getChunk failed with: " << error; });
        }
        return chunkStore;
    }

    static constexpr uint64_t CHUNK_SIZE_32{32U};
    static constexpr uint64_t CHUNK_SIZE_64{64U};
    static constexpr uint64_t CHUNK_SIZE_128{128};
    static constexpr uint64_t CHUNK_SIZE_256{256U};

    iox::BumpAllocator* allocator;
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
    ::testing::Test::RecordProperty("TEST_ID", "df439901-8d42-4532-8494-21d5447ef7d7");
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});

    IOX_EXPECT_FATAL_FAILURE([&] { sut->configureMemoryManager(mempoolconf, *allocator, *allocator); },
                             iox::PoshError::MEPOO__MEMPOOL_CONFIG_MUST_BE_ORDERED_BY_INCREASING_SIZE);
}

TEST_F(MemoryManager_test, WrongCallOfConfigureMemoryManagerReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "04cbf769-8721-454b-b038-96dd467ac3c2");
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    IOX_EXPECT_FATAL_FAILURE([&] { sut->configureMemoryManager(mempoolconf, *allocator, *allocator); },
                             iox::PoshError::MEPOO__MEMPOOL_ADDMEMPOOL_AFTER_GENERATECHUNKMANAGEMENTPOOL);
}

TEST_F(MemoryManager_test, GetMempoolInfoMethodForOutOfBoundaryMempoolIndexReturnsZeroForAllMempoolAttributes)
{
    ::testing::Test::RecordProperty("TEST_ID", "897e635e-d6df-46be-a3f0-7373b4116102");
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
    ::testing::Test::RecordProperty("TEST_ID", "e3c05153-add7-4098-8045-88960970d2a7");
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});

    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    EXPECT_EQ(sut->getNumberOfMemPools(), 3U);
}

TEST_F(MemoryManager_test, GetChunkMethodWithNoMemPoolInMemConfigReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "dff31ea2-8ae0-4786-8c97-633af59c287d");

    constexpr uint64_t USER_PAYLOAD_SIZE{15U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    constexpr auto EXPECTED_ERROR{iox::mepoo::MemoryManager::Error::NO_MEMPOOLS_AVAILABLE};
    sut->getChunk(chunkSettings)
        .and_then(
            [&](auto&) { GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR << "' but did not fail"; })
        .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_CHUNK_WITHOUT_MEMPOOL);
}


TEST_F(MemoryManager_test, GetChunkMethodWithChunkSizeGreaterThanAvailableChunkSizeInMemPoolConfigReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2104b74-5092-484d-bb9d-554996dffbc5");
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    constexpr uint64_t USER_PAYLOAD_SIZE{200U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    constexpr auto EXPECTED_ERROR{iox::mepoo::MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE};
    sut->getChunk(chunkSettings)
        .and_then(
            [&](auto&) { GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR << "' but did not fail"; })
        .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE);
}

TEST_F(MemoryManager_test, GetChunkMethodWhenNoFreeChunksInMemPoolConfigReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f201458-040e-43b1-a51b-698c2957ca7c");
    constexpr uint32_t CHUNK_COUNT{1U};
    constexpr uint64_t PAYLOAD_SIZE{100U};
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    auto chunkSettingsResult = ChunkSettings::create(PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();
    auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings);

    constexpr auto EXPECTED_ERROR{iox::mepoo::MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};
    sut->getChunk(chunkSettings)
        .and_then(
            [&](auto&) { GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR << "' but did not fail"; })
        .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_POOL_IS_RUNNING_OUT_OF_CHUNKS);
}

TEST_F(MemoryManager_test, VerifyGetChunkMethodWhenTheRequestedChunkIsAvailableInMemPoolConfig)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5069a9d-ae2f-4466-ae13-53b0794dd292");
    constexpr uint32_t CHUNK_COUNT{10U};
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    constexpr uint64_t USER_PAYLOAD_SIZE{50U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    sut->getChunk(chunkSettings).and_then([&](auto& chunk) { EXPECT_TRUE(chunk); }).or_else([](const auto& error) {
        GTEST_FAIL() << "getChunk failed with: " << error;
    });
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolAllChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6623841b-baf0-4636-a5d4-b21e4678b7e8");
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    constexpr uint64_t USER_PAYLOAD_SIZE{50U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings);

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolToMuchChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8af072c7-425b-4820-bc28-0c1e6bad0441");
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);

    constexpr auto EXPECTED_ERROR{iox::mepoo::MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};
    sut->getChunk(chunkSettings_128)
        .and_then(
            [&](auto&) { GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR << "' but did not fail"; })
        .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });
}

TEST_F(MemoryManager_test, freeChunkSingleMemPoolFullToEmptyToFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "0cbb56ae-c5a3-46b6-bfab-abbf91b0ed64");
    constexpr uint32_t CHUNK_COUNT{100U};

    // chunks are freed when they go out of scope
    {
        mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
        sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

        auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);

        EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
    }

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, 0U);

    auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);

    EXPECT_EQ(sut->getMemPoolInfo(0U).m_usedChunks, CHUNK_COUNT);
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolSingleChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "b22f804d-2e12-40c3-8bec-f9a0ab375e98");
    constexpr uint32_t CHUNK_COUNT{10U};

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});

    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    for (const auto& chunkSettings : {chunkSettings_32, chunkSettings_64, chunkSettings_128, chunkSettings_256})
    {
        sut->getChunk(chunkSettings).and_then([&](auto& chunk) { EXPECT_TRUE(chunk); }).or_else([](const auto& error) {
            GTEST_FAIL() << "getChunk failed with: " << error;
        });
    }
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolAllChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cdb65377-7579-4c76-9931-9552071fff5c");
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
    auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
    auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
    auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolTooMuchChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "975e1347-9b39-4fda-a95a-0725b04f7d7d");
    constexpr uint32_t CHUNK_COUNT{100U};

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
    auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
    auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
    auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

    constexpr auto EXPECTED_ERROR{iox::mepoo::MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};

    for (const auto& chunkSettings : {chunkSettings_32, chunkSettings_64, chunkSettings_128, chunkSettings_256})
    {
        sut->getChunk(chunkSettings)
            .and_then([&](auto&) {
                GTEST_FAIL() << "getChunk for payload size " << chunkSettings.userPayloadSize() << " should fail with '"
                             << EXPECTED_ERROR << "' but did not fail";
            })
            .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });
    }
}

TEST_F(MemoryManager_test, emptyMemPoolDoesNotResultInAcquiringChunksFromOtherMemPools)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2ff3937-6cdf-43ad-bec6-5203521a8f57");
    constexpr uint32_t CHUNK_COUNT{100};

    mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
    mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto chunkStore = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);

    constexpr auto EXPECTED_ERROR{iox::mepoo::MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS};
    sut->getChunk(chunkSettings_64)
        .and_then(
            [&](auto&) { GTEST_FAIL() << "getChunk should fail with '" << EXPECTED_ERROR << "' but did not fail"; })
        .or_else([&](const auto& error) { EXPECT_EQ(error, EXPECTED_ERROR); });

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));
}

TEST_F(MemoryManager_test, freeChunkMultiMemPoolFullToEmptyToFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "0eddc5b5-e28f-43df-9da7-2c12014284a5");
    constexpr uint32_t CHUNK_COUNT{100U};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;

        mempoolconf.addMemPool({CHUNK_SIZE_32, CHUNK_COUNT});
        mempoolconf.addMemPool({CHUNK_SIZE_64, CHUNK_COUNT});
        mempoolconf.addMemPool({CHUNK_SIZE_128, CHUNK_COUNT});
        mempoolconf.addMemPool({CHUNK_SIZE_256, CHUNK_COUNT});
        sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

        auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
        auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
        auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
        auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

        EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
        EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));

    auto chunkStore_32 = getChunksFromSut(CHUNK_COUNT, chunkSettings_32);
    auto chunkStore_64 = getChunksFromSut(CHUNK_COUNT, chunkSettings_64);
    auto chunkStore_128 = getChunksFromSut(CHUNK_COUNT, chunkSettings_128);
    auto chunkStore_256 = getChunksFromSut(CHUNK_COUNT, chunkSettings_256);

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(CHUNK_COUNT));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(CHUNK_COUNT));
}

TEST_F(MemoryManager_test, getChunkWithUserPayloadSizeZeroShouldNotFail)
{
    ::testing::Test::RecordProperty("TEST_ID", "9fbfe1ff-9d59-449b-b164-433bbb031125");
    constexpr uint64_t USER_PAYLOAD_SIZE{0U};
    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    mempoolconf.addMemPool({32, 10});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    sut->getChunk(chunkSettings).and_then([&](auto& chunk) { EXPECT_TRUE(chunk); }).or_else([](const auto& error) {
        GTEST_FAIL() << "getChunk failed with: " << error;
    });
}

TEST_F(MemoryManager_test, addMemPoolWithChunkCountZeroShouldFail)
{
    ::testing::Test::RecordProperty("TEST_ID", "be653b65-a2d1-42eb-98b5-d161c6ba7c08");
    mempoolconf.addMemPool({32, 0});

    IOX_EXPECT_FATAL_FAILURE([&] { sut->configureMemoryManager(mempoolconf, *allocator, *allocator); }, iox::er::FATAL);
}

TEST(MemoryManagerEnumString_test, asStringLiteralConvertsEnumValuesToStrings)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f6c3942-0af5-4c48-b44c-7268191dbac5");
    using Error = iox::mepoo::MemoryManager::Error;

    // each bit corresponds to an enum value and must be set to true on test
    uint64_t testedEnumValues{0U};
    uint64_t loopCounter{0U};
    for (const auto& sut :
         {Error::NO_MEMPOOLS_AVAILABLE, Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE, Error::MEMPOOL_OUT_OF_CHUNKS})
    {
        auto enumString = iox::mepoo::asStringLiteral(sut);

        switch (sut)
        {
        case Error::NO_MEMPOOLS_AVAILABLE:
            EXPECT_THAT(enumString, StrEq("MemoryManager::Error::NO_MEMPOOLS_AVAILABLE"));
            break;
        case Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE:
            EXPECT_THAT(enumString, StrEq("MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE"));
            break;
        case Error::MEMPOOL_OUT_OF_CHUNKS:
            EXPECT_THAT(enumString, StrEq("MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS"));
            break;
        }

        testedEnumValues |= 1U << static_cast<uint64_t>(sut);
        ++loopCounter;
    }

    uint64_t expectedTestedEnumValues = (1U << loopCounter) - 1;
    EXPECT_EQ(testedEnumValues, expectedTestedEnumValues);
}

TEST(MemoryManagerEnumString_test, LogStreamConvertsEnumValueToString)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a3539e5-5465-4352-b2b7-a850e104c173");
    iox::testing::Logger_Mock loggerMock;

    auto sut = iox::mepoo::MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS;

    {
        IOX_LOGSTREAM_MOCK(loggerMock) << sut;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(iox::mepoo::asStringLiteral(sut)));
}

} // namespace
