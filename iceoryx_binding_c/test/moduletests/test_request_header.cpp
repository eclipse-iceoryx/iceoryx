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
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"

using namespace iox::popo;

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
    RequestHeader* baseHeader{new (chunk.userHeader()) RequestHeader(iox::cxx::UniqueId(), 0U)};
    void* payload = nullptr;
    const void* constPayload = nullptr;
    iox_request_header_t sut = nullptr;
    iox_const_request_header_t sutConst = nullptr;
};


TEST_F(iox_request_header_test, createRequestHeaderFromPayloadWorks)
{
    ASSERT_THAT(sut, Ne(nullptr));
    ASSERT_THAT(sutConst, Ne(nullptr));

    EXPECT_THAT(sut, baseHeader);
    EXPECT_THAT(sutConst, baseHeader);
}

TEST_F(iox_request_header_test, setSequenceIdWorks)
{
    constexpr int64_t SOME_LUCKY_SEQUENCE_ID = 182673231;
    EXPECT_THAT(iox_request_header_get_sequence_id(sut), Eq(0U));
    EXPECT_THAT(iox_request_header_get_sequence_id_const(sutConst), Eq(0U));

    iox_request_header_set_sequence_id(sut, SOME_LUCKY_SEQUENCE_ID);

    EXPECT_THAT(iox_request_header_get_sequence_id(sut), Eq(SOME_LUCKY_SEQUENCE_ID));
    EXPECT_THAT(iox_request_header_get_sequence_id_const(sutConst), Eq(SOME_LUCKY_SEQUENCE_ID));
}

TEST_F(iox_request_header_test, rpcHeaderVersionIsSetCorrectly)
{
    EXPECT_THAT(iox_request_header_get_rpc_header_version(sut), Eq(RpcBaseHeader::RPC_HEADER_VERSION));
    EXPECT_THAT(iox_request_header_get_rpc_header_version_const(sutConst), Eq(RpcBaseHeader::RPC_HEADER_VERSION));
}

TEST_F(iox_request_header_test, getUserPayloadWorks)
{
    EXPECT_THAT(iox_request_header_get_user_payload(sut), Eq(payload));
    EXPECT_THAT(iox_request_header_get_user_payload_const(sutConst), Eq(constPayload));
}

TEST_F(iox_request_header_test, getChunkHeaderWorks)
{
    EXPECT_THAT(reinterpret_cast<iox::mepoo::ChunkHeader*>(iox_request_header_get_chunk_header(sut)),
                Eq(baseHeader->getChunkHeader()));
    EXPECT_THAT(reinterpret_cast<const iox::mepoo::ChunkHeader*>(iox_request_header_get_chunk_header_const(sutConst)),
                Eq(baseHeader->getChunkHeader()));
}
} // namespace
