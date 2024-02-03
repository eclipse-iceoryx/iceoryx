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
#include "iceoryx_binding_c/response_header.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;

class iox_response_header_test : public Test
{
  public:
    void SetUp() override
    {
        payload = baseHeader->getUserPayload();
        constPayload = baseHeader->getUserPayload();
        sut = iox_response_header_from_payload(payload);
        sutConst = iox_response_header_from_payload_const(constPayload);
    }

    void TearDown() override
    {
        ++initialSequenceId;
    }

    static int64_t initialSequenceId;
    ChunkMock<int64_t, ResponseHeader> chunk;
    ResponseHeader* baseHeader{new (chunk.userHeader()) ResponseHeader(iox::UniqueId(), 0U, initialSequenceId)};
    void* payload = nullptr;
    const void* constPayload = nullptr;
    iox_response_header_t sut = nullptr;
    iox_const_response_header_t sutConst = nullptr;
};

int64_t iox_response_header_test::initialSequenceId = 9128;

TEST_F(iox_response_header_test, createResponseHeaderFromPayloadWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "58958618-6b9e-45cc-bea5-745091b6f3c5");
    ASSERT_THAT(sut, Ne(nullptr));
    ASSERT_THAT(sutConst, Ne(nullptr));

    EXPECT_THAT(sut, baseHeader);
    EXPECT_THAT(sutConst, baseHeader);
}

TEST_F(iox_response_header_test, getSequenceIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a6f9a297-7120-480c-b5e5-692608972d58");
    EXPECT_THAT(iox_response_header_get_sequence_id(sut), Eq(initialSequenceId));
    EXPECT_THAT(iox_response_header_get_sequence_id_const(sutConst), Eq(initialSequenceId));
}

TEST_F(iox_response_header_test, rpcHeaderVersionIsSetCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca6bb413-bdb0-46ad-bf4f-4a0b3925acf9");
    EXPECT_THAT(iox_response_header_get_rpc_header_version(sut), Eq(RpcBaseHeader::RPC_HEADER_VERSION));
    EXPECT_THAT(iox_response_header_get_rpc_header_version_const(sutConst), Eq(RpcBaseHeader::RPC_HEADER_VERSION));
}

TEST_F(iox_response_header_test, setServerErrorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4db6dbc9-a622-410b-897e-f13bc6b5d135");
    EXPECT_FALSE(iox_response_header_has_server_error(sut));
    EXPECT_FALSE(iox_response_header_has_server_error_const(sutConst));

    iox_response_header_set_server_error(sut);

    EXPECT_TRUE(iox_response_header_has_server_error(sut));
    EXPECT_TRUE(iox_response_header_has_server_error_const(sutConst));
}

TEST_F(iox_response_header_test, getUserPayloadWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "13d614bf-40e9-4209-890f-11c329acc9e6");
    EXPECT_THAT(iox_response_header_get_user_payload(sut), Eq(payload));
    EXPECT_THAT(iox_response_header_get_user_payload_const(sutConst), Eq(constPayload));
}

TEST_F(iox_response_header_test, getChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6629be80-eff7-4059-91a8-a8c1e0e23db4");
    EXPECT_THAT(reinterpret_cast<iox::mepoo::ChunkHeader*>(iox_response_header_get_chunk_header(sut)),
                Eq(baseHeader->getChunkHeader()));
    EXPECT_THAT(reinterpret_cast<const iox::mepoo::ChunkHeader*>(iox_response_header_get_chunk_header_const(sutConst)),
                Eq(baseHeader->getChunkHeader()));
}

TEST_F(iox_response_header_test, responseHeaderFromPayloadWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "01f6e7c4-7c96-436c-9f3e-756dd873a880");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_from_payload(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderFromPayloadConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "31aa2eb6-1629-4017-9c03-1dd21585d232");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_from_payload_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderHasServerErrorWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4bac8f1-7f66-43cc-ab10-39b7ad090582");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_has_server_error(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderSetServerErrorWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc0168fc-019b-4c78-be8e-a76b020ace3c");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_set_server_error(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderHasServerErrorConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "e3acb033-95af-4147-a2b1-ee91fe5702eb");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_has_server_error_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetRpcHeaderVersionWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "03213308-c7d5-4d12-bc98-e4db27125aca");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_rpc_header_version(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetRpcHeaderVersionConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "73cbe3f8-4a65-4900-b6a1-6accd6f46631");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_rpc_header_version_const(nullptr); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetSequenceIdWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6aef7857-00fe-444d-8c91-78173db08bd7");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_sequence_id(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetSequenceIdConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "2acc4ea5-45ed-448a-9ae2-0743b8e973d4");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_sequence_id_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetUserPayloadWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "b883c99b-cb38-48c2-86d1-7023b7d455bc");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_user_payload(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetUserPayloadConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "30d061cb-2ed4-40da-a44a-db981b178266");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_user_payload_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetChunkHeaderWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "089f6fc7-f364-495c-b61f-7218cfc2ea36");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_chunk_header(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_response_header_test, responseHeaderGetChunkHeaderConstWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "f21d6d60-1538-4121-a52e-b2940764dffb");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_response_header_get_chunk_header_const(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

} // namespace
