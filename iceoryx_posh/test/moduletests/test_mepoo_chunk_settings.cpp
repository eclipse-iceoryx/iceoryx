// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using iox::mepoo::ChunkHeader;
using iox::mepoo::ChunkSettings;
using PayloadOffset_t = iox::mepoo::ChunkHeader::PayloadOffset_t;

class ChunkSettings_test : public Test
{
  public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_F(ChunkSettings_test, AllParameterMinimal_ResultsIn_RequiredChunkSizeOfChunkHeader)
{
    constexpr uint32_t PAYLOAD_SIZE{0U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{1U};
    constexpr uint32_t CUSTOM_HEADER_SIZE{0U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    constexpr uint32_t EXPECTED_SIZE{sizeof(ChunkHeader)};

    auto sutResult =
        ChunkSettings::create(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(EXPECTED_SIZE));
}

TEST_F(ChunkSettings_test, ZeroPayloadAndDefaultValues_ResultsIn_RequiredChunkSizeOfChunkHeader)
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

// BEGIN EXCEEDING CHUNK SIZE TESTS

TEST_F(ChunkSettings_test, NoCustomPayloadAlignmentAndTooLargePayload_Fails)
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

TEST_F(ChunkSettings_test, CustomPayloadAlignmentAndTooLargePayload_Fails)
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

TEST_F(ChunkSettings_test, CustomHeaderAndTooLargePayload_Fails)
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

TEST_F(ChunkSettings_test, PayloadAlignmentNotPowerOfTwo_Fails)
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

TEST_F(ChunkSettings_test, CustomHeaderAlignmentNotPowerOfTwo_Fails)
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

TEST_F(ChunkSettings_test, CustomHeaderAlignmentLargerThanChunkHeaderAlignment_Fails)
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

TEST_F(ChunkSettings_test, CustomHeaderSizeNotMultipleOfAlignment_Fails)
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

TEST_P(ChunkSettings_AlteringPayloadWithoutCustomHeader, requiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

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

    auto sutResult =
    ChunkSettings::create(payload.size, payload.alignment);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

class ChunkSettings_AlteringPayloadWithCustomHeader : public ::testing::TestWithParam<PayloadParams>
{
  protected:
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

// BEGIN ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ZERO

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeEqualsToOne_AlignmentEqualsToZero_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{1U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{0U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeLessThanChunkHeader_AlignmentEqualsToZero_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) / 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{0U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeEqualsToChunkHeader_AlignmentEqualsToZero_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{0U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeGreaterThanChunkHeader_AlignmentEqualsToZero_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{0U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

// END ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ZERO

// BEGIN ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ONE

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeEqualsToOne_AlignmentEqualsToOne_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{1U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeLessThanChunkHeader_AlignmentEqualsToOne_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) / 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeEqualsToChunkHeader_AlignmentEqualsToOne_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeGreaterThanChunkHeader_AlignmentEqualsToOne_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{1U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

// END ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ONE

// BEGIN ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT LESS THAN ChunkHeader ALIGNMENT

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeLessThanChunkHeader_AlignmentLessThanChunkHeaderAlignment_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) / 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader) / 2U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeEqualsToChunkHeader_AlignmentLessThanChunkHeaderAlignment_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader) / 2U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeGreaterThanChunkHeader_AlignmentLessThanChunkHeaderAlignment_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader) / 2U};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

// END ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT LESS THAN ChunkHeader ALIGNMENT

// BEGIN ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ChunkHeader ALIGNMENT

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeEqualsToChunkHeader_AlignmentEqualToChunkHeaderAlignment_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader)};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader)};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

TEST_P(ChunkSettings_AlteringPayloadWithCustomHeader,
       CustomHeader_SizeGreaterThanChunkHeader_AlignmentEqualToChunkHeaderAlignment_RequiredChunkSizeIsCorrect)
{
    const auto payload = GetParam();

    constexpr uint32_t CUSTOM_HEADER_SIZE{sizeof(ChunkHeader) * 2U};
    constexpr uint32_t CUSTOM_HEADER_ALIGNMENT{sizeof(ChunkHeader)};

    const uint32_t expectedSize = expectedChunkSizeWithCustomHeader(payload, CUSTOM_HEADER_SIZE);

    auto sutResult =
        ChunkSettings::create(payload.size, payload.alignment, CUSTOM_HEADER_SIZE, CUSTOM_HEADER_ALIGNMENT);
    ASSERT_FALSE(sutResult.has_error());
    auto& sut = sutResult.value();

    EXPECT_THAT(sut.requiredChunkSize(), Eq(expectedSize));
}

// END ALTERING CUSTOM HEADER SIZE WITH ALIGNMENT EQUAL TO ChunkHeader ALIGNMENT

// END PARAMETERIZED TESTS FOR REQUIRED CHUNK SIZE

} // namespace
