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
        ++headerVersion;
    }

    static int64_t initialSequenceId;
    static uint8_t headerVersion;
    ChunkMock<int64_t, RpcBaseHeader> chunk;
    RpcBaseHeader* baseHeader{new (chunk.userHeader())
                                  RpcBaseHeader(iox::cxx::UniqueId(), 0U, initialSequenceId, headerVersion)};
    void* payload = nullptr;
    const void* constPayload = nullptr;
    iox_response_header_t sut = nullptr;
    iox_const_response_header_t sutConst = nullptr;
};

int64_t iox_response_header_test::initialSequenceId = 9128;
uint8_t iox_response_header_test::headerVersion = 32U;

TEST_F(iox_response_header_test, createResponseHeaderFromPayloadWorks)
{
    ASSERT_THAT(sut, Ne(nullptr));
    ASSERT_THAT(sutConst, Ne(nullptr));

    EXPECT_THAT(sut, baseHeader);
    EXPECT_THAT(sutConst, baseHeader);
}

TEST_F(iox_response_header_test, getSequenceIdWorks)
{
    EXPECT_THAT(iox_response_header_get_sequence_id(sut), Eq(initialSequenceId));
    EXPECT_THAT(iox_response_header_get_sequence_id_const(sutConst), Eq(initialSequenceId));
}

TEST_F(iox_response_header_test, rpcHeaderVersionIsSetCorrectly)
{
    EXPECT_THAT(iox_response_header_get_rpc_header_version(sut), Eq(headerVersion));
    EXPECT_THAT(iox_response_header_get_rpc_header_version_const(sutConst), Eq(headerVersion));
}

TEST_F(iox_response_header_test, setServerErrorWorks)
{
    EXPECT_FALSE(iox_response_header_has_server_error(sut));
    EXPECT_FALSE(iox_response_header_has_server_error_const(sutConst));

    iox_response_header_set_server_error(sut);

    EXPECT_TRUE(iox_response_header_has_server_error(sut));
    EXPECT_TRUE(iox_response_header_has_server_error_const(sutConst));
}

TEST_F(iox_response_header_test, getUserPayloadWorks)
{
    EXPECT_THAT(iox_response_header_get_user_payload(sut), Eq(payload));
    EXPECT_THAT(iox_response_header_get_user_payload_const(sutConst), Eq(constPayload));
}

TEST_F(iox_response_header_test, getChunkHeaderWorks)
{
    EXPECT_THAT(reinterpret_cast<iox::mepoo::ChunkHeader*>(iox_response_header_get_chunk_header(sut)),
                Eq(baseHeader->getChunkHeader()));
    EXPECT_THAT(reinterpret_cast<const iox::mepoo::ChunkHeader*>(iox_response_header_get_chunk_header_const(sutConst)),
                Eq(baseHeader->getChunkHeader()));
}
} // namespace
