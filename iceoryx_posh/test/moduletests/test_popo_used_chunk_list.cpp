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

#include "iceoryx_posh/internal/popo/used_chunk_list.hpp"

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/assertions.hpp"
#include "iox/bump_allocator.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::mepoo;
using namespace iox::popo;

class UsedChunkList_test : public Test
{
  public:
    void SetUp() override
    {
        static constexpr uint32_t NUM_CHUNKS_IN_POOL = 100U;
        static constexpr uint64_t CHUNK_SIZE = 128U;
        MePooConfig mempoolconf;
        mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});

        iox::BumpAllocator memoryAllocator{m_memory.get(), MEMORY_SIZE};
        memoryManager.configureMemoryManager(mempoolconf, memoryAllocator, memoryAllocator);
    };

    void TearDown() override{};

    SharedChunk getChunkFromMemoryManager()
    {
        constexpr uint64_t USER_PAYLOAD_SIZE{32U};
        auto chunkSettings =
            iox::mepoo::ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT)
                .expect("Valid 'ChunkSettings'");

        return memoryManager.getChunk(chunkSettings).expect("Obtaining chunk");
    }

    void createMultipleChunks(uint32_t numberOfChunks, std::function<void(SharedChunk&&)> testHook)
    {
        ASSERT_TRUE(testHook);
        for (uint32_t i = 0; i < numberOfChunks; ++i)
        {
            testHook(getChunkFromMemoryManager());
        }
    }

    void checkIfEmpty()
    {
        SCOPED_TRACE(std::string("Empty check"));
        for (uint32_t i = 0; i < USED_CHUNK_LIST_CAPACITY; ++i)
        {
            EXPECT_TRUE(sut.insert(getChunkFromMemoryManager()));
        }
    }

    MemoryManager memoryManager;

    static constexpr uint32_t USED_CHUNK_LIST_CAPACITY{10U};
    UsedChunkList<USED_CHUNK_LIST_CAPACITY> sut;

  private:
    static constexpr size_t MEGABYTE = 1U << 20U;
    static constexpr size_t MEMORY_SIZE = 4U * MEGABYTE;
    std::unique_ptr<char[]> m_memory{new char[MEMORY_SIZE]};
};

TEST_F(UsedChunkList_test, OneChunkCanBeAdded)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe1ee816-b0a6-4468-9652-b0b32e310960");
    EXPECT_TRUE(sut.insert(getChunkFromMemoryManager()));
}

TEST_F(UsedChunkList_test, AddSameChunkTwiceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cad24b29-fc1b-4c71-b1be-c04d8653ec9f");
    auto chunk = getChunkFromMemoryManager();
    sut.insert(chunk);

    EXPECT_TRUE(sut.insert(chunk));
}

TEST_F(UsedChunkList_test, MultipleChunksCanBeAdded)
{
    ::testing::Test::RecordProperty("TEST_ID", "14eafeb1-ff36-415d-a7f5-a31250332efc");
    EXPECT_TRUE(sut.insert(getChunkFromMemoryManager()));
    EXPECT_TRUE(sut.insert(getChunkFromMemoryManager()));
    EXPECT_TRUE(sut.insert(getChunkFromMemoryManager()));
}

TEST_F(UsedChunkList_test, AddChunksUpToCapacityWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1b1b9b29-e00f-4791-9a62-f8fd398fb955");
    createMultipleChunks(USED_CHUNK_LIST_CAPACITY, [this](SharedChunk&& chunk) { EXPECT_TRUE(sut.insert(chunk)); });
}

TEST_F(UsedChunkList_test, AddChunksUntilOverflowIsHandledGracefully)
{
    ::testing::Test::RecordProperty("TEST_ID", "6922015f-bf26-4bf3-8b7d-8adf6e846030");
    createMultipleChunks(USED_CHUNK_LIST_CAPACITY, [this](SharedChunk&& chunk) { EXPECT_TRUE(sut.insert(chunk)); });

    EXPECT_FALSE(sut.insert(getChunkFromMemoryManager()));
}

TEST_F(UsedChunkList_test, OneChunkCanBeRemoved)
{
    ::testing::Test::RecordProperty("TEST_ID", "50ffb5df-59ef-4dd4-a2a6-c7ad342c24ae");
    auto chunk = getChunkFromMemoryManager();
    auto chunkHeader = chunk.getChunkHeader();
    sut.insert(chunk);

    SharedChunk removedChunk;
    EXPECT_TRUE(sut.remove(chunkHeader, removedChunk));
    EXPECT_TRUE(removedChunk);

    checkIfEmpty();
}

TEST_F(UsedChunkList_test, RemoveSameChunkAddedTwiceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d87cd89-d413-44e9-b728-472eef78a8f9");
    auto chunk = getChunkFromMemoryManager();
    auto chunkHeader = chunk.getChunkHeader();
    sut.insert(chunk);
    sut.insert(chunk);

    for (auto i = 0; i < 2; ++i)
    {
        SharedChunk removedChunk;
        EXPECT_TRUE(sut.remove(chunkHeader, removedChunk));
        EXPECT_TRUE(removedChunk);
    }

    checkIfEmpty();
}

TEST_F(UsedChunkList_test, MultipleChunksCanBeRemoved)
{
    ::testing::Test::RecordProperty("TEST_ID", "13dd689b-d0b4-4d30-a154-0b8a9e7de5e1");
    std::vector<ChunkHeader*> chunkHeaderInUse;
    createMultipleChunks(3U, [&](SharedChunk&& chunk) {
        chunkHeaderInUse.push_back(chunk.getChunkHeader());
        sut.insert(chunk);
    });

    for (auto chunkHeader : chunkHeaderInUse)
    {
        SharedChunk removedChunk;
        EXPECT_TRUE(sut.remove(chunkHeader, removedChunk));
        EXPECT_TRUE(removedChunk);
    }

    checkIfEmpty();
}

TEST_F(UsedChunkList_test, MultipleChunksCanBeRemovedInReverseOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "bab1402e-1217-4d3f-8386-c78777c709bf");
    std::vector<ChunkHeader*> chunkHeaderInUse;
    createMultipleChunks(3U, [&](SharedChunk&& chunk) {
        chunkHeaderInUse.push_back(chunk.getChunkHeader());
        sut.insert(chunk);
    });

    constexpr uint32_t removeOrderIndices[]{2U, 1U, 0U};
    for (auto index : removeOrderIndices)
    {
        SharedChunk removedChunk;
        EXPECT_TRUE(sut.remove(chunkHeaderInUse[index], removedChunk));
        EXPECT_TRUE(removedChunk);
    }

    checkIfEmpty();
}

TEST_F(UsedChunkList_test, MultipleChunksCanBeRemovedInArbitraryOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f8030d5-be3e-4804-a1f3-c3f7ebcbf88b");
    std::vector<ChunkHeader*> chunkHeaderInUse;
    createMultipleChunks(3U, [&](SharedChunk&& chunk) {
        chunkHeaderInUse.push_back(chunk.getChunkHeader());
        sut.insert(chunk);
    });

    constexpr uint32_t removeOrderIndices[]{0U, 2U, 1U};
    for (auto index : removeOrderIndices)
    {
        SharedChunk removedChunk;
        EXPECT_TRUE(sut.remove(chunkHeaderInUse[index], removedChunk));
        EXPECT_TRUE(removedChunk);
    }

    checkIfEmpty();
}

TEST_F(UsedChunkList_test, UsedChunkListCanBeFilledToCapacityAndFullyEmptied)
{
    ::testing::Test::RecordProperty("TEST_ID", "5932b727-dfbe-4041-985d-7a819c8ea06c");
    std::vector<ChunkHeader*> chunkHeaderInUse;
    createMultipleChunks(USED_CHUNK_LIST_CAPACITY, [&](SharedChunk&& chunk) {
        chunkHeaderInUse.push_back(chunk.getChunkHeader());
        EXPECT_TRUE(sut.insert(chunk));
    });

    for (auto chunkHeader : chunkHeaderInUse)
    {
        SharedChunk removedChunk;
        EXPECT_TRUE(sut.remove(chunkHeader, removedChunk));
        EXPECT_TRUE(removedChunk);
    }

    checkIfEmpty();
}

TEST_F(UsedChunkList_test, RemoveChunkFromEmptyListIsHandledGracefully)
{
    ::testing::Test::RecordProperty("TEST_ID", "2c4a64d1-07cc-4334-89bf-dd58ad291af5");
    auto chunk = getChunkFromMemoryManager();
    auto chunkHeader = chunk.getChunkHeader();

    SharedChunk chunkNotInList;
    EXPECT_FALSE(sut.remove(chunkHeader, chunkNotInList));
    EXPECT_FALSE(chunkNotInList);
}

TEST_F(UsedChunkList_test, RemoveChunkNotInListIsHandledGracefully)
{
    ::testing::Test::RecordProperty("TEST_ID", "ecca24b5-c526-4c41-b78f-ff34d6e9ee3e");
    createMultipleChunks(3U, [&](SharedChunk&& chunk) { sut.insert(chunk); });

    auto chunk = getChunkFromMemoryManager();
    auto chunkHeader = chunk.getChunkHeader();

    SharedChunk chunkNotInList;
    EXPECT_FALSE(sut.remove(chunkHeader, chunkNotInList));
    EXPECT_FALSE(chunkNotInList);
}

TEST_F(UsedChunkList_test, RemoveChunkNotInListDoesNotRemoveOtherChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a01903b-3fd2-4632-a941-60621d6f3450");
    std::vector<ChunkHeader*> chunkHeaderInUse;
    createMultipleChunks(3U, [&](SharedChunk&& chunk) {
        chunkHeaderInUse.push_back(chunk.getChunkHeader());
        sut.insert(chunk);
    });

    auto chunk = getChunkFromMemoryManager();
    auto chunkHeader = chunk.getChunkHeader();
    SharedChunk chunkNotInList;
    sut.remove(chunkHeader, chunkNotInList);

    for (auto chunkHeader : chunkHeaderInUse)
    {
        SharedChunk removedChunk;
        EXPECT_TRUE(sut.remove(chunkHeader, removedChunk));
        EXPECT_TRUE(removedChunk);
    }
}

TEST_F(UsedChunkList_test, ChunksAddedToTheUsedChunkKeepsTheChunkAlive)
{
    ::testing::Test::RecordProperty("TEST_ID", "ea43942e-1000-4dbf-ad05-00af18373fc1");
    EXPECT_THAT(memoryManager.getMemPoolInfo(0U).m_usedChunks, Eq(0U));

    sut.insert(getChunkFromMemoryManager());

    EXPECT_THAT(memoryManager.getMemPoolInfo(0U).m_usedChunks, Eq(1U));
}

TEST_F(UsedChunkList_test, RemovingChunkFromListLetsTheSharedChunkReturnOwnershipToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "058ded9f-fa74-4a37-a2c1-3a9511f3153d");
    {
        auto chunk = getChunkFromMemoryManager();
        auto chunkHeader = chunk.getChunkHeader();
        sut.insert(chunk);

        SharedChunk removedChunk;
        sut.remove(chunkHeader, removedChunk);
    }

    EXPECT_THAT(memoryManager.getMemPoolInfo(0U).m_usedChunks, Eq(0U));
}

TEST_F(UsedChunkList_test, CallingCleanupReleasesAllChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "765e2726-b022-41fc-a839-77db9ac07d2b");
    std::vector<ChunkHeader*> chunkHeaderInUse;
    createMultipleChunks(USED_CHUNK_LIST_CAPACITY, [&](SharedChunk&& chunk) {
        chunkHeaderInUse.push_back(chunk.getChunkHeader());
        sut.insert(chunk);
    });

    sut.cleanup();

    EXPECT_THAT(memoryManager.getMemPoolInfo(0U).m_usedChunks, Eq(0U));
    checkIfEmpty();
}
} // namespace
