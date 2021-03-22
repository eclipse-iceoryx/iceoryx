// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using iox::mepoo::ChunkHeader;
using PayloadOffset_t = iox::mepoo::ChunkHeader::PayloadOffset_t;

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

    static constexpr uint32_t PAYLOAD_ALIGNMENT = iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT;
    static constexpr uint32_t CUSTOM_HEADER_SIZE = iox::CHUNK_NO_CUSTOM_HEADER_SIZE;
    static constexpr uint32_t CUSTOM_HEADER_ALIGNMENT = iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT;

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

    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, *allocator, *allocator); }, ".*");
}

TEST_F(MemoryManager_test, wrongcallConfigureMemoryManager)
{
    mempoolconf.addMemPool({32, 10});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    EXPECT_THAT(sut->getNumberOfMemPools(), Eq(1U));
    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, *allocator, *allocator); }, ".*");
}

TEST_F(MemoryManager_test, getNumberOfMemPools)
{
    EXPECT_THAT(sut->getNumberOfMemPools(), Eq(0U));
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({64, 10});
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    EXPECT_THAT(sut->getNumberOfMemPools(), Eq(3U));
}

TEST_F(MemoryManager_test, getChunkWithNoMemPool)
{
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::SEVERE));
        });

    auto bla = sut->getChunk(15, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    EXPECT_THAT(bla, Eq(false));

    ASSERT_THAT(detectedError.has_value(), Eq(true));
    EXPECT_THAT(detectedError.value(), Eq(iox::Error::kMEPOO__MEMPOOL_GETCHUNK_CHUNK_WITHOUT_MEMPOOL));
}

TEST_F(MemoryManager_test, getTooLargeChunk)
{
    mempoolconf.addMemPool({32, 10});
    mempoolconf.addMemPool({64, 10});
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::SEVERE));
        });

    auto bla = sut->getChunk(200, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    EXPECT_THAT(bla, Eq(false));

    ASSERT_THAT(detectedError.has_value(), Eq(true));
    EXPECT_THAT(detectedError.value(), Eq(iox::Error::kMEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE));
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolSingleChunk)
{
    mempoolconf.addMemPool({128, 10});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    auto bla = sut->getChunk(50, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    EXPECT_THAT(bla, Eq(true));
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolAllChunks)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({128, ChunkCount});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(50, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
}

TEST_F(MemoryManager_test, getChunkSingleMemPoolToMuchChunks)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({128, ChunkCount});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(128, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }
    EXPECT_THAT(sut->getChunk(128, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(false));
}


TEST_F(MemoryManager_test, freeChunkSingleMemPoolFullToEmptyToFull)
{
    constexpr uint32_t ChunkCount{100};

    // chunks are freed when they go out of scope
    {
        std::vector<iox::mepoo::SharedChunk> chunkStore;
        mempoolconf.addMemPool({128, ChunkCount});
        sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
        for (size_t i = 0; i < ChunkCount; i++)
        {
            chunkStore.push_back(sut->getChunk(128, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
            EXPECT_THAT(chunkStore.back(), Eq(true));
        }

        EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(128, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
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

    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    auto bla = sut->getChunk(32, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(64, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(128, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    EXPECT_THAT(bla, Eq(true));

    bla = sut->getChunk(256, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    EXPECT_THAT(bla, Eq(true));
}

TEST_F(MemoryManager_test, getChunkMultiMemPoolAllChunks)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({32, ChunkCount});
    mempoolconf.addMemPool({64, ChunkCount});
    mempoolconf.addMemPool({128, ChunkCount});
    mempoolconf.addMemPool({256, ChunkCount});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(32U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(64U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(128U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(256U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
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
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(32U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        chunkStore.push_back(sut->getChunk(64U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        chunkStore.push_back(sut->getChunk(128U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        chunkStore.push_back(sut->getChunk(256U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
    }

    EXPECT_THAT(sut->getChunk(32U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(false));
    EXPECT_THAT(sut->getChunk(64U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(false));
    EXPECT_THAT(sut->getChunk(128U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(false));
    EXPECT_THAT(sut->getChunk(256U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(false));
}

TEST_F(MemoryManager_test, emptyMemPoolDoesNotResultInAcquiringChunksFromOtherMemPools)
{
    constexpr uint32_t ChunkCount{100};

    mempoolconf.addMemPool({32, ChunkCount});
    mempoolconf.addMemPool({64, ChunkCount});
    mempoolconf.addMemPool({128, ChunkCount});
    mempoolconf.addMemPool({256, ChunkCount});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(64U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
    }

    EXPECT_THAT(sut->getChunk(64U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(false));

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));
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
        sut->configureMemoryManager(mempoolconf, *allocator, *allocator);

        for (size_t i = 0; i < ChunkCount; i++)
        {
            chunkStore.push_back(sut->getChunk(32U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
            chunkStore.push_back(sut->getChunk(64U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
            chunkStore.push_back(sut->getChunk(128U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
            chunkStore.push_back(sut->getChunk(256U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        }

        EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
        EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(ChunkCount));
        EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(ChunkCount));
        EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(ChunkCount));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0U));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0U));

    std::vector<iox::mepoo::SharedChunk> chunkStore;
    for (size_t i = 0; i < ChunkCount; i++)
    {
        chunkStore.push_back(sut->getChunk(32U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(64U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(128U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));

        chunkStore.push_back(sut->getChunk(256U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
        EXPECT_THAT(chunkStore.back(), Eq(true));
    }

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(ChunkCount));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(ChunkCount));
}

TEST_F(MemoryManager_test, getChunkWithSizeZeroShouldNotFail)
{
    mempoolconf.addMemPool({32, 10});
    sut->configureMemoryManager(mempoolconf, *allocator, *allocator);
    EXPECT_THAT(sut->getChunk(0U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(true));
}

TEST_F(MemoryManager_test, addMemPoolWithChunkCountZeroShouldFail)
{
    mempoolconf.addMemPool({32, 0});
    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, *allocator, *allocator); }, ".*");
}

TEST_F(MemoryManager_test, requiredChunkSize_AllParameterMinimal_ResultsIn_SizeOfChunkHeader)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{1U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{0U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    constexpr uint32_t EXPECTED_SIZE{sizeof(ChunkHeader)};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(MemoryManager_test, requiredChunkSize_ZeroPayloadAndDefaultValues_ResultsIn_SizeOfChunkHeader)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(ChunkHeader)};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

// BEGIN EXCEEDING CHUNK SIZE TESTS

TEST_F(MemoryManager_test, requiredChunkSize_WithoutCustomPayloadAlignmentAndTooLargePayload_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{std::numeric_limits<uint32_t>::max()};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

TEST_F(MemoryManager_test, requiredChunkSize_WithCustomPayloadAlignmentAndTooLargePayload_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{std::numeric_limits<uint32_t>::max()};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(ChunkHeader) * 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

TEST_F(MemoryManager_test, requiredChunkSize_WithCustomHeaderAndTooLargePayload_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{std::numeric_limits<uint32_t>::max()};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(ChunkHeader) * 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{8U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{8U};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

// END EXCEEDING CHUNK SIZE TESTS

// BEGIN INVALID CUSTOM HEADER AND PAYLOAD ALIGNMENT TESTS

TEST_F(MemoryManager_test, requiredChunkSize_WithPayloadAlignmentOfZero_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{0U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{0U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

TEST_F(MemoryManager_test, requiredChunkSize_WithCustomHeaderAlignmentOfZero_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{1U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{0U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{0U};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

TEST_F(MemoryManager_test, requiredChunkSize_WithCustomHeaderAlignmentLargerThanChunkHeaderAlignment_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{8U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{2 * alignof(ChunkHeader)};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

TEST_F(MemoryManager_test, requiredChunkSize_WithCustomHeaderSizeNotMultipleOfAlignment_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{12U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{8U};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

// END INVALID CUSTOM HEADER AND PAYLOAD ALIGNMENT TESTS

// BEGIN PARAMETERIZED TESTS FOR REQUIRED CHUNK SIZE

struct PayloadParams
{
    uint32_t size{0U};
    uint32_t alignment{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};

    static constexpr uint32_t MAX_ALIGNMENT{1ULL << 31};
};

class MemoryManager_AlteringPayloadWithoutCustomHeader : public ::testing::TestWithParam<PayloadParams>
{
};

// without a custom header, the payload is located right after the ChunkHeader, therefore the payload size and alignment
// parameters are made dependant on the ChunkHeader
INSTANTIATE_TEST_CASE_P(MemoryManager_test,
                        MemoryManager_AlteringPayloadWithoutCustomHeader,
                        ::testing::Values(
                            // alignment = 1
                            PayloadParams{0U, 1U},
                            PayloadParams{1U, 1U},
                            PayloadParams{sizeof(ChunkHeader), 1U},
                            PayloadParams{sizeof(ChunkHeader) * 42U, 1U},
                            // alignment = alignof(ChunkHeader) / 2
                            PayloadParams{0U, alignof(ChunkHeader) / 2},
                            PayloadParams{1U, alignof(ChunkHeader) / 2},
                            PayloadParams{sizeof(ChunkHeader), alignof(ChunkHeader) / 2},
                            PayloadParams{sizeof(ChunkHeader) * 42U, alignof(ChunkHeader) / 2},
                            // alignment = alignof(ChunkHeader)
                            PayloadParams{0U, alignof(ChunkHeader)},
                            PayloadParams{1U, alignof(ChunkHeader)},
                            PayloadParams{sizeof(ChunkHeader), alignof(ChunkHeader)},
                            PayloadParams{sizeof(ChunkHeader) * 42U, alignof(ChunkHeader)},
                            // alignment = alignof(ChunkHeader) * 2
                            PayloadParams{0U, alignof(ChunkHeader) * 2},
                            PayloadParams{1U, alignof(ChunkHeader) * 2},
                            PayloadParams{sizeof(ChunkHeader), alignof(ChunkHeader) * 2},
                            PayloadParams{sizeof(ChunkHeader) * 42U, alignof(ChunkHeader) * 2},
                            // alignment = PayloadParams::MAX_ALIGNMENT
                            PayloadParams{0U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{1U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(ChunkHeader), PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(ChunkHeader) * 42U, PayloadParams::MAX_ALIGNMENT}));

TEST_P(MemoryManager_AlteringPayloadWithoutCustomHeader, requiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    const uint32_t expectedSize = [&payload] {
        if (payload.alignment <= alignof(ChunkHeader))
        {
            // payload is always adjacent
            return static_cast<uint32_t>(sizeof(ChunkHeader)) + payload.size;
        }
        else
        {
            // payload is not necessarily adjacent
            auto prePayloadAlignmentOverhangOfChunkHeder = sizeof(ChunkHeader) - alignof(ChunkHeader);
            return static_cast<uint32_t>(prePayloadAlignmentOverhangOfChunkHeder) + payload.alignment + payload.size;
        }
    }();

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

class MemoryManager_AlteringPayloadWithCustomHeader : public ::testing::TestWithParam<PayloadParams>
{
  protected:
};

// with a custom header, the payload is located right after the PayloadOffset_t, therefore the payload size and
// alignment parameters are made dependant on the PayloadOffset_t
INSTANTIATE_TEST_CASE_P(MemoryManager_test,
                        MemoryManager_AlteringPayloadWithCustomHeader,
                        ::testing::Values(
                            // alignment = 1
                            PayloadParams{0U, 1U},
                            PayloadParams{1U, 1U},
                            PayloadParams{sizeof(PayloadOffset_t), 1U},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, 1U},
                            // alignment = alignof(PayloadOffset_t) / 2
                            PayloadParams{0U, alignof(PayloadOffset_t) / 2},
                            PayloadParams{1U, alignof(PayloadOffset_t) / 2},
                            PayloadParams{sizeof(PayloadOffset_t), alignof(PayloadOffset_t) / 2},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, alignof(PayloadOffset_t) / 2},
                            // alignment = alignof(PayloadOffset_t)
                            PayloadParams{0U, alignof(PayloadOffset_t)},
                            PayloadParams{1U, alignof(PayloadOffset_t)},
                            PayloadParams{sizeof(PayloadOffset_t), alignof(PayloadOffset_t)},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, alignof(PayloadOffset_t)},
                            // alignment = alignof(PayloadOffset_t) * 2
                            PayloadParams{0U, alignof(PayloadOffset_t) * 2},
                            PayloadParams{1U, alignof(PayloadOffset_t) * 2},
                            PayloadParams{sizeof(PayloadOffset_t), alignof(PayloadOffset_t) * 2},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, alignof(PayloadOffset_t) * 2},
                            // alignment = PayloadParams::MAX_ALIGNMENT
                            PayloadParams{0U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{1U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(PayloadOffset_t), PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, PayloadParams::MAX_ALIGNMENT}));

uint32_t expectedChunkSizeWithCustomHeader(const PayloadParams& payload, uint32_t customHeaderSize)
{
    const uint32_t customHeaderSizeAndPaddingToBackOffset =
        iox::algorithm::max(customHeaderSize, static_cast<uint32_t>(alignof(PayloadOffset_t)));

    if (payload.alignment <= alignof(PayloadOffset_t))
    {
        // back-offset is always adjacent to the custom header (as much as possible with the alignment constraints)
        constexpr uint32_t BACK_OFFSET_SIZE{sizeof(PayloadOffset_t)};
        return static_cast<uint32_t>(sizeof(ChunkHeader)) + customHeaderSizeAndPaddingToBackOffset + BACK_OFFSET_SIZE
               + payload.size;
    }
    else
    {
        // back-offset is not necessarily adjacent to the custom header
        const uint32_t paddingBytesAndBackOffsetSize = payload.alignment;
        return static_cast<uint32_t>(sizeof(ChunkHeader)) + customHeaderSizeAndPaddingToBackOffset
               + paddingBytesAndBackOffsetSize + payload.size;
    }
}

// BEGIN ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ONE

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeEqualsToOne_AlignmentEqualsToOne_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{1U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeLessThanChunkHeader_AlignmentEqualsToOne_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) / 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeEqualsToChunkHeader_AlignmentEqualsToOne_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeGreaterThanChunkHeader_AlignmentEqualsToOne_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

// END ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ONE

// BEGIN ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT LESS THAN ChunkHeader ALIGNMENT

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeLessThanChunkHeader_AlignmentLessThanChunkHeaderAlignment_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) / 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader) / 2U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeEqualsToChunkHeader_AlignmentLessThanChunkHeaderAlignment_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader) / 2U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeGreaterThanChunkHeader_AlignmentLessThanChunkHeaderAlignment_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader) / 2U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

// END ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT LESS THAN ChunkHeader ALIGNMENT

// BEGIN ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ChunkHeader ALIGNMENT

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeEqualsToChunkHeader_AlignmentEqualToChunkHeaderAlignment_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader)};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

TEST_P(MemoryManager_AlteringPayloadWithCustomHeader,
       requiredChunkSize_CustomHeader_SizeGreaterThanChunkHeader_AlignmentEqualToChunkHeaderAlignment_IsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader)};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(expectedSize));
}

// END ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ChunkHeader ALIGNMENT

// END PARAMETERIZED TESTS FOR REQUIRED CHUNK SIZE

} // namespace
