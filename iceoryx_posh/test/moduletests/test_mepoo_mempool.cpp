// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::mepoo;

class MemPool_test : public Test
{
  public:
    static constexpr uint32_t NUMBER_OF_CHUNKS{100U};
    static constexpr uint32_t CHUNK_SIZE{64U};

    using FreeListIndex_t = iox::mepoo::MemPool::freeList_t::Index_t;
    static constexpr FreeListIndex_t LOFFLI_MEMORY_REQUIREMENT{
        iox::mepoo::MemPool::freeList_t::requiredIndexMemorySize(NUMBER_OF_CHUNKS) + 10000U};

    MemPool_test()
        : allocator(m_rawMemory, NUMBER_OF_CHUNKS * CHUNK_SIZE + LOFFLI_MEMORY_REQUIREMENT)
        , sut(CHUNK_SIZE, NUMBER_OF_CHUNKS, allocator, allocator)
    {
    }

    void SetUp(){};
    void TearDown(){};

    alignas(MemPool::CHUNK_MEMORY_ALIGNMENT) uint8_t
        m_rawMemory[NUMBER_OF_CHUNKS * CHUNK_SIZE + LOFFLI_MEMORY_REQUIREMENT];
    iox::posix::Allocator allocator;

    MemPool sut;
};

TEST_F(MemPool_test, MempoolCtorInitialisesTheObjectWithValuesPassedToTheCtor)
{
    char memory[8192];
    iox::posix::Allocator allocator{memory, 8192U};

    iox::mepoo::MemPool sut(CHUNK_SIZE, NUMBER_OF_CHUNKS, allocator, allocator);

    EXPECT_THAT(sut.getChunkSize(), Eq(CHUNK_SIZE));
    EXPECT_THAT(sut.getChunkCount(), Eq(NUMBER_OF_CHUNKS));
    EXPECT_THAT(sut.getMinFree(), Eq(NUMBER_OF_CHUNKS));
    EXPECT_THAT(sut.getUsedChunks(), Eq(0U));
}

TEST_F(MemPool_test, MempoolCtorWhenChunkSizeIsNotAMultipleOfAlignmentReturnError)
{
    char memory[8192U];
    iox::posix::Allocator allocator{memory, 100U};
    constexpr uint32_t NOT_ALLIGNED_CHUNKED_SIZE{33U};

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    iox::mepoo::MemPool sut(NOT_ALLIGNED_CHUNKED_SIZE, NUMBER_OF_CHUNKS, allocator, allocator);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(),
                Eq(iox::Error::kMEPOO__MEMPOOL_CHUNKSIZE_MUST_BE_MULTIPLE_OF_CHUNK_MEMORY_ALIGNMENT));
}

TEST_F(MemPool_test, MempoolCtorWhenChunkSizeIsSmallerThanChunkMemoryAlignmentGetsTerminated)
{
    constexpr uint32_t CHUNK_SIZE_SMALLER_THAN_MEMORY_ALIGNMENT = iox::mepoo::MemPool::CHUNK_MEMORY_ALIGNMENT - 1U;

    EXPECT_DEATH(
        { iox::mepoo::MemPool sut(CHUNK_SIZE_SMALLER_THAN_MEMORY_ALIGNMENT, NUMBER_OF_CHUNKS, allocator, allocator); },
        ".*");
}

TEST_F(MemPool_test, MempoolCtorWhenNumberOfChunksIsZeroGetsTerminated)
{
    constexpr uint32_t INVALID_NUMBER_OF_CHUNKS = 0U;
    EXPECT_DEATH({ iox::mepoo::MemPool sut(CHUNK_SIZE, INVALID_NUMBER_OF_CHUNKS, allocator, allocator); }, ".*");
}
TEST_F(MemPool_test, GetChunkMethodWhenAllTheChunksAreUsedReturnsNullPointer)
{
    for (uint8_t i = 0U; i < NUMBER_OF_CHUNKS; i++)
    {
        sut.getChunk();
    }

    EXPECT_THAT(sut.getChunk(), Eq(nullptr));
}

TEST_F(MemPool_test, WritingDataToAChunkStoresTheCorrespondingDataInTheChunk)
{
    std::vector<uint8_t*> chunks;
    for (uint8_t i = 0U; i < NUMBER_OF_CHUNKS; i++)
    {
        auto chunk = sut.getChunk();
        ASSERT_THAT(chunk, Ne(nullptr));
        chunks.push_back(reinterpret_cast<uint8_t*>(chunk));
        *chunks.back() = i;
    }

    for (uint8_t i = 0U; i < NUMBER_OF_CHUNKS; i++)
    {
        EXPECT_THAT(*chunks[i], Eq(i));
    }
}

TEST_F(MemPool_test, GetChunkSizeMethodReturnsTheSizeOfTheChunk)
{
    EXPECT_THAT(sut.getChunkSize(), Eq(CHUNK_SIZE));
}

TEST_F(MemPool_test, GetChunkCountMethodReturnsTheNumberOfChunks)
{
    EXPECT_THAT(sut.getChunkCount(), Eq(NUMBER_OF_CHUNKS));
}

TEST_F(MemPool_test, GetUsedChunksMethodReturnsTheNumberOfUsedChunks)
{
    for (uint32_t i = 0U; i < NUMBER_OF_CHUNKS; ++i)
    {
        sut.getChunk();
        EXPECT_THAT(sut.getUsedChunks(), Eq(i + 1U));
    }
}

TEST_F(MemPool_test, VerifyFreeChunkMethodWhichFreesTheUsedChunk)
{
    std::vector<uint8_t*> chunks;
    for (uint32_t i = 0U; i < NUMBER_OF_CHUNKS; ++i)
    {
        chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
    }

    for (uint32_t i = 0U; i < NUMBER_OF_CHUNKS; ++i)
    {
        sut.freeChunk(chunks[i]);
        EXPECT_THAT(sut.getUsedChunks(), Eq(NUMBER_OF_CHUNKS - (i + 1U)));
    }
}

TEST_F(MemPool_test, FreeChunkMethodWhenSameChunkIsTriedToFreeTwiceReturnsError)
{
    std::vector<uint8_t*> chunks;
    constexpr uint32_t INDEX{0U};
    chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
    sut.freeChunk(chunks[INDEX]);
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    sut.freeChunk(chunks[INDEX]);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__MEMPOOL_POSSIBLE_DOUBLE_FREE));
}

TEST_F(MemPool_test, FreeChunkMethodWhenTheChunkIndexIsInvalidReturnsError)
{
    std::vector<uint8_t*> chunks;
    constexpr uint32_t INVALID_INDEX{1U};
    chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));

    EXPECT_DEATH({ sut.freeChunk(chunks[INVALID_INDEX]); }, ".*");
}

TEST_F(MemPool_test, GetMinFreeMethodReturnsTheNumberOfFreeChunks)
{
    std::vector<uint8_t*> chunks;
    for (uint32_t i = 0U; i < NUMBER_OF_CHUNKS; ++i)
    {
        chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
        EXPECT_THAT(sut.getMinFree(), Eq(NUMBER_OF_CHUNKS - (i + 1U)));
    }
}

TEST_F(MemPool_test, dieWhenMempoolChunkSizeIsSmallerThan32Bytes)
{
    EXPECT_DEATH({ iox::mepoo::MemPool sut(12, 10, allocator, allocator); }, ".*");
}

TEST_F(MemPool_test, dieWhenMempoolChunkSizeIsNotPowerOf32)
{
    EXPECT_DEATH({ iox::mepoo::MemPool sut(333, 10, allocator, allocator); }, ".*");
}

} // namespace
