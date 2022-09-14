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

#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"

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
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::cxx::success<void>()));

    auto sendResult = send(std::move(sutProducer));

    EXPECT_FALSE(sendResult.has_error());
}

TEST_F(Response_test, SendOnMoveDestinationCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "b86b5884-0319-4819-8bfa-186ac629cd27");
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(iox::cxx::success<void>()));

    auto movedSut = std::move(sutProducer);
    auto sendResult = send(std::move(movedSut));

    EXPECT_FALSE(sendResult.has_error());
}

TEST_F(Response_test, SendCallsInterfaceMockWithErrorResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "5038ae30-2f09-4f7b-81e4-a7f5bc1b3db4");
    constexpr ServerSendError SERVER_SEND_ERROR{ServerSendError::CLIENT_NOT_AVAILABLE};
    const iox::cxx::expected<ServerSendError> mockSendResult = iox::cxx::error<ServerSendError>{SERVER_SEND_ERROR};
    EXPECT_CALL(mockInterface, mockSend(_)).WillOnce(Return(mockSendResult));

    auto sendResult = send(std::move(sutProducer));

    ASSERT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.get_error(), Eq(SERVER_SEND_ERROR));
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
