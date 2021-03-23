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

#include "iceoryx_posh/mepoo/chunk_settings.hpp"

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using iox::mepoo::ChunkHeader;
using iox::mepoo::ChunkSettings;
using PayloadOffset_t = iox::mepoo::ChunkHeader::PayloadOffset_t;

// BEGIN GETTER METHOD TESTS

TEST(ChunkSettings_test, payloadSizeReturnsCorrectValue)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{128U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{64U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{4U};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.payloadSize(), Eq(PAYLOAD_SIZE));
}

TEST(ChunkSettings_test, payloadAlignmentReturnsCorrectValue)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{128U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{64U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{4U};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.payloadAlignment(), Eq(PAYLOAD_ALIGNMENT));
}

TEST(ChunkSettings_test, customHeaderSizeReturnsCorrectValue)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{128U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{64U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{4U};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.customHeaderSize(), Eq(CUSTOM_HEADER_SIZE));
}

TEST(ChunkSettings_test, customHeaderAlignmentReturnsCorrectValue)
{
    constexpr uint32_t PAYLOAD_SIZE{42U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{128U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{64U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{4U};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.customHeaderAlignment(), Eq(CUSTOM_HEADER_ALIGNMENT));
}

TEST(ChunkSettings_test, requiredChunkSizeReturnsCorrectValue)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    constexpr uint32_t EXPECTED_SIZE{sizeof(ChunkHeader)};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(EXPECTED_SIZE));
}

// END GETTER METHOD TESTS

// BEGIN EXCEEDING CHUNK SIZE TESTS

TEST(ChunkSettings_test, NoCustomPayloadAlignmentAndTooLargePayload_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{std::numeric_limits<uint32_t>::max()};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    ASSERT_TRUE(sutResult.has_error());
    EXPECT_THAT(sutResult.get_error(), Eq(ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE));
}

TEST(ChunkSettings_test, CustomPayloadAlignmentAndTooLargePayload_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{std::numeric_limits<uint32_t>::max()};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(ChunkHeader) * 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{iox::CHUNK_NO_CUSTOM_HEADER_SIZE};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    ASSERT_TRUE(sutResult.has_error());
    EXPECT_THAT(sutResult.get_error(), Eq(ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE));
}

TEST(ChunkSettings_test, CustomHeaderAndTooLargePayload_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{std::numeric_limits<uint32_t>::max()};
    constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(ChunkHeader) * 2};
    constexpr uint32_t CUSTOM_HEADER_SIZE{8U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{8U};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    ASSERT_TRUE(sutResult.has_error());
    EXPECT_THAT(sutResult.get_error(), Eq(ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE));
}

// END EXCEEDING CHUNK SIZE TESTS

// BEGIN INVALID CUSTOM HEADER AND PAYLOAD ALIGNMENT TESTS

TEST(ChunkSettings_test, PayloadAlignmentNotPowerOfTwo_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{13U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{0U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    ASSERT_TRUE(sutResult.has_error());
    EXPECT_THAT(sutResult.get_error(), Eq(ChunkSettings::Error::ALIGNMENT_NOT_POWER_OF_TWO));
}

TEST(ChunkSettings_test, CustomHeaderAlignmentNotPowerOfTwo_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{1U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{0U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{42};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    ASSERT_TRUE(sutResult.has_error());
    EXPECT_THAT(sutResult.get_error(), Eq(ChunkSettings::Error::ALIGNMENT_NOT_POWER_OF_TWO));
}

TEST(ChunkSettings_test, CustomHeaderAlignmentLargerThanChunkHeaderAlignment_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{8U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{2 * alignof(ChunkHeader)};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    ASSERT_TRUE(sutResult.has_error());
    EXPECT_THAT(sutResult.get_error(),
                Eq(ChunkSettings::Error::CUSTOM_HEADER_ALIGNMENT_EXCEEDS_CHUNK_HEADER_ALIGNMENT));
}

TEST(ChunkSettings_test, CustomHeaderSizeNotMultipleOfAlignment_Fails)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};
    constexpr uint32_t CUSTOM_HEADER_SIZE{12U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{8U};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);

    ASSERT_TRUE(sutResult.has_error());
    EXPECT_THAT(sutResult.get_error(), Eq(ChunkSettings::Error::CUSTOM_HEADER_SIZE_NOT_MULTIPLE_OF_ITS_ALIGNMENT));
}

// END INVALID CUSTOM HEADER AND PAYLOAD ALIGNMENT TESTS

// BEGIN PARAMETERIZED TESTS FOR REQUIRED CHUNK SIZE

struct PayloadParams
{
    uint32_t size{0U};
    uint32_t alignment{iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT};

    static constexpr uint32_t MAX_ALIGNMENT{1ULL << 31};
};

class ChunkSettings_AlteringPayloadWithoutCustomHeader : public ::testing::TestWithParam<PayloadParams>
{
};

// without a custom header, the payload is located right after the ChunkHeader, therefore the payload size and alignment
// parameters are made dependant on the ChunkHeader
INSTANTIATE_TEST_CASE_P(ChunkSettings_test,
                        ChunkSettings_AlteringPayloadWithoutCustomHeader,
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
                            PayloadParams{sizeof(ChunkHeader) * 42U, alignof(ChunkHeader) * 2},
                            // alignment = PayloadParams::MAX_ALIGNMENT
                            PayloadParams{0U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{1U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(ChunkHeader), PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(ChunkHeader) * 42U, PayloadParams::MAX_ALIGNMENT}));

TEST_P(ChunkSettings_AlteringPayloadWithoutCustomHeader, RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    SCOPED_TRACE(std::string("Payload size = ") + std::to_string(payload.size) + std::string("; alignment = ")
                 + std::to_string(payload.alignment));

    const uint32_t expectedSize = [&payload] {
        if (payload.alignment <= alignof(ChunkHeader))
        {
            // payload is always adjacent
            return static_cast<uint32_t>(sizeof(ChunkHeader)) + payload.size;
        }
        else
        {
            // payload is not necessarily adjacent
            auto prePayloadAlignmentOverhangOfChunkHeder = sizeof(ChunkHeader) - alignof(ChunkHeader);
            return static_cast<uint32_t>(prePayloadAlignmentOverhangOfChunkHeder) + payload.alignment + payload.size;
        }
    }();

    auto sutResult = ChunkSettings::create(payload.size, payload.alignment);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

class ChunkSettings_AlteringPayloadWithCustomHeader : public ::testing::TestWithParam<PayloadParams>
{
};

// with a custom header, the payload is located right after the PayloadOffset_t, therefore the payload size and
// alignment parameters are made dependant on the PayloadOffset_t
INSTANTIATE_TEST_CASE_P(ChunkSettings_test,
                        ChunkSettings_AlteringPayloadWithCustomHeader,
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
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, alignof(PayloadOffset_t) * 2},
                            // alignment = PayloadParams::MAX_ALIGNMENT
                            PayloadParams{0U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{1U, PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(PayloadOffset_t), PayloadParams::MAX_ALIGNMENT},
                            PayloadParams{sizeof(PayloadOffset_t) * 42U, PayloadParams::MAX_ALIGNMENT}));

uint32_t expectedChunkSizeWithCustomHeader(const PayloadParams& payload, uint32_t customHeaderSize)
{
    const uint32_t customHeaderSizeAndPaddingToBackOffset =
        iox::algorithm::max(customHeaderSize, static_cast<uint32_t>(alignof(PayloadOffset_t)));

    if (payload.alignment <= alignof(PayloadOffset_t))
    {
        // back-offset is always adjacent to the custom header (as much as possible with the alignment constraints)
        constexpr uint32_t BACK_OFFSET_SIZE{sizeof(PayloadOffset_t)};
        return static_cast<uint32_t>(sizeof(ChunkHeader)) + customHeaderSizeAndPaddingToBackOffset + BACK_OFFSET_SIZE
               + payload.size;
    }
    else
    {
        // back-offset is not necessarily adjacent to the custom header
        const uint32_t paddingBytesAndBackOffsetSize = payload.alignment;
        return static_cast<uint32_t>(sizeof(ChunkHeader)) + customHeaderSizeAndPaddingToBackOffset
               + paddingBytesAndBackOffsetSize + payload.size;
    }
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       MultipleCustomHeaderSizesAndAlignments_ResultsIn_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    SCOPED_TRACE(std::string("Payload size = ") + std::to_string(payload.size) + std::string("; alignment = ")
                 + std::to_string(payload.alignment));

    constexpr uint32_t CUSTOM_HEADER_SIZES[]{
        1U, sizeof(ChunkHeader) / 2U, sizeof(ChunkHeader), sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENTS[]{0U, 1U, alignof(ChunkHeader) / 2U, alignof(ChunkHeader)};

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

            const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, customHeaderSize);

            auto sutResult =
                ChunkSettings::create(payload.size, payload.alignment, customHeaderSize, customHeaderAlignment);
            ASSERT_FALSE(sutResult.has_error());
            auto& sut = sutResult.value();

            EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
        }
    }
}

// END PARAMETERIZED TESTS FOR REQUIRED CHUNK SIZE

} // namespace
