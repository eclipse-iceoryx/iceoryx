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
    ::testing::Test::RecordProperty("TEST_ID", "b15b0da5-74e0-481b-87b6-53888b8a9890");
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
    ::testing::Test::RecordProperty("TEST_ID", "ee06090a-8e3c-4df2-b74e-ed50e29b84e6");
    char memory[8192U];
    iox::posix::Allocator allocator{memory, 100U};
    constexpr uint32_t NOT_ALLIGNED_CHUNKED_SIZE{33U};

    iox::optional<iox::PoshError> detectedError;
    auto errorHandlerGuard = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::PoshError>(
        [&detectedError](const iox::PoshError error, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    iox::mepoo::MemPool sut(NOT_ALLIGNED_CHUNKED_SIZE, NUMBER_OF_CHUNKS, allocator, allocator);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(),
                Eq(iox::PoshError::MEPOO__MEMPOOL_CHUNKSIZE_MUST_BE_MULTIPLE_OF_CHUNK_MEMORY_ALIGNMENT));
}

TEST_F(MemPool_test, MempoolCtorWhenChunkSizeIsSmallerThanChunkMemoryAlignmentGetsTerminated)
{
    ::testing::Test::RecordProperty("TEST_ID", "52df897a-0847-476c-9d2f-99cb16432199");
    constexpr uint32_t CHUNK_SIZE_SMALLER_THAN_MEMORY_ALIGNMENT = iox::mepoo::MemPool::CHUNK_MEMORY_ALIGNMENT - 1U;

    EXPECT_DEATH(
        { iox::mepoo::MemPool sut(CHUNK_SIZE_SMALLER_THAN_MEMORY_ALIGNMENT, NUMBER_OF_CHUNKS, allocator, allocator); },
        ".*");
}

TEST_F(MemPool_test, MempoolCtorWhenNumberOfChunksIsZeroGetsTerminated)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5c43e1b-e5b5-4a69-8c4c-8b95752ade8e");
    constexpr uint32_t INVALID_NUMBER_OF_CHUNKS = 0U;
    EXPECT_DEATH({ iox::mepoo::MemPool sut(CHUNK_SIZE, INVALID_NUMBER_OF_CHUNKS, allocator, allocator); }, ".*");
}
TEST_F(MemPool_test, GetChunkMethodWhenAllTheChunksAreUsedReturnsNullPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "43caa0b0-419b-4749-988d-c3af5ada35ad");
    for (uint8_t i = 0U; i < NUMBER_OF_CHUNKS; i++)
    {
        sut.getChunk();
    }

    EXPECT_THAT(sut.getChunk(), Eq(nullptr));
}

TEST_F(MemPool_test, WritingDataToAChunkStoresTheCorrespondingDataInTheChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "4550d044-d1c8-493d-b839-40509b03407f");
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
    ::testing::Test::RecordProperty("TEST_ID", "f32e1446-1a60-4d1d-b713-4193b49466bd");
    EXPECT_THAT(sut.getChunkSize(), Eq(CHUNK_SIZE));
}

TEST_F(MemPool_test, GetChunkCountMethodReturnsTheNumberOfChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8012260-ec47-4e0e-9d15-964740887938");
    EXPECT_THAT(sut.getChunkCount(), Eq(NUMBER_OF_CHUNKS));
}

TEST_F(MemPool_test, GetUsedChunksMethodReturnsTheNumberOfUsedChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e2d1a62-5bbf-4139-a7cf-c0ca9650441f");
    for (uint32_t i = 0U; i < NUMBER_OF_CHUNKS; ++i)
    {
        sut.getChunk();
        EXPECT_THAT(sut.getUsedChunks(), Eq(i + 1U));
    }
}

TEST_F(MemPool_test, VerifyFreeChunkMethodWhichFreesTheUsedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "e590fa58-732f-4027-85a9-9e0c8593ea70");
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
    ::testing::Test::RecordProperty("TEST_ID", "0fc95552-4714-4a8a-9144-98aafbd37dc7");
    std::vector<uint8_t*> chunks;
    constexpr uint32_t INDEX{0U};
    chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
    sut.freeChunk(chunks[INDEX]);
    iox::optional<iox::PoshError> detectedError;
    auto errorHandlerGuard = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::PoshError>(
        [&detectedError](const iox::PoshError error, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    sut.freeChunk(chunks[INDEX]);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(), Eq(iox::PoshError::POSH__MEMPOOL_POSSIBLE_DOUBLE_FREE));
}

TEST_F(MemPool_test, FreeChunkMethodWhenTheChunkIndexIsInvalidReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb68953e-d47a-4658-8109-6b1974a8baab");
    std::vector<uint8_t*> chunks;
    constexpr uint32_t INVALID_INDEX{1U};
    chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));

    EXPECT_DEATH({ sut.freeChunk(chunks[INVALID_INDEX]); }, ".*");
}

TEST_F(MemPool_test, GetMinFreeMethodReturnsTheNumberOfFreeChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b6cf614e-836a-4a15-850e-700031bfa016");
    std::vector<uint8_t*> chunks;
    for (uint32_t i = 0U; i < NUMBER_OF_CHUNKS; ++i)
    {
        chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
        EXPECT_THAT(sut.getMinFree(), Eq(NUMBER_OF_CHUNKS - (i + 1U)));
    }
}

TEST_F(MemPool_test, dieWhenMempoolChunkSizeIsSmallerThan32Bytes)
{
    ::testing::Test::RecordProperty("TEST_ID", "7704246e-42b5-46fd-8827-ebac200390e1");
    EXPECT_DEATH({ iox::mepoo::MemPool sut(12, 10, allocator, allocator); }, ".*");
}

TEST_F(MemPool_test, dieWhenMempoolChunkSizeIsNotPowerOf32)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a354976-235a-4a94-8af2-2bc872f705f4");
    EXPECT_DEATH({ iox::mepoo::MemPool sut(333, 10, allocator, allocator); }, ".*");
}

} // namespace
