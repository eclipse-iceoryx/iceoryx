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

    static constexpr uint32_t CHUNK_SIZE{64U};
    static constexpr uint32_t NUMBER_OF_CHUNKS{10U};
    char memory[4096];
    iox::posix::Allocator allocator{memory, 4096U};

    MemPool mempool{CHUNK_SIZE, NUMBER_OF_CHUNKS, &allocator, &allocator};
    MemPool chunkMgmtPool{CHUNK_SIZE, NUMBER_OF_CHUNKS, &allocator, &allocator};
    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    SharedChunk sut{chunkManagement};
};

TEST_F(SharedChunk_Test, PassingNullPointerToSharedChunkConstructorWithChunkManagmentStoresNullPointerInChunkManagement)
{
    ChunkManagement* chunkManagement(nullptr);
    SharedChunk sut{chunkManagement};

    EXPECT_THAT(sut.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test,
       PassingNullPointerToSharedChunkConstructorWithRelativePointerStoresNullPointerInChunkManagement)
{
    iox::relative_ptr<ChunkManagement> relativeptr(nullptr);

    SharedChunk sut{relativeptr};

    EXPECT_THAT(sut.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, VerifyCopyConstructorForSharedChunkWithChunkManagementAsNullPointer)
{
    ChunkManagement* chunkManagement(nullptr);
    SharedChunk sut1{chunkManagement};

    SharedChunk sut2(sut1);

    EXPECT_THAT(sut2.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, VerifyCopyConstructorOfSharedChunk)
{
    SharedChunk sut1{chunkManagement};

    SharedChunk sut2(sut1);

    EXPECT_THAT(sut2.releaseWithRelativePtr()->m_mempool->getChunkSize(),
                Eq(sut1.releaseWithRelativePtr()->m_mempool->getChunkSize()));
}

TEST_F(SharedChunk_Test, VerifyMoveConstructorOfSharedChunk)
{
    SharedChunk sut1{chunkManagement};

    SharedChunk sut2(std::move(sut1));

    EXPECT_THAT(sut2.releaseWithRelativePtr()->m_mempool->getChunkSize(), Eq(CHUNK_SIZE));
}

TEST_F(SharedChunk_Test, VerifyMoveConstructorForSharedChunkWithChunkManagementAsNullPointer)
{
    ChunkManagement* chunkManagement(nullptr);
    SharedChunk sut1{chunkManagement};

    SharedChunk sut2(std::move(sut1));

    EXPECT_THAT(sut2.releaseWithRelativePtr(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, VerifiyCopyAssigmentWithSharedChunk)
{
    SharedChunk sut1(chunkManagement);
    SharedChunk sut2(nullptr);

    sut2 = sut1;

    EXPECT_THAT(sut2.releaseWithRelativePtr()->m_mempool->getChunkSize(),
                Eq(sut1.releaseWithRelativePtr()->m_mempool->getChunkSize()));
}

TEST_F(SharedChunk_Test, VerifyCopyAssignmentForSharedChunkWithChunkManagementAsNullPointer)
{
    sut = nullptr;

    EXPECT_THAT(sut.releaseWithRelativePtr(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, VerifiyMoveAssigmentForSharedChunk)
{
    SharedChunk sut1(chunkManagement);
    SharedChunk sut2(chunkManagement);

    sut2 = std::move(sut1);

    EXPECT_THAT(sut2.releaseWithRelativePtr()->m_mempool->getChunkSize(), Eq(CHUNK_SIZE));
}

TEST_F(SharedChunk_Test, VerifyMoveAssignmenForSharedChunkWithChunkManagementAsNullPointer)
{
    ChunkManagement* chunkManagement(nullptr);
    SharedChunk sut1{chunkManagement};

    SharedChunk sut2 = std::move(sut1);

    EXPECT_THAT(sut2.releaseWithRelativePtr(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, GetChunkHeaderMethodReturnsNullPointerWhenSharedChunkObjectIsInitialisedWithNullPointer)
{
    SharedChunk sut(nullptr);

    EXPECT_THAT(sut.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, GetChunkHeaderMethodReturnsValidPointerWhenSharedChunkObjectIsInitialisedWithAValidPointer)
{
    void* newChunk = mempool.getChunk();
    SharedChunk sut(GetChunkManagement(newChunk));

    EXPECT_THAT(sut.getChunkHeader(), Eq(newChunk));
}

TEST_F(SharedChunk_Test, EqualityOperatorOnTwoSharedChunkWithTheSameContentReturnsTrue)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2{chunkManagement};

    EXPECT_TRUE(sut2 == sut1);
}

TEST_F(SharedChunk_Test, EqualityOperatorOnTwoSharedChunkWithDifferentContentReturnsFalse)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2(nullptr);

    EXPECT_FALSE(sut1 == sut2);
}

TEST_F(SharedChunk_Test, EqualityOperatorOnSharedChunkAndSharedChunkPayloadWithDifferentChunkManagementsReturnFalse)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2{nullptr};

    EXPECT_FALSE(sut1 == sut2.getPayload());
}

TEST_F(SharedChunk_Test, EqualityOperatorOnSharedChunkAndSharedChunkPayloadWithSameChunkManagementsReturnTrue)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2{chunkManagement};

    EXPECT_TRUE(sut1 == sut2.getPayload());
}

TEST_F(SharedChunk_Test, BoolOperatorOnValidSharedChunkReturnsTrue)
{
    EXPECT_TRUE(sut);
}

TEST_F(SharedChunk_Test, BoolOperatorOnSharedChunkWithChunkManagementAsNullPointerReturnsFalse)
{
    SharedChunk sut(nullptr);

    EXPECT_FALSE(sut);
}

TEST_F(SharedChunk_Test, HasNoOtherOwnersMethodWithChunkManagementEqualNullPointerReturnTrue)
{
    SharedChunk sut(nullptr);

    EXPECT_TRUE(sut.hasNoOtherOwners());
}

TEST_F(SharedChunk_Test, HasNoOtherOwnersMethodForSingleOwnerWhen_m_chunkmanagementisValidReturnsTrue)
{
    EXPECT_TRUE(sut.hasNoOtherOwners());
}

TEST_F(SharedChunk_Test, HasNoOtherOwnersMethodForMultipleOwnerWhen_m_chunkmanagementisValidReturnsFalse)
{
    SharedChunk sut1{chunkManagement};

    SharedChunk sut2(sut1);

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

    SharedChunk sut1(GetChunkManagement(newChunk));

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

TEST_F(SharedChunk_Test, NonEqualityOperatorOnTwoSharedChunkWithDifferentContentReturnsTrue)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2;

    EXPECT_TRUE(sut1 != sut2);
}

TEST_F(SharedChunk_Test, NonEqualityOperatorOnTwoSharedChunkWithSameContentReturnsFalse)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2(chunkManagement);

    EXPECT_FALSE(sut1 != sut2);
}

TEST_F(SharedChunk_Test, NonEqualityOperatorOnSharedChunkAndSharedChunkPayloadWithDifferentChunkManagementsReturnTrue)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2(nullptr);

    EXPECT_TRUE(sut1 != sut2.getPayload());
}

TEST_F(SharedChunk_Test, NonEqualityOperatorOnSharedChunkAndSharedChunkPayloadWithSameChunkManagementsReturnFalse)
{
    SharedChunk sut1{chunkManagement};
    SharedChunk sut2(chunkManagement);

    EXPECT_FALSE(sut1 != sut2.getPayload());
}

TEST_F(SharedChunk_Test,
       ReleaseMethodReturnsChunkManagementPointerOfSharedChunkObjectAndSetsTheChunkManagementRelativePointerToNull)
{
    ChunkManagement* returnValue = sut.release();

    EXPECT_THAT(returnValue->m_mempool->getChunkSize(), CHUNK_SIZE);
    EXPECT_THAT(returnValue->m_mempool->getChunkCount(), NUMBER_OF_CHUNKS);
    EXPECT_FALSE(sut);
}

TEST_F(SharedChunk_Test,
       ReleaseMethodReturnsRelativeChunkManagementPointerOfSharedChunkObjectSetsTheChunkManagementRelativePointerToNull)
{
    auto returnValue = sut.releaseWithRelativePtr();

    EXPECT_THAT(returnValue->m_mempool->getChunkSize(), CHUNK_SIZE);
    EXPECT_THAT(returnValue->m_mempool->getChunkCount(), NUMBER_OF_CHUNKS);
    EXPECT_FALSE(sut);
}
