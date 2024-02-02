// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/shm_safe_unmanaged_chunk.hpp"

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/assertions.hpp"
#include "iox/bump_allocator.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::mepoo;

class ShmSafeUnmanagedChunk_test : public Test
{
  public:
    void SetUp() override
    {
        MePooConfig mempoolconf;
        mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS_IN_POOL});
        memoryManager.configureMemoryManager(mempoolconf, m_memoryAllocator, m_memoryAllocator);
    }

    SharedChunk getChunkFromMemoryManager()
    {
        auto chunkSettings =
            iox::mepoo::ChunkSettings::create(sizeof(bool), alignof(bool)).expect("Valid 'ChunkSettings'");

        return memoryManager.getChunk(chunkSettings).expect("Obtaining chunk");
    }

    iox::mepoo::MemoryManager memoryManager;

  private:
    static constexpr size_t KILOBYTE = 1 << 10;
    static constexpr size_t MEMORY_SIZE = 100 * KILOBYTE;
    std::unique_ptr<char[]> m_memory{new char[MEMORY_SIZE]};
    static constexpr uint32_t NUM_CHUNKS_IN_POOL = 100;
    static constexpr uint64_t CHUNK_SIZE = 128;

    iox::BumpAllocator m_memoryAllocator{m_memory.get(), MEMORY_SIZE};
};

TEST_F(ShmSafeUnmanagedChunk_test, DefaultConstructedResultsInLogicalNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "7311a772-6787-47f6-a8c6-b674e3feb84e");
    ShmSafeUnmanagedChunk sut;

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST_F(ShmSafeUnmanagedChunk_test, ConstructedWithEmptySharedChunkResultsInLogicalNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e2aced3-7dea-4349-a87e-eacb16059182");
    SharedChunk sharedChunk;
    ShmSafeUnmanagedChunk sut(sharedChunk);

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST_F(ShmSafeUnmanagedChunk_test, CallIsLogicalNullptrOnSutConstructedWithSharedChunkResultsNotInLogicalNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "b11245ae-d15b-460f-8688-144f2ef50f72");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);

    EXPECT_FALSE(sut.isLogicalNullptr());

    sut.releaseToSharedChunk();
}

TEST_F(ShmSafeUnmanagedChunk_test, CallIsLogicalNullptrOnSutPreviouslyCalledReleaseToSharedChunkResultsInLogicalNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae81e3d9-a84d-47d0-beec-97845b0c3dba");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);
    sut.releaseToSharedChunk();

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST_F(ShmSafeUnmanagedChunk_test,
       CallIsLogicalNullptrOnSutPreviouslyCalledDuplicateToSharedChunkResultsNotInLogicalNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "178c3a1a-2f74-47d1-99e8-f3b60a32ada0");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);
    sut.cloneToSharedChunk();

    EXPECT_FALSE(sut.isLogicalNullptr());

    sut.releaseToSharedChunk();
}

TEST_F(ShmSafeUnmanagedChunk_test, CallReleaseToSharedChunkOnDefaultConstructedSutResultsInEmptySharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe82457b-c9e2-48d2-86f8-0f4067ed0ff8");
    ShmSafeUnmanagedChunk sut;

    EXPECT_FALSE(sut.releaseToSharedChunk());
}

TEST_F(ShmSafeUnmanagedChunk_test, CallReleaseToSharedChunkOnSutConstructedWithSharedChunkResultsInNotEmptySharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "62b9cff4-1e4f-4504-8b63-261b98f19758");
    ShmSafeUnmanagedChunk sut(getChunkFromMemoryManager());

    EXPECT_EQ(memoryManager.getMemPoolInfo(0).m_usedChunks, 1U);
    EXPECT_TRUE(sut.releaseToSharedChunk());
    EXPECT_EQ(memoryManager.getMemPoolInfo(0).m_usedChunks, 0U);
}

TEST_F(ShmSafeUnmanagedChunk_test,
       CallReleaseToSharedChunkTwiceOnSutConstructedWithSharedChunkResultsInEmptySharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "76cddbd2-7578-431b-9aed-c3ba52856027");
    ShmSafeUnmanagedChunk sut(getChunkFromMemoryManager());
    sut.releaseToSharedChunk();

    EXPECT_FALSE(sut.releaseToSharedChunk());
}

TEST_F(ShmSafeUnmanagedChunk_test, CallDuplicateToSharedChunkOnDefaultConstructedSutResultsEmptySharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "ced30491-3ec2-4818-904a-4bff6eb31b0f");
    ShmSafeUnmanagedChunk sut;

    EXPECT_FALSE(sut.cloneToSharedChunk());

    sut.releaseToSharedChunk();
}

TEST_F(ShmSafeUnmanagedChunk_test,
       CallDuplicateToSharedChunkOnSutConstructedWithSharedChunkResultsInNotEmptySharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "259b124c-1ac0-40cc-b832-0d2bf298e2eb");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);

    auto duplicatedSharedChunk = sut.cloneToSharedChunk();
    EXPECT_TRUE(duplicatedSharedChunk);

    sut.releaseToSharedChunk();
    EXPECT_EQ(memoryManager.getMemPoolInfo(0).m_usedChunks, 1U);
}

TEST_F(ShmSafeUnmanagedChunk_test,
       CallDuplicateToSharedChunkOnSutPreviouslyCalledReleaseToSharedChunkResultsInEmptySharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9469e24-5bb9-4077-856a-a04d3693439d");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);
    sut.releaseToSharedChunk();

    EXPECT_FALSE(sut.cloneToSharedChunk());
}

TEST_F(ShmSafeUnmanagedChunk_test, CallGetChunkHeaderOnNonConstDefaultConstructedSutResultsInNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca9d879c-73e5-466f-a84c-356719b660f5");
    ShmSafeUnmanagedChunk sut;

    EXPECT_EQ(sut.getChunkHeader(), nullptr);
}

TEST_F(ShmSafeUnmanagedChunk_test, CallGetChunkHeaderOnNonConstSutConstructedWithSharedChunkResultsInValidHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "2975af2c-7e1e-486e-bfee-e28018884655");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);

    auto chunkHeader = sut.getChunkHeader();
    EXPECT_NE(chunkHeader, nullptr);
    EXPECT_EQ(chunkHeader, sharedChunk.getChunkHeader());

    sut.releaseToSharedChunk();
}

TEST_F(ShmSafeUnmanagedChunk_test, CallGetChunkHeaderOnConstDefaultConstructedSutResultsInNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "944a4dd4-8961-4962-82d5-7a042e1f7288");
    const ShmSafeUnmanagedChunk sut;

    EXPECT_EQ(sut.getChunkHeader(), nullptr);
}

TEST_F(ShmSafeUnmanagedChunk_test, CallGetChunkHeaderOnConstSutConstructedWithSharedChunkResultsInValidHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "d54a2f30-0b98-430e-9080-b3dc4818cc5e");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);

    [&](const ShmSafeUnmanagedChunk& sut) {
        auto chunkHeader = sut.getChunkHeader();
        EXPECT_NE(chunkHeader, nullptr);
        EXPECT_EQ(chunkHeader, sharedChunk.getChunkHeader());
    }(sut);

    sut.releaseToSharedChunk();
}

TEST_F(ShmSafeUnmanagedChunk_test, CallNonConstGetChunkHeaderResultsInNonConstChunkHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "68288e57-b923-43bd-baa4-2a820b84dafa");
    auto isNonConstReturn =
        std::is_same<decltype(std::declval<ShmSafeUnmanagedChunk>().getChunkHeader()), ChunkHeader*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST_F(ShmSafeUnmanagedChunk_test, CallConstGetChunkHeaderResultsInConstChunkHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "6cca0b6b-c124-4040-939d-645831dc0aa1");
    auto isConstReturn =
        std::is_same<decltype(std::declval<const ShmSafeUnmanagedChunk>().getChunkHeader()), const ChunkHeader*>::value;
    EXPECT_TRUE(isConstReturn);
}

TEST_F(ShmSafeUnmanagedChunk_test, CallIsNotLogicalNullptrAndHasNoOtherOwnersOnDefaultConstructedResultsInFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "21bae265-f6b8-438c-b838-e749de87faa8");
    ShmSafeUnmanagedChunk sut;

    EXPECT_FALSE(sut.isNotLogicalNullptrAndHasNoOtherOwners());
}

TEST_F(ShmSafeUnmanagedChunk_test,
       CallIsNotLogicalNullptrAndHasNoOtherOwnersOnOnSutConstructedWithSharedChunkResultsInTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cae3e82c-82b3-490d-820b-901bc838b9e6");
    ShmSafeUnmanagedChunk sut(getChunkFromMemoryManager());

    EXPECT_TRUE(sut.isNotLogicalNullptrAndHasNoOtherOwners());

    sut.releaseToSharedChunk();
}

TEST_F(ShmSafeUnmanagedChunk_test,
       CallIsNotLogicalNullptrAndHasNoOtherOwnersOnOnSutConstructedWithSharedChunkAndOtherOwnerResultsInFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "59b23c6a-e566-41a1-993c-7ff013e71e8d");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);

    EXPECT_FALSE(sut.isNotLogicalNullptrAndHasNoOtherOwners());

    sut.releaseToSharedChunk();
}

TEST_F(
    ShmSafeUnmanagedChunk_test,
    CallIsNotLogicalNullptrAndHasNoOtherOwnersOnOnSutConstructedWithSharedChunkAndOtherOwnerReleasedOwnershipResultsInTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ced90e5-0c6f-4831-883b-d35f9d8c7af5");
    auto sharedChunk = getChunkFromMemoryManager();

    ShmSafeUnmanagedChunk sut(sharedChunk);

    // release ownership by assigning an empty SharedChunk
    sharedChunk = SharedChunk();

    EXPECT_TRUE(sut.isNotLogicalNullptrAndHasNoOtherOwners());

    sut.releaseToSharedChunk();
}

TEST_F(ShmSafeUnmanagedChunk_test,
       CallIsNotLogicalNullptrAndHasNoOtherOwnersOnOnSutPreviouslyCalledReleaseToSharedChunkResultsInFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "1564e9ca-77a1-4e68-83c0-ec6032c61e6b");
    ShmSafeUnmanagedChunk sut(getChunkFromMemoryManager());
    sut.releaseToSharedChunk();

    EXPECT_FALSE(sut.isNotLogicalNullptrAndHasNoOtherOwners());
}

} // namespace
