// Copyright (c) 2019, 2021 by  Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/bump_allocator.hpp"
#include "test.hpp"

namespace
{
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
        auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
        EXPECT_FALSE(chunkSettingsResult.has_error());
        if (chunkSettingsResult.has_error())
        {
            return nullptr;
        }
        auto& chunkSettings = chunkSettingsResult.value();
        ChunkHeader* chunkHeader = new (memoryChunk) ChunkHeader(mempool.getChunkSize(), chunkSettings);

        new (v) ChunkManagement{chunkHeader, &mempool, &chunkMgmtPool};
        return v;
    }

    static constexpr uint32_t CHUNK_SIZE{64U};
    static constexpr uint32_t NUMBER_OF_CHUNKS{10U};
    static constexpr uint32_t USER_PAYLOAD_SIZE{64U};

    char memory[4096U];
    iox::BumpAllocator allocator{memory, 4096U};
    MemPool mempool{sizeof(ChunkHeader) + USER_PAYLOAD_SIZE, NUMBER_OF_CHUNKS, allocator, allocator};
    MemPool chunkMgmtPool{CHUNK_SIZE, NUMBER_OF_CHUNKS, allocator, allocator};
    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    SharedChunk sut{chunkManagement};
};

TEST_F(SharedChunk_Test, SharedChunkObjectUpOnInitilizationSetsTheChunkHeaderToNullPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "37eabb48-502c-4c06-a0f8-237796702107");
    SharedChunk sut;

    EXPECT_THAT(sut.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, VerifyCopyConstructorOfSharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "bee0c277-8542-4a72-be1e-7940ba5390e7");
    SharedChunk sut1(sut);

    EXPECT_EQ(sut.release(), sut1.release());
}

TEST_F(SharedChunk_Test, VerifyMoveConstructorOfSharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "c13e0a4c-556b-4ab1-a750-17b3b0f7a01d");
    SharedChunk sut1(chunkManagement);

    SharedChunk sut2(std::move(sut1));

    EXPECT_FALSE(sut1);
    EXPECT_THAT(sut2.release(), Eq(chunkManagement));
}

TEST_F(SharedChunk_Test, VerifiyCopyAssigmentWithSharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b18be03-beeb-449d-8b04-01a40c648d95");
    SharedChunk sut1;

    sut1 = sut;

    EXPECT_EQ(sut.release(), sut1.release());
}

TEST_F(SharedChunk_Test, VerifiyMoveAssigmentForSharedChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "99e37ef4-493d-43d3-a14d-ed12299ce9ae");
    SharedChunk sut1(chunkManagement);
    SharedChunk sut2;

    sut2 = std::move(sut1);

    EXPECT_FALSE(sut1);
    EXPECT_THAT(sut2.release(), Eq(chunkManagement));
}

TEST_F(SharedChunk_Test, CompareWithSameMemoryChunkComparesToUserPayload)
{
    ::testing::Test::RecordProperty("TEST_ID", "010e9453-2140-4342-afa9-bbab8a835aff");
    EXPECT_THAT(sut == sut.getUserPayload(), Eq(true));
}

TEST_F(SharedChunk_Test, GetChunkHeaderMethodReturnsNullPointerWhenSharedChunkObjectIsInitialisedWithNullPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "211d82c7-4b8c-49c6-a9dd-b8e79c481d90");
    SharedChunk sut;

    EXPECT_THAT(sut.getChunkHeader(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, GetChunkHeaderMethodReturnsValidPointerWhenSharedChunkObjectIsInitialisedWithAValidPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c3a3769-5825-4bb0-9daa-0bce0152a26b");
    void* newChunk = mempool.getChunk();
    SharedChunk sut(GetChunkManagement(newChunk));

    EXPECT_THAT(sut.getChunkHeader(), Eq(newChunk));
}

TEST_F(SharedChunk_Test, EqualityOperatorOnTwoSharedChunkWithTheSameContentReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "65a7ce87-5f64-48af-830d-71ab03db39e9");
    SharedChunk sut1{chunkManagement};

    EXPECT_TRUE(sut == sut1);
}

TEST_F(SharedChunk_Test, EqualityOperatorOnTwoSharedChunkWithDifferentContentReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "84ebd9aa-83f1-4978-bd96-e4d8c03452bf");
    SharedChunk sut1;

    EXPECT_FALSE(sut == sut1);
}

TEST_F(SharedChunk_Test, EqualityOperatorOnSharedChunkAndSharedChunkPayloadWithDifferentChunkManagementsReturnFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e473823-5e95-4454-9740-e731c7aa46a7");
    SharedChunk sut1;

    EXPECT_FALSE(sut1 == sut.getUserPayload());
}

TEST_F(SharedChunk_Test, EqualityOperatorOnSharedChunkAndSharedChunkPayloadWithSameChunkManagementsReturnTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2d4ced9-ec74-47c5-abc2-ec9952168622");
    SharedChunk sut1{chunkManagement};

    EXPECT_TRUE(sut == sut1.getUserPayload());
}

TEST_F(SharedChunk_Test, BoolOperatorOnValidSharedChunkReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3627515b-3f96-407f-a89e-a8f73aa8c70c");
    EXPECT_TRUE(sut);
}

TEST_F(SharedChunk_Test, BoolOperatorOnSharedChunkWithChunkManagementAsNullPointerReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "53b2fd08-0188-439b-adc1-0602c97b8946");
    SharedChunk sut;

    EXPECT_FALSE(sut);
}


TEST_F(SharedChunk_Test, GetUserPayloadMethodReturnsNullPointerWhen_m_chunkmanagmentIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9eca2ed-d4e5-4739-8537-bd5ce273a939");
    SharedChunk sut1;

    EXPECT_THAT(sut1.getUserPayload(), Eq(nullptr));
}

TEST_F(SharedChunk_Test, GetUserPayloadMethodReturnsValidPointerWhen_m_chunkmanagmentIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "89ec859c-75cc-439c-84f5-0021696fee5e");
    using DATA_TYPE = uint32_t;
    constexpr DATA_TYPE USER_DATA{7337U};
    ChunkHeader* newChunk = static_cast<ChunkHeader*>(mempool.getChunk());

    auto chunkSettingsResult = ChunkSettings::create(sizeof(DATA_TYPE), alignof(DATA_TYPE));
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    new (newChunk) ChunkHeader(mempool.getChunkSize(), chunkSettings);
    new (static_cast<DATA_TYPE*>(newChunk->userPayload())) DATA_TYPE{USER_DATA};

    iox::mepoo::SharedChunk sut1(GetChunkManagement(newChunk));
    EXPECT_THAT(*static_cast<DATA_TYPE*>(sut1.getUserPayload()), Eq(USER_DATA));
}

TEST_F(SharedChunk_Test, MultipleSharedChunksCleanup)
{
    ::testing::Test::RecordProperty("TEST_ID", "a8675bfb-cfa6-4cab-9ffe-2e8cfb4a2519");
    {
        SharedChunk sut3, sut4, sut5;
        {
            {
                SharedChunk sut6, sut7, sut8;
                {
                    iox::mepoo::SharedChunk sut2(GetChunkManagement(mempool.getChunk()));

                    sut3 = sut2;
                    sut4 = sut2;
                    sut5 = sut3;
                    sut6 = sut5;
                    sut7 = sut4;
                    sut8 = sut2;

                    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2U));
                    EXPECT_THAT(mempool.getUsedChunks(), Eq(2U));
                }
                EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2U));
                EXPECT_THAT(mempool.getUsedChunks(), Eq(2U));
            }
            EXPECT_THAT(mempool.getUsedChunks(), Eq(2U));
            EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2U));
        }
        EXPECT_THAT(mempool.getUsedChunks(), Eq(2U));
        EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2U));
    }
    EXPECT_THAT(mempool.getUsedChunks(), Eq(1U));
    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(1U));
}


TEST_F(SharedChunk_Test, MultipleChunksCleanup)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d6f4575-7d86-4314-851f-537c7b3e75e5");
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
                        EXPECT_THAT(mempool.getUsedChunks(), Eq(9U));
                        EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(9U));
                    }
                    EXPECT_THAT(mempool.getUsedChunks(), Eq(7U));
                    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(7U));
                }
                EXPECT_THAT(mempool.getUsedChunks(), Eq(5U));
                EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(5U));
            }
            EXPECT_THAT(mempool.getUsedChunks(), Eq(3U));
            EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(3U));
        }
        EXPECT_THAT(mempool.getUsedChunks(), Eq(2U));
        EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(2U));
    }
    EXPECT_THAT(mempool.getUsedChunks(), Eq(1U));
    EXPECT_THAT(chunkMgmtPool.getUsedChunks(), Eq(1U));
}

TEST_F(SharedChunk_Test, NonEqualityOperatorOnTwoSharedChunkWithDifferentContentReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "50b24296-10ca-44e4-8150-bdf6202c589d");
    SharedChunk sut1;

    EXPECT_TRUE(sut1 != sut);
}

TEST_F(SharedChunk_Test, NonEqualityOperatorOnTwoSharedChunkWithSameContentReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "152831f0-19c9-473e-8599-9fd0828dd173");
    SharedChunk sut1{chunkManagement};

    EXPECT_FALSE(sut1 != sut);
}

TEST_F(SharedChunk_Test, NonEqualityOperatorOnSharedChunkAndSharedChunkPayloadWithDifferentChunkManagementsReturnTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "7e9666a7-13e8-46c4-ad06-59e2b3d62cca");
    SharedChunk sut1;

    EXPECT_TRUE(sut != sut1.getUserPayload());
}

TEST_F(SharedChunk_Test, NonEqualityOperatorOnSharedChunkAndSharedChunkPayloadWithSameChunkManagementsReturnFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "b07f53b5-51c8-471f-9c63-05f432ec79e6");
    SharedChunk sut1{chunkManagement};

    EXPECT_FALSE(sut != sut1.getUserPayload());
}

TEST_F(SharedChunk_Test, ReleaseMethodReturnsChunkManagementPointerOfSharedChunkObjectAndSetsTheChunkHeaderToNull)
{
    ::testing::Test::RecordProperty("TEST_ID", "16d50171-28d2-4caa-b1b3-cdb56e366e26");
    ChunkManagement* returnValue = sut.release();

    EXPECT_EQ(returnValue, chunkManagement);
    EXPECT_EQ(sut.getChunkHeader(), nullptr);
}

} // namespace
