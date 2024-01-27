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

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "test_popo_server_port_common.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"

namespace iox_test_popo_server_port
{
// NOTE tests related to QueueFullPolicy are done in test_client_server.cpp integration test

// BEGIN isOffered, offer and stopOffer tests

TEST_F(ServerPort_test, InitialIsOfferedOnPortWithOfferOnCreateIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "36800bff-c190-4e9f-b77c-de7a8530f35b");
    auto& sut = serverPortWithOfferOnCreate;
    EXPECT_TRUE(sut.portUser.isOffered());
}

TEST_F(ServerPort_test, InitialIsOfferedOnPortWithoutOfferOnCreateIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed3d652b-a77b-47a3-85c1-edb3042ad6f8");
    auto& sut = serverPortWithoutOfferOnCreate;
    EXPECT_FALSE(sut.portUser.isOffered());
}

TEST_F(ServerPort_test, OfferWhenAlreadyOfferedKeepsIsOfferedTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe3e2cac-df63-44c9-b333-c62db08713b8");
    auto& sut = serverPortWithOfferOnCreate;
    sut.portUser.offer();
    EXPECT_TRUE(sut.portUser.isOffered());
}

TEST_F(ServerPort_test, OfferWhenNotAlreadyOfferedChangesIsOfferedToTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "db41200b-fd65-4bbc-92b0-0e13d973c730");
    auto& sut = serverPortWithoutOfferOnCreate;
    sut.portUser.offer();
    EXPECT_TRUE(sut.portUser.isOffered());
}

TEST_F(ServerPort_test, StopOfferWhenAlreadyOfferedChangesIsOfferedToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "71053a00-f93a-48fa-9fdb-eec9297fd6f1");
    auto& sut = serverPortWithOfferOnCreate;
    sut.portUser.stopOffer();
    EXPECT_FALSE(sut.portUser.isOffered());
}

TEST_F(ServerPort_test, StopOfferWhenNotOfferedKeepsIsOfferedFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "5fa3c4b1-7a0d-4b8d-9164-86574c019a65");
    auto& sut = serverPortWithoutOfferOnCreate;
    sut.portUser.stopOffer();
    EXPECT_FALSE(sut.portUser.isOffered());
}

TEST_F(ServerPort_test, OfferWhenThereIntermediatelyWasAStopOfferResultsInIsOfferedTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "0391523f-2c3b-4d7c-a329-3350f0659251");
    auto& sut = serverPortWithOfferOnCreate;
    sut.portUser.stopOffer();
    sut.portUser.offer();
    EXPECT_TRUE(sut.portUser.isOffered());
}

TEST_F(ServerPort_test, StopOfferWhenThereIntermediatelyWasAOfferResultsInIsOfferedFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "06c8ef10-798f-47fc-9146-5516ff40cffa");
    auto& sut = serverPortWithoutOfferOnCreate;
    sut.portUser.offer();
    sut.portUser.stopOffer();
    EXPECT_FALSE(sut.portUser.isOffered());
}

// END isOffered, offer and stopOffer tests

// BEGIN hasClients tests

TEST_F(ServerPort_test, HasClientsWithoutOfferIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a0d3ac7-fc4c-4d37-98af-ec4a3f8afe09");
    auto& sut = serverPortWithoutOfferOnCreate;
    EXPECT_FALSE(sut.portUser.hasClients());
}

TEST_F(ServerPort_test, HasClientsWithNoClientsIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "00e3f107-f9d1-42b7-b1ec-59daa0e275e5");
    auto& sut = serverPortWithOfferOnCreate;
    EXPECT_FALSE(sut.portUser.hasClients());
}

TEST_F(ServerPort_test, HasClientsWithClientIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e0e81dd-8d39-435f-b7a7-8f7121356fa4");
    auto& sut = serverPortWithOfferOnCreate;

    addClientQueue(sut);

    EXPECT_TRUE(sut.portUser.hasClients());
}

TEST_F(ServerPort_test, HasClientsWithNoClientsButIntermediatelyHavingClientsIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f890938-e6ae-4dce-a4e8-acbed1934153");
    auto& sut = serverPortWithOfferOnCreate;

    addClientQueue(sut);
    removeClientQueue(sut);

    EXPECT_FALSE(sut.portUser.hasClients());
}

// END hasClients tests

// BEGIN hasNewRequests tests

TEST_F(ServerPort_test, HasNewRequestsWithoutOfferIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "acac09bb-4e80-426a-b23b-16e55c1ce1c8");
    auto& sut = serverPortWithoutOfferOnCreate;
    EXPECT_FALSE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithNoRequestsIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7aa181d-bd4f-46f5-ad57-2cc54a4ec93f");
    auto& sut = serverPortWithOfferOnCreate;
    EXPECT_FALSE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithOneRequestIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "0235ce49-96b3-41c0-aac1-e0d6f6bc2b1f");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);

    EXPECT_TRUE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithNoRequestsButPreviouslyHavingOneIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "29b0a347-2439-488d-ba13-962c7acffac1");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);
    IOX_DISCARD_RESULT(sut.portUser.getRequest());

    EXPECT_FALSE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithOneRequestButIntermediatelyHavingNoneIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a9fc0d89-7b97-48c4-8ba4-e4b22355116b");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);
    IOX_DISCARD_RESULT(sut.portUser.getRequest());
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);

    EXPECT_TRUE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithMultipleRequestsIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "30e36e69-4f23-41ee-aac7-d34240f075ae");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{2U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);

    EXPECT_TRUE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithFullRequestQueueIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e0aea054-4f58-443b-abbb-e0e3c6c5c5c6");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{QUEUE_CAPACITY};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);

    EXPECT_TRUE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithMultipleRequestsAndAllButOneRemovedIsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "96c42094-9931-46a3-8fd6-e7d28b490527");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{2U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);
    IOX_DISCARD_RESULT(sut.portUser.getRequest());

    EXPECT_TRUE(sut.portUser.hasNewRequests());
}

TEST_F(ServerPort_test, HasNewRequestsWithNoRequestsButIntermediatelyHavingMultipleRequestsIsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "44845e16-ec0a-4b4e-a000-ff62f377c0b9");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{2U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);
    IOX_DISCARD_RESULT(sut.portUser.getRequest());
    IOX_DISCARD_RESULT(sut.portUser.getRequest());

    EXPECT_FALSE(sut.portUser.hasNewRequests());
}

// END hasNewRequests tests

// BEGIN getRequest tests

TEST_F(ServerPort_test, GetRequestWithoutOfferResultsInNoPendingRequests)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c555224-8b03-46c5-a988-059ac2656149");
    auto& sut = serverPortWithoutOfferOnCreate;

    sut.portUser.getRequest()
        .and_then([&](const auto&) {
            GTEST_FAIL() << "Expected ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER but "
                            "got request";
        })
        .or_else([&](const auto& error) {
            EXPECT_THAT(error, Eq(ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER));
        });
}

TEST_F(ServerPort_test, GetRequestWithNoRequestsResultsInNoPendingRequests)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b78536b-6902-4e55-aef2-654da5afdb80");
    auto& sut = serverPortWithOfferOnCreate;

    sut.portUser.getRequest()
        .and_then(
            [&](const auto&) { GTEST_FAIL() << "Expected ServerRequestResult::NO_PENDING_REQUESTS but got request"; })
        .or_else([&](const auto& error) { EXPECT_THAT(error, Eq(ServerRequestResult::NO_PENDING_REQUESTS)); });
}

TEST_F(ServerPort_test, GetRequestWithOneRequestsResultsInRequestHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3b79e97-00bb-4e36-931d-88cfbe027a07");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    constexpr uint64_t REQUEST_DATA{42};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS, REQUEST_DATA);

    sut.portUser.getRequest()
        .and_then([&](const auto& req) { EXPECT_THAT(this->getRequestData(req), Eq(REQUEST_DATA)); })
        .or_else([&](const auto& error) { GTEST_FAIL() << "Expected RequestHeader but got error: " << error; });
}

TEST_F(ServerPort_test, GetRequestWithNoRequestsButIntermediatelyHavingOneResultsInNoPendingRequests)
{
    ::testing::Test::RecordProperty("TEST_ID", "a1df6ee7-a936-446d-8960-5b9d6a93ad62");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);
    IOX_DISCARD_RESULT(sut.portUser.getRequest());

    sut.portUser.getRequest()
        .and_then(
            [&](const auto&) { GTEST_FAIL() << "Expected ServerRequestResult::NO_PENDING_REQUESTS but got request"; })
        .or_else([&](const auto& error) { EXPECT_THAT(error, Eq(ServerRequestResult::NO_PENDING_REQUESTS)); });
}

TEST_F(ServerPort_test, GetRequestWithOneRequestsButIntermediatelyHavingNoneResultsInRequestHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "ea4154a8-5a46-4c5d-b9cb-91adf6c1ff75");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t REQUEST_DATA_1{13};
    constexpr uint64_t REQUEST_DATA_2{73};

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS, REQUEST_DATA_1);
    IOX_DISCARD_RESULT(sut.portUser.getRequest());
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS, REQUEST_DATA_2);

    sut.portUser.getRequest()
        .and_then([&](const auto& req) { EXPECT_THAT(this->getRequestData(req), Eq(REQUEST_DATA_2)); })
        .or_else([&](const auto& error) { GTEST_FAIL() << "Expected RequestHeader but got error: " << error; });
}

TEST_F(ServerPort_test, GetRequestWithMultipleRequestsResultsInAsManyRequestHeaderAsRequests)
{
    ::testing::Test::RecordProperty("TEST_ID", "35032bbb-ec59-4b54-ba27-bdb364105162");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t REQUEST_DATA_BASE{37};

    constexpr uint64_t NUMBER_OF_REQUESTS{2U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS, REQUEST_DATA_BASE);

    sut.portUser.getRequest()
        .and_then([&](const auto& req) { EXPECT_THAT(this->getRequestData(req), Eq(REQUEST_DATA_BASE)); })
        .or_else([&](const auto& error) { GTEST_FAIL() << "Expected RequestHeader but got error: " << error; });

    sut.portUser.getRequest()
        .and_then([&](const auto& req) { EXPECT_THAT(this->getRequestData(req), Eq(REQUEST_DATA_BASE + 1)); })
        .or_else([&](const auto& error) { GTEST_FAIL() << "Expected RequestHeader but got error: " << error; });
}

TEST_F(ServerPort_test, GetRequestWithMaximalHeldChunksInParallelResultsInRequestHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "19c19b39-2dd1-4784-a1c1-adfba56248e8");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t REQUEST_DATA_BASE{7337};
    // the maximum number of request which can be held in parallel must be larger than
    // MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY; if it would be the same, the server would have to release one request
    // before a new one could be fetched and for a short time window the requirement of being able to hold
    // MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY would be broken
    constexpr uint64_t MAX_REQUEST_HELD_IN_PARALLEL = iox::MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY + 1;

    pushRequests(sut.requestQueuePusher, MAX_REQUEST_HELD_IN_PARALLEL, REQUEST_DATA_BASE);

    for (uint64_t i = 0; i < MAX_REQUEST_HELD_IN_PARALLEL; ++i)
    {
        sut.portUser.getRequest()
            .and_then([&](const auto& req) { EXPECT_THAT(this->getRequestData(req), Eq(REQUEST_DATA_BASE + i)); })
            .or_else([&](const auto& error) { GTEST_FAIL() << "Expected RequestHeader but got error: " << error; });
    }
}

TEST_F(ServerPort_test, GetRequestWhenProcessingTooManyRequestsInParallelResultsIn_TOO_MANY_REQUESTS_HELD_IN_PARALLEL)
{
    ::testing::Test::RecordProperty("TEST_ID", "87aa5ddf-80aa-4d32-b901-a82a99fb4a2f");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t REQUEST_DATA_BASE{7337};
    // the maximum number of request which can be held in parallel must be larger than
    // MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY; if it would be the same, the server would have to release one request
    // before a new one could be fetched and for a short time window the requirement of being able to hold
    // MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY would be broken
    constexpr uint64_t MAX_REQUEST_HELD_IN_PARALLEL = iox::MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY + 1;

    pushRequests(sut.requestQueuePusher, MAX_REQUEST_HELD_IN_PARALLEL + 1, REQUEST_DATA_BASE);

    for (uint64_t i = 0; i < MAX_REQUEST_HELD_IN_PARALLEL; ++i)
    {
        IOX_DISCARD_RESULT(sut.portUser.getRequest());
    }

    sut.portUser.getRequest()
        .and_then([&](const auto&) {
            GTEST_FAIL() << "Expected ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL but got request";
        })
        .or_else([&](const auto& error) {
            EXPECT_THAT(error, Eq(ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL));
        });
}

// END getRequest tests

// BEGIN releaseRequest tests

TEST_F(ServerPort_test, ReleaseRequestWithValidRequestHeaderWorksAndReleasesTheChunkToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "ffb5df3f-2f2d-40a9-b0b6-53b44daa5568");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    constexpr uint64_t REQUEST_DATA{42};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS, REQUEST_DATA);

    sut.portUser.getRequest()
        .and_then([&](const auto& req) {
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(1U));
            sut.portUser.releaseRequest(req);
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(0U));
        })
        .or_else([&](const auto& error) { GTEST_FAIL() << "Expected RequestHeader but got error: " << error; });
}

TEST_F(ServerPort_test, ReleaseRequestWithInvalidChunkCallsTheErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "63bf7750-3193-4711-baa8-6cee500297da");
    auto& sut = serverPortWithOfferOnCreate;

    auto sharedChunk = getChunkWithInitializedRequestHeaderAndData();

    sut.portUser.releaseRequest(static_cast<const RequestHeader*>(sharedChunk.getChunkHeader()->userHeader()));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__CHUNK_RECEIVER_INVALID_CHUNK_TO_RELEASE_FROM_USER);
}

TEST_F(ServerPort_test, ReleaseRequestWithNullptrRequestHeaderCallsTheErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "b505019f-ba47-4df4-ba5e-d2d16e5c44cd");
    auto& sut = serverPortWithOfferOnCreate;

    sut.portUser.releaseRequest(nullptr);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__SERVER_PORT_INVALID_REQUEST_TO_RELEASE_FROM_USER);
}

// END releaseRequest tests

// BEGIN releaseQueuedRequests tests

TEST_F(ServerPort_test, ReleaseQueuedRequestsReleasesAllChunksToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "90d918e4-a1cf-4a78-b987-7f0c03ca8805");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t NUMBER_OF_REQUESTS{3U};
    constexpr uint64_t REQUEST_DATA{42};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS, REQUEST_DATA);

    EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUESTS));
    sut.portUser.releaseQueuedRequests();
    EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(0U));
}

// END releaseQueuedRequests tests

// BEGIN hasLostRequestsSinceLastCall tests

TEST_F(ServerPort_test, HasLostRequestsSinceLastCallWhenNoRequestsAreLostReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "134a100a-e28e-461e-9860-635a85589cc1");
    auto& sut = serverPortWithOfferOnCreate;
    EXPECT_FALSE(sut.portUser.hasLostRequestsSinceLastCall());
}

TEST_F(ServerPort_test, HasLostRequestsSinceLastCallWithFullQueueReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c7aff9a-2460-4b7a-9b78-8ca5a4f80503");
    auto& sut = serverPortWithOfferOnCreate;

    pushRequests(sut.requestQueuePusher, QUEUE_CAPACITY);

    EXPECT_FALSE(sut.portUser.hasLostRequestsSinceLastCall());
}

TEST_F(ServerPort_test, HasLostRequestsSinceLastCallWhenOneRequestIsLostReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a300681-4420-45be-8241-cb172ca61561");
    auto& sut = serverPortWithOfferOnCreate;

    pushRequests(sut.requestQueuePusher, QUEUE_CAPACITY + 1);

    EXPECT_TRUE(sut.portUser.hasLostRequestsSinceLastCall());
}

TEST_F(ServerPort_test, HasLostRequestsSinceLastCallWhenMultipleRequestAreLostReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "2235a5cf-cf04-42f1-929e-20e0f73bc2f5");
    auto& sut = serverPortWithOfferOnCreate;

    pushRequests(sut.requestQueuePusher, QUEUE_CAPACITY + 2);

    EXPECT_TRUE(sut.portUser.hasLostRequestsSinceLastCall());
}

TEST_F(ServerPort_test, HasLostRequestsSinceLastCallWhenNoFurtherRequestAreLostReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "27a8ddcc-e22b-4bd9-abbd-ed61ac88d01c");
    auto& sut = serverPortWithOfferOnCreate;

    pushRequests(sut.requestQueuePusher, QUEUE_CAPACITY + 1);
    IOX_DISCARD_RESULT(sut.portUser.hasLostRequestsSinceLastCall());

    EXPECT_FALSE(sut.portUser.hasLostRequestsSinceLastCall());
}

TEST_F(ServerPort_test, HasLostRequestsSinceLastCallWhenFurtherRequestAreLostReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "21fd5e5e-93e4-4ef9-8448-d1c6ae6e3989");
    auto& sut = serverPortWithOfferOnCreate;

    pushRequests(sut.requestQueuePusher, QUEUE_CAPACITY + 1);
    IOX_DISCARD_RESULT(sut.portUser.hasLostRequestsSinceLastCall());
    pushRequests(sut.requestQueuePusher, 1);

    EXPECT_TRUE(sut.portUser.hasLostRequestsSinceLastCall());
}

TEST_F(ServerPort_test, HasLostRequestsSinceLastCallWhenNoRequestAreLostAfterRemovingRequestFromQueueReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "248ddc67-5717-4b33-8b04-80d8246fedb3");
    auto& sut = serverPortWithOfferOnCreate;

    pushRequests(sut.requestQueuePusher, QUEUE_CAPACITY + 1);
    IOX_DISCARD_RESULT(sut.portUser.hasLostRequestsSinceLastCall());
    IOX_DISCARD_RESULT(sut.portUser.getRequest());
    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);

    EXPECT_FALSE(sut.portUser.hasLostRequestsSinceLastCall());
}

TEST_F(ServerPort_test,
       HasLostRequestsSinceLastCallWithBlockProducerRequestQueueFullPolicyAndIntermediatelyBlockingReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "301beffe-acae-4863-9eb9-96d7629f81f8");
    auto& sut = serverOptionsWithBlockProducerRequestQueueFullPolicy;

    constexpr uint64_t REQUEST_DATA_BASE{666};

    EXPECT_TRUE(
        pushRequests(sut.requestQueuePusher, QUEUE_CAPACITY, REQUEST_DATA_BASE, QueueFullPolicy::BLOCK_PRODUCER));
    // queue is full and push does not succeed
    EXPECT_FALSE(
        pushRequests(sut.requestQueuePusher, 1, REQUEST_DATA_BASE + QUEUE_CAPACITY, QueueFullPolicy::BLOCK_PRODUCER));

    // ensure FIFO semantic
    for (uint64_t i = 0; i < QUEUE_CAPACITY; ++i)
    {
        sut.portUser.getRequest()
            .and_then([&](const auto& req) {
                EXPECT_THAT(this->getRequestData(req), Eq(REQUEST_DATA_BASE + i));
                sut.portUser.releaseRequest(req);
            })
            .or_else([&](const auto& error) { GTEST_FAIL() << "Expected RequestHeader but got error: " << error; });
    }

    // since push was not successful the ChunkDistributor would have tried again and no chunk is lost
    EXPECT_FALSE(sut.portUser.hasLostRequestsSinceLastCall());
}

// END hasLostRequestsSinceLastCall tests

// BEGIN allocateResponse tests

TEST_F(ServerPort_test, AllocateResponseWithNullptrAsRequestHeaderCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "05891e1b-3aa4-4aa9-aafd-0649f88d2982");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t USER_PAYLOAD_SIZE{8U};
    constexpr uint32_t USER_PAYLOAD_ALIGNMENT{8U};
    constexpr RequestHeader* REQUEST_HEADER_NULLPTR{nullptr};

    sut.portUser.allocateResponse(REQUEST_HEADER_NULLPTR, USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](const auto&) {
            GTEST_FAIL() << "Expected AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER but got chunk";
        })
        .or_else(
            [&](const auto& error) { EXPECT_THAT(error, Eq(AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER)); });
}

TEST_F(ServerPort_test,
       AllocateResponseWithInvalidPayloadParameterReturns_INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER)
{
    ::testing::Test::RecordProperty("TEST_ID", "6015d3d6-73ae-46fe-994d-445f3f8f366d");
    auto& sut = serverPortWithOfferOnCreate;

    constexpr uint64_t INVALID_USER_PAYLOAD_SIZE{23U};
    constexpr uint32_t INVALID_USER_PAYLOAD_ALIGNMENT{15U};

    constexpr uint64_t NUMBER_OF_REQUESTS{1U};
    pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);
    auto requestResult = sut.portUser.getRequest();
    ASSERT_FALSE(requestResult.has_error());
    auto requestHeader = requestResult.value();

    sut.portUser.allocateResponse(requestHeader, INVALID_USER_PAYLOAD_SIZE, INVALID_USER_PAYLOAD_ALIGNMENT)
        .and_then([&](const auto&) {
            GTEST_FAIL() << "Expected AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER but got chunk";
        })
        .or_else([&](const auto& error) {
            EXPECT_THAT(error, Eq(AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER));
        });
}

TEST_F(ServerPort_test, AllocateResponseWithValidParameterReturnsResponseHeader)
{
    ::testing::Test::RecordProperty("TEST_ID", "128b7d92-a30f-4c9d-b1c6-39a03ca29499");
    auto& sut = serverPortWithOfferOnCreate;

    allocateResponseWithRequestHeaderAndThen(sut, [&](const auto, auto) { GTEST_SUCCEED(); });
}

// END allocateResponse tests

// BEGIN releaseResponse tests

TEST_F(ServerPort_test, ReleaseResponseWithValidResponseHeaderWorksAndReleasesTheChunkToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f5ff38e-92a2-4169-98ea-38985350dc85");
    auto& sut = serverPortWithOfferOnCreate;

    allocateResponseWithRequestHeaderAndThen(sut, [&](const auto, auto res) {
        constexpr uint64_t NUMBER_OF_REQUEST_CHUNKS{1U};
        constexpr uint64_t NUMBER_OF_RESPONSE_CHUNKS{1U};
        EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUEST_CHUNKS + NUMBER_OF_RESPONSE_CHUNKS));
        sut.portUser.releaseResponse(res);
        EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUEST_CHUNKS));
    });
}

TEST_F(ServerPort_test, ReleaseResponseWithInvalidChunkCallsTheErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb09c788-0e1b-41c6-877a-1b13a37829d4");
    auto& sut = serverPortWithOfferOnCreate;

    allocateResponseWithRequestHeaderAndThen(sut, [&](const auto, auto res) {
        sut.portUser.releaseResponse(res);
        // since the response is already freed, it should not be in the UsedChunkList anymore and the error handler
        // should be called
        sut.portUser.releaseResponse(res);
    });

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__CHUNK_SENDER_INVALID_CHUNK_TO_FREE_FROM_USER);
}

TEST_F(ServerPort_test, ReleaseResponseWithWithNullptrResponseHeaderCallsTheErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "ecb40c4d-7b95-4780-9b51-ac1708830453");
    auto& sut = serverPortWithOfferOnCreate;

    sut.portUser.releaseResponse(nullptr);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__SERVER_PORT_INVALID_RESPONSE_TO_FREE_FROM_USER);
}

// END releaseResponse tests

// BEGIN sendResponse tests

TEST_F(ServerPort_test, SendResponseWithWithNullptrResponseHeaderCallsTheErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d0b0a7f-d1ab-4249-b885-cbc6429eab83");
    auto& sut = serverPortWithOfferOnCreate;

    sut.portUser.sendResponse(nullptr)
        .and_then([&]() { GTEST_FAIL() << "Expected response not successfully sent"; })
        .or_else([&](auto error) { EXPECT_THAT(error, Eq(ServerSendError::INVALID_RESPONSE)); });

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__SERVER_PORT_INVALID_RESPONSE_TO_SEND_FROM_USER);
}

TEST_F(ServerPort_test, SendResponseWithoutOfferReleasesTheChunkToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc4e31b1-18bf-4c42-9084-dd7abd52609b");
    auto& sut = serverPortWithoutOfferOnCreate;

    allocateResponseWithRequestHeaderAndThen(sut, [&](const auto, auto res) {
        constexpr uint64_t NUMBER_OF_REQUEST_CHUNKS{1U};
        constexpr uint64_t NUMBER_OF_RESPONSE_CHUNKS{1U};
        EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUEST_CHUNKS + NUMBER_OF_RESPONSE_CHUNKS));
        sut.portUser.sendResponse(res)
            .and_then([&]() { GTEST_FAIL() << "Expected response not successfully sent"; })
            .or_else([&](auto error) { EXPECT_THAT(error, Eq(ServerSendError::NOT_OFFERED)); });
        EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUEST_CHUNKS));
    });
}

TEST_F(ServerPort_test, SendResponseWithInvalidClientQueueIdReleasesTheChunkToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "45823507-b83b-496b-965e-a48bd3c07b9e");
    auto& sut = serverPortWithOfferOnCreate;

    // the client is not yet connected to the 'clientResponseQueue' which ID is used to send the responses to
    allocateResponseWithRequestHeaderAndThen(sut, [&](const auto, auto res) {
        constexpr uint64_t NUMBER_OF_REQUEST_CHUNKS{1U};
        constexpr uint64_t NUMBER_OF_RESPONSE_CHUNKS{1U};
        EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUEST_CHUNKS + NUMBER_OF_RESPONSE_CHUNKS));
        sut.portUser.sendResponse(res)
            .and_then([&]() { GTEST_FAIL() << "Expected response not successfully sent"; })
            .or_else([&](auto error) { EXPECT_THAT(error, Eq(ServerSendError::CLIENT_NOT_AVAILABLE)); });
        EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUEST_CHUNKS));
    });
}

TEST_F(ServerPort_test, SendResponseWithValidClientQueueIdReleasesDeliversToTheClientQueue)
{
    ::testing::Test::RecordProperty("TEST_ID", "15a0fe2f-166c-4174-88e8-ee0073d0e64f");
    auto& sut = serverPortWithOfferOnCreate;

    addClientQueue(sut);

    constexpr uint64_t RESPONSE_DATA{111U};
    allocateResponseWithRequestHeaderAndThen(sut, [&](const auto, auto res) {
        new (ChunkHeader::fromUserHeader(res)->userPayload()) int64_t(RESPONSE_DATA);
        sut.portUser.sendResponse(res)
            .and_then([&]() { GTEST_SUCCEED() << "Response successfully sent"; })
            .or_else([&](auto error) { GTEST_FAIL() << "Expected response to be sent but got error: " << error; });
    });

    auto maybeChunk [[maybe_unused]] = clientResponseQueue.tryPop()
                                           .and_then([&](const auto& chunk) {
                                               auto data = *static_cast<uint64_t*>(chunk.getUserPayload());
                                               EXPECT_THAT(data, Eq(RESPONSE_DATA));
                                           })
                                           .or_else([&]() { GTEST_FAIL() << "Expected response but got none"; });

    constexpr uint64_t NUMBER_OF_REQUEST_CHUNKS{1U};
    constexpr uint64_t NUMBER_OF_RESPONSE_CHUNKS{1U};
    EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_REQUEST_CHUNKS + NUMBER_OF_RESPONSE_CHUNKS));
}

// END sendResponse tests

// BEGIN condition variable tests

TEST_F(ServerPort_test, ConditionVariableInitiallyNotSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "c84c6fd9-2691-4c9f-8306-2ac476e89f5d");
    auto& sut = serverPortWithOfferOnCreate;
    EXPECT_FALSE(sut.portUser.isConditionVariableSet());
}

TEST_F(ServerPort_test, SettingConditionVariableWithoutConditionVariablePresentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ce878180-626e-4b94-93b0-9d465f054971");
    iox::popo::ConditionVariableData condVar{"hypnotoad"};
    constexpr uint32_t NOTIFICATION_INDEX{1};

    auto& sut = serverPortWithOfferOnCreate;
    sut.portUser.setConditionVariable(condVar, NOTIFICATION_INDEX);

    EXPECT_TRUE(sut.portUser.isConditionVariableSet());
}

TEST_F(ServerPort_test, UnsettingConditionVariableWithConditionVariablePresentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a84b8bf2-877f-495c-9b90-933bfffe1d4d");
    iox::popo::ConditionVariableData condVar{"brain slug"};
    constexpr uint32_t NOTIFICATION_INDEX{2};

    auto& sut = serverPortWithOfferOnCreate;
    sut.portUser.setConditionVariable(condVar, NOTIFICATION_INDEX);

    sut.portUser.unsetConditionVariable();

    EXPECT_FALSE(sut.portUser.isConditionVariableSet());
}

TEST_F(ServerPort_test, UnsettingConditionVariableWithoutConditionVariablePresentIsHandledGracefully)
{
    ::testing::Test::RecordProperty("TEST_ID", "92c5e4f4-8766-4807-be78-95c677fa6a4c");
    auto& sut = serverPortWithOfferOnCreate;
    sut.portUser.unsetConditionVariable();

    EXPECT_FALSE(sut.portUser.isConditionVariableSet());
}

// END condition variable tests

// BEGIN ServerRequestResult string tests

TEST_F(ServerPort_test, asStringLiteralConvertsRequestResultValuesToStrings)
{
    ::testing::Test::RecordProperty("TEST_ID", "b48bbfaa-982e-4f8c-97e7-998a1c6a3a7b");
    using ServerRequestResult = iox::popo::ServerRequestResult;

    // each bit corresponds to an enum value and must be set to true on test
    uint64_t testedEnumValues{0U};
    uint64_t loopCounter{0U};
    for (const auto& sut : {ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL,
                            ServerRequestResult::NO_PENDING_REQUESTS,
                            ServerRequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR,
                            ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER})
    {
        auto enumString = iox::popo::asStringLiteral(sut);

        switch (sut)
        {
        case ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL:
            EXPECT_THAT(enumString, StrEq("ServerRequestResult::TOO_MANY_REQUESTS_HELD_IN_PARALLEL"));
            break;
        case ServerRequestResult::NO_PENDING_REQUESTS:
            EXPECT_THAT(enumString, StrEq("ServerRequestResult::NO_PENDING_REQUESTS"));
            break;
        case ServerRequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR:
            EXPECT_THAT(enumString, StrEq("ServerRequestResult::UNDEFINED_CHUNK_RECEIVE_ERROR"));
            break;
        case ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER:
            EXPECT_THAT(enumString, StrEq("ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER"));
            break;
        }

        testedEnumValues |= 1U << static_cast<uint64_t>(sut);
        ++loopCounter;
    }

    uint64_t expectedTestedEnumValues = (1U << loopCounter) - 1;
    EXPECT_EQ(testedEnumValues, expectedTestedEnumValues);
}

TEST_F(ServerPort_test, LogStreamConvertsAllocationErrorValueToString)
{
    ::testing::Test::RecordProperty("TEST_ID", "06d66398-318f-4531-a253-9a8e8b42fc00");
    iox::testing::Logger_Mock loggerMock;

    auto sut = iox::popo::ServerRequestResult::NO_PENDING_REQUESTS;

    {
        IOX_LOGSTREAM_MOCK(loggerMock) << sut;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(iox::popo::asStringLiteral(sut)));
}

// END ServerRequestResult string tests

// BEGIN ServerSendError string tests

TEST_F(ServerPort_test, asStringLiteralConvertsServerSendErrorValuesToStrings)
{
    ::testing::Test::RecordProperty("TEST_ID", "0932177c-5aa9-4f7f-8522-1febaf377bf0");
    using ServerSendError = iox::popo::ServerSendError;

    // each bit corresponds to an enum value and must be set to true on test
    uint64_t testedEnumValues{0U};
    uint64_t loopCounter{0U};
    for (const auto& sut :
         {ServerSendError::NOT_OFFERED, ServerSendError::CLIENT_NOT_AVAILABLE, ServerSendError::INVALID_RESPONSE})
    {
        auto enumString = iox::popo::asStringLiteral(sut);

        switch (sut)
        {
        case ServerSendError::NOT_OFFERED:
            EXPECT_THAT(enumString, StrEq("ServerSendError::NOT_OFFERED"));
            break;
        case ServerSendError::CLIENT_NOT_AVAILABLE:
            EXPECT_THAT(enumString, StrEq("ServerSendError::CLIENT_NOT_AVAILABLE"));
            break;
        case ServerSendError::INVALID_RESPONSE:
            EXPECT_THAT(enumString, StrEq("ServerSendError::INVALID_RESPONSE"));
            break;
        }

        testedEnumValues |= 1U << static_cast<uint64_t>(sut);
        ++loopCounter;
    }

    uint64_t expectedTestedEnumValues = (1U << loopCounter) - 1;
    EXPECT_EQ(testedEnumValues, expectedTestedEnumValues);
}

TEST_F(ServerPort_test, LogStreamConvertsServerSendErrorValueToString)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a06b21d-ccab-457d-80b6-dc37843a0575");
    iox::testing::Logger_Mock loggerMock;

    auto sut = iox::popo::ServerSendError::CLIENT_NOT_AVAILABLE;

    {
        IOX_LOGSTREAM_MOCK(loggerMock) << sut;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(iox::popo::asStringLiteral(sut)));
}

// END ServerSendError string tests

} // namespace iox_test_popo_server_port
