// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/rpc_header.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"

using namespace iox::popo;
using namespace iox::testing;

extern "C" {
#include "iceoryx_binding_c/request_header.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;

class iox_request_header_test : public Test
{
  public:
    void SetUp() override
    {
        payload = baseHeader->getUserPayload();
        constPayload = baseHeader->getUserPayload();
        sut = iox_request_header_from_payload(payload);
        sutConst = iox_request_header_from_payload_const(constPayload);
    }

    void TearDown() override
    {
    }

    ChunkMock<int64_t, RequestHeader> chunk;
    RequestHeader* baseHeader{new (chunk.userHeader()) RequestHeader(iox::UniqueId(), 0U)};
    void* payload = nullptr;
    const void* constPayload = nullptr;
    iox_request_header_t sut = nullptr;
    iox_const_request_header_t sutConst = nullptr;
};


TEST_F(iox_request_header_test, createRequestHeaderFromPayloadWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe54a86a-167c-442e-b64c-7bf5b6c590de");
    ASSERT_THAT(sut, Ne(nullptr));
    ASSERT_THAT(sutConst, Ne(nullptr));

    EXPECT_THAT(sut, baseHeader);
    EXPECT_THAT(sutConst, baseHeader);
}

TEST_F(iox_request_header_test, setSequenceIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "68a34b83-77aa-42c1-87ae-52c21aab809e");
    constexpr int64_t SOME_LUCKY_SEQUENCE_ID = 182673231;
    EXPECT_THAT(iox_request_header_get_sequence_id(sut), Eq(0U));
    EXPECT_THAT(iox_request_header_get_sequence_id_const(sutConst), Eq(0U));

    iox_request_header_set_sequence_id(sut, SOME_LUCKY_SEQUENCE_ID);

    EXPECT_THAT(iox_request_header_get_sequence_id(sut), Eq(SOME_LUCKY_SEQUENCE_ID));
    EXPECT_THAT(iox_request_header_get_sequence_id_const(sutConst), Eq(SOME_LUCKY_SEQUENCE_ID));
}

TEST_F(iox_request_header_test, rpcHeaderVersionIsSetCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "81424891-1f62-4d9f-8886-a8ba80d58e0b");
    EXPECT_THAT(iox_request_header_get_rpc_header_version(sut), Eq(RpcBaseHeader::RPC_HEADER_VERSION));
    EXPECT_THAT(iox_request_header_get_rpc_header_version_const(sutConst), Eq(RpcBaseHeader::RPC_HEADER_VERSION));
}

TEST_F(iox_request_header_test, getUserPayloadWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "29672d68-2d09-44a1-9461-81338146d59b");
    EXPECT_THAT(iox_request_header_get_user_payload(sut), Eq(payload));
    EXPECT_THAT(iox_request_header_get_user_payload_const(sutConst), Eq(constPayload));
}

TEST_F(iox_request_header_test, getChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "bc2ebd2e-ca45-4f93-b3f9-a843fdea6315");
    EXPECT_THAT(reinterpret_cast<iox::mepoo::ChunkHeader*>(iox_request_header_get_chunk_header(sut)),
                Eq(baseHeader->getChunkHeader()));
    EXPECT_THAT(reinterpret_cast<const iox::mepoo::ChunkHeader*>(iox_request_header_get_chunk_header_const(sutConst)),
                Eq(baseHeader->getChunkHeader()));
}

TEST_F(iox_request_header_test, requestHeaderFromPayloadWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "edc6ecf7-b530-4b0c-b4b8-67bf001ff433");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_from_payload(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderFromPayloadConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c643aab-53fa-4139-8840-2165b0560df7");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_from_payload_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderSetSequenceIdWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a270e15-8dd5-4ed7-a3dd-eeead6e3dbef");
    constexpr int64_t SOME_LUCKY_SEQUENCE_ID = 182673231;
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_set_sequence_id(nullptr, SOME_LUCKY_SEQUENCE_ID); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetRpcHeaderVersionWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "93362715-2db1-46a6-8ac1-aa109b2c885a");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_rpc_header_version(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetRpcHeaderVersionConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1960c937-13bd-43fc-9807-e9afc8f049fe");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_rpc_header_version_const(nullptr); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetSequenceIdWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b8ca05a-29be-4a4d-9636-b408829454c2");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_sequence_id(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetSequenceIdConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ffbb763e-2e2e-41e8-8193-96ce8ea2a8a8");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_sequence_id_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetUserPayloadWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b794a6a-6cfe-491d-86e8-1b0e6859497c");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_user_payload(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetUserPayloadConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb8f3020-b010-467c-91f1-6f684f3b36bb");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_user_payload_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetChunkHeaderWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "df2a439b-1746-4b95-895a-6c42085906c2");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_chunk_header(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_request_header_test, requestHeaderGetChunkHeaderConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "57fbe663-a746-45e8-a18d-4b8aa3bfc344");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_request_header_get_chunk_header_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

} // namespace
