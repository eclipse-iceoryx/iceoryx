// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

#include "test.hpp"

#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::mepoo;

using UserPayloadOffset_t = ChunkHeader::UserPayloadOffset_t;

TEST(ChunkHeader_test, ChunkHeaderHasInitializedMembers)
{
    ::testing::Test::RecordProperty("TEST_ID", "b998bcb8-7db0-457d-a25a-86eae34f68dd");
    constexpr uint32_t CHUNK_SIZE{753U};
    constexpr uint32_t USER_PAYLOAD_SIZE{8U};
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT{iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    EXPECT_THAT(sut.chunkSize(), Eq(CHUNK_SIZE));

    // deliberately used a magic number to make the test fail when CHUNK_HEADER_VERSION changes
    EXPECT_THAT(sut.chunkHeaderVersion(), Eq(1U));

    EXPECT_THAT(sut.originId(), Eq(iox::popo::UniquePortId(iox::popo::InvalidPortId)));

    EXPECT_THAT(sut.sequenceNumber(), Eq(0U));

    EXPECT_THAT(sut.userHeaderId(), Eq(ChunkHeader::NO_USER_HEADER));
    EXPECT_THAT(sut.userHeaderSize(), Eq(0U));
    EXPECT_THAT(sut.userPayloadSize(), Eq(USER_PAYLOAD_SIZE));
    EXPECT_THAT(sut.userPayloadAlignment(), Eq(USER_PAYLOAD_ALIGNMENT));

    // a default created ChunkHeader has always an adjacent user-payload
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(sut.userPayload())};
    EXPECT_THAT(userPayloadStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, ChunkHeaderBinaryCompatibilityCheck)
{
    ::testing::Test::RecordProperty("TEST_ID", "8f88f81a-7e18-11ec-b34d-dd7741c14c43");

    // NOTE: when this test fails and needs to be changed,
    //       most probably one needs to increment the CHUNK_HEADER_VERSION

    // When this struct is touched, the CHUNK_HEADER_VERSION must be changed
    struct ExpectedChunkHeaderLayout
    {
        uint32_t chunkSize{0U};
        uint8_t chunkHeaderVersion{0U};
        uint8_t reserved{0U};
        uint16_t userHeaderId{0};
        uint64_t originId{0U};
        uint64_t sequenceNumber{0U};
        uint32_t userHeaderSize{0U};
        uint32_t userPayloadSize{0U};
        uint32_t userPayloadAlignment{0U};
        uint32_t userPayloadOffset{0U};
    };

    constexpr auto EXPECTED_CHUNK_HEADER_VERSION{1U};
    EXPECT_THAT(ChunkHeader::CHUNK_HEADER_VERSION, Eq(EXPECTED_CHUNK_HEADER_VERSION));

    EXPECT_THAT(sizeof(ChunkHeader), Eq(sizeof(ExpectedChunkHeaderLayout)));
    EXPECT_THAT(alignof(ChunkHeader), Eq(alignof(ExpectedChunkHeaderLayout)));

    // The sut is always zeroized and a single member is set to a specific value
    // if a reinterpret cast to a ChunkHeader and access to the corresponding value
    // results in the previously set pattern, the layout matches and the ChunkHeader
    // did not change its ABI
    // Unfortunately offsetof cannot be used since the members of ChunkHeader are private
    ExpectedChunkHeaderLayout sut{};

    auto zeroizeSut = [&] { std::memset(static_cast<void*>(&sut), 0, sizeof(ExpectedChunkHeaderLayout)); };

    constexpr uint8_t PATTERN{42U};

#define IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(member)                                                             \
    zeroizeSut();                                                                                                      \
    sut.member = PATTERN;                                                                                              \
    ASSERT_TRUE((std::is_same<decltype(std::declval<ChunkHeader>().member()),                                          \
                              decltype(ExpectedChunkHeaderLayout::member)>::value));                                   \
    EXPECT_THAT(reinterpret_cast<ChunkHeader*>(&sut)->member(), Eq(PATTERN));

    IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(chunkSize);
    IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(chunkHeaderVersion);
    IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(userHeaderId);
    IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(sequenceNumber);
    IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(userHeaderSize);
    IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(userPayloadSize);
    IOX_TEST_CHUNK_HEADER_MEMBER_COMPATIBILITY(userPayloadAlignment);

    // special handling for originId since it is a UniquePortId
    zeroizeSut();
    sut.originId = PATTERN;
    using OriginIdType = decltype(ExpectedChunkHeaderLayout::originId);
    ASSERT_TRUE((std::is_same<decltype(std::declval<ChunkHeader>().originId())::value_type, OriginIdType>::value));
    auto originId = static_cast<OriginIdType>(reinterpret_cast<ChunkHeader*>(&sut)->originId());
    EXPECT_THAT(originId, Eq(PATTERN));

    // special handling for userPayloadOffset since it cannot easily be accessed
    zeroizeSut();
    sut.userPayloadOffset = PATTERN;
    ASSERT_TRUE((
        std::is_same<ChunkHeader::UserPayloadOffset_t, decltype(ExpectedChunkHeaderLayout::userPayloadOffset)>::value));
    auto userPayloadPointer = reinterpret_cast<ChunkHeader*>(&sut)->userPayload();
    // this is a bit of a white box test but after all, all the other stuff in this test case is also white box
    auto userPayloadOffset = reinterpret_cast<uint64_t>(userPayloadPointer) - reinterpret_cast<uint64_t>(&sut);
    EXPECT_THAT(userPayloadOffset, Eq(static_cast<uint64_t>(PATTERN)));
}

TEST(ChunkHeader_test, ChunkHeaderUserPayloadSizeTypeIsLargeEnoughForMempoolChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "540e2e95-0890-4522-ae7f-c6d867679e0b");
    using ChunkSize_t = iox::platform::invoke_result<decltype(&MemPool::getChunkSize), MemPool>::type;

    auto maxOfChunkSizeType = std::numeric_limits<ChunkSize_t>::max();
    auto maxOfUserPayloadSizeType = std::numeric_limits<decltype(std::declval<ChunkHeader>().userPayloadSize())>::max();

    // the user-payload will never be larger than the chunk
    // if the user-payload type can hold at least the maximum chunk size there will never be an overflow
    EXPECT_THAT(maxOfUserPayloadSizeType, Ge(maxOfChunkSizeType));
}

TEST(ChunkHeader_test, UserPayloadFunctionCalledFromNonConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d6b0fcce-8b49-429c-a1d2-c047b4b2e368");
    constexpr uint32_t CHUNK_SIZE{753U};
    constexpr uint32_t USER_PAYLOAD_SIZE{8U};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    // a default created ChunkHeader has always an adjacent user-payload
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(sut.userPayload())};
    EXPECT_THAT(userPayloadStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, UserPayloadFunctionCalledFromConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "091451db-09ed-4aa4-bcfe-d45c0c6d6c14");
    constexpr uint32_t CHUNK_SIZE{753U};
    constexpr uint32_t USER_PAYLOAD_SIZE{8U};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    const ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    // a default created ChunkHeader has always an adjacent user-payload
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(sut.userPayload())};
    EXPECT_THAT(userPayloadStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, UserPayloadFunctionCalledFromNonConstChunkHeaderReturnsNonConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "8876c866-2dda-4fb4-a96b-cee7f6b5e00b");
    auto isNonConstReturn = std::is_same<decltype(std::declval<ChunkHeader>().userPayload()), void*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST(ChunkHeader_test, UserPayloadFunctionCalledFromConstChunkHeaderReturnsConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "34957f49-06b3-426b-a0ba-df9071611fb2");
    auto isConstReturn = std::is_same<decltype(std::declval<const ChunkHeader>().userPayload()), const void*>::value;
    EXPECT_TRUE(isConstReturn);
}

TEST(ChunkHeader_test, UserHeaderFunctionCalledFromNonConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ddbb681-e8bb-42e0-9546-e9fd64c44be0");
    alignas(ChunkHeader) static uint8_t storage[1024 * 1024];

    constexpr uint32_t CHUNK_SIZE{753U};
    constexpr uint32_t USER_PAYLOAD_SIZE{8U};
    constexpr uint32_t USER_HEADER_SIZE{16U};
    constexpr uint32_t USER_HEADER_ALIGNMENT{8U};

    auto chunkSettingsResult = ChunkSettings::create(
        USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    iox::cxx::unique_ptr<ChunkHeader> sut{new (storage) ChunkHeader(CHUNK_SIZE, chunkSettings), [](ChunkHeader*) {}};

    // the user-header is always adjacent to the ChunkHeader
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(sut.get())};
    const uint64_t userHeaderStartAddress{reinterpret_cast<uint64_t>(sut->userHeader())};
    EXPECT_THAT(userHeaderStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, UserHeaderFunctionCalledFromConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "04f902b9-0870-4ba4-b761-856e9bbfcd85");
    alignas(ChunkHeader) static uint8_t storage[1024 * 1024];

    constexpr uint32_t CHUNK_SIZE{753U};
    constexpr uint32_t USER_PAYLOAD_SIZE{8U};
    constexpr uint32_t USER_HEADER_SIZE{16U};
    constexpr uint32_t USER_HEADER_ALIGNMENT{8U};

    auto chunkSettingsResult = ChunkSettings::create(
        USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    iox::cxx::unique_ptr<const ChunkHeader> sut{new (storage) ChunkHeader(CHUNK_SIZE, chunkSettings),
                                                [](const ChunkHeader*) {}};

    // the user-header is always adjacent to the ChunkHeader
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(sut.get())};
    const uint64_t userHeaderStartAddress{reinterpret_cast<uint64_t>(sut->userHeader())};
    EXPECT_THAT(userHeaderStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, UserHeaderFunctionCalledFromNonConstChunkHeaderReturnsNonConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "420fc560-c309-4fc2-9224-aec70c8212cc");
    auto isNonConstReturn = std::is_same<decltype(std::declval<ChunkHeader>().userHeader()), void*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST(ChunkHeader_test, UserHeaderFunctionCalledFromConstChunkHeaderReturnsConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca80c797-a22a-4d47-ba79-0f858b5d7836");
    auto isConstReturn = std::is_same<decltype(std::declval<const ChunkHeader>().userHeader()), const void*>::value;
    EXPECT_TRUE(isConstReturn);
}

TEST(ChunkHeader_test, FromUserPayloadFunctionCalledWithNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "9297471a-42ab-454c-9a10-ae2dd71e6bf9");
    constexpr void* USER_PAYLOAD{nullptr};
    auto chunkHeader = ChunkHeader::fromUserPayload(USER_PAYLOAD);
    EXPECT_THAT(chunkHeader, Eq(nullptr));
}

TEST(ChunkHeader_test, FromUserPayloadFunctionCalledWithConstNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "de23b1ac-6552-4613-ac66-707d22157a0c");
    constexpr const void* USER_PAYLOAD{nullptr};
    auto chunkHeader = ChunkHeader::fromUserPayload(USER_PAYLOAD);
    EXPECT_THAT(chunkHeader, Eq(nullptr));
}

TEST(ChunkHeader_test, FromUserPayloadFunctionCalledWithNonConstParamReturnsNonConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "835d2c6e-490d-478d-a882-f51d113a73a3");
    auto isNonConstReturn =
        std::is_same<decltype(ChunkHeader::fromUserPayload(std::declval<void*>())), ChunkHeader*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST(ChunkHeader_test, FromUserPayloadFunctionCalledWithConstParamReturnsConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "57ea3b52-fad4-4250-ab4d-276d5472edf1");
    auto isConstReturn =
        std::is_same<decltype(ChunkHeader::fromUserPayload(std::declval<const void*>())), const ChunkHeader*>::value;
    EXPECT_TRUE(isConstReturn);
}

TEST(ChunkHeader_test, FromUserHeaderFunctionCalledWithNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "56fc6dd9-791b-4803-bd1a-5a0b53b0875d");
    constexpr void* USER_HEADER{nullptr};
    auto chunkHeader = ChunkHeader::fromUserHeader(USER_HEADER);
    EXPECT_THAT(chunkHeader, Eq(nullptr));
}

TEST(ChunkHeader_test, FromUserHeaderFunctionCalledWithConstNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "f388ae41-7102-4d35-806e-4a572890e7a0");
    constexpr const void* USER_HEADER{nullptr};
    auto chunkHeader = ChunkHeader::fromUserHeader(USER_HEADER);
    EXPECT_THAT(chunkHeader, Eq(nullptr));
}

TEST(ChunkHeader_test, FromUserHeaderFunctionCalledWithNonConstParamReturnsNonConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "993df4e6-2cc2-4ae1-b4d4-01a4ded65ba4");
    auto isNonConstReturn =
        std::is_same<decltype(ChunkHeader::fromUserHeader(std::declval<void*>())), ChunkHeader*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST(ChunkHeader_test, FromUserHeaderFunctionCalledWithConstParamReturnsConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b9a09b8-0baa-4a30-95e4-d081e01038d2");
    auto isConstReturn =
        std::is_same<decltype(ChunkHeader::fromUserHeader(std::declval<const void*>())), const ChunkHeader*>::value;
    EXPECT_TRUE(isConstReturn);
}

TEST(ChunkHeader_test, UsedChunkSizeIsSizeOfChunkHeaderWhenUserPayloadIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "67d3907e-5090-4814-b726-2a37e8780395");
    constexpr uint32_t CHUNK_SIZE{2 * sizeof(ChunkHeader)};
    constexpr uint32_t USER_PAYLOAD_SIZE{0U};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    EXPECT_THAT(sut.usedSizeOfChunk(), Eq(sizeof(ChunkHeader)));
}

TEST(ChunkHeader_test, UsedChunkSizeIsSizeOfChunkHeaderPlusOneWhenUserPayloadIsOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a0cd3a1-3edc-441a-9ba6-486a463ad9d7");
    constexpr uint32_t CHUNK_SIZE{2 * sizeof(ChunkHeader)};
    constexpr uint32_t USER_PAYLOAD_SIZE{1U};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    ChunkHeader sut{CHUNK_SIZE, chunkSettings};

    EXPECT_THAT(sut.usedSizeOfChunk(), Eq(sizeof(ChunkHeader) + USER_PAYLOAD_SIZE));
}

TEST(ChunkHeader_test, ConstructorTerminatesWhenUserPayloadSizeExceedsChunkSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "c8f911eb-ed0d-495a-8858-9fc45f5a06e8");
    constexpr uint32_t CHUNK_SIZE{128U};
    constexpr uint32_t USER_PAYLOAD_SIZE{2U * CHUNK_SIZE};

    auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(chunkSettingsResult.has_error());
    auto& chunkSettings = chunkSettingsResult.value();

    EXPECT_DEATH({ ChunkHeader sut(CHUNK_SIZE, chunkSettings); }, ".*");
}

// BEGIN PARAMETERIZED TESTS FOR CHUNK HEADER

struct PayloadParams
{
    uint32_t size{0U};
    uint32_t alignment{iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT};
};


void createChunksOnMultipleAddresses(const PayloadParams& userPayloadParams,
                                     const uint32_t userHeaderSize,
                                     const uint32_t userHeaderAlignment,
                                     const std::function<void(ChunkHeader& chunkHeader)> testHook)
{
    ASSERT_TRUE(testHook);

    constexpr size_t MAX_USER_PAYLOAD_ALIGNMENT_FOR_TEST{512U};
    ASSERT_THAT(MAX_USER_PAYLOAD_ALIGNMENT_FOR_TEST, Gt(alignof(ChunkHeader)));

    constexpr size_t STORAGE_ALIGNMENT{2 * MAX_USER_PAYLOAD_ALIGNMENT_FOR_TEST};
    alignas(STORAGE_ALIGNMENT) static uint8_t storage[1024 * 1024];
    ASSERT_THAT(reinterpret_cast<uint64_t>(storage) % STORAGE_ALIGNMENT, Eq(0U));

    // storage alignment boundaries                    -> ⊥               ⊥               ⊥               ⊥
    // max user-payload alignment for test boundaries  -> ⊥       ⊥       ⊥       ⊥       ⊥       ⊥       ⊥
    // ChunkHeader alignment boundaries                -> ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥ ⊥

    // the test creates ChunkHeader on multiple address boundaries in order to have all possible scenarios up to a
    // user-payload boundary of 512; this boundary is more than large enough since a user-payload alignment of 2 times
    // the ChunkHeader alignment would already be sufficient to test all corner cases
    for (auto alignedChunkAddress = alignof(ChunkHeader); alignedChunkAddress <= MAX_USER_PAYLOAD_ALIGNMENT_FOR_TEST;
         alignedChunkAddress += alignof(ChunkHeader))
    {
        SCOPED_TRACE(std::string("Chunk placed on address ") + iox::cxx::convert::toString(alignedChunkAddress));

        auto chunkSettingsResult = ChunkSettings::create(
            userPayloadParams.size, userPayloadParams.alignment, userHeaderSize, userHeaderAlignment);
        ASSERT_FALSE(chunkSettingsResult.has_error());
        auto& chunkSettings = chunkSettingsResult.value();
        auto chunkSize = chunkSettings.requiredChunkSize();

        auto chunkHeader = new (&storage[alignedChunkAddress]) ChunkHeader(chunkSize, chunkSettings);

        testHook(*chunkHeader);

        chunkHeader->~ChunkHeader();
    }
}

void checkUserHeaderIdAndSizeAndPayloadSizeAndAlignmentIsSet(const ChunkHeader& sut,
                                                             const PayloadParams& userPayloadParams,
                                                             const uint16_t userHeaderId,
                                                             const uint32_t userHeaderSize)
{
    SCOPED_TRACE(std::string("Check user-header id and size and user-payload alignment ist correctly set"));
    EXPECT_EQ(sut.userPayloadSize(), userPayloadParams.size);
    // a user-payload alignment of zero will internally be set to one
    auto adjustedAlignment = userPayloadParams.alignment == 0U ? 1U : userPayloadParams.alignment;
    EXPECT_EQ(sut.userPayloadAlignment(), adjustedAlignment);
    EXPECT_EQ(sut.userHeaderId(), userHeaderId);
    EXPECT_EQ(sut.userHeaderSize(), userHeaderSize);
}

void checkUserPayloadNotOverlappingWithChunkHeader(const ChunkHeader& sut)
{
    SCOPED_TRACE(std::string("Check user-payload not overlapping with ChunkHeader"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(sut.userPayload())};

    EXPECT_THAT(userPayloadStartAddress - chunkStartAddress, Ge(sizeof(ChunkHeader)));
}

void checkUserPayloadNotOverlappingWithUserHeader(const ChunkHeader& sut, const uint32_t userHeaderSize)
{
    SCOPED_TRACE(std::string("Check user-payload not overlapping with user-header"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(sut.userPayload())};
    const uint64_t userHeaderSizeAndPadding{
        iox::algorithm::max(userHeaderSize, static_cast<uint32_t>(alignof(UserPayloadOffset_t)))};
    constexpr uint64_t BACK_OFFSET_SIZE{sizeof(UserPayloadOffset_t)};
    const uint64_t expectedRequiredSpace{sizeof(ChunkHeader) + userHeaderSizeAndPadding + BACK_OFFSET_SIZE};

    EXPECT_THAT(userPayloadStartAddress - chunkStartAddress, Ge(expectedRequiredSpace));
}

void checkUserHeaderIsAdjacentToChunkHeader(const ChunkHeader& sut)
{
    SCOPED_TRACE(std::string("Check user-header is adjacent to ChunkHeader"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t userHeaderStartAddress{reinterpret_cast<uint64_t>(sut.userHeader())};

    EXPECT_EQ(userHeaderStartAddress - chunkStartAddress, sizeof(ChunkHeader));
}

void checkUserPayloadSize(const ChunkHeader& sut, const PayloadParams& userPayloadParams)
{
    SCOPED_TRACE(std::string("Check user-payload size"));
    EXPECT_EQ(sut.userPayloadSize(), userPayloadParams.size);
}

void checkUserPayloadAlignment(const ChunkHeader& sut, const PayloadParams& userPayloadParams)
{
    SCOPED_TRACE(std::string("Check user-payload alignment"));
    // a user-payload alignment of zero will internally be set to one
    auto adjustedAlignment = userPayloadParams.alignment == 0U ? 1U : userPayloadParams.alignment;
    EXPECT_EQ(reinterpret_cast<uint64_t>(sut.userPayload()) % adjustedAlignment, 0U);
}

void checkUsedSizeOfChunk(const ChunkHeader& sut, const PayloadParams& userPayloadParams)
{
    SCOPED_TRACE(std::string("Check used size of chunk"));
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(&sut)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(sut.userPayload())};
    const uint64_t expectedUsedSizeOfChunk{userPayloadStartAddress + userPayloadParams.size - chunkStartAddress};

    EXPECT_EQ(sut.usedSizeOfChunk(), expectedUsedSizeOfChunk);
    EXPECT_THAT(sut.usedSizeOfChunk(), Le(sut.chunkSize()));
}

void checkConversionOfUserPayloadPointerToChunkHeader(const ChunkHeader& sut)
{
    SCOPED_TRACE(std::string("Check conversion of user-payload pointer to ChunkHeader pointer"));
    const auto userPayload = sut.userPayload();
    EXPECT_EQ(ChunkHeader::fromUserPayload(userPayload), &sut);
}

void checkConversionOfUserHeaderPointerToChunkHeader(const ChunkHeader& sut)
{
    SCOPED_TRACE(std::string("Check conversion of user-Header pointer to ChunkHeader pointer"));
    const auto userHeader = sut.userHeader();
    EXPECT_EQ(ChunkHeader::fromUserHeader(userHeader), &sut);
}

class ChunkHeader_AlteringUserPayloadWithoutUserHeader : public ::testing::TestWithParam<PayloadParams>
{
};

// without a user-header, the user-payload is located right after the ChunkHeader, therefore the payload size and
// alignment parameters are made dependant on the ChunkHeader
INSTANTIATE_TEST_SUITE_P(ChunkHeader_test,
                         ChunkHeader_AlteringUserPayloadWithoutUserHeader,
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

TEST_P(ChunkHeader_AlteringUserPayloadWithoutUserHeader, CheckIntegrityOfChunkHeaderWithoutUserHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "e80e27f1-de7c-4be4-a83e-9f0eb766de12");
    const auto userPayloadParams = GetParam();

    SCOPED_TRACE(std::string("User-Payload: size = ") + iox::cxx::convert::toString(userPayloadParams.size)
                 + std::string("; alignment = ") + iox::cxx::convert::toString(userPayloadParams.alignment));

    constexpr uint32_t USER_HEADER_SIZE{iox::CHUNK_NO_USER_HEADER_SIZE};
    constexpr uint32_t USER_HEADER_ALIGNMENT{iox::CHUNK_NO_USER_HEADER_ALIGNMENT};

    createChunksOnMultipleAddresses(userPayloadParams, USER_HEADER_SIZE, USER_HEADER_ALIGNMENT, [&](ChunkHeader& sut) {
        checkUserHeaderIdAndSizeAndPayloadSizeAndAlignmentIsSet(
            sut, userPayloadParams, ChunkHeader::NO_USER_HEADER, USER_HEADER_SIZE);
        checkUserPayloadNotOverlappingWithChunkHeader(sut);
        checkUserPayloadSize(sut, userPayloadParams);
        checkUserPayloadAlignment(sut, userPayloadParams);
        checkUsedSizeOfChunk(sut, userPayloadParams);
        checkConversionOfUserPayloadPointerToChunkHeader(sut);
    });
}

class ChunkHeader_AlteringUserPayloadWithUserHeader : public ::testing::TestWithParam<PayloadParams>
{
};

// with a user-header, the user-payload is located right after the UserPayloadOffset_t, therefore the user-payload size
// and alignment parameters are made dependant on the UserPayloadOffset_t
INSTANTIATE_TEST_SUITE_P(ChunkHeader_test,
                         ChunkHeader_AlteringUserPayloadWithUserHeader,
                         ::testing::Values(
                             // alignment = 0
                             PayloadParams{0U, 0U},
                             PayloadParams{1U, 0U},
                             PayloadParams{sizeof(UserPayloadOffset_t), 0U},
                             PayloadParams{sizeof(UserPayloadOffset_t) * 42U, 0U},
                             // alignment = 1
                             PayloadParams{0U, 1U},
                             PayloadParams{1U, 1U},
                             PayloadParams{sizeof(UserPayloadOffset_t), 1U},
                             PayloadParams{sizeof(UserPayloadOffset_t) * 42U, 1U},
                             // alignment = alignof(UserPayloadOffset_t) / 2
                             PayloadParams{0U, alignof(UserPayloadOffset_t) / 2},
                             PayloadParams{1U, alignof(UserPayloadOffset_t) / 2},
                             PayloadParams{sizeof(UserPayloadOffset_t), alignof(UserPayloadOffset_t) / 2},
                             PayloadParams{sizeof(UserPayloadOffset_t) * 42U, alignof(UserPayloadOffset_t) / 2},
                             // alignment = alignof(UserPayloadOffset_t)
                             PayloadParams{0U, alignof(UserPayloadOffset_t)},
                             PayloadParams{1U, alignof(UserPayloadOffset_t)},
                             PayloadParams{sizeof(UserPayloadOffset_t), alignof(UserPayloadOffset_t)},
                             PayloadParams{sizeof(UserPayloadOffset_t) * 42U, alignof(UserPayloadOffset_t)},
                             // alignment = alignof(UserPayloadOffset_t) * 2
                             PayloadParams{0U, alignof(UserPayloadOffset_t) * 2},
                             PayloadParams{1U, alignof(UserPayloadOffset_t) * 2},
                             PayloadParams{sizeof(UserPayloadOffset_t), alignof(UserPayloadOffset_t) * 2},
                             PayloadParams{sizeof(UserPayloadOffset_t) * 42U, alignof(UserPayloadOffset_t) * 2}));

TEST_P(ChunkHeader_AlteringUserPayloadWithUserHeader, CheckIntegrityOfChunkHeaderWithUserHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0d45cb6-cf0c-44e7-b759-fdc25535c9c4");
    const auto userPayloadParams = GetParam();

    SCOPED_TRACE(std::string("User-Payload: size = ") + iox::cxx::convert::toString(userPayloadParams.size)
                 + std::string("; alignment = ") + iox::cxx::convert::toString(userPayloadParams.alignment));

    constexpr uint32_t SMALL_USER_HEADER{alignof(ChunkHeader)};
    static_assert(SMALL_USER_HEADER < sizeof(ChunkHeader), "For this test the size must be smaller than ChunkHeader");
    constexpr uint32_t USER_HEADER_SIZES[]{1U, SMALL_USER_HEADER, sizeof(ChunkHeader), sizeof(ChunkHeader) * 2U};
    constexpr uint32_t USER_HEADER_ALIGNMENTS[]{0U, 1U, alignof(ChunkHeader) / 2U, alignof(ChunkHeader)};

    for (const auto userHeaderAlignment : USER_HEADER_ALIGNMENTS)
    {
        SCOPED_TRACE(std::string("User-Header alignment = ") + iox::cxx::convert::toString(userHeaderAlignment));

        for (const auto userHeaderSize : USER_HEADER_SIZES)
        {
            SCOPED_TRACE(std::string("User-Header size = ") + iox::cxx::convert::toString(userHeaderSize));

            if (userHeaderSize < userHeaderAlignment)
            {
                // the size must always be a multiple of the alignment
                continue;
            }

            createChunksOnMultipleAddresses(
                userPayloadParams, userHeaderSize, userHeaderAlignment, [&](ChunkHeader& sut) {
                    checkUserHeaderIdAndSizeAndPayloadSizeAndAlignmentIsSet(
                        sut, userPayloadParams, ChunkHeader::UNKNOWN_USER_HEADER, userHeaderSize);
                    checkUserHeaderIsAdjacentToChunkHeader(sut);
                    checkUserPayloadNotOverlappingWithUserHeader(sut, userHeaderSize);
                    checkUserPayloadSize(sut, userPayloadParams);
                    checkUserPayloadAlignment(sut, userPayloadParams);
                    checkUsedSizeOfChunk(sut, userPayloadParams);
                    checkConversionOfUserPayloadPointerToChunkHeader(sut);
                    checkConversionOfUserHeaderPointerToChunkHeader(sut);
                });
        }
    }
}

// END PARAMETERIZED TESTS FOR CHUNK HEADER

} // namespace
