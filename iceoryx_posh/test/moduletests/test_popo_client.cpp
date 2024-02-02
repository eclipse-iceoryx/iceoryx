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

#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/client_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox::popo;
using ::testing::_;

struct DummyRequest
{
    uint64_t data{0};
};
struct DummyResponse
{
    uint64_t data{0};
};

using TestClient = ClientImpl<DummyRequest, DummyResponse, MockBaseClient>;

class Client_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    static constexpr uint64_t PAYLOAD_SIZE{sizeof(DummyRequest)};
    static constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(DummyRequest)};

    ChunkMock<DummyRequest, RequestHeader> requestMock;
    ChunkMock<DummyResponse, ResponseHeader> responseMock;

    ServiceDescription sd{"a one", "a two", "a three"};
    static constexpr uint64_t RESPONSE_QUEUE_CAPACITY{123U};
    ClientOptions options{RESPONSE_QUEUE_CAPACITY};
    TestClient sut{sd, options};
};

TEST_F(Client_test, ConstructorForwardsArgumentsToBaseClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1795dae-f85d-4755-8ff2-d58e3f1a0b95");

    EXPECT_THAT(sut.serviceDescription, Eq(sd));
    EXPECT_THAT(sut.clientOptions, Eq(options));
}

TEST_F(Client_test, LoanCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bbddee9-49c9-4183-8120-224067260151");

    const iox::expected<RequestHeader*, AllocationError> allocateRequestResult = iox::ok(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, allocateRequest(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT)).WillOnce(Return(allocateRequestResult));

    auto loanResult = sut.loan();
    ASSERT_FALSE(loanResult.has_error());
    EXPECT_THAT(&loanResult.value().getRequestHeader(), Eq(requestMock.userHeader()));

    EXPECT_CALL(sut.mockPort, releaseRequest(requestMock.userHeader())).Times(1);
}

TEST_F(Client_test, LoanCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "9029b641-09d9-4f47-8627-63fc0c45d7ef");

    constexpr AllocationError ALLOCATION_ERROR{AllocationError::RUNNING_OUT_OF_CHUNKS};
    const iox::expected<RequestHeader*, AllocationError> allocateRequestResult = iox::err(ALLOCATION_ERROR);

    EXPECT_CALL(sut.mockPort, allocateRequest(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT)).WillOnce(Return(allocateRequestResult));

    auto loanResult = sut.loan();
    ASSERT_TRUE(loanResult.has_error());
    EXPECT_THAT(loanResult.error(), Eq(ALLOCATION_ERROR));
}

TEST_F(Client_test, SendCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "92d23ecc-5c37-4254-bb09-ccc0e0e7e0fa");

    const iox::expected<RequestHeader*, AllocationError> allocateRequestResult = iox::ok(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, allocateRequest(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT)).WillOnce(Return(allocateRequestResult));

    auto loanResult = sut.loan();
    ASSERT_FALSE(loanResult.has_error());

    auto request = std::move(loanResult.value());

    EXPECT_CALL(sut.mockPort, sendRequest(requestMock.userHeader())).WillOnce(Return(iox::ok()));

    sut.send(std::move(request))
        .and_then([&]() { GTEST_SUCCEED() << "Request successfully sent"; })
        .or_else([&](auto error) {
            GTEST_FAIL() << "Expected request to be sent but got error: " << static_cast<uint64_t>(error);
        });
}

TEST_F(Client_test, TakeCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "688ed3d9-4292-4cde-81e3-8c2b4d6e3a5f");

    const iox::expected<const ResponseHeader*, ChunkReceiveResult> getResponseResult =
        iox::ok<const ResponseHeader*>(responseMock.userHeader());

    EXPECT_CALL(sut.mockPort, getResponse()).WillOnce(Return(getResponseResult));

    const auto takeResult = sut.take();
    ASSERT_FALSE(takeResult.has_error());
    EXPECT_THAT(&takeResult.value().getResponseHeader(), Eq(responseMock.userHeader()));

    EXPECT_CALL(sut.mockPort, releaseResponse(responseMock.userHeader())).Times(1);
}

TEST_F(Client_test, TakeCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "1aac50ca-5455-4579-9e69-769d569e2b4b");

    constexpr ChunkReceiveResult CHUNK_RECEIVE_RESULT{ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL};
    const iox::expected<const ResponseHeader*, ChunkReceiveResult> getResponseResult = iox::err(CHUNK_RECEIVE_RESULT);

    EXPECT_CALL(sut.mockPort, getResponse()).WillOnce(Return(getResponseResult));

    auto takeResult = sut.take();
    ASSERT_TRUE(takeResult.has_error());
    EXPECT_THAT(takeResult.error(), Eq(CHUNK_RECEIVE_RESULT));
}

} // namespace
