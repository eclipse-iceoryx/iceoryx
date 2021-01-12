// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox::mepoo;

template <uint32_t N>
struct alignas(32) ChunkWithPayload
{
    ChunkHeader m_chunkHeader;
    uint8_t m_payload[N];
};

template <uint32_t N>
struct alignas(32) ChunkWithCustomHeaderAndPayload
{
    ChunkHeader m_chunkHeader;
    uint64_t m_customHeader;
    uint8_t m_payload[N];
};

class ChunkHeader_test : public Test
{
    void SetUp(){};
    void TearDown(){};
};

TEST_F(ChunkHeader_test, ChunkHeaderHasInitializedMembers)
{
    ChunkHeader sut;

    EXPECT_THAT(sut.chunkSize, Eq(0U));

    // deliberately used a magic number to make the test fail when CHUNK_HEADER_VERSION changes
    EXPECT_THAT(sut.chunkHeaderVersion, Eq(1U));

    EXPECT_THAT(sut.reserved1, Eq(0U));
    EXPECT_THAT(sut.reserved2, Eq(0U));
    EXPECT_THAT(sut.reserved3, Eq(0U));

    EXPECT_THAT(sut.originId, Eq(iox::UniquePortId(iox::popo::InvalidId)));

    EXPECT_THAT(sut.sequenceNumber, Eq(0U));

    EXPECT_THAT(sut.payloadSize, Eq(0U));
    // a default created ChunkHeader has always an adjacent payload
    EXPECT_THAT(sut.payloadOffset, Eq(sizeof(ChunkHeader)));
}

TEST_F(ChunkHeader_test, ChunkHeaderPayloadSizeIsLargeEnoughForMempoolChunk)
{
    using ChunkSize_t = std::result_of<decltype (&MemPool::getChunkSize)(MemPool)>::type;

    auto maxChunkSize = std::numeric_limits<ChunkSize_t>::max();
    auto maxPayloadSize = std::numeric_limits<decltype(ChunkHeader::payloadSize)>::max();

    // the payload will never be larger than the chunk
    // if the payload can hold at least the maximum chunk size there will never be an overflow
    EXPECT_THAT(maxPayloadSize, Ge(maxChunkSize));
}

TEST_F(ChunkHeader_test, CustomHeaderMethodReturnsCorrectCustomHeaderPointer)
{
    constexpr int32_t PAYLOAD_SIZE{128};
    ChunkWithCustomHeaderAndPayload<PAYLOAD_SIZE> chunk;

    EXPECT_THAT(chunk.m_chunkHeader.customHeader<uint64_t>(), Eq(&chunk.m_customHeader));
}

TEST_F(ChunkHeader_test, PayloadMethodReturnsCorrectPayloadPointer)
{
    constexpr int32_t PAYLOAD_SIZE{128};
    ChunkWithPayload<PAYLOAD_SIZE> chunk;

    EXPECT_THAT(chunk.m_chunkHeader.payload(), Eq(chunk.m_payload));
}

TEST_F(ChunkHeader_test, FromPayloadFunctionReturnsCorrectChunkHeaderPointer)
{
    constexpr int32_t PAYLOAD_SIZE{128};
    ChunkWithPayload<PAYLOAD_SIZE> chunk;

    EXPECT_THAT(ChunkHeader::fromPayload(chunk.m_payload), Eq(&chunk.m_chunkHeader));
}

TEST_F(ChunkHeader_test, FromPayloadFunctionCalledWithNullptrReturnsNullptr)
{
    EXPECT_THAT(ChunkHeader::fromPayload(nullptr), Eq(nullptr));
}

TEST_F(ChunkHeader_test, usedChunkSizeIsSizeOfChunkHeaderWhenPayloadIsZero)
{
    ChunkHeader sut;
    sut.chunkSize = 2 * sizeof(ChunkHeader);
    EXPECT_THAT(sut.usedSizeOfChunk(), Eq(sizeof(ChunkHeader)));
}

TEST_F(ChunkHeader_test, usedChunkSizeIsSizeOfChunkHeaderPlusOneWhenPayloadIsOne)
{
    ChunkHeader sut;
    sut.chunkSize = 2 * sizeof(ChunkHeader);
    constexpr uint32_t PAYLOAD_SIZE{1U};
    sut.payloadSize = PAYLOAD_SIZE;
    EXPECT_THAT(sut.usedSizeOfChunk(), Eq(sizeof(ChunkHeader) + PAYLOAD_SIZE));
}

TEST_F(ChunkHeader_test, usedChunkSizeTerminatesWhenPayloadSizeExceedsChunkSize)
{
    ChunkHeader sut;
    sut.chunkSize = 2 * sizeof(ChunkHeader);
    constexpr uint32_t PAYLOAD_SIZE{std::numeric_limits<uint32_t>::max()};
    sut.payloadSize = PAYLOAD_SIZE;

    EXPECT_DEATH(sut.usedSizeOfChunk(), ".*");
}
