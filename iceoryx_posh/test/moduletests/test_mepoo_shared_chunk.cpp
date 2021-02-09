// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

using namespace ::testing;

using namespace iox::mepoo;

class SharedChunk_Test : public Test
{
  public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    ChunkManagement* GetChunkManagement(void* memoryChunk)
    {
        ChunkManagement* v = static_cast<ChunkManagement*>(chunkMgmtPool.getChunk());
        ChunkHeader* chunkHeader = new (memoryChunk) ChunkHeader();
        new (v) ChunkManagement{chunkHeader, &mempool, &chunkMgmtPool};
        return v;
    }

    char memory[4096];
    iox::posix::Allocator allocator{memory, 4096};
    MemPool mempool{64, 10, &allocator, &allocator};
    MemPool chunkMgmtPool{64, 10, &allocator, &allocator};
    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    iox::mepoo::SharedChunk sut{chunkManagement};
};


TEST_F(SharedChunk_Test, PassingNullPointerToSharedChunkConstructorWithChunkManagmentStoresNullPointerInChunkManagement)
{
    ChunkManagement* chunkManagement(nullptr);
    iox::mepoo::SharedChunk sut{chunkManagement};

    EXPECT_THAT(sut.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test,
       PassingNullPointerToSharedChunkConstructorWithRelativePointerStoresNullPointerInChunkManagement)
{
    iox::relative_ptr<ChunkManagement> relativeptr(nullptr);

    iox::mepoo::SharedChunk sut{relativeptr};

    EXPECT_THAT(sut.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, CopyConstructorCopiesTheSharedChunkContentOfOneObjectToOther)
{
    ChunkManagement* chunkManagement(nullptr);
    iox::mepoo::SharedChunk sut1{chunkManagement};

    iox::mepoo::SharedChunk sut2(sut1);

    EXPECT_THAT(sut2.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, TestMoveConstructorForSharedChunk)
{
    ChunkManagement* chunkManagement(nullptr);
    iox::mepoo::SharedChunk sut1{chunkManagement};

    iox::mepoo::SharedChunk sut2(std::move(sut1));

    EXPECT_THAT(sut2.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, VerifiyCopyAssigmentWithSharedChunk)
{
    iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
    sut2 = sut;

    EXPECT_THAT(sut2.getChunkHeader(), Eq(sut.getChunkHeader()));
}

TEST_F(SharedChunk_Test, VerifyCopyAssignmentWithSharedChunkUsingNullPointer)
{
    iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
    sut2 = nullptr;

    EXPECT_THAT(sut2.getChunkHeader(), Eq(nullptr));
}


TEST_F(SharedChunk_Test, VerifyMoveAssignmentUsingNullPointer)
{
    ChunkManagement* chunkManagement(nullptr);
    iox::mepoo::SharedChunk sut1{chunkManagement};

    iox::mepoo::SharedChunk sut2 = std::move(sut1);

    EXPECT_THAT(sut2.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, DestructorDefaultCTor)
{
    {
        iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
        EXPECT_THAT(mempool.getUsedChunks(), Eq(2));
    }
    EXPECT_THAT(mempool.getUsedChunks(), Eq(1));
}

TEST_F(SharedChunk_Test, GetChunkHeaderMethodReturnsNullPointerWhenSharedChunkObjectIsInitialisedWithNullPointer)
{
    SharedChunk sut1(nullptr);
    EXPECT_THAT(sut1.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, GetChunkHeaderMethodReturnsValidPointerWhenSharedChunkObjectIsInitialisedWithAValidPointer)
{
    void* newChunk = mempool.getChunk();
    iox::mepoo::SharedChunk sut1(GetChunkManagement(newChunk));

    EXPECT_THAT(sut1.getChunkHeader(), Eq(newChunk));
}

TEST_F(SharedChunk_Test, ComparingTwoSharedChunkWithTheSameContentReturnsTrue)
{
    iox::mepoo::SharedChunk sut1(nullptr);
    iox::mepoo::SharedChunk sut2(sut1);

    EXPECT_TRUE(sut2 == sut1);
}

TEST_F(SharedChunk_Test, ComparingTwoSharedChunkWithDifferentContentReturnsFalse)
{
    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    iox::mepoo::SharedChunk sut1{chunkManagement};
    iox::mepoo::SharedChunk sut2(nullptr);

    EXPECT_FALSE(sut1 == sut2);
}

TEST_F(SharedChunk_Test, CompareWithSameMemoryChunkComparesPayload)
{
    EXPECT_THAT(sut == sut.getPayload(), Eq(true));
}

TEST_F(SharedChunk_Test, CompareWithAnotherMemoryChunk)
{
    EXPECT_THAT(sut == memoryChunk, Eq(false));
}

TEST_F(SharedChunk_Test, CompareValidWithNullptr)
{
    EXPECT_THAT(sut == nullptr, Eq(false));
}

TEST_F(SharedChunk_Test, CompareInvalidWithNullptr)
{
    sut = nullptr;
    EXPECT_THAT(sut == nullptr, Eq(true));
}

TEST_F(SharedChunk_Test, boolOperatorIsSet)
{
    EXPECT_THAT(sut, Eq(true));
}

TEST_F(SharedChunk_Test, boolOperatorIsNotSet)
{
    iox::mepoo::SharedChunk sut2(nullptr);
    EXPECT_THAT(sut2, Eq(false));
}

TEST_F(SharedChunk_Test, HasNoOtherOwnersMethodWithChunkManagementEqualNullPointerReturnTrue)
{
    iox::mepoo::SharedChunk sut1(nullptr);
    EXPECT_TRUE(sut1.hasNoOtherOwners());
}

TEST_F(SharedChunk_Test, HasNoOtherOwnersMethodForSingleOwnerReturnsTrueWhen_m_chunkmanagementisValid)
{
    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    iox::mepoo::SharedChunk sut1{chunkManagement};

    EXPECT_TRUE(sut1.hasNoOtherOwners());
}

TEST_F(SharedChunk_Test, HasNoOtherOwnersMethodForMultipleOwnerReturnsFalseWhen_m_chunkmanagementisValid)
{
    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    iox::mepoo::SharedChunk sut1{chunkManagement};

    iox::mepoo::SharedChunk sut2(sut1);
    EXPECT_FALSE(sut1.hasNoOtherOwners());
}

TEST_F(SharedChunk_Test, GetPayloadMethodReturnsNullPointerWhen_m_chunkmanagmentIsInvalid)
{
    SharedChunk sut1(nullptr);
    EXPECT_THAT(sut1.getPayload(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, GetPayloadMethodReturnsValidPointerWhen_m_chunkmanagmentIsValid)
{
    ChunkHeader* newChunk = static_cast<ChunkHeader*>(mempool.getChunk());
    new (newChunk) ChunkHeader();
    new (static_cast<int*>(newChunk->payload())) int{1337};

    iox::mepoo::SharedChunk sut1(GetChunkManagement(newChunk));
    EXPECT_THAT(*static_cast<int*>(sut1.getPayload()), Eq(1337));
}

TEST_F(SharedChunk_Test, MultipleSharedChunksCleanup)
{
    {
        SharedChunk sut3{nullptr}, sut4{nullptr}, sut5{nullptr};
        {
            {
                SharedChunk sut6{nullptr}, sut7{nullptr}, sut8{nullptr};
                {
                    iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));

                    sut3 = sut2;
                    sut4 = sut2;
                    sut5 = sut3;
                    sut6 = sut5;
                    sut7 = sut4;
                    sut8 = sut2;

                    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2));
                    EXPECT_THAT(mempool.getUsedChunks(), Eq(2));
                }
                EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2));
                EXPECT_THAT(mempool.getUsedChunks(), Eq(2));
            }
            EXPECT_THAT(mempool.getUsedChunks(), Eq(2));
            EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2));
        }
        EXPECT_THAT(mempool.getUsedChunks(), Eq(2));
        EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2));
    }
    EXPECT_THAT(mempool.getUsedChunks(), Eq(1));
    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(1));
}


TEST_F(SharedChunk_Test, MultipleChunksCleanup)
{
    {
        iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
        {
            iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
            {
                iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
                iox::mepoo::SharedChunk sut4(GetChunkManagement(mempool.getChunk()));
                {
                    iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
                    iox::mepoo::SharedChunk sut4(GetChunkManagement(mempool.getChunk()));
                    {
                        iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));
                        iox::mepoo::SharedChunk sut4(GetChunkManagement(mempool.getChunk()));
                        EXPECT_THAT(mempool.getUsedChunks(), Eq(9));
                        EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(9));
                    }
                    EXPECT_THAT(mempool.getUsedChunks(), Eq(7));
                    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(7));
                }
                EXPECT_THAT(mempool.getUsedChunks(), Eq(5));
                EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(5));
            }
            EXPECT_THAT(mempool.getUsedChunks(), Eq(3));
            EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(3));
        }
        EXPECT_THAT(mempool.getUsedChunks(), Eq(2));
        EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2));
    }
    EXPECT_THAT(mempool.getUsedChunks(), Eq(1));
    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(1));
}

TEST_F(SharedChunk_Test, NotEqualOperatorReturnsTrueWhenDefaultConstructedChunkIsComparedToValidChunk)
{
    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    iox::mepoo::SharedChunk sut1{chunkManagement};
    iox::mepoo::SharedChunk sut2;

    EXPECT_TRUE(sut1 != sut2);
}

TEST_F(SharedChunk_Test, NotEqualOperatorReturnsFalseWhenInvalidChunkIsComparedToNullPointer)
{
    iox::mepoo::SharedChunk sut1(nullptr);

    EXPECT_FALSE(sut1 != nullptr);
}
