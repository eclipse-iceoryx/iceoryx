// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by Bartlomiej Kozaryna <kozarynabartlomiej@gmail.com>. All rights reserved.
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

#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/popo/server.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"
#include "iox/atomic.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::popo;
using namespace iox::capro;
using namespace iox::runtime;
using namespace iox::roudi_env;

constexpr uint64_t SIZE_LARGER_THAN_4GB = std::numeric_limits<uint32_t>::max() + 41065UL;

class DummyRequest
{
  public:
    DummyRequest() = default;
    DummyRequest(uint64_t augend, uint64_t addend)
        : augend(augend)
        , addend(addend)
    {
    }
    uint64_t augend{0U};
    uint64_t addend{0U};
};

class DummyResponse
{
  public:
    DummyResponse() = default;
    DummyResponse(uint64_t sum)
        : sum(sum)
    {
    }
    uint64_t sum{0U};
};

struct BigPayloadStruct
{
    uint8_t bigPayload[SIZE_LARGER_THAN_4GB]{0U};
};

class ClientServer_test : public RouDi_GTest
{
  protected:
    ClientServer_test(iox::IceoryxConfig&& config)
        : RouDi_GTest(std::move(config))
    {
    }

  public:
    ClientServer_test()
        : RouDi_GTest(MinimalIceoryxConfigBuilder().create())
    {
    }

    void SetUp() override
    {
        PoshRuntime::initRuntime("together");
        deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    static constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    Watchdog deadlockWatchdog{DEADLOCK_TIMEOUT};
    ServiceDescription sd{"blue", "and", "yellow"};
    ServiceDescription sdUnmatch{"white", "blue", "red"};
    iox::popo::ClientOptions clientOptions;
    iox::popo::ServerOptions serverOptions;
};
constexpr iox::units::Duration ClientServer_test::DEADLOCK_TIMEOUT;

class BigPayloadClientServer_test : public ClientServer_test
{
    static constexpr uint64_t additionalSizeForUserHeader =
        2 * std::max(sizeof(iox::popo::RequestHeader), sizeof(iox::popo::ResponseHeader));

  public:
    BigPayloadClientServer_test()
        : ClientServer_test(MinimalIceoryxConfigBuilder()
                                .payloadChunkSize(SIZE_LARGER_THAN_4GB + additionalSizeForUserHeader)
                                .payloadChunkCount(2)
                                .create())
    {
    }

    void SetUp() override
    {
        PoshRuntime::initRuntime("together");
        deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    static constexpr iox::units::Duration DEADLOCK_TIMEOUT{10_s};
    Watchdog deadlockWatchdog{DEADLOCK_TIMEOUT};
};

TEST_F(ClientServer_test, TypedApiWithMatchingOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a14eb330-1b7d-4243-be4d-009f9e67a232");

    constexpr int64_t SEQUENCE_ID{73};
    constexpr uint64_t AUGEND{13U};
    constexpr uint64_t ADDEND{42U};

    Client<DummyRequest, DummyResponse> client{sd};
    Server<DummyRequest, DummyResponse> server{sd};

    // send request
    {
        auto loanResult = client.loan();
        ASSERT_FALSE(loanResult.has_error());
        auto& request = loanResult.value();
        request.getRequestHeader().setSequenceId(SEQUENCE_ID);
        request->augend = AUGEND;
        request->addend = ADDEND;
        ASSERT_FALSE(client.send(std::move(request)).has_error());
    }

    // take request and send response
    {
        auto takeResult = server.take();
        ASSERT_FALSE(takeResult.has_error());
        auto& request = takeResult.value();

        auto loanResult = server.loan(request);
        ASSERT_FALSE(loanResult.has_error());
        auto& response = loanResult.value();
        response->sum = request->augend + request->addend;
        ASSERT_FALSE(server.send(std::move(response)).has_error());
    }

    // take response
    {
        auto takeResult = client.take();
        ASSERT_FALSE(takeResult.has_error());
        auto& response = takeResult.value();
        EXPECT_THAT(response.getResponseHeader().getSequenceId(), Eq(SEQUENCE_ID));
        EXPECT_THAT(response->sum, Eq(AUGEND + ADDEND));
    }
}

TEST_F(ClientServer_test, UnypedApiWithMatchingOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e0a26c45-8eb9-4a68-be23-60d447f6bdc8");

    constexpr int64_t SEQUENCE_ID{37};
    constexpr uint64_t AUGEND{7U};
    constexpr uint64_t ADDEND{4U};

    UntypedClient client{sd};
    UntypedServer server{sd};

    // send request
    {
        auto loanResult = client.loan(sizeof(DummyRequest), alignof(DummyRequest));
        ASSERT_FALSE(loanResult.has_error());
        auto request = static_cast<DummyRequest*>(loanResult.value());
        RequestHeader::fromPayload(request)->setSequenceId(SEQUENCE_ID);
        request->augend = AUGEND;
        request->addend = ADDEND;
        ASSERT_FALSE(client.send(request).has_error());
    }

    // take request and send response
    {
        auto takeResult = server.take();
        ASSERT_FALSE(takeResult.has_error());
        auto request = static_cast<const DummyRequest*>(takeResult.value());

        auto loanResult =
            server.loan(RequestHeader::fromPayload(request), sizeof(DummyResponse), alignof(DummyResponse));
        ASSERT_FALSE(loanResult.has_error());
        auto response = static_cast<DummyResponse*>(loanResult.value());
        response->sum = request->augend + request->addend;
        ASSERT_FALSE(server.send(response).has_error());
        server.releaseRequest(request);
    }

    // take response
    {
        auto takeResult = client.take();
        ASSERT_FALSE(takeResult.has_error());
        auto response = static_cast<const DummyResponse*>(takeResult.value());
        EXPECT_THAT(ResponseHeader::fromPayload(response)->getSequenceId(), Eq(SEQUENCE_ID));
        EXPECT_THAT(response->sum, Eq(AUGEND + ADDEND));
        client.releaseResponse(response);
    }
}

TEST_F(ClientServer_test, MultipleClientsWithMatchingOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "dba14d17-c2ee-4cfe-b535-7ad9ccf9d58a");

    constexpr uint64_t NUMBER_OF_REQUESTS{2U};

    const std::vector<int64_t> SEQUENCE_ID{111, 222};
    const std::vector<uint64_t> AUGEND{10U, 20U};
    const std::vector<uint64_t> ADDEND{11U, 22U};

    Client<DummyRequest, DummyResponse> client_1{sd};
    Client<DummyRequest, DummyResponse> client_2{sd};
    std::vector<Client<DummyRequest, DummyResponse>*> client{&client_1, &client_2};

    Server<DummyRequest, DummyResponse> server{sd};

    // send requests
    for (auto i = 0U; i < NUMBER_OF_REQUESTS; ++i)
    {
        auto loanResult = client[i]->loan(AUGEND[i], ADDEND[i]);
        ASSERT_FALSE(loanResult.has_error());
        auto& request = loanResult.value();
        request.getRequestHeader().setSequenceId(SEQUENCE_ID[i]);
        ASSERT_FALSE(request.send().has_error());
    }

    // take request and send response
    for (auto i = 0U; i < NUMBER_OF_REQUESTS; ++i)
    {
        auto takeResult = server.take();
        ASSERT_FALSE(takeResult.has_error());
        auto& request = takeResult.value();

        auto loanResult = server.loan(request, request->augend + request->addend);
        ASSERT_FALSE(loanResult.has_error());
        auto& response = loanResult.value();
        ASSERT_FALSE(response.send().has_error());
    }

    // take response
    for (auto i = 0U; i < NUMBER_OF_REQUESTS; ++i)
    {
        auto takeResult = client[i]->take();
        ASSERT_FALSE(takeResult.has_error());
        auto& response = takeResult.value();
        EXPECT_THAT(response.getResponseHeader().getSequenceId(), Eq(SEQUENCE_ID[i]));
        EXPECT_THAT(response->sum, Eq(AUGEND[i] + ADDEND[i]));
    }
}

TEST_F(ClientServer_test, ClientWithNotMatchingServiceDescriptionIsNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "f95b6904-1956-4610-8e09-edb23680689d");

    Client<DummyRequest, DummyResponse> client{sdUnmatch};
    Server<DummyRequest, DummyResponse> server{sd};

    EXPECT_FALSE(server.hasClients());
    EXPECT_THAT(client.getConnectionState(), Ne(iox::ConnectionState::CONNECTED));
}

TEST_F(ClientServer_test, ClientWithNotMatchingResponseQueueFullPolicyIsNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "311ca039-ed59-4602-ba98-5f5767a4fe68");

    clientOptions.responseQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    serverOptions.clientTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;

    Client<DummyRequest, DummyResponse> client{sd, clientOptions};
    Server<DummyRequest, DummyResponse> server{sd, serverOptions};

    EXPECT_FALSE(server.hasClients());
    EXPECT_THAT(client.getConnectionState(), Ne(iox::ConnectionState::CONNECTED));
}

TEST_F(ClientServer_test, ClientWithNotMatchingServerTooSlowPolicyIsNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ad6b384-dd14-4b6a-bb81-bbf4f9d9cfec");

    clientOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;
    serverOptions.requestQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;

    Client<DummyRequest, DummyResponse> client{sd, clientOptions};
    Server<DummyRequest, DummyResponse> server{sd, serverOptions};

    EXPECT_FALSE(server.hasClients());
    EXPECT_THAT(client.getConnectionState(), Ne(iox::ConnectionState::CONNECTED));
}

TEST_F(ClientServer_test, SlowServerDoesNotBlockClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "5866ef06-941d-4f72-858f-cd07dd26c4fc");

    constexpr int64_t SEQUENCE_ID{42};
    constexpr int64_t NUMBER_OF_OVERFLOWS{1};

    clientOptions.responseQueueCapacity = 10U;
    serverOptions.requestQueueCapacity = 1U;

    Client<DummyRequest, DummyResponse> client{sd, clientOptions};
    Server<DummyRequest, DummyResponse> server{sd, serverOptions};

    // send request till queue overflows
    const int64_t iMax = static_cast<int64_t>(serverOptions.requestQueueCapacity) + NUMBER_OF_OVERFLOWS;
    for (int64_t i = 0; i < iMax; ++i)
    {
        auto loanResult = client.loan();
        ASSERT_FALSE(loanResult.has_error());
        auto& request = loanResult.value();
        request.getRequestHeader().setSequenceId(SEQUENCE_ID + i);
        EXPECT_FALSE(request.send().has_error());
    }

    auto takeResult = server.take();
    EXPECT_FALSE(takeResult.has_error());
    auto& response = takeResult.value();
    EXPECT_THAT(response.getRequestHeader().getSequenceId(), Eq(SEQUENCE_ID + NUMBER_OF_OVERFLOWS));
}

TEST_F(ClientServer_test, SlowClientDoesNotBlockServer)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6b07850-2b95-4977-ae7e-ff2ff64175a5");

    clientOptions.responseQueueCapacity = 1U;
    serverOptions.requestQueueCapacity = 10U;

    Client<DummyRequest, DummyResponse> client{sd, clientOptions};
    Server<DummyRequest, DummyResponse> server{sd, serverOptions};

    int64_t latestSequenceId{13};
    // send request and responses
    for (uint64_t i = 0; i < serverOptions.requestQueueCapacity; ++i)
    {
        // send request
        {
            auto loanResult = client.loan();
            ASSERT_FALSE(loanResult.has_error());
            auto& request = loanResult.value();
            ++latestSequenceId;
            request.getRequestHeader().setSequenceId(latestSequenceId);
            EXPECT_FALSE(request.send().has_error());
        }

        // take request and send response
        {
            auto takeResult = server.take();
            ASSERT_FALSE(takeResult.has_error());
            auto& request = takeResult.value();
            auto loanResult = server.loan(request);
            ASSERT_FALSE(loanResult.has_error());
            auto& response = loanResult.value();
            ASSERT_FALSE(response.send().has_error());
        }
    }

    auto takeResult = client.take();
    EXPECT_FALSE(takeResult.has_error());
    auto& response = takeResult.value();
    EXPECT_THAT(response.getResponseHeader().getSequenceId(), Eq(latestSequenceId));
}

TEST_F(ClientServer_test, ServerTakeRequestUnblocksClientSendingRequest)
{
    ::testing::Test::RecordProperty("TEST_ID", "c92e454c-f851-418b-80fa-cfbf79aadaa2");

    clientOptions.responseQueueCapacity = 10U;
    clientOptions.responseQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    clientOptions.serverTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    serverOptions.requestQueueCapacity = 1U;
    serverOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    serverOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    Client<DummyRequest, DummyResponse> client{sd, clientOptions};
    Server<DummyRequest, DummyResponse> server{sd, serverOptions};

    ASSERT_TRUE(server.hasClients());
    ASSERT_THAT(client.getConnectionState(), Eq(iox::ConnectionState::CONNECTED));

    iox::concurrent::Atomic<bool> wasRequestSent{false};

    // block in a separate thread
    Barrier isThreadStarted(1U);
    std::thread blockingClient([&] {
        auto sendRequest = [&]() {
            auto loanResult = client.loan();
            ASSERT_FALSE(loanResult.has_error());
            EXPECT_FALSE(loanResult.value().send().has_error());
        };

        // send request till queue is full
        for (uint64_t i = 0; i < serverOptions.requestQueueCapacity; ++i)
        {
            sendRequest();
        }

        // signal that an blocking send is expected
        isThreadStarted.notify();
        sendRequest();
        wasRequestSent = true;
    });

    // wait some time to check if the client is blocked
    constexpr std::chrono::milliseconds SLEEP_TIME{100U};
    isThreadStarted.wait();
    std::this_thread::sleep_for(SLEEP_TIME);
    EXPECT_THAT(wasRequestSent.load(), Eq(false));

    EXPECT_FALSE(server.take().has_error());

    blockingClient.join(); // ensure the wasRequestSent store happens before the read
    EXPECT_THAT(wasRequestSent.load(), Eq(true));
}

TEST_F(ClientServer_test, ClientTakesResponseUnblocksServerSendingResponse)
{
    ::testing::Test::RecordProperty("TEST_ID", "79ee88e5-ca7f-4908-8405-78b71d3fc9ab");

    clientOptions.responseQueueCapacity = 1U;
    clientOptions.responseQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    clientOptions.serverTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    serverOptions.requestQueueCapacity = 10U;
    serverOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    serverOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    Client<DummyRequest, DummyResponse> client{sd, clientOptions};
    Server<DummyRequest, DummyResponse> server{sd, serverOptions};

    ASSERT_TRUE(server.hasClients());
    ASSERT_THAT(client.getConnectionState(), Eq(iox::ConnectionState::CONNECTED));

    // send requests to fill request queue
    for (uint64_t i = 0; i < clientOptions.responseQueueCapacity + 1; ++i)
    {
        auto clientLoanResult = client.loan();
        ASSERT_FALSE(clientLoanResult.has_error());
        EXPECT_FALSE(clientLoanResult.value().send().has_error());
    }

    iox::concurrent::Atomic<bool> wasResponseSent{false};

    // block in a separate thread
    Barrier isThreadStarted(1U);
    std::thread blockingServer([&] {
        auto processRequest = [&]() {
            auto takeResult = server.take();
            ASSERT_FALSE(takeResult.has_error());
            auto loanResult = server.loan(takeResult.value());
            ASSERT_FALSE(loanResult.has_error());
            EXPECT_FALSE(loanResult.value().send().has_error());
        };

        for (uint64_t i = 0; i < clientOptions.responseQueueCapacity; ++i)
        {
            processRequest();
        }

        isThreadStarted.notify();
        processRequest();
        wasResponseSent = true;
    });

    // wait some time to check if the server is blocked
    constexpr std::chrono::milliseconds SLEEP_TIME{100U};
    isThreadStarted.wait();
    std::this_thread::sleep_for(SLEEP_TIME);
    EXPECT_THAT(wasResponseSent.load(), Eq(false));

    EXPECT_FALSE(client.take().has_error());

    blockingServer.join(); // ensure the wasResponseSent store happens before the read
    EXPECT_THAT(wasResponseSent.load(), Eq(true));
}

#ifdef TEST_WITH_HUGE_PAYLOAD

TEST_F(BigPayloadClientServer_test, TypedApiWithBigPayloadWithMatchingOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9838d2dc-bd87-42aa-b581-a9526e35e46a");

    constexpr int64_t SEQUENCE_ID{73};
    constexpr uint64_t FIRST{4095};
    constexpr uint64_t LAST{SIZE_LARGER_THAN_4GB - 1};
    constexpr uint64_t STEP{4096};
    constexpr uint8_t SHIFT{13U};

    Client<BigPayloadStruct, BigPayloadStruct> client{sd};
    Server<BigPayloadStruct, BigPayloadStruct> server{sd};

    // send request
    {
        auto loanResult = client.loan();
        ASSERT_FALSE(loanResult.has_error());
        auto& request = loanResult.value();
        request.getRequestHeader().setSequenceId(SEQUENCE_ID);
        uint8_t valueCounter = 0;
        for (uint64_t i = FIRST; i <= LAST; i += STEP)
        {
            request->bigPayload[i] = valueCounter;
            valueCounter++;
        }
        ASSERT_FALSE(client.send(std::move(request)).has_error());
    }

    // take request and send response
    {
        auto takeResult = server.take();
        ASSERT_FALSE(takeResult.has_error());
        auto& request = takeResult.value();

        auto loanResult = server.loan(request);
        ASSERT_FALSE(loanResult.has_error());
        auto& response = loanResult.value();
        for (uint64_t i = FIRST; i <= LAST; i += STEP)
        {
            response->bigPayload[i] = request->bigPayload[i] + SHIFT;
        }
        ASSERT_FALSE(server.send(std::move(response)).has_error());
    }

    // take response
    {
        auto takeResult = client.take();
        ASSERT_FALSE(takeResult.has_error());
        auto& response = takeResult.value();
        EXPECT_THAT(response.getResponseHeader().getSequenceId(), Eq(SEQUENCE_ID));
        uint8_t valueCounter = 0;
        for (uint64_t i = FIRST; i <= LAST; i += STEP)
        {
            ASSERT_THAT(response->bigPayload[i], Eq(static_cast<uint8_t>(valueCounter + SHIFT)));
            valueCounter++;
        }
    }
}

TEST_F(BigPayloadClientServer_test, UntypedApiWithBigPayloadWithMatchingOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c784d7f-6fe8-2137-b267-7f3e70a307f3");

    constexpr int64_t SEQUENCE_ID{37};
    constexpr uint64_t FIRST{4095};
    constexpr uint64_t LAST{SIZE_LARGER_THAN_4GB - 1};
    constexpr uint64_t STEP{4096};
    constexpr uint8_t SHIFT{13U};

    UntypedClient client{sd};
    UntypedServer server{sd};

    // send request
    {
        auto loanResult = client.loan(sizeof(BigPayloadStruct), alignof(BigPayloadStruct));
        ASSERT_FALSE(loanResult.has_error());
        auto request = static_cast<BigPayloadStruct*>(loanResult.value());
        RequestHeader::fromPayload(request)->setSequenceId(SEQUENCE_ID);
        uint8_t valueCounter = 0;
        for (uint64_t i = FIRST; i <= LAST; i += STEP)
        {
            request->bigPayload[i] = valueCounter;
            valueCounter++;
        }
        ASSERT_FALSE(client.send(request).has_error());
    }

    // take request and send response
    {
        auto takeResult = server.take();
        ASSERT_FALSE(takeResult.has_error());
        auto request = static_cast<const BigPayloadStruct*>(takeResult.value());

        auto loanResult =
            server.loan(RequestHeader::fromPayload(request), sizeof(BigPayloadStruct), alignof(BigPayloadStruct));
        ASSERT_FALSE(loanResult.has_error());
        auto response = static_cast<BigPayloadStruct*>(loanResult.value());
        for (uint64_t i = FIRST; i <= LAST; i += STEP)
        {
            response->bigPayload[i] = request->bigPayload[i] + SHIFT;
        }
        ASSERT_FALSE(server.send(response).has_error());
        server.releaseRequest(request);
    }

    // take response
    {
        auto takeResult = client.take();
        ASSERT_FALSE(takeResult.has_error());
        auto response = static_cast<const BigPayloadStruct*>(takeResult.value());
        EXPECT_THAT(ResponseHeader::fromPayload(response)->getSequenceId(), Eq(SEQUENCE_ID));
        uint8_t valueCounter = 0;
        for (uint64_t i = FIRST; i <= LAST; i += STEP)
        {
            ASSERT_THAT(response->bigPayload[i], Eq(static_cast<uint8_t>(valueCounter + SHIFT)));
            valueCounter++;
        }
        client.releaseResponse(response);
    }
}

#endif // RUN_BIG_PAYLOAD_TESTS

} // namespace
