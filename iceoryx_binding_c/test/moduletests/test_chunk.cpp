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

extern "C" {
#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/runtime.h"
}

#include "iceoryx_posh/testing/roudi_gtest.hpp"

namespace
{
using namespace iox;
using namespace iox::mepoo;


class Chunk_test : public RouDi_GTest
{
  public:
    void SetUp() override
    {
        iox_runtime_init("hypnotoad");
        publisher = iox_pub_init(&publisherStorage, "All", "Glory", "Hypnotoad", nullptr);
        ASSERT_NE(publisher, nullptr);
    }

    void TearDown() override
    {
        iox_pub_deinit(publisher);
    }

    iox_pub_storage_t publisherStorage;
    iox_pub_t publisher{nullptr};
};

TEST_F(Chunk_test, GettingChunkHeaderFromNonConstUserPayloadWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a044b28d-ad7e-45ed-a0e2-e431ef1eacf0");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_chunk(publisher, &userPayload, USER_PAYLOAD_SIZE), AllocationResult_SUCCESS);

    auto chunkHeader = iox_chunk_header_from_user_payload(userPayload);
    ASSERT_NE(chunkHeader, nullptr);

    // a default created ChunkHeader has always an adjacent user-payload
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(chunkHeader)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(userPayload)};
    EXPECT_THAT(userPayloadStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST_F(Chunk_test, GettingChunkHeaderFromConstUserPayloadWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f7bb07a-f0dd-4b58-af84-5daec365d9e2");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_chunk(publisher, &userPayload, USER_PAYLOAD_SIZE), AllocationResult_SUCCESS);
    const void* constUserPayload = userPayload;

    auto chunkHeader = iox_chunk_header_from_user_payload_const(constUserPayload);
    ASSERT_NE(chunkHeader, nullptr);

    // a default created ChunkHeader has always an adjacent user-payload
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(chunkHeader)};
    const uint64_t userPayloadStartAddress{reinterpret_cast<uint64_t>(userPayload)};
    EXPECT_THAT(userPayloadStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST_F(Chunk_test, UserPayloadChunkHeaderUserPayloadRoundtripWorksForNonConst)
{
    ::testing::Test::RecordProperty("TEST_ID", "ea220aac-4d7d-41c2-92ea-7f929b824555");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_chunk(publisher, &userPayload, USER_PAYLOAD_SIZE), AllocationResult_SUCCESS);
    const void* constUserPayload = userPayload;

    auto chunkHeader = iox_chunk_header_from_user_payload_const(constUserPayload);
    auto userPayloadRoundtrip = iox_chunk_header_to_user_payload_const(chunkHeader);

    EXPECT_EQ(userPayloadRoundtrip, userPayload);
}

TEST_F(Chunk_test, UserPayloadChunkHeaderUserPayloadRoundtripWorksForConst)
{
    ::testing::Test::RecordProperty("TEST_ID", "e094616d-6d99-4b7f-a619-dd98ec7d1e44");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_chunk(publisher, &userPayload, USER_PAYLOAD_SIZE), AllocationResult_SUCCESS);

    auto chunkHeader = iox_chunk_header_from_user_payload(userPayload);
    auto userPayloadRoundtrip = iox_chunk_header_to_user_payload(chunkHeader);

    EXPECT_EQ(userPayloadRoundtrip, userPayload);
}

TEST_F(Chunk_test, GettingUserHeaderFromNonConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0df7284-a377-4c6a-b22b-454d3f7c7b88");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT(64U);
    constexpr uint32_t USER_HEADER_SIZE = 16U;
    constexpr uint32_t USER_HEADER_ALIGNMENT = 8U;
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_aligned_chunk_with_user_header(publisher,
                                                          &userPayload,
                                                          USER_PAYLOAD_SIZE,
                                                          USER_PAYLOAD_ALIGNMENT,
                                                          USER_HEADER_SIZE,
                                                          USER_HEADER_ALIGNMENT),
              AllocationResult_SUCCESS);

    auto chunkHeader = iox_chunk_header_from_user_payload(userPayload);

    auto userHeader = iox_chunk_header_to_user_header(chunkHeader);

    // a default created ChunkHeader has always an adjacent user-payload
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(chunkHeader)};
    const uint64_t userHeaderStartAddress{reinterpret_cast<uint64_t>(userHeader)};
    EXPECT_THAT(userHeaderStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST_F(Chunk_test, GettingUserHeaderFromConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "77f4a193-7f44-43ce-8bd8-f9916b8d83dd");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT(64U);
    constexpr uint32_t USER_HEADER_SIZE = 16U;
    constexpr uint32_t USER_HEADER_ALIGNMENT = 8U;
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_aligned_chunk_with_user_header(publisher,
                                                          &userPayload,
                                                          USER_PAYLOAD_SIZE,
                                                          USER_PAYLOAD_ALIGNMENT,
                                                          USER_HEADER_SIZE,
                                                          USER_HEADER_ALIGNMENT),
              AllocationResult_SUCCESS);

    const iox_chunk_header_t* chunkHeader = iox_chunk_header_from_user_payload(userPayload);

    auto userHeader = iox_chunk_header_to_user_header_const(chunkHeader);

    // a default created ChunkHeader has always an adjacent user-payload
    const uint64_t chunkStartAddress{reinterpret_cast<uint64_t>(chunkHeader)};
    const uint64_t userHeaderStartAddress{reinterpret_cast<uint64_t>(userHeader)};
    EXPECT_THAT(userHeaderStartAddress - chunkStartAddress, Eq(sizeof(ChunkHeader)));
}

TEST_F(Chunk_test, GettingHeaderPropertiesFromConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d561c7e-ae29-11ec-b909-0242ac120002");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT(64U);
    constexpr uint32_t USER_HEADER_SIZE = 16U;
    constexpr uint32_t USER_HEADER_ALIGNMENT = 8U;
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_aligned_chunk_with_user_header(publisher,
                                                          &userPayload,
                                                          USER_PAYLOAD_SIZE,
                                                          USER_PAYLOAD_ALIGNMENT,
                                                          USER_HEADER_SIZE,
                                                          USER_HEADER_ALIGNMENT),
              AllocationResult_SUCCESS);

    const iox_chunk_header_t* chunkHeader = iox_chunk_header_from_user_payload(userPayload);

    ASSERT_EQ(iox_chunk_header_user_chunk_size(chunkHeader), USER_HEADER_SIZE + USER_PAYLOAD_SIZE);
    ASSERT_EQ(iox_chunk_header_user_header_size(chunkHeader), USER_HEADER_SIZE);
    ASSERT_EQ(iox_chunk_header_user_payload_size(chunkHeader), USER_PAYLOAD_SIZE);
    ASSERT_EQ(iox_chunk_header_user_payload_alignment(chunkHeader), USER_PAYLOAD_ALIGNMENT);
}

TEST_F(Chunk_test, GettingSequenceNumberFromConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fd564011-77a7-494a-93ae-f5b3f895cf95");
    constexpr uint32_t USER_PAYLOAD_SIZE(42U);
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT(64U);
    constexpr uint32_t USER_HEADER_SIZE = 16U;
    constexpr uint32_t USER_HEADER_ALIGNMENT = 8U;

    void* userPayload1{nullptr};
    ASSERT_EQ(iox_pub_loan_aligned_chunk_with_user_header(publisher,
                                                          &userPayload1,
                                                          USER_PAYLOAD_SIZE,
                                                          USER_PAYLOAD_ALIGNMENT,
                                                          USER_HEADER_SIZE,
                                                          USER_HEADER_ALIGNMENT),
              AllocationResult_SUCCESS);
    const iox_chunk_header_t* chunkHeader1 = iox_chunk_header_from_user_payload(userPayload1);

    void* userPayload2{nullptr};
    ASSERT_EQ(iox_pub_loan_aligned_chunk_with_user_header(publisher,
                                                          &userPayload2,
                                                          USER_PAYLOAD_SIZE,
                                                          USER_PAYLOAD_ALIGNMENT,
                                                          USER_HEADER_SIZE,
                                                          USER_HEADER_ALIGNMENT),
              AllocationResult_SUCCESS);

    const iox_chunk_header_t* chunkHeader2 = iox_chunk_header_from_user_payload(userPayload2);

    ASSERT_GT(iox_chunk_header_sequence_number(chunkHeader2), iox_chunk_header_sequence_number(chunkHeader1));
}

} // namespace
