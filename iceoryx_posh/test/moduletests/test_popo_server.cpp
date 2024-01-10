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

#include "iceoryx_posh/popo/server.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/server_mock.hpp"

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

using TestServer = ServerImpl<DummyRequest, DummyResponse, MockBaseServer>;

class Server_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    static constexpr uint64_t PAYLOAD_SIZE{sizeof(DummyResponse)};
    static constexpr uint32_t PAYLOAD_ALIGNMENT{alignof(DummyResponse)};

    ChunkMock<DummyRequest, RequestHeader> requestMock;
    ChunkMock<DummyResponse, ResponseHeader> responseMock;

    ServiceDescription sd{"go", "go", "go"};
    static constexpr uint64_t REQUEST_QUEUE_CAPACITY{3U};
    ServerOptions options{REQUEST_QUEUE_CAPACITY};
    TestServer sut{sd, options};
};

TEST_F(Server_test, ConstructorForwardsArgumentsToBaseClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "6b98a782-dddf-4e17-99ac-90c7bc03fa7d");

    EXPECT_THAT(sut.serviceDescription, Eq(sd));
    EXPECT_THAT(sut.serverOptions, Eq(options));
}

TEST_F(Server_test, TakeCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "a8a76781-7599-4bb9-b3fc-1c9f06ae372b");

    const iox::expected<const RequestHeader*, ServerRequestResult> getRequestResult =
        iox::ok<const RequestHeader*>(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, getRequest()).WillOnce(Return(getRequestResult));

    const auto takeResult = sut.take();
    ASSERT_FALSE(takeResult.has_error());
    EXPECT_THAT(&takeResult.value().getRequestHeader(), Eq(requestMock.userHeader()));

    EXPECT_CALL(sut.mockPort, releaseRequest(requestMock.userHeader())).Times(1);
}

TEST_F(Server_test, TakeCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "a9049459-99b6-4567-b022-99299bd423b6");

    constexpr ServerRequestResult SERVER_REQUEST_RESULT{ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL};
    const iox::expected<const RequestHeader*, ServerRequestResult> getRequestResult = iox::err(SERVER_REQUEST_RESULT);

    EXPECT_CALL(sut.mockPort, getRequest()).WillOnce(Return(getRequestResult));

    auto takeResult = sut.take();
    ASSERT_TRUE(takeResult.has_error());
    EXPECT_THAT(takeResult.error(), Eq(SERVER_REQUEST_RESULT));
}

TEST_F(Server_test, LoanCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "926d394d-2a8f-486a-a422-28e424cf266a");

    const iox::expected<const RequestHeader*, ServerRequestResult> getRequestResult =
        iox::ok<const RequestHeader*>(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, getRequest()).WillOnce(Return(getRequestResult));
    auto takeResult = sut.take();
    ASSERT_FALSE(takeResult.has_error());
    const auto request = std::move(takeResult.value());

    const iox::expected<ResponseHeader*, AllocationError> allocateResponseResult =
        iox::ok<ResponseHeader*>(responseMock.userHeader());

    EXPECT_CALL(sut.mockPort, allocateResponse(requestMock.userHeader(), PAYLOAD_SIZE, PAYLOAD_ALIGNMENT))
        .WillOnce(Return(allocateResponseResult));

    auto loanResult = sut.loan(request);
    ASSERT_FALSE(loanResult.has_error());
    EXPECT_THAT(&loanResult.value().getResponseHeader(), Eq(responseMock.userHeader()));

    EXPECT_CALL(sut.mockPort, releaseRequest(requestMock.userHeader())).Times(1);
    EXPECT_CALL(sut.mockPort, releaseResponse(responseMock.userHeader())).Times(1);
}

TEST_F(Server_test, LoanCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "2302a61f-bc18-4cad-babb-9a4aeabf1cc7");

    const iox::expected<const RequestHeader*, ServerRequestResult> getRequestResult =
        iox::ok<const RequestHeader*>(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, getRequest()).WillOnce(Return(getRequestResult));
    auto takeResult = sut.take();
    ASSERT_FALSE(takeResult.has_error());
    const auto request = std::move(takeResult.value());

    constexpr AllocationError ALLOCATION_ERROR{AllocationError::RUNNING_OUT_OF_CHUNKS};
    const iox::expected<ResponseHeader*, AllocationError> allocateResponseResult = iox::err(ALLOCATION_ERROR);

    EXPECT_CALL(sut.mockPort, allocateResponse(requestMock.userHeader(), PAYLOAD_SIZE, PAYLOAD_ALIGNMENT))
        .WillOnce(Return(allocateResponseResult));

    auto loanResult = sut.loan(request);
    ASSERT_TRUE(loanResult.has_error());
    EXPECT_THAT(loanResult.error(), Eq(ALLOCATION_ERROR));

    EXPECT_CALL(sut.mockPort, releaseRequest(requestMock.userHeader())).Times(1);
}

TEST_F(Server_test, SendCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "535414bd-1846-4254-8fa1-78a296c185b5");

    const iox::expected<const RequestHeader*, ServerRequestResult> getRequestResult =
        iox::ok<const RequestHeader*>(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, getRequest()).WillOnce(Return(getRequestResult));
    auto takeResult = sut.take();
    ASSERT_FALSE(takeResult.has_error());
    const auto request = std::move(takeResult.value());

    const iox::expected<ResponseHeader*, AllocationError> allocateResponseResult =
        iox::ok<ResponseHeader*>(responseMock.userHeader());

    EXPECT_CALL(sut.mockPort, allocateResponse(requestMock.userHeader(), PAYLOAD_SIZE, PAYLOAD_ALIGNMENT))
        .WillOnce(Return(allocateResponseResult));

    auto loanResult = sut.loan(request);
    auto response = std::move(loanResult.value());

    EXPECT_CALL(sut.mockPort, sendResponse(responseMock.userHeader())).WillOnce(Return(iox::ok()));

    sut.send(std::move(response))
        .and_then([&]() { GTEST_SUCCEED() << "Response successfully sent"; })
        .or_else([&](auto error) {
            GTEST_FAIL() << "Expected response to be sent but got error: " << static_cast<uint64_t>(error);
        });

    EXPECT_CALL(sut.mockPort, releaseRequest(requestMock.userHeader())).Times(1);
}

} // namespace
