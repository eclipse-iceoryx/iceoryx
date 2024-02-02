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

#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "mocks/server_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox::popo;
using ::testing::_;

using TestUntypedServer = iox::popo::UntypedServerImpl<MockBaseServer>;

class UntypedServer_test : public Test
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

    ServiceDescription sd{"shores", "ring", "bell"};
    static constexpr uint64_t REQUEST_QUEUE_CAPACITY{7U};
    ServerOptions options{REQUEST_QUEUE_CAPACITY};
    TestUntypedServer sut{sd, options};
};

TEST_F(UntypedServer_test, ConstructorForwardsArgumentsToBaseClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "04b0ec2e-0eb7-4dd4-a3dd-c73c1705ca1a");

    EXPECT_THAT(sut.serviceDescription, Eq(sd));
    EXPECT_THAT(sut.serverOptions, Eq(options));
}

TEST_F(UntypedServer_test, TakeCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "0bcaf64f-66d6-4906-ad6e-9bf3ce168c30");

    const iox::expected<const RequestHeader*, ServerRequestResult> getRequestResult =
        iox::ok<const RequestHeader*>(requestMock.userHeader());

    EXPECT_CALL(sut.mockPort, getRequest()).WillOnce(Return(getRequestResult));

    auto takeResult = sut.take();
    ASSERT_FALSE(takeResult.has_error());
    EXPECT_THAT(takeResult.value(), Eq(requestMock.sample()));
}

TEST_F(UntypedServer_test, TakeCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "224e93e3-47f4-4533-8fac-9cb7bbb87f08");

    constexpr ServerRequestResult SERVER_REQUEST_RESULT{ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL};
    const iox::expected<const RequestHeader*, ServerRequestResult> getRequestResult = iox::err(SERVER_REQUEST_RESULT);

    EXPECT_CALL(sut.mockPort, getRequest()).WillOnce(Return(getRequestResult));

    auto takeResult = sut.take();
    ASSERT_TRUE(takeResult.has_error());
    EXPECT_THAT(takeResult.error(), Eq(SERVER_REQUEST_RESULT));
}

TEST_F(UntypedServer_test, ReleaseRequestWithValidPayloadPointerCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b522a1b-e30a-4036-be12-72fe7c7c04f1");

    EXPECT_CALL(sut.mockPort, releaseRequest(requestMock.userHeader())).Times(1);

    sut.releaseRequest(requestMock.sample());
}

TEST_F(UntypedServer_test, ReleaseRequestWithNullpointerDoesNotCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "8136b357-33a9-4f6d-a2fc-88901664917c");

    EXPECT_CALL(sut.mockPort, releaseRequest(_)).Times(0);

    sut.releaseRequest(nullptr);
}

TEST_F(UntypedServer_test, LoanCallsUnderlyingPortWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "f39d58f3-b25e-4515-852d-c3afa5519e5a");

    constexpr uint64_t PAYLOAD_SIZE{8U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{32U};
    const iox::expected<ResponseHeader*, AllocationError> allocateResponseResult =
        iox::ok<ResponseHeader*>(responseMock.userHeader());

    EXPECT_CALL(sut.mockPort, allocateResponse(requestMock.userHeader(), PAYLOAD_SIZE, PAYLOAD_ALIGNMENT))
        .WillOnce(Return(allocateResponseResult));

    auto loanResult = sut.loan(requestMock.userHeader(), PAYLOAD_SIZE, PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(loanResult.has_error());
    EXPECT_THAT(loanResult.value(), Eq(responseMock.sample()));
}

TEST_F(UntypedServer_test, LoanCallsUnderlyingPortWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "d813b550-64b2-490f-a9f4-bafc9ddc7696");

    constexpr uint64_t PAYLOAD_SIZE{8U};
    constexpr uint32_t PAYLOAD_ALIGNMENT{32U};
    constexpr AllocationError ALLOCATION_ERROR{AllocationError::RUNNING_OUT_OF_CHUNKS};
    const iox::expected<ResponseHeader*, AllocationError> allocateResponseResult = iox::err(ALLOCATION_ERROR);

    EXPECT_CALL(sut.mockPort, allocateResponse(requestMock.userHeader(), PAYLOAD_SIZE, PAYLOAD_ALIGNMENT))
        .WillOnce(Return(allocateResponseResult));

    auto loanResult = sut.loan(requestMock.userHeader(), PAYLOAD_SIZE, PAYLOAD_ALIGNMENT);
    ASSERT_TRUE(loanResult.has_error());
    EXPECT_THAT(loanResult.error(), Eq(ALLOCATION_ERROR));
}

TEST_F(UntypedServer_test, SendWithValidPayloadPointerCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "707cfdd8-05ae-490c-8cbb-9a4253135937");

    EXPECT_CALL(sut.mockPort, sendResponse(responseMock.userHeader())).WillOnce(Return(iox::ok()));

    sut.send(responseMock.sample())
        .and_then([&]() { GTEST_SUCCEED() << "Response successfully sent"; })
        .or_else([&](auto error) { GTEST_FAIL() << "Expected response to be sent but got error: " << error; });
}

TEST_F(UntypedServer_test, SendWithNullpointerDoesNotCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7552fc4-1e7c-40c2-b0b4-a24593dc0607");

    EXPECT_CALL(sut.mockPort, sendResponse(_)).Times(0);

    sut.send(nullptr)
        .and_then([&]() { GTEST_FAIL() << "Expected response not successfully sent"; })
        .or_else([&](auto error) { EXPECT_THAT(error, Eq(ServerSendError::INVALID_RESPONSE)); });
}

TEST_F(UntypedServer_test, ReleaseResponseWithValidPayloadPointerCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "db4be6c4-fe6f-439c-b1c7-875909dd9573");

    EXPECT_CALL(sut.mockPort, releaseResponse(responseMock.userHeader())).Times(1);

    sut.releaseResponse(responseMock.sample());
}

TEST_F(UntypedServer_test, ReleaseResponseWithNullpointerDoesNotCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "63cdadf3-5745-4196-a5dc-5b8925c14289");

    EXPECT_CALL(sut.mockPort, releaseResponse(_)).Times(0);

    sut.releaseResponse(nullptr);
}

} // namespace
