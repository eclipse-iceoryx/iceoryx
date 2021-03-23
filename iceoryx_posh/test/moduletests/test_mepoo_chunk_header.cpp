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

#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::mepoo;

using PayloadOffset_t = ChunkHeader::PayloadOffset_t;

TEST(ChunkHeader_test, ChunkHeaderHasInitializedMembers)
{
    constexpr uint32_t CHUNK_SIZE{753U};
    constexpr uint32_t PAYLOAD_SIZE{8U};

    auto chunkSettingsResult = ChunkSettings::create(PAYLOAD_SIZE, iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    EXPECT_THAT(sut.chunkSize, Eq(CHUNK_SIZE));

    // deliberately used a magic number to make the test fail when CHUNK_HEADER_VERSION changes
    EXPECT_THAT(sut.chunkHeaderVersion, Eq(1U));

    EXPECT_THAT(sut.reserved1, Eq(0U));
    EXPECT_THAT(sut.reserved2, Eq(0U));
    EXPECT_THAT(sut.reserved3, Eq(0U));

    EXPECT_THAT(sut.originId, Eq(iox::UniquePortId(iox::popo::InvalidId)));

    EXPECT_THAT(sut.sequenceNumber, Eq(0U));

    EXPECT_THAT(sut.payloadSize, Eq(PAYLOAD_SIZE));
    // a default created ChunkHeader has always an adjacent payload
    EXPECT_THAT(sut.payloadOffset, Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, ChunkHeaderPayloadSizeTypeIsLargeEnoughForMempoolChunk)
{
    using ChunkSize_t = std::result_of<decltype (&MemPool::getChunkSize)(MemPool)>::type;

    auto maxOfChunkSizeType = std::numeric_limits<ChunkSize_t>::max();
    auto maxOfPayloadSizeType = std::numeric_limits<decltype(ChunkHeader::payloadSize)>::max();

    // the payload will never be larger than the chunk
    // if the payload can hold at least the maximum chunk size there will never be an overflow
    EXPECT_THAT(maxOfPayloadSizeType, Ge(maxOfChunkSizeType));
}

TEST(ChunkHeader_test, FromPayloadFunctionCalledWithNullptrReturnsNullptr)
{
    EXPECT_THAT(ChunkHeader::fromPayload(nullptr), Eq(nullptr));
}

TEST(ChunkHeader_test, usedChunkSizeIsSizeOfChunkHeaderWhenPayloadIsZero)
{
    constexpr uint32_t CHUNK_SIZE{32U};
    constexpr uint32_t PAYLOAD_SIZE{0U};

    auto chunkSettingsResult = ChunkSettings::create(PAYLOAD_SIZE, iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    sut.chunkSize = 2 * sizeof(ChunkHeader);
    EXPECT_THAT(sut.usedSizeOfChunk(), Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, usedChunkSizeIsSizeOfChunkHeaderPlusOneWhenPayloadIsOne)
{
    constexpr uint32_t CHUNK_SIZE{128U};
    constexpr uint32_t PAYLOAD_SIZE{1U};

    auto chunkSettingsResult = ChunkSettings::create(PAYLOAD_SIZE, iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    sut.chunkSize = 2 * sizeof(ChunkHeader);
    EXPECT_THAT(sut.usedSizeOfChunk(), Eq(sizeof(ChunkHeader) + PAYLOAD_SIZE));
}

TEST(ChunkHeader_test, ConstructorTerminatesWhenPayloadSizeExceedsChunkSize)
{
    constexpr uint32_t CHUNK_SIZE{128U};
    constexpr uint32_t PAYLOAD_SIZE{2U * CHUNK_SIZE};

    auto chunkSettingsResult = ChunkSettings::create(PAYLOAD_SIZE, iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    EXPECT_DEATH({ ChunkHeader sut(CHUNK_SIZE, chunkSettings); }, ".*");
}

// BEGIN PARAMETERIZED TESTS FOR CHUNK HEADER

struct PayloadParams
{
    uint32_t size{0U};
    uint32_t alignment{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
};


void createChunksOnMultipleAddresses(const PayloadParams& payloadParams,
                                     const uint32_t customHeaderSize,
                                     const uint32_t customHeaderAlignment,
                                     const std::function<void(ChunkHeader& chunkHeader)> testHook)
{
    ASSERT_TRUE(testHook);

    constexpr size_t MAX_PAYLOAD_ALIGNMENT_FOR_TEST{512U};
    ASSERT_THAT(MAX_PAYLOAD_ALIGNMENT_FOR_TEST, Gt(alignof(ChunkHeader)));

    constexpr size_t STORAGE_ALIGNMENT{2 * MAX_PAYLOAD_ALIGNMENT_FOR_TEST};
    alignas(STORAGE_ALIGNMENT) static uint8_t storage[1024 * 1024];
    ASSERT_THAT(reinterpret_cast<uint64_t>(storage) % STORAGE_ALIGNMENT, Eq(0U));

    // storage alignment boundaries               -> ⊥               ⊥               ⊥               ⊥               ⊥
    // max payload alignment for test boundaries  -> ⊥       ⊥       ⊥       ⊥       ⊥       ⊥       ⊥       ⊥       ⊥
    // ChunkHeader alignment boundaries           -> ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥

    // the test creates ChunkHeader on multiple address boundaries in order to have all possible scenarios up to a
    // payload boundary of 512; this boundary is more than large enough since a payload alignment of 2 times the
    // ChunkHeader alignment would already be sufficient to test all corner cases
    for (auto alignedChunkAddress = alignof(ChunkHeader); alignedChunkAddress <= MAX_PAYLOAD_ALIGNMENT_FOR_TEST;
         alignedChunkAddress += alignof(ChunkHeader))
    {
        SCOPED_TRACE(std::string("Chunk placed on address ") + std::to_string(alignedChunkAddress));

        auto chunkSettingsResult =
            ChunkSettings::create(payloadParams.size, payloadParams.alignment, customHeaderSize, customHeaderAlignment);
        ASSERT_FALSE(chunkSettingsResult.has_error());
        auto& chunkSettings = chunkSettingsResult.value();
        auto chunkSize = chunkSettings.requiredChunkSize();

        auto chunkHeader = new (&storage[alignedChunkAddress]) ChunkHeader(chunkSize, chunkSettings);

        testHook(*chunkHeader);

        chunkHeader->~ChunkHeader();
    }
}

void checkPayloadNotOverlappingWithChunkHeader(const ChunkHeader& sut)
{
    SCOPED_TRACE(std::string("Check payload not overlapping with ChunkHeader"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t payloadStartAddress{reinterpret_cast<uint64_t>(sut.payload())};

    EXPECT_THAT(payloadStartAddress - chunkStartAddress, Ge(sizeof(ChunkHeader)));
}

void checkPayloadNotOverlappingWithCustomHeader(const ChunkHeader& sut, const uint32_t customHeaderSize)
{
    SCOPED_TRACE(std::string("Check payload not overlapping with custom header"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t payloadStartAddress{reinterpret_cast<uint64_t>(sut.payload())};
    const uint64_t customHeaderSizeAndPadding{
        iox::algorithm::max(customHeaderSize, static_cast<uint32_t>(alignof(PayloadOffset_t)))};
    constexpr uint64_t BACK_OFFSET_SIZE{sizeof(PayloadOffset_t)};
    const uint64_t expectedRequiredSpace{sizeof(ChunkHeader) + customHeaderSizeAndPadding + BACK_OFFSET_SIZE};

    EXPECT_THAT(payloadStartAddress - chunkStartAddress, Ge(expectedRequiredSpace));
}

void checkCustomHeaderIsAdjacentToChunkHeader(const ChunkHeader& sut)
{
    SCOPED_TRACE(std::string("Check custom header is adjacent to ChunkHeader"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t customHeaderStartAddress{reinterpret_cast<uint64_t>(sut.customHeader<void>())};

    EXPECT_EQ(customHeaderStartAddress - chunkStartAddress, sizeof(ChunkHeader));
}

void checkPayloadSize(const ChunkHeader& sut, const PayloadParams& payloadParams)
{
    SCOPED_TRACE(std::string("Check payload size"));
    EXPECT_EQ(sut.payloadSize, payloadParams.size);
}

void checkPayloadAlignment(const ChunkHeader& sut, const PayloadParams& payloadParams)
{
    SCOPED_TRACE(std::string("Check payload alignment"));
    // a payload alignment of zero will internally be set to one
    auto adjustedAlignment = payloadParams.alignment == 0U ? 1U : payloadParams.alignment;
    EXPECT_EQ(reinterpret_cast<uint64_t>(sut.payload()) % adjustedAlignment, 0U);
}

void checkUsedSizeOfChunk(const ChunkHeader& sut, const PayloadParams& payloadParams)
{
    SCOPED_TRACE(std::string("Check used size of chunk"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t payloadStartAddress{reinterpret_cast<uint64_t>(sut.payload())};
    const uint64_t expectedUsedSizeOfChunk{payloadStartAddress + payloadParams.size - chunkStartAddress};

    EXPECT_EQ(sut.usedSizeOfChunk(), expectedUsedSizeOfChunk);
    EXPECT_THAT(sut.usedSizeOfChunk(), Le(sut.chunkSize));
}

void checkConversionOfPayloadPointerToChunkHeader(const ChunkHeader& sut)
{
    SCOPED_TRACE(std::string("Check conversion of payload pointer to ChunkHeader pointer"));
    const auto payload = sut.payload();
    EXPECT_EQ(ChunkHeader::fromPayload(payload), &sut);
}

class ChunkHeader_AlteringPayloadWithoutCustomHeader : public ::testing::TestWithParam<PayloadParams>
{
};

// without a custom header, the payload is located right after the ChunkHeader, therefore the payload size and alignment
// parameters are made dependant on the ChunkHeader
INSTANTIATE_TEST_CASE_P(ChunkHeader_test,
                        ChunkHeader_AlteringPayloadWithoutCustomHeader,
                        ::testing::Values(
                            // alignment = 0
                            PayloadParams{0U, 0U},
                            PayloadParams{1U, 0U},
                            PayloadParams{sizeof(ChunkHeader), 0U},
                            PayloadParams{sizeof(ChunkHeader) * 42U, 0U},
                            // alignment = 1
                            PayloadParams{0U, 1U},
                            PayloadParams{1U, 1U},
                            PayloadParams{sizeof(ChunkHeader), 1U},
                            PayloadParams{sizeof(ChunkHeader) * 42U, 1U},
                            // alignment = alignof(ChunkHeader) / 2
                            PayloadParams{0U, alignof(ChunkHeader) / 2},
                            PayloadParams{1U, alignof(ChunkHeader) / 2},
                            PayloadParams{sizeof(ChunkHeader), alignof(ChunkHeader) / 2},
                            PayloadParams{sizeof(ChunkHeader) * 42U, alignof(ChunkHeader) / 2},
                            // alignment = alignof(ChunkHeader)
                            PayloadParams{0U, alignof(ChunkHeader)},
                            PayloadParams{1U, alignof(ChunkHeader)},
                            PayloadParams{sizeof(ChunkHeader), alignof(ChunkHeader)},
                            PayloadParams{sizeof(ChunkHeader) * 42U, alignof(ChunkHeader)},
                            // alignment = alignof(ChunkHeader) * 2
                            PayloadParams{0U, alignof(ChunkHeader) * 2},
                            PayloadParams{1U, alignof(ChunkHeader) * 2},
                            PayloadParams{sizeof(ChunkHeader), alignof(ChunkHeader) * 2},
                            PayloadParams{sizeof(ChunkHeader) * 42U, alignof(ChunkHeader) * 2}));

TEST_P(ChunkHeader_AlteringPayloadWithoutCustomHeader, checkIntegrityOfChunkHeaderWithoutCustomHeader)
{
    const auto payloadParams = GetParam();

    SCOPED_TRACE(std::string("Payload size = ") + std::to_string(payloadParams.size) + std::string("; alignment = ")
                 + std::to_string(payloadParams.alignment));

    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    createChunksOnMultipleAddresses(payloadParams, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT, [&](ChunkHeader& sut) {
        checkPayloadNotOverlappingWithChunkHeader(sut);
        checkPayloadSize(sut, payloadParams);
        checkPayloadAlignment(sut, payloadParams);
        checkUsedSizeOfChunk(sut, payloadParams);
        checkConversionOfPayloadPointerToChunkHeader(sut);
    });
}

class ChunkHeader_AlteringPayloadWithCustomHeader : public ::testing::TestWithParam<PayloadParams>
{
};

// with a custom header, the payload is located right after the PayloadOffset_t, therefore the payload size and
// alignment parameters are made dependant on the PayloadOffset_t
INSTANTIATE_TEST_CASE_P(ChunkHeader_test,
                        ChunkHeader_AlteringPayloadWithCustomHeader,
                        ::testing::Values(
                            // alignment = 0
                            PayloadParams{0U, 0U},
                            PayloadParams{1U, 0U},
                            PayloadParams{sizeof(PayloadOffset_t), 0U},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, 0U},
                            // alignment = 1
                            PayloadParams{0U, 1U},
                            PayloadParams{1U, 1U},
                            PayloadParams{sizeof(PayloadOffset_t), 1U},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, 1U},
                            // alignment = alignof(PayloadOffset_t) / 2
                            PayloadParams{0U, alignof(PayloadOffset_t) / 2},
                            PayloadParams{1U, alignof(PayloadOffset_t) / 2},
                            PayloadParams{sizeof(PayloadOffset_t), alignof(PayloadOffset_t) / 2},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, alignof(PayloadOffset_t) / 2},
                            // alignment = alignof(PayloadOffset_t)
                            PayloadParams{0U, alignof(PayloadOffset_t)},
                            PayloadParams{1U, alignof(PayloadOffset_t)},
                            PayloadParams{sizeof(PayloadOffset_t), alignof(PayloadOffset_t)},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, alignof(PayloadOffset_t)},
                            // alignment = alignof(PayloadOffset_t) * 2
                            PayloadParams{0U, alignof(PayloadOffset_t) * 2},
                            PayloadParams{1U, alignof(PayloadOffset_t) * 2},
                            PayloadParams{sizeof(PayloadOffset_t), alignof(PayloadOffset_t) * 2},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, alignof(PayloadOffset_t) * 2}));

TEST_P(ChunkHeader_AlteringPayloadWithCustomHeader, checkIntegrityOfChunkHeaderWithCustomHeader)
{
    const auto payloadParams = GetParam();

    SCOPED_TRACE(std::string("Payload size = ") + std::to_string(payloadParams.size) + std::string("; alignment = ")
                 + std::to_string(payloadParams.alignment));

    constexpr uint32_t CUSTOM_HEADER_SIZES[]{
        1U, sizeof(ChunkHeader) / 2U, sizeof(ChunkHeader), sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENTS[]{0U, 1U, alignof(ChunkHeader), alignof(ChunkHeader)};

    for (const auto customHeaderAlignment : CUSTOM_HEADER_ALIGNMENTS)
    {
        SCOPED_TRACE(std::string("Custom header alignment = ") + std::to_string(customHeaderAlignment));

        for (const auto customHeaderSize : CUSTOM_HEADER_SIZES)
        {
            SCOPED_TRACE(std::string("Custom header size = ") + std::to_string(customHeaderSize));

            if (customHeaderSize < customHeaderAlignment)
            {
                // the size must always be a multiple of the alignment
                continue;
            }

            createChunksOnMultipleAddresses(
                payloadParams, customHeaderSize, customHeaderAlignment, [&](ChunkHeader& sut) {
                    checkCustomHeaderIsAdjacentToChunkHeader(sut);
                    checkPayloadNotOverlappingWithCustomHeader(sut, customHeaderSize);
                    checkPayloadSize(sut, payloadParams);
                    checkPayloadAlignment(sut, payloadParams);
                    checkUsedSizeOfChunk(sut, payloadParams);
                    checkConversionOfPayloadPointerToChunkHeader(sut);
                });
        }
    }
}

// END PARAMETERIZED TESTS FOR CHUNK HEADER

} // namespace
