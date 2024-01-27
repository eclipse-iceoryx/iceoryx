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

#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "iox/unique_ptr.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "test.hpp"
#include "test_popo_smart_chunk_common.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;
using namespace test_smart_chunk_common;
using ::testing::_;

class Request_test : public RequestTestCase, public Test
{
};

TEST_F(Request_test, SendCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "cc78dd7b-4dce-43ea-a798-c9aaf0646b49");
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::ok()));

    auto sendResult = sutProducer.send();

    EXPECT_FALSE(sendResult.has_error());
    EXPECT_FALSE(sutProducer);
}

TEST_F(Request_test, SendOnMoveDestinationCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "9a6d018e-77b4-4081-984e-39a5229b7fb8");
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::ok()));

    auto movedSut = std::move(sutProducer);
    auto sendResult = movedSut.send();

    EXPECT_FALSE(sendResult.has_error());
    EXPECT_FALSE(sutProducer);
}

TEST_F(Request_test, SendCallsInterfaceMockWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "127ceb5e-aa9f-4900-9347-33f8925088ba");
    constexpr ClientSendError CLIENT_SEND_ERROR{ClientSendError::SERVER_NOT_AVAILABLE};
    const iox::expected<void, ClientSendError> mockSendResult = iox::err(CLIENT_SEND_ERROR);
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(mockSendResult));

    auto sendResult = sutProducer.send();

    ASSERT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.error(), Eq(CLIENT_SEND_ERROR));
    EXPECT_FALSE(sutProducer);
}

TEST_F(Request_test, SendingAlreadySentRequestCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "e010085d-3674-4a7e-8704-73405ab49afa");
    constexpr ClientSendError CLIENT_SEND_ERROR{ClientSendError::INVALID_REQUEST};
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::ok()));

    EXPECT_FALSE(sutProducer.send().has_error());

    auto sendResult = sutProducer.send();

    ASSERT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.error(), Eq(CLIENT_SEND_ERROR));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__SENDING_EMPTY_REQUEST);
}

TEST_F(Request_test, SendingMovedRequestCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "c49cf937-c831-45e6-8d1b-bba37e786979");
    constexpr ClientSendError CLIENT_SEND_ERROR{ClientSendError::INVALID_REQUEST};

    auto movedSut = std::move(sutProducer);
    auto sendResult = sutProducer.send();

    ASSERT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.error(), Eq(CLIENT_SEND_ERROR));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__SENDING_EMPTY_REQUEST);
}

TEST_F(Request_test, GetRequestHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0d40751-17e0-46a9-b3e1-176232bd9e33");

    const auto& constSutProducer = sutProducer;
    const auto& constSutConsumer = sutConsumer;

    EXPECT_THAT(&sutProducer.getRequestHeader(), Eq(chunkMock.userHeader()));
    EXPECT_THAT(&constSutProducer.getRequestHeader(), Eq(chunkMock.userHeader()));
    EXPECT_THAT(&sutConsumer.getRequestHeader(), Eq(chunkMock.userHeader()));
    EXPECT_THAT(&constSutConsumer.getRequestHeader(), Eq(chunkMock.userHeader()));
}

} // namespace
