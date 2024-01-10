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

#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/client_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox::popo;
using ::testing::_;

using TestUntypedClient = iox::popo::UntypedClientImpl<MockBaseClient>;

class UntypedClient_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

  protected:
    ChunkMock<uint64_t, RequestHeader> requestMock;
    ChunkMock<uint64_t, ResponseHeader> responseMock;

    ServiceDescription sd{"oh", "captain", "my captain"};
    static constexpr uint64_t RESPONSE_QUEUE_CAPACITY{123U};
    ClientOptions options{RESPONSE_QUEUE_CAPACITY};
    TestUntypedClient sut{sd, options};
};

TEST_F(UntypedClient_test, ConstructorForwardsArgumentsToBaseClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "9df896dd-90b2-4b8c-bf14-71c7ec9a467e");

    EXPECT_THAT(sut.serviceDescription, Eq(sd));
    EXPECT_THAT(sut.clientOptions, Eq(options));
}

TEST_F(UntypedClient_test, LoanCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "acb900fd-288f-4ef3-96b9-6843cd869893");

    constexpr uint64_t PAYLOAD_SIZE{8U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{32U};
    const iox::expected<RequestHeader*, AllocationError> allocateRequestResult =
        iox::ok<RequestHeader*>(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, allocateRequest(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT)).WillOnce(Return(allocateRequestResult));

    auto loanResult = sut.loan(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(loanResult.has_error());
    EXPECT_THAT(loanResult.value(), Eq(requestMock.sample()));
}

TEST_F(UntypedClient_test, LoanCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "905d8a67-1fde-4960-a95d-51c5ca4b6ed9");

    constexpr uint64_t PAYLOAD_SIZE{8U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{32U};
    constexpr AllocationError ALLOCATION_ERROR{AllocationError::RUNNING_OUT_OF_CHUNKS};
    const iox::expected<RequestHeader*, AllocationError> allocateRequestResult = iox::err(ALLOCATION_ERROR);

    EXPECT_CALL(sut.mockPort, allocateRequest(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT)).WillOnce(Return(allocateRequestResult));

    auto loanResult = sut.loan(PAYLOAD_SIZE, PAYLOAD_ALIGNMENT);
    ASSERT_TRUE(loanResult.has_error());
    EXPECT_THAT(loanResult.error(), Eq(ALLOCATION_ERROR));
}

TEST_F(UntypedClient_test, ReleaseRequestWithValidPayloadPointerCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c235e87-b790-4a21-abba-19a279d76431");

    EXPECT_CALL(sut.mockPort, releaseRequest(requestMock.userHeader())).Times(1);

    sut.releaseRequest(requestMock.sample());
}

TEST_F(UntypedClient_test, ReleaseRequestWithNullpointerDoesNotCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "f70434c9-ba47-4733-a9e1-824a8c85aac0");

    EXPECT_CALL(sut.mockPort, releaseRequest(_)).Times(0);

    sut.releaseRequest(nullptr);
}

TEST_F(UntypedClient_test, SendWithValidPayloadPointerCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "74d86b31-24a8-409e-8b85-7b9ec1c7ad3d");

    EXPECT_CALL(sut.mockPort, sendRequest(requestMock.userHeader())).WillOnce(Return(iox::ok()));

    sut.send(requestMock.sample())
        .and_then([&]() { GTEST_SUCCEED() << "Request successfully sent"; })
        .or_else([&](auto error) { GTEST_FAIL() << "Expected request to be sent but got error: " << error; });
}

TEST_F(UntypedClient_test, SendWithNullpointerDoesNotCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3b13638-a32d-48fd-a099-fa8516511ef8");

    EXPECT_CALL(sut.mockPort, sendRequest(_)).Times(0);

    sut.send(nullptr)
        .and_then([&]() { GTEST_FAIL() << "Expected request not successfully sent"; })
        .or_else([&](auto error) { EXPECT_THAT(error, Eq(ClientSendError::INVALID_REQUEST)); });
}

TEST_F(UntypedClient_test, TakeCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ca260e9-89bb-48aa-8504-0375e35eef9f");

    const iox::expected<const ResponseHeader*, ChunkReceiveResult> getResponseResult =
        iox::ok<const ResponseHeader*>(responseMock.userHeader());

    EXPECT_CALL(sut.mockPort, getResponse()).WillOnce(Return(getResponseResult));

    auto takeResult = sut.take();
    ASSERT_FALSE(takeResult.has_error());
    EXPECT_THAT(takeResult.value(), Eq(responseMock.sample()));
}

TEST_F(UntypedClient_test, TakeCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff524011-3a79-4960-9379-571e2eb87b16");

    constexpr ChunkReceiveResult CHUNK_RECEIVE_RESULT{ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL};
    const iox::expected<const ResponseHeader*, ChunkReceiveResult> getResponseResult = iox::err(CHUNK_RECEIVE_RESULT);

    EXPECT_CALL(sut.mockPort, getResponse()).WillOnce(Return(getResponseResult));

    auto takeResult = sut.take();
    ASSERT_TRUE(takeResult.has_error());
    EXPECT_THAT(takeResult.error(), Eq(CHUNK_RECEIVE_RESULT));
}

TEST_F(UntypedClient_test, ReleaseResponseWithValidPayloadPointerCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "7fd4433f-5894-430a-9c06-60ff39572920");

    EXPECT_CALL(sut.mockPort, releaseResponse(responseMock.userHeader())).Times(1);

    sut.releaseResponse(responseMock.sample());
}

TEST_F(UntypedClient_test, ReleaseResponseWithNullpointerDoesNotCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "969f3c0e-c30a-449c-a9d2-a63b6ed314ec");

    EXPECT_CALL(sut.mockPort, releaseResponse(_)).Times(0);

    sut.releaseResponse(nullptr);
}

} // namespace
