// Copyright (c) 2019, 2021 by  Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

using namespace ::testing;

class alignas(32) MemPool_test : public Test
{
  public:
    static constexpr uint32_t NumberOfChunks{100U};
    static constexpr uint32_t ChunkSize{64U};

    static constexpr uint32_t LoFFLiMemoryRequirement{
        iox::mepoo::MemPool::freeList_t::requiredMemorySize(NumberOfChunks) + 10000};

    MemPool_test()
        : allocator(m_rawMemory, NumberOfChunks * ChunkSize + LoFFLiMemoryRequirement)
        , sut(ChunkSize, NumberOfChunks, &allocator, &allocator)
    {
    }

    void SetUp(){};
    void TearDown(){};

    alignas(32) uint8_t m_rawMemory[NumberOfChunks * ChunkSize + LoFFLiMemoryRequirement];
    iox::posix::Allocator allocator;

    iox::mepoo::MemPool sut;
};

TEST_F(MemPool_test, MempoolCtorInitialisesTheObjectWithValuesPassedToTheCtor)
{
    char memory[8192];
    iox::posix::Allocator allocator1{memory, 8192U};

    iox::mepoo::MemPool sut1(ChunkSize, NumberOfChunks, &allocator1, &allocator1);

    EXPECT_THAT(sut1.getChunkSize(), Eq(ChunkSize));
    EXPECT_THAT(sut1.getChunkCount(), Eq(NumberOfChunks));
    EXPECT_THAT(sut1.getMinFree(), Eq(NumberOfChunks));
}

TEST_F(MemPool_test, MempoolCtorWhenChunkSizeIsNotAMultipleOfAlignmentReturnError)
{
    char memory[8192U];
    iox::posix::Allocator allocator1{memory, 100U};
    uint32_t NotAlignedChunkSize = 33U;

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    iox::mepoo::MemPool sut1(NotAlignedChunkSize, NumberOfChunks, &allocator1, &allocator1);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(
        detectedError.value(),
        Eq(iox::Error::
               kMEPOO__MEMPOOL_CHUNKSIZE_MUST_BE_LARGER_THAN_SHARED_MEMORY_ALIGNMENT_AND_MULTIPLE_OF_ALIGNMENT));
}

TEST_F(MemPool_test, MempoolCtorWhenChunkSizeIsSmallerThan32BytesGetsTerminated)
{
    uint32_t chunkSizeSmallerThan32Bytes = 12U;
    EXPECT_DEATH({ iox::mepoo::MemPool sut(chunkSizeSmallerThan32Bytes, NumberOfChunks, &allocator, &allocator); },
                 ".*");
}

TEST_F(MemPool_test, MempoolCtorWhenNumberOfChunksIsZeroGetsTerminated)
{
    uint32_t invalidNumberOfChunks = 0U;
    EXPECT_DEATH({ iox::mepoo::MemPool sut(ChunkSize, invalidNumberOfChunks, &allocator, &allocator); }, ".*");
}
TEST_F(MemPool_test, GetChunkMethodWhenAllTheChunksAreUsedReturnsNullPointer)
{
    for (uint8_t i = 0; i < NumberOfChunks; i++)
    {
        sut.getChunk();
    }

    EXPECT_THAT(sut.getChunk(), Eq(nullptr));
}

TEST_F(MemPool_test, WritingDataToAChunkStoresTheCorrespondingDataInTheChunk)
{
    std::vector<uint8_t*> chunks;
    for (uint8_t i = 0; i < NumberOfChunks; i++)
    {
        auto chunk = sut.getChunk();
        ASSERT_THAT(chunk, Ne(nullptr));
        chunks.push_back(reinterpret_cast<uint8_t*>(chunk));
        *chunks.back() = i;
    }

    for (uint8_t i = 0; i < NumberOfChunks; i++)
    {
        EXPECT_THAT(*chunks[i], Eq(i));
    }
}

TEST_F(MemPool_test, GetChunkSizeMethodReturnsTheSizeOfTheChunk)
{
    EXPECT_THAT(sut.getChunkSize(), Eq(ChunkSize));
}

TEST_F(MemPool_test, GetChunkCountMethodReturnsTheNumberOfChunks)
{
    EXPECT_THAT(sut.getChunkCount(), Eq(NumberOfChunks));
}

TEST_F(MemPool_test, GetUsedChunksMethodReturnsTheNumberOfUsedChunks)
{
    for (uint32_t i = 0; i < NumberOfChunks; ++i)
    {
        sut.getChunk();
        EXPECT_THAT(sut.getUsedChunks(), Eq(i + 1U));
    }
}

TEST_F(MemPool_test, VerifyFreeChunkMethodWhichFreesTheUsedChunk)
{
    std::vector<uint8_t*> chunks;
    for (uint32_t i = 0; i < NumberOfChunks; ++i)
    {
        chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
    }

    for (uint32_t i = 0; i < NumberOfChunks; ++i)
    {
        sut.freeChunk(chunks[i]);
        EXPECT_THAT(sut.getUsedChunks(), Eq(NumberOfChunks - (i + 1U)));
    }
}

TEST_F(MemPool_test, FreeChunkMethodWhenSameChunkIsTriedToFreeTwiceReturnsError)
{
    std::vector<uint8_t*> chunks;
    uint32_t index = 0U;
    chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
    sut.freeChunk(chunks[index]);
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    sut.freeChunk(chunks[index]);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__MEMPOOL_POSSIBLE_DOUBLE_FREE));
}

TEST_F(MemPool_test, FreeChunkMethodWhenTheChunkIndexIsInvalidReturnsError)
{
    std::vector<uint8_t*> chunks;
    uint32_t invalidindex = 1U;
    chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));

    EXPECT_DEATH({ sut.freeChunk(chunks[invalidindex]); }, ".*");
}

TEST_F(MemPool_test, GetMinFreeMethodReturnsTheNumberOfFreeChunks)
{
    std::vector<uint8_t*> chunks;
    for (uint32_t i = 0; i < NumberOfChunks; ++i)
    {
        chunks.push_back(reinterpret_cast<uint8_t*>(sut.getChunk()));
        EXPECT_THAT(sut.getMinFree(), Eq(NumberOfChunks - (i + 1U)));
    }
}
