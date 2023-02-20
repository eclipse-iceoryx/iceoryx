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

using iox::UniqueId;

class RpcBaseHeaderAccess : public RpcBaseHeader
{
  public:
    using RpcBaseHeader::m_lastKnownClientQueueIndex;
    using RpcBaseHeader::m_uniqueClientQueueId;
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
        UniqueId(), LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID, RpcBaseHeader::RPC_HEADER_VERSION)};
};

void checkRpcBaseHeader(const RpcBaseHeaderAccess* sut,
                        const UniqueId& uniqueClientQueueId,
                        const uint32_t lastKnownClientQueueIndex,
                        const int64_t sequenceId,
                        const uint8_t rpcHeaderVersion)
{
    EXPECT_THAT(sut->getRpcHeaderVersion(), rpcHeaderVersion);
    EXPECT_THAT(sut->m_uniqueClientQueueId, Eq(uniqueClientQueueId));
    EXPECT_THAT(sut->m_lastKnownClientQueueIndex, lastKnownClientQueueIndex);
    EXPECT_THAT(sut->getSequenceId(), Eq(sequenceId));
}

TEST_F(RpcBaseHeader_test, ConstructorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "54b62ac7-30a7-424b-b149-8255afbf0a0d");
    const UniqueId uniqueClientQueueId;
    constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{13};
    constexpr int64_t SEQUENCE_ID{42};
    constexpr uint8_t RPC_HEADER_VERSION{222};

    ChunkMock<bool, RpcBaseHeader> chunk;
    new (chunk.userHeader())
        RpcBaseHeader(uniqueClientQueueId, LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID, RPC_HEADER_VERSION);

    checkRpcBaseHeader(static_cast<RpcBaseHeaderAccess*>(chunk.userHeader()),
                       uniqueClientQueueId,
                       LAST_KNOWN_CLIENT_QUEUE_INDEX,
                       SEQUENCE_ID,
                       RPC_HEADER_VERSION);
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionFromNonConstContextWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c58aa0ac-8897-4ac5-a2aa-53999902f504");
    EXPECT_THAT(sut->getChunkHeader(), Eq(chunk.chunkHeader()));
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionFromConstContextWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6fa9caf1-7ebb-4995-a684-4416d6644b7e");
    EXPECT_THAT(static_cast<const RpcBaseHeader*>(sut)->getChunkHeader(), Eq(chunk.chunkHeader()));
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionCalledFromNonConstContextReturnsNonConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "3105ac6e-62cd-4655-a6d8-b70593a77c60");
    auto isNonConstReturn =
        std::is_same<decltype(std::declval<RpcBaseHeader>().getChunkHeader()), iox::mepoo::ChunkHeader*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST_F(RpcBaseHeader_test, GetChunkHeaderFunctionCalledFromConstContextReturnsConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "36e1e4fc-ac81-4fd7-95ff-38afa391a3da");
    auto isConstReturn = std::is_same<decltype(std::declval<const RpcBaseHeader>().getChunkHeader()),
                                      const iox::mepoo::ChunkHeader*>::value;
    EXPECT_TRUE(isConstReturn);
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionFromNonConstContextWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ee7b88e-8fc1-4b6b-a84b-f89c9480855e");
    EXPECT_THAT(sut->getUserPayload(), Eq(chunk.chunkHeader()->userPayload()));
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionFromConstContextWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ac0611a-f4c8-414e-bab2-fc6a41a68f9c");
    EXPECT_THAT(static_cast<const RpcBaseHeader*>(sut)->getUserPayload(), Eq(chunk.chunkHeader()->userPayload()));
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionCalledFromNonConstContextReturnsNonConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b815d45-1dc2-44f1-9baf-013d8e76e5ca");
    auto isNonConstReturn = std::is_same<decltype(std::declval<RpcBaseHeader>().getUserPayload()), void*>::value;
    EXPECT_TRUE(isNonConstReturn);
}

TEST_F(RpcBaseHeader_test, GetUserPayloadFunctionCalledFromConstContextReturnsConstType)
{
    ::testing::Test::RecordProperty("TEST_ID", "06c91e8c-7495-40da-88ed-c201f3cf8da1");
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
    RequestHeader* sut{new (chunk.userHeader()) RequestHeader(UniqueId(), LAST_KNOWN_CLIENT_QUEUE_INDEX)};
};

TEST_F(RequestHeader_test, ConstructorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4af7c64c-5f9f-4598-b405-567658e128db");
    const UniqueId uniqueClientQueueId;
    constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{13};
    constexpr int64_t EXPECTED_SEQUENCE_ID{0};
    constexpr uint8_t EXPECTED_RPC_HEADER_VERSION{RpcBaseHeader::RPC_HEADER_VERSION};

    ChunkMock<bool, RequestHeader> chunk;
    auto requestHeader = new (chunk.userHeader()) RequestHeader(uniqueClientQueueId, LAST_KNOWN_CLIENT_QUEUE_INDEX);

    checkRpcBaseHeader(reinterpret_cast<RpcBaseHeaderAccess*>(requestHeader),
                       uniqueClientQueueId,
                       LAST_KNOWN_CLIENT_QUEUE_INDEX,
                       EXPECTED_SEQUENCE_ID,
                       EXPECTED_RPC_HEADER_VERSION);
}

TEST_F(RequestHeader_test, SetSequenceIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fde17d21-33b9-4c23-a482-9bce99b8c346");
    constexpr int64_t SEQUENCE_ID{666};

    sut->setSequenceId(SEQUENCE_ID);

    EXPECT_THAT(sut->getSequenceId(), Eq(SEQUENCE_ID));
}

TEST_F(RequestHeader_test, GetRequestHeaderFromPayloadWithNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b67e56a-7245-48f3-8de6-f4a9e8f30b8e");
    void* payloadPointer{nullptr};
    auto requestHeader = RequestHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(nullptr));
}

TEST_F(RequestHeader_test, GetRequestHeaderFromConstPayloadWithNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6ce3b3b-226f-4286-a8db-63feed5ef882");
    const void* payloadPointer{nullptr};
    auto requestHeader = RequestHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(nullptr));
}

TEST_F(RequestHeader_test, GetRequestHeaderFromPayloadWithNonNullptrReturnsRequestHeaderPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7ddff21-4f6f-4688-a35d-d43296876e82");
    void* payloadPointer{sut->getUserPayload()};
    auto requestHeader = RequestHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(sut));
}

TEST_F(RequestHeader_test, GetRequestHeaderFromConstPayloadNonNullptrReturnsRequestHeaderPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "5cf198ca-d345-446b-8c3e-3deec5799573");
    const void* payloadPointer{sut->getUserPayload()};
    auto requestHeader = RequestHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(sut));
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
                            ResponseHeader(UniqueId(), LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID)};
};

TEST_F(ResponseHeader_test, ConstructorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec3d90c3-2126-420c-a31c-f1c6a0731791");
    const UniqueId uniqueClientQueueId;
    constexpr uint32_t LAST_KNOWN_CLIENT_QUEUE_INDEX{17};
    constexpr int64_t SEQUENCE_ID{555};
    constexpr uint8_t EXPECTED_RPC_HEADER_VERSION{RpcBaseHeader::RPC_HEADER_VERSION};

    ChunkMock<bool, ResponseHeader> chunk;
    auto responseHeader =
        new (chunk.userHeader()) ResponseHeader(uniqueClientQueueId, LAST_KNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID);

    checkRpcBaseHeader(reinterpret_cast<RpcBaseHeaderAccess*>(responseHeader),
                       uniqueClientQueueId,
                       LAST_KNOWN_CLIENT_QUEUE_INDEX,
                       SEQUENCE_ID,
                       EXPECTED_RPC_HEADER_VERSION);

    EXPECT_THAT(responseHeader->hasServerError(), Eq(false));
}

TEST_F(ResponseHeader_test, SetServerErrorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b455d8dc-2349-4618-b73f-4567c70b616a");
    sut->setServerError();

    EXPECT_THAT(sut->hasServerError(), Eq(true));
}

TEST_F(ResponseHeader_test, GetResponseHeaderFromPayloadWithNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "564a2240-1bc9-4d94-b1ba-0b75d6db3df6");
    void* payloadPointer{nullptr};
    auto requestHeader = ResponseHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(nullptr));
}

TEST_F(ResponseHeader_test, GetResponseHeaderFromConstPayloadWithNullptrReturnsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "656d7937-6276-4de4-ba82-2db90524951e");
    const void* payloadPointer{nullptr};
    auto requestHeader = ResponseHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(nullptr));
}

TEST_F(ResponseHeader_test, GetResponseHeaderFromPayloadWithNonNullptrReturnsRequestHeaderPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "4170f552-a90a-412d-8cbd-217d9ca989ce");
    void* payloadPointer{sut->getUserPayload()};
    auto requestHeader = ResponseHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(sut));
}

TEST_F(ResponseHeader_test, GetResponseHeaderFromConstPayloadNonNullptrReturnsRequestHeaderPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "81de8904-6aaa-4390-a132-881f963a0ede");
    const void* payloadPointer{sut->getUserPayload()};
    auto requestHeader = ResponseHeader::fromPayload(payloadPointer);

    EXPECT_THAT(requestHeader, Eq(sut));
}

} // namespace
