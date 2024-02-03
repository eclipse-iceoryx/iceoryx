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

class Response_test : public ResponseTestCase, public Test
{
};

TEST_F(Response_test, SendCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "70361e1e-78ea-48a2-bd5c-679d604e5da4");
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::ok()));

    auto sendResult = sutProducer.send();

    EXPECT_FALSE(sendResult.has_error());
    EXPECT_FALSE(sutProducer);
}

TEST_F(Response_test, SendOnMoveDestinationCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "b86b5884-0319-4819-8bfa-186ac629cd27");
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::ok()));

    auto movedSut = std::move(sutProducer);
    auto sendResult = movedSut.send();

    EXPECT_FALSE(sendResult.has_error());
    EXPECT_FALSE(sutProducer);
}

TEST_F(Response_test, SendCallsInterfaceMockWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "5038ae30-2f09-4f7b-81e4-a7f5bc1b3db4");
    constexpr ServerSendError SERVER_SEND_ERROR{ServerSendError::CLIENT_NOT_AVAILABLE};
    const iox::expected<void, ServerSendError> mockSendResult = iox::err(SERVER_SEND_ERROR);
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(mockSendResult));

    auto sendResult = sutProducer.send();

    ASSERT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.error(), Eq(SERVER_SEND_ERROR));
    EXPECT_FALSE(sutProducer);
}

TEST_F(Response_test, SendingAlreadySentResponseCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "45e592d2-69d9-47cf-8cdf-b1bdf8592947");
    constexpr ServerSendError SERVER_SEND_ERROR{ServerSendError::INVALID_RESPONSE};
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::ok()));

    EXPECT_FALSE(sutProducer.send().has_error());

    auto sendResult = sutProducer.send();

    ASSERT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.error(), Eq(SERVER_SEND_ERROR));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__SENDING_EMPTY_RESPONSE);
}

TEST_F(Response_test, SendingMovedResponseCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e8d7aa2-58d6-421f-9df8-f0fff3f1b9ee");
    constexpr ServerSendError SERVER_SEND_ERROR{ServerSendError::INVALID_RESPONSE};

    auto movedSut = std::move(sutProducer);
    auto sendResult = sutProducer.send();

    ASSERT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.error(), Eq(SERVER_SEND_ERROR));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__SENDING_EMPTY_RESPONSE);
}

TEST_F(Response_test, GetResponseHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c05ccd09-fbff-4d93-90e3-8f1509b8abd8");

    const auto& constSutProducer = sutProducer;
    const auto& constSutConsumer = sutConsumer;

    EXPECT_THAT(&sutProducer.getResponseHeader(), Eq(chunkMock.userHeader()));
    EXPECT_THAT(&constSutProducer.getResponseHeader(), Eq(chunkMock.userHeader()));
    EXPECT_THAT(&sutConsumer.getResponseHeader(), Eq(chunkMock.userHeader()));
    EXPECT_THAT(&constSutConsumer.getResponseHeader(), Eq(chunkMock.userHeader()));
}

} // namespace
