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

#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::mepoo;
using namespace iox::roudi_env;
using namespace iox::testing;

class Chunk_test : public RouDi_GTest
{
  public:
    Chunk_test()
        : RouDi_GTest(MinimalIceoryxConfigBuilder().create())
    {
    }

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
    constexpr uint64_t USER_PAYLOAD_SIZE(42U);
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
    constexpr uint64_t USER_PAYLOAD_SIZE(42U);
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
    constexpr uint64_t USER_PAYLOAD_SIZE(42U);
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
    constexpr uint64_t USER_PAYLOAD_SIZE(42U);
    void* userPayload{nullptr};
    ASSERT_EQ(iox_pub_loan_chunk(publisher, &userPayload, USER_PAYLOAD_SIZE), AllocationResult_SUCCESS);

    auto chunkHeader = iox_chunk_header_from_user_payload(userPayload);
    auto userPayloadRoundtrip = iox_chunk_header_to_user_payload(chunkHeader);

    EXPECT_EQ(userPayloadRoundtrip, userPayload);
}

TEST_F(Chunk_test, GettingUserHeaderFromNonConstChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0df7284-a377-4c6a-b22b-454d3f7c7b88");
    constexpr uint64_t USER_PAYLOAD_SIZE(42U);
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
    constexpr uint64_t USER_PAYLOAD_SIZE(42U);
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

TEST_F(Chunk_test, GettingChunkHeaderToUserPayloadFromNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ebe5462-c8f4-4572-b396-ae66f223de2b");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_chunk_header_from_user_payload(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(Chunk_test, GettingChunkHeaderToUserPayloadConstFromNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0b27790-66eb-4f43-8f30-ec242508d7fd");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_chunk_header_to_user_payload_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(Chunk_test, GettingChunkHeaderToUserHeaderFromNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "375dae26-76ba-40b2-9c33-768aa33d135f");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_chunk_header_to_user_header(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(Chunk_test, GettingChunkHeaderToUserHeaderConstFromNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "96b7691e-d0bf-4cb4-bf4b-39784dc70e92");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_chunk_header_to_user_header_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(Chunk_test, GettingChunkHeaderFromUserPayloadFromNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "5ced7508-2ee6-4e2b-bf66-e60d8b4d968c");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_chunk_header_from_user_payload(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(Chunk_test, GettingChunkHeaderFromUserPayloadConstFromNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "8814d1c4-a5a9-4fa7-9520-507ca8745242");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_chunk_header_from_user_payload_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

} // namespace
