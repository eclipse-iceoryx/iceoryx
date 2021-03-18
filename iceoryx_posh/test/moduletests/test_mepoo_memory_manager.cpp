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

    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, allocator, allocator); }, ".*");
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
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

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
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    auto bla = sut->getChunk(50, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
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
        chunkStore.push_back(sut->getChunk(50, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT));
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
        sut->configureMemoryManager(mempoolconf, allocator, allocator);
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

    sut->configureMemoryManager(mempoolconf, allocator, allocator);

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
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

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
    sut->configureMemoryManager(mempoolconf, allocator, allocator);

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

    EXPECT_THAT(sut->getMemPoolInfo(0).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(1).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(2).m_usedChunks, Eq(0u));
    EXPECT_THAT(sut->getMemPoolInfo(3).m_usedChunks, Eq(0u));

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
    sut->configureMemoryManager(mempoolconf, allocator, allocator);
    EXPECT_THAT(sut->getChunk(0U, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT), Eq(true));
}

TEST_F(MemoryManager_test, addMemPoolWithChunkCountZeroShouldFail)
{
    mempoolconf.addMemPool({32, 0});
    EXPECT_DEATH({ sut->configureMemoryManager(mempoolconf, allocator, allocator); }, ".*");
}

TEST_F(MemoryManager_test, requiredChunkSize_AllParameterMinimal_ResultsIn_SizeOfChunkHeader)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{1U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{0U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader)};

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

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader)};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(MemoryManager_test, requiredChunkSize_PayloadAlignment_LessThan_ChunkHeaderAlignment_ResultsIn_AdjacentPayload)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(iox::mepoo::ChunkHeader) / 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader) + PAYLOAD_SIZE};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(MemoryManager_test, requiredChunkSize_PayloadAlignment_EqualTo_ChunkHeaderAlignment_ResultsIn_AdjacentPayload)
{
    constexpr uint32_t PAYLOAD_SIZE{73U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(iox::mepoo::ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader) + PAYLOAD_SIZE};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(MemoryManager_test,
       requiredChunkSize_PayloadAlignment_GreaterThan_ChunkHeaderAlignment_ResultsIn_AddingPaddingBytes)
{
    constexpr uint32_t PAYLOAD_SIZE{37U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(iox::mepoo::ChunkHeader) * 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};
    constexpr uint32_t PADDING_BYTES{PAYLOAD_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader) - alignof(iox::mepoo::ChunkHeader) + PADDING_BYTES
                                     + PAYLOAD_SIZE};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(MemoryManager_test,
       requiredChunkSize_CustomHeaderAlignment_LessThan_ChunkHeaderAlignment_AdjacentPayload_ResultsIn_NoPaddingBytes)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(iox::mepoo::ChunkHeader::PayloadOffset_t)};
    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(iox::mepoo::ChunkHeader::PayloadOffset_t)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{alignof(iox::mepoo::ChunkHeader::PayloadOffset_t)};
    constexpr uint32_t BACK_OFFSET{sizeof(iox::mepoo::ChunkHeader::PayloadOffset_t)};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader) + CUSTOM_HEADER_SIZE + BACK_OFFSET + PAYLOAD_SIZE};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(alignof(iox::mepoo::ChunkHeader::PayloadOffset_t), Lt(alignof(iox::mepoo::ChunkHeader)));
    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(
    MemoryManager_test,
    requiredChunkSize_CustomHeaderAlignment_LessThan_ChunkHeaderAlignment_NonAdjacentPayload_ResultsIn_AddingPaddingBytes)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(iox::mepoo::ChunkHeader::PayloadOffset_t)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{alignof(iox::mepoo::ChunkHeader::PayloadOffset_t)};
    constexpr uint32_t PADDING_BYTES{PAYLOAD_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader) + CUSTOM_HEADER_SIZE + PADDING_BYTES
                                     + PAYLOAD_SIZE};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(alignof(iox::mepoo::ChunkHeader::PayloadOffset_t), Lt(alignof(iox::mepoo::ChunkHeader)));
    EXPECT_THAT(PADDING_BYTES, Ge(sizeof(iox::mepoo::ChunkHeader::PayloadOffset_t)));
    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(MemoryManager_test,
       requiredChunkSize_CustomHeaderAlignment_EqualTo_ChunkHeaderAlignment_ResultsIn_NonAdjacentPayload)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(iox::mepoo::ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{alignof(iox::mepoo::ChunkHeader)};
    constexpr uint32_t PADDING_BYTES{PAYLOAD_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader) + CUSTOM_HEADER_SIZE + PADDING_BYTES
                                     + PAYLOAD_SIZE};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(PADDING_BYTES, Ge(sizeof(iox::mepoo::ChunkHeader::PayloadOffset_t)));
    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

TEST_F(MemoryManager_test, requiredChunkSize_CustomHeaderAndPayloadAlignment_ResultsIn_AddingPaddingBytes)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(iox::mepoo::ChunkHeader) * 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(iox::mepoo::ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{alignof(iox::mepoo::ChunkHeader)};
    constexpr uint32_t PADDING_BYTES{PAYLOAD_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(iox::mepoo::ChunkHeader) + CUSTOM_HEADER_SIZE + PADDING_BYTES
                                     + PAYLOAD_SIZE};

    auto chunkSize = iox::mepoo::MemoryManager::requiredChunkSize(
        PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    EXPECT_THAT(PADDING_BYTES, Ge(sizeof(iox::mepoo::ChunkHeader::PayloadOffset_t)));
    EXPECT_THAT(chunkSize, Eq(EXPECTED_SIZE));
}

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
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(iox::mepoo::ChunkHeader) * 2};
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
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(iox::mepoo::ChunkHeader) * 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{8U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{8U};

    EXPECT_DEATH(
        {
            iox::mepoo::MemoryManager::requiredChunkSize(
                PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
        },
        ".*");
}

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
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{2 * alignof(iox::mepoo::ChunkHeader)};

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
