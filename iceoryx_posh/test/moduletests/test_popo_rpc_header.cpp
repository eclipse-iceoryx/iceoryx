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

#include "iceoryx_posh/popo/rpc_header.hpp"

#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;

class RpcBaseHeaderAccess : public RpcBaseHeader
{
  public:
    using RpcBaseHeader::m_clientQueueUniquePortId;
    using RpcBaseHeader::m_lastKnownClientQueueIndex;
};

class RpcBaseHeader_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    ChunkMock<bool, RpcBaseHeader> chunk;
    static constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{73};
    static constexpr int64_t SEQUENCE_ID{37};
    RpcBaseHeader* sut{new (chunk.userHeader()) RpcBaseHeader(
        iox::UniquePortId(), LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID, RpcBaseHeader::RPC_HEADER_VERSION)};
};

void checkRpcBaseHeader(const RpcBaseHeaderAccess* sut,
                        const iox::UniquePortId& clientQueueUniquePortId,
                        const uint32_t lastKnownClientQueueIndex,
                        const int64_t sequenceId,
                        const uint8_t rpcHeaderVersion)
{
    EXPECT_THAT(sut->getRpcHeaderVersion(), rpcHeaderVersion);
    EXPECT_THAT(sut->m_clientQueueUniquePortId, Eq(clientQueueUniquePortId));
    EXPECT_THAT(sut->m_lastKnownClientQueueIndex, lastKnownClientQueueIndex);
    EXPECT_THAT(sut->getSequenceId(), Eq(sequenceId));
}

TEST_F(RpcBaseHeader_test, ConstructorWorks)
{
    const iox::UniquePortId clientQueueUniquePortId;
    constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{13};
    constexpr int64_t SEQUENCE_ID{42};
    constexpr uint8_t RPC_HEADER_VERSION{222};

    ChunkMock<bool, RpcBaseHeader> chunk;
    new (chunk.userHeader())
        RpcBaseHeader(clientQueueUniquePortId, LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID, RPC_HEADER_VERSION);

    checkRpcBaseHeader(static_cast<RpcBaseHeaderAccess*>(chunk.userHeader()),
                       clientQueueUniquePortId,
                       LAST_KNOWN_CLIENT_QUEUE_INDEX,
                       SEQUENCE_ID,
                       RPC_HEADER_VERSION);
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionFromNonConstContextWorks)
{
    EXPECT_THAT(static_cast<RpcBaseHeader*>(sut)->getChunkHeader(), Eq(chunk.chunkHeader()));
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionFromConstContextWorks)
{
    EXPECT_THAT(static_cast<const RpcBaseHeader*>(sut)->getChunkHeader(), Eq(chunk.chunkHeader()));
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionCalledFromNonConstContextReturnsNonConstType)
{
    auto isNonConstReturn =
        std::is_same<decltype(std::declval<RpcBaseHeader>().getChunkHeader()), iox::mepoo::ChunkHeader*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionCalledFromConstContextReturnsConstType)
{
    auto isConstReturn = std::is_same<decltype(std::declval<const RpcBaseHeader>().getChunkHeader()),
                                      const iox::mepoo::ChunkHeader*>::value;
    EXPECT_TRUE(isConstReturn);
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionFromNonConstContextWorks)
{
    EXPECT_THAT(static_cast<RpcBaseHeader*>(sut)->getUserPayload(), Eq(chunk.chunkHeader()->userPayload()));
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionFromConstContextWorks)
{
    EXPECT_THAT(static_cast<const RpcBaseHeader*>(sut)->getUserPayload(), Eq(chunk.chunkHeader()->userPayload()));
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionCalledFromNonConstContextReturnsNonConstType)
{
    auto isNonConstReturn = std::is_same<decltype(std::declval<RpcBaseHeader>().getUserPayload()), void*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionCalledFromConstContextReturnsConstType)
{
    auto isConstReturn =
        std::is_same<decltype(std::declval<const RpcBaseHeader>().getUserPayload()), const void*>::value;
    EXPECT_TRUE(isConstReturn);
}

class RequestHeader_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    ChunkMock<bool, RequestHeader> chunk;
    static constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{7};
    RequestHeader* sut{new (chunk.userHeader()) RequestHeader(iox::UniquePortId(), LAST_KNOWN_CLIENT_QUEUE_INDEX)};
};

TEST_F(RequestHeader_test, ConstructorWorks)
{
    const iox::UniquePortId clientQueueUniquePortId;
    constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{13};
    constexpr int64_t EXPECTED_SEQUENCE_ID{0};
    constexpr uint8_t EXPECTED_RPC_HEADER_VERSION{RpcBaseHeader::RPC_HEADER_VERSION};

    ChunkMock<bool, RequestHeader> chunk;
    auto requestHeader = new (chunk.userHeader()) RequestHeader(clientQueueUniquePortId, LAST_KNOWN_CLIENT_QUEUE_INDEX);

    checkRpcBaseHeader(reinterpret_cast<RpcBaseHeaderAccess*>(requestHeader),
                       clientQueueUniquePortId,
                       LAST_KNOWN_CLIENT_QUEUE_INDEX,
                       EXPECTED_SEQUENCE_ID,
                       EXPECTED_RPC_HEADER_VERSION);

    EXPECT_THAT(requestHeader->isFireAndForget(), Eq(false));
}

TEST_F(RequestHeader_test, SetSequenceIdWorks)
{
    constexpr int64_t SEQUENCE_ID{666};

    sut->setSequenceId(SEQUENCE_ID);

    EXPECT_THAT(sut->getSequenceId(), Eq(SEQUENCE_ID));
}

TEST_F(RequestHeader_test, SetFireAndForgetWorks)
{
    sut->setFireAndForget();

    EXPECT_THAT(sut->isFireAndForget(), Eq(true));
}

class ResponseHeader_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    ChunkMock<bool, ResponseHeader> chunk;
    static constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{13};
    static constexpr int64_t SEQUENCE_ID{1111};
    ResponseHeader* sut{new (chunk.userHeader())
                            ResponseHeader(iox::UniquePortId(), LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID)};
};

TEST_F(ResponseHeader_test, ConstructorWorks)
{
    const iox::UniquePortId clientQueueUniquePortId;
    constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{17};
    constexpr int64_t SEQUENCE_ID{555};
    constexpr uint8_t EXPECTED_RPC_HEADER_VERSION{RpcBaseHeader::RPC_HEADER_VERSION};

    ChunkMock<bool, ResponseHeader> chunk;
    auto responseHeader =
        new (chunk.userHeader()) ResponseHeader(clientQueueUniquePortId, LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID);

    checkRpcBaseHeader(reinterpret_cast<RpcBaseHeaderAccess*>(responseHeader),
                       clientQueueUniquePortId,
                       LAST_KNOWN_CLIENT_QUEUE_INDEX,
                       SEQUENCE_ID,
                       EXPECTED_RPC_HEADER_VERSION);

    EXPECT_THAT(responseHeader->hasServerError(), Eq(false));
}

TEST_F(ResponseHeader_test, SetServerErrorWorks)
{
    sut->setServerError();

    EXPECT_THAT(sut->hasServerError(), Eq(true));
}

} // namespace
