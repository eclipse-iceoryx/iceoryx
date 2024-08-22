// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/ports/client_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iox/assertions.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::testing;
using namespace iox::capro;
using namespace iox::popo;

class ClientPort_test : public Test
{
    static constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    Watchdog m_deadlockWatchdog{DEADLOCK_TIMEOUT};

    struct SutClientPort
    {
        SutClientPort(const ServiceDescription& serviceDescription,
                      const iox::RuntimeName_t& runtimeName,
                      const ClientOptions& clientOptions,
                      iox::mepoo::MemoryManager& memoryManager)
            : portData(
                serviceDescription, runtimeName, iox::roudi::DEFAULT_UNIQUE_ROUDI_ID, clientOptions, &memoryManager)
        {
        }

        ClientPortData portData;
        ClientPortUser portUser{portData};
        ClientPortRouDi portRouDi{portData};
        ChunkQueuePusher<ClientChunkQueueData_t> responseQueuePusher{&portData.m_chunkReceiverData};
    };

  public:
    ClientPort_test()
    {
        iox::mepoo::MePooConfig mempoolconf;
        mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS});
        m_memoryManager.configureMemoryManager(mempoolconf, m_memoryAllocator, m_memoryAllocator);
    }

    void SetUp() override
    {
        m_deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });

        // this is basically what RouDi does when a client is requested
        tryAdvanceToState(clientPortWithConnectOnCreate, iox::ConnectionState::CONNECTED);
        tryAdvanceToState(clientPortWithoutConnectOnCreate, iox::ConnectionState::NOT_CONNECTED);
    }

    void TearDown() override
    {
    }

    void tryAdvanceToState(SutClientPort& clientPort, const iox::ConnectionState targetState)
    {
        auto maybeCaProMessage = clientPort.portRouDi.tryGetCaProMessage();
        if (targetState == iox::ConnectionState::NOT_CONNECTED
            && clientPort.portData.m_connectionState.load() == targetState)
        {
            return;
        }

        ASSERT_TRUE(maybeCaProMessage.has_value());
        auto& clientMessage = maybeCaProMessage.value();
        ASSERT_THAT(clientMessage.m_type, Eq(CaproMessageType::CONNECT));
        ASSERT_THAT(clientMessage.m_chunkQueueData, Ne(nullptr));
        ASSERT_THAT(clientPort.portData.m_connectionState.load(), Eq(iox::ConnectionState::CONNECT_REQUESTED));
        if (clientPort.portData.m_connectionState.load() == targetState)
        {
            return;
        }

        if (targetState == iox::ConnectionState::WAIT_FOR_OFFER)
        {
            CaproMessage serverMessageNack{CaproMessageType::NACK, m_serviceDescription};
            clientPort.portRouDi.dispatchCaProMessageAndGetPossibleResponse(serverMessageNack);
            ASSERT_THAT(clientPort.portData.m_connectionState.load(), Eq(targetState));
            return;
        }

        CaproMessage serverMessageAck{CaproMessageType::ACK, m_serviceDescription};
        serverMessageAck.m_chunkQueueData = &serverChunkQueueData;
        clientPort.portRouDi.dispatchCaProMessageAndGetPossibleResponse(serverMessageAck);
        ASSERT_THAT(clientPort.portData.m_connectionState.load(), Eq(iox::ConnectionState::CONNECTED));
        if (clientPort.portData.m_connectionState.load() == targetState)
        {
            return;
        }

        CaproMessage serverMessageDisconnect{CaproMessageType::DISCONNECT, m_serviceDescription};
        clientPort.portRouDi.dispatchCaProMessageAndGetPossibleResponse(serverMessageDisconnect);
        ASSERT_THAT(clientPort.portData.m_connectionState.load(), Eq(iox::ConnectionState::DISCONNECT_REQUESTED));
        if (clientPort.portData.m_connectionState.load() == targetState)
        {
            return;
        }

        constexpr bool NOT_IMPLEMENTED{true};
        ASSERT_FALSE(NOT_IMPLEMENTED);
    }

    SutClientPort& initAndGetClientPortForStateTransitionTests()
    {
        clientPortForStateTransitionTests.reset();
        clientPortForStateTransitionTests.emplace(
            m_serviceDescription, m_runtimeName, m_clientOptionsWithoutConnectOnCreate, m_memoryManager);
        return clientPortForStateTransitionTests.value();
    }

    uint32_t getNumberOfUsedChunks() const
    {
        return m_memoryManager.getMemPoolInfo(0U).m_usedChunks;
    }

    iox::mepoo::SharedChunk getChunkFromMemoryManager(uint64_t userPayloadSize, uint32_t userHeaderSize)
    {
        auto chunkSettings = iox::mepoo::ChunkSettings::create(userPayloadSize,
                                                               iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT,
                                                               userHeaderSize,
                                                               iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT)
                                 .expect("Valid 'ChunkSettings'");

        return m_memoryManager.getChunk(chunkSettings).expect("Obtaining chunk");
    }

    /// @return true if all pushes succeed, false if a push failed and a chunk was lost
    bool pushResponses(ChunkQueuePusher<ClientChunkQueueData_t>& chunkQueuePusher, uint64_t numberOfPushes)
    {
        for (auto i = 0U; i < numberOfPushes; ++i)
        {
            constexpr uint64_t USER_PAYLOAD_SIZE{10};
            auto sharedChunk = getChunkFromMemoryManager(USER_PAYLOAD_SIZE, sizeof(ResponseHeader));
            if (!chunkQueuePusher.push(sharedChunk))
            {
                chunkQueuePusher.lostAChunk();
                return false;
            }
        }
        return true;
    }

    static constexpr uint64_t QUEUE_CAPACITY{4};

  private:
    static constexpr uint32_t NUM_CHUNKS = 1024U;
    static constexpr uint64_t CHUNK_SIZE = 128U;
    static constexpr size_t MEMORY_SIZE = 1024U * 1024U;
    uint8_t m_memory[MEMORY_SIZE];
    iox::BumpAllocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MemoryManager m_memoryManager;

    ServiceDescription m_serviceDescription{"hyp", "no", "toad"};
    iox::RuntimeName_t m_runtimeName{"hypnotoad"};

    ClientOptions m_clientOptionsWithConnectOnCreate = [&] {
        ClientOptions options;
        options.connectOnCreate = true;
        options.responseQueueCapacity = QUEUE_CAPACITY;
        return options;
    }();
    ClientOptions m_clientOptionsWithoutConnectOnCreate = [] {
        ClientOptions options;
        options.connectOnCreate = false;
        options.responseQueueCapacity = QUEUE_CAPACITY;
        return options;
    }();

    ClientOptions m_clientOptionsWithBlockProducerResponseQueueFullPolicy = [&] {
        ClientOptions options;
        options.responseQueueCapacity = QUEUE_CAPACITY;
        options.responseQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
        return options;
    }();

    ClientOptions m_clientOptionsWithWaitForConsumerServerTooSlowPolicy = [&] {
        ClientOptions options;
        options.responseQueueCapacity = QUEUE_CAPACITY;
        options.serverTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
        return options;
    }();

    iox::optional<SutClientPort> clientPortForStateTransitionTests;

  public:
    static constexpr uint64_t USER_PAYLOAD_SIZE{32U};
    static constexpr uint32_t USER_PAYLOAD_ALIGNMENT{8U};

    ServerChunkQueueData_t serverChunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                iox::popo::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};
    ChunkQueuePopper<ServerChunkQueueData_t> serverRequestQueue{&serverChunkQueueData};

    SutClientPort clientPortWithConnectOnCreate{
        m_serviceDescription, m_runtimeName, m_clientOptionsWithConnectOnCreate, m_memoryManager};
    SutClientPort clientPortWithoutConnectOnCreate{
        m_serviceDescription, m_runtimeName, m_clientOptionsWithoutConnectOnCreate, m_memoryManager};
    SutClientPort clientPortWithBlockProducerResponseQueuePolicy{
        m_serviceDescription, m_runtimeName, m_clientOptionsWithBlockProducerResponseQueueFullPolicy, m_memoryManager};
    SutClientPort clientPortWithWaitForConsumerServerTooSlowPolicy{
        m_serviceDescription, m_runtimeName, m_clientOptionsWithWaitForConsumerServerTooSlowPolicy, m_memoryManager};
};
constexpr iox::units::Duration ClientPort_test::DEADLOCK_TIMEOUT;

// NOTE tests related to QueueFullPolicy are done in test_client_server.cpp integration test

// BEGIN ClientPortUser tests

TEST_F(ClientPort_test, InitialConnectionStateOnPortWithConnectOnCreateIs_CONNECTED)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d6dd457-b111-45d8-8bac-ae354288ff93");
    auto& sut = clientPortWithConnectOnCreate;
    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::CONNECTED));
}

TEST_F(ClientPort_test, InitialConnectionStateOnPortWithoutConnectOnCreateIs_NOT_CONNECTED)
{
    ::testing::Test::RecordProperty("TEST_ID", "068d6415-1554-4f67-85da-0dd1dab77e68");
    auto& sut = clientPortWithoutConnectOnCreate;
    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::NOT_CONNECTED));
}

TEST_F(ClientPort_test, AllocateRequestDoesNotFailAndUsesTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "d82b0152-8ed4-4022-ada8-f8926f27a9b1");
    auto& sut = clientPortWithConnectOnCreate;
    EXPECT_THAT(getNumberOfUsedChunks(), Eq(0U));

    auto maybeRequest = sut.portUser.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(maybeRequest.has_error());

    EXPECT_THAT(getNumberOfUsedChunks(), Eq(1U));
}

TEST_F(ClientPort_test, ReleaseRequestWithNullptrCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "f21bc4ab-4080-4994-b862-5cb8c8738b46");
    auto& sut = clientPortWithConnectOnCreate;

    sut.portUser.releaseRequest(nullptr);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__CLIENT_PORT_INVALID_REQUEST_TO_FREE_FROM_USER);
}

TEST_F(ClientPort_test, ReleaseRequestWithValidRequestWorksAndReleasesTheChunkToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2eb1ec3-78de-453b-bf97-860f3c57362b");
    auto& sut = clientPortWithConnectOnCreate;
    sut.portUser.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](auto& requestHeader) {
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(1U));
            sut.portUser.releaseRequest(requestHeader);
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(0U));
        })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });
}

TEST_F(ClientPort_test, SendRequestWithNullptrOnConnectedClientPortCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "e50da541-7621-46e8-accb-46a6b5d7e69b");
    auto& sut = clientPortWithConnectOnCreate;

    sut.portUser.sendRequest(nullptr)
        .and_then([&]() { GTEST_FAIL() << "Expected request not successfully sent"; })
        .or_else([&](auto error) { EXPECT_THAT(error, Eq(ClientSendError::INVALID_REQUEST)); });

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__CLIENT_PORT_INVALID_REQUEST_TO_SEND_FROM_USER);
}

TEST_F(ClientPort_test, SendRequestOnConnectedClientPortEnqueuesRequestToServerQueue)
{
    ::testing::Test::RecordProperty("TEST_ID", "861efd1d-31ae-436d-9a0c-84da5bf99a57");
    constexpr int64_t SEQUENCE_ID{42U};
    auto& sut = clientPortWithConnectOnCreate;
    auto allocateResult = sut.portUser.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(allocateResult.has_error());
    auto* requestHeader = allocateResult.value();
    requestHeader->setSequenceId(SEQUENCE_ID);
    sut.portUser.sendRequest(requestHeader)
        .and_then([&]() { GTEST_SUCCEED() << "Request successfully sent"; })
        .or_else([&](auto error) { GTEST_FAIL() << "Expected response to be sent but got error: " << error; });

    serverRequestQueue.tryPop()
        .and_then([&](auto& sharedChunk) {
            auto requestHeader = static_cast<RequestHeader*>(sharedChunk.getChunkHeader()->userHeader());
            EXPECT_THAT(requestHeader->getSequenceId(), Eq(SEQUENCE_ID));
        })
        .or_else([&] {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });
}

TEST_F(ClientPort_test,
       SendRequestOnNotConnectedClientPortDoesNotEnqueuesRequestToServerQueueAndReleasesTheChunkToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "46c418a8-4f4f-4393-a190-8f5d41deb05e");
    auto& sut = clientPortWithoutConnectOnCreate;
    auto allocateResult = sut.portUser.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(allocateResult.has_error());
    auto* requestHeader = allocateResult.value();

    EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(1U));
    sut.portUser.sendRequest(requestHeader)
        .and_then([&]() { GTEST_FAIL() << "Expected request not successfully sent"; })
        .or_else([&](auto error) { EXPECT_THAT(error, Eq(ClientSendError::NO_CONNECT_REQUESTED)); });
    EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(0U));

    EXPECT_FALSE(serverRequestQueue.tryPop().has_value());
}

TEST_F(ClientPort_test, ConnectAfterPreviousSendRequestCallDoesNotEnqueuesRequestToServerQueue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3348d22d-d08e-4855-8316-8b2ce77274ee");
    auto& sut = clientPortWithoutConnectOnCreate;
    auto allocateResult = sut.portUser.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(allocateResult.has_error());
    auto* requestHeader = allocateResult.value();

    sut.portUser.sendRequest(requestHeader)
        .and_then([&]() { GTEST_FAIL() << "Expected request not successfully sent"; })
        .or_else([&](auto error) { EXPECT_THAT(error, Eq(ClientSendError::NO_CONNECT_REQUESTED)); });

    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::CONNECTED);

    EXPECT_FALSE(serverRequestQueue.tryPop().has_value());
}

TEST_F(ClientPort_test, GetResponseOnNotConnectedClientPortHasNoResponse)
{
    ::testing::Test::RecordProperty("TEST_ID", "ecb320c9-1c95-410e-84d6-9aa9763b9768");
    auto& sut = clientPortWithoutConnectOnCreate;
    sut.portUser.getResponse()
        .and_then([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        })
        .or_else([&](auto& err) { EXPECT_THAT(err, Eq(iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)); });
}

TEST_F(ClientPort_test, GetResponseOnConnectedClientPortWithEmptyResponseQueueHasNoResponse)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e6efd53-c056-4d95-9d73-2fcfe7a6b69a");
    auto& sut = clientPortWithConnectOnCreate;
    sut.portUser.getResponse()
        .and_then([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        })
        .or_else([&](auto& err) { EXPECT_THAT(err, Eq(iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)); });
}

TEST_F(ClientPort_test, GetResponseOnConnectedClientPortWithNonEmptyResponseQueueHasResponse)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9625942-d69f-404a-a419-cf2f5f20dd85");
    constexpr int64_t SEQUENCE_ID{13U};
    auto& sut = clientPortWithConnectOnCreate;

    constexpr uint64_t USER_PAYLOAD_SIZE{10};
    auto sharedChunk = getChunkFromMemoryManager(USER_PAYLOAD_SIZE, sizeof(ResponseHeader));
    new (sharedChunk.getChunkHeader()->userHeader())
        ResponseHeader(iox::UniqueId(), RpcBaseHeader::UNKNOWN_CLIENT_QUEUE_INDEX, SEQUENCE_ID);
    sut.responseQueuePusher.push(sharedChunk);

    sut.portUser.getResponse()
        .and_then([&](auto& responseHeader) { EXPECT_THAT(responseHeader->getSequenceId(), Eq(SEQUENCE_ID)); })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });
}

TEST_F(ClientPort_test, ReleaseResponseWithNullptrCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "b6ad4c2a-7c52-45ee-afd3-29c286489311");
    auto& sut = clientPortWithConnectOnCreate;

    sut.portUser.releaseResponse(nullptr);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POPO__CLIENT_PORT_INVALID_RESPONSE_TO_RELEASE_FROM_USER);
}

TEST_F(ClientPort_test, ReleaseResponseWithValidResponseReleasesChunkToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f625d3e-9ef3-4329-9c80-95af0327cbc0");
    auto& sut = clientPortWithConnectOnCreate;

    constexpr uint64_t USER_PAYLOAD_SIZE{10};

    iox::optional<iox::mepoo::SharedChunk> sharedChunk{
        getChunkFromMemoryManager(USER_PAYLOAD_SIZE, sizeof(ResponseHeader))};
    sut.responseQueuePusher.push(sharedChunk.value());
    sharedChunk.reset();

    sut.portUser.getResponse()
        .and_then([&](auto& responseHeader) {
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(1U));
            sut.portUser.releaseResponse(responseHeader);
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(0U));
        })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });
}

TEST_F(ClientPort_test, ReleaseQueuedResponsesReleasesAllChunksToTheMempool)
{
    ::testing::Test::RecordProperty("TEST_ID", "d51674b7-ad92-47cc-85d9-06169e8a813b");
    auto& sut = clientPortWithConnectOnCreate;

    constexpr uint64_t USER_PAYLOAD_SIZE{10};
    constexpr uint32_t NUMBER_OF_QUEUED_RESPONSES{3};

    for (uint32_t i = 0; i < NUMBER_OF_QUEUED_RESPONSES; ++i)
    {
        iox::optional<iox::mepoo::SharedChunk> sharedChunk{
            getChunkFromMemoryManager(USER_PAYLOAD_SIZE, sizeof(ResponseHeader))};
        sut.responseQueuePusher.push(sharedChunk.value());
        sharedChunk.reset();
    }

    EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(NUMBER_OF_QUEUED_RESPONSES));
    sut.portUser.releaseQueuedResponses();
    EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(0U));
}

TEST_F(ClientPort_test, HasNewResponseOnEmptyResponseQueueReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "42f50429-e1e1-41b9-bbcd-5d14a0eda189");
    auto& sut = clientPortWithConnectOnCreate;
    EXPECT_FALSE(sut.portUser.hasNewResponses());
}

TEST_F(ClientPort_test, HasNewResponseOnNonEmptyResponseQueueReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b0dbb32-2d5b-4eac-96d3-6cf7a8cbac15");
    auto& sut = clientPortWithConnectOnCreate;

    constexpr uint64_t USER_PAYLOAD_SIZE{10};
    auto sharedChunk = getChunkFromMemoryManager(USER_PAYLOAD_SIZE, sizeof(ResponseHeader));
    sut.responseQueuePusher.push(sharedChunk);

    EXPECT_TRUE(sut.portUser.hasNewResponses());
}

TEST_F(ClientPort_test, HasNewResponseOnEmptyResponseQueueAfterPreviouslyNotEmptyReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "9cd91de8-9687-436a-9d7d-95d2754eee30");
    auto& sut = clientPortWithConnectOnCreate;

    constexpr uint64_t USER_PAYLOAD_SIZE{10};
    auto sharedChunk = getChunkFromMemoryManager(USER_PAYLOAD_SIZE, sizeof(ResponseHeader));
    sut.responseQueuePusher.push(sharedChunk);

    EXPECT_FALSE(sut.portUser.getResponse().has_error());

    EXPECT_FALSE(sut.portUser.hasNewResponses());
}

TEST_F(ClientPort_test, HasLostResponsesSinceLastCallWithoutLosingResponsesReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "8eba3173-6b4a-4073-90ad-133e279a6215");
    auto& sut = clientPortWithConnectOnCreate;
    EXPECT_FALSE(sut.portUser.hasLostResponsesSinceLastCall());
}

TEST_F(ClientPort_test, HasLostResponsesSinceLastCallWithoutLosingResponsesAndQueueFullReturnsFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a5765a9-dc20-40fc-95ea-84391e7a927e");
    auto& sut = clientPortWithConnectOnCreate;

    EXPECT_TRUE(pushResponses(sut.responseQueuePusher, QUEUE_CAPACITY));
    EXPECT_FALSE(sut.portUser.hasLostResponsesSinceLastCall());
}

TEST_F(ClientPort_test, HasLostResponsesSinceLastCallWithLosingResponsesReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "28bb1d1f-4b6f-4f03-ba31-24fa4d75a44d");
    auto& sut = clientPortWithConnectOnCreate;

    EXPECT_FALSE(pushResponses(sut.responseQueuePusher, QUEUE_CAPACITY + 1U));
    EXPECT_TRUE(sut.portUser.hasLostResponsesSinceLastCall());
}

TEST_F(ClientPort_test, HasLostResponsesSinceLastCallReturnsFalseAfterPreviouslyReturningTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "233cf99e-52fc-4e9c-b2bf-77928a4370ab");
    auto& sut = clientPortWithConnectOnCreate;

    EXPECT_FALSE(pushResponses(sut.responseQueuePusher, QUEUE_CAPACITY + 1U));
    EXPECT_TRUE(sut.portUser.hasLostResponsesSinceLastCall());
    EXPECT_FALSE(sut.portUser.hasLostResponsesSinceLastCall());
}

TEST_F(ClientPort_test, ConditionVariableInitiallyNotSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "a9b75cb2-9968-4b90-b444-92d8cff2ca97");
    auto& sut = clientPortWithConnectOnCreate;
    EXPECT_FALSE(sut.portUser.isConditionVariableSet());
}

TEST_F(ClientPort_test, SettingConditionVariableWithoutConditionVariablePresentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "86c03248-f9a6-4f4b-830f-fac5ec8c5cc3");
    iox::popo::ConditionVariableData condVar{"hypnotoad"};
    constexpr uint32_t NOTIFICATION_INDEX{1};

    auto& sut = clientPortWithConnectOnCreate;
    sut.portUser.setConditionVariable(condVar, NOTIFICATION_INDEX);

    EXPECT_TRUE(sut.portUser.isConditionVariableSet());
}

TEST_F(ClientPort_test, UnsettingConditionVariableWithConditionVariablePresentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f10db20-e236-4b9d-9162-4d8ea5c9f4c9");
    iox::popo::ConditionVariableData condVar{"brain slug"};
    constexpr uint32_t NOTIFICATION_INDEX{2};

    auto& sut = clientPortWithConnectOnCreate;
    sut.portUser.setConditionVariable(condVar, NOTIFICATION_INDEX);

    sut.portUser.unsetConditionVariable();

    EXPECT_FALSE(sut.portUser.isConditionVariableSet());
}

TEST_F(ClientPort_test, UnsettingConditionVariableWithoutConditionVariablePresentIsHandledGracefully)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e89da27-ba82-46f7-ad41-844373e103e7");
    auto& sut = clientPortWithConnectOnCreate;
    sut.portUser.unsetConditionVariable();

    EXPECT_FALSE(sut.portUser.isConditionVariableSet());
}

TEST_F(ClientPort_test, ConnectOnNotConnectedClientPortResultsInStateChange)
{
    ::testing::Test::RecordProperty("TEST_ID", "52c6cc2f-58c9-4215-9c91-71f0e7b8e40d");
    auto& sut = clientPortWithoutConnectOnCreate;

    sut.portUser.connect();

    EXPECT_TRUE(sut.portRouDi.tryGetCaProMessage().has_value());
}

TEST_F(ClientPort_test, ConnectOnConnectedClientPortResultsInNoStateChange)
{
    ::testing::Test::RecordProperty("TEST_ID", "08e3e53b-9303-4d5f-8f1d-c5878adf5783");
    auto& sut = clientPortWithConnectOnCreate;

    sut.portUser.connect();

    EXPECT_FALSE(sut.portRouDi.tryGetCaProMessage().has_value());
}

TEST_F(ClientPort_test, DisconnectOnConnectedClientPortResultsInStateChange)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d1d4ce8-737f-4438-bd61-173625032c76");
    auto& sut = clientPortWithConnectOnCreate;

    sut.portUser.disconnect();

    EXPECT_TRUE(sut.portRouDi.tryGetCaProMessage().has_value());
}

TEST_F(ClientPort_test, DisconnectOnNotConnectedClientPortResultsInNoStateChange)
{
    ::testing::Test::RecordProperty("TEST_ID", "82ff5a16-2b4f-4480-88b1-8983242ed677");
    auto& sut = clientPortWithoutConnectOnCreate;

    sut.portUser.disconnect();

    EXPECT_FALSE(sut.portRouDi.tryGetCaProMessage().has_value());
}

TEST_F(ClientPort_test, asStringLiteralConvertsClientSendErrorValuesToStrings)
{
    ::testing::Test::RecordProperty("TEST_ID", "9faca6d8-ea10-4577-b37a-73f346ae4adc");
    using ClientSendError = iox::popo::ClientSendError;

    // each bit corresponds to an enum value and must be set to true on test
    uint64_t testedEnumValues{0U};
    uint64_t loopCounter{0U};
    for (const auto& sut : {ClientSendError::NO_CONNECT_REQUESTED,
                            ClientSendError::SERVER_NOT_AVAILABLE,
                            ClientSendError::INVALID_REQUEST})
    {
        auto enumString = iox::popo::asStringLiteral(sut);

        switch (sut)
        {
        case ClientSendError::NO_CONNECT_REQUESTED:
            EXPECT_THAT(enumString, StrEq("ClientSendError::NO_CONNECT_REQUESTED"));
            break;
        case ClientSendError::SERVER_NOT_AVAILABLE:
            EXPECT_THAT(enumString, StrEq("ClientSendError::SERVER_NOT_AVAILABLE"));
            break;
        case ClientSendError::INVALID_REQUEST:
            EXPECT_THAT(enumString, StrEq("ClientSendError::INVALID_REQUEST"));
            break;
        }

        testedEnumValues |= 1U << static_cast<uint64_t>(sut);
        ++loopCounter;
    }

    uint64_t expectedTestedEnumValues = (1U << loopCounter) - 1;
    EXPECT_EQ(testedEnumValues, expectedTestedEnumValues);
}

TEST_F(ClientPort_test, LogStreamConvertsClientSendErrorValueToString)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5b4421c-6b05-44ea-b7a6-823b3714fabd");
    iox::testing::Logger_Mock loggerMock;

    auto sut = iox::popo::ClientSendError::SERVER_NOT_AVAILABLE;

    {
        IOX_LOGSTREAM_MOCK(loggerMock) << sut;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(iox::popo::asStringLiteral(sut)));
}

// END ClientPortUser tests


// BEGIN ClientPortRouDi tests

TEST_F(ClientPort_test, GetResponseQueueFullPolicyOnPortWithDefaultOptionIsDiscardOldesData)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf169034-c413-4362-a6cd-72ec0d6cf958");
    auto& sut = clientPortWithConnectOnCreate;

    EXPECT_THAT(sut.portRouDi.getResponseQueueFullPolicy(), Eq(QueueFullPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(ClientPort_test, GetResponseQueueFullPolicyOnPortWithBlockProducerOptionIsBlockProducer)
{
    ::testing::Test::RecordProperty("TEST_ID", "40c3b25e-8a95-415b-9acb-6a67fd7d868a");
    auto& sut = clientPortWithBlockProducerResponseQueuePolicy;

    EXPECT_THAT(sut.portRouDi.getResponseQueueFullPolicy(), Eq(QueueFullPolicy::BLOCK_PRODUCER));
}

TEST_F(ClientPort_test, GetServerTooSlowPolicyOnPortWithWaitForConsumerOptionIsWaitForConsumer)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0036542-bb93-4975-b70b-ec40b0947d13");
    auto& sut = clientPortWithWaitForConsumerServerTooSlowPolicy;

    EXPECT_THAT(sut.portRouDi.getServerTooSlowPolicy(), Eq(ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER));
}

TEST_F(ClientPort_test, TryGetCaProMessageOnConnectHasCaProMessageTypeConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "eac43f13-b486-4e8b-a5b9-4fc274113d08");
    auto& sut = clientPortWithoutConnectOnCreate;

    sut.portUser.connect();

    auto caproMessage = sut.portRouDi.tryGetCaProMessage();

    ASSERT_TRUE(caproMessage.has_value());
    EXPECT_THAT(caproMessage->m_type, Eq(CaproMessageType::CONNECT));
}

TEST_F(ClientPort_test, TryGetCaProMessageOnDisconnectHasCaProMessageTypeDisconnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "53bb7a12-affb-4ad0-8846-4fb20bbe4a72");
    auto& sut = clientPortWithConnectOnCreate;

    sut.portUser.disconnect();

    auto caproMessage = sut.portRouDi.tryGetCaProMessage();

    ASSERT_TRUE(caproMessage.has_value());
    EXPECT_THAT(caproMessage->m_type, Eq(CaproMessageType::DISCONNECT));
}

TEST_F(ClientPort_test, ReleaseAllChunksWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0d88645-3c8f-47e1-8989-7557675c1207");
    auto& sut = clientPortWithConnectOnCreate;

    // produce chunks for the chunk sender
    sut.portUser.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](auto& requestHeader) { EXPECT_FALSE(sut.portUser.sendRequest(requestHeader).has_error()); })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });

    // produce chunks for the chunk receiver
    EXPECT_TRUE(pushResponses(sut.responseQueuePusher, QUEUE_CAPACITY));

    EXPECT_THAT(getNumberOfUsedChunks(), Ne(0U));

    sut.portRouDi.releaseAllChunks();

    // this is not part of the client port but holds the chunk from 'sendRequest'
    serverRequestQueue.clear();

    EXPECT_THAT(getNumberOfUsedChunks(), Eq(0U));
}

// BEGIN Valid transitions

TEST_F(ClientPort_test, StateNotConnectedWithCaProMessageTypeOfferRemainsInStateNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "849f1825-61da-4bad-8390-b14173905611");
    auto& sut = initAndGetClientPortForStateTransitionTests();

    auto caproMessage = CaproMessage{CaproMessageType::OFFER, sut.portData.m_serviceDescription};
    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::NOT_CONNECTED));
    ASSERT_FALSE(responseCaproMessage.has_value());
}

TEST_F(ClientPort_test, StateNotConnectedWithCaProMessageTypeConnectTransitionsToStateConnectRequested)
{
    ::testing::Test::RecordProperty("TEST_ID", "72c72160-f53e-4062-90cb-b7a51017b5be");
    auto& sut = initAndGetClientPortForStateTransitionTests();

    auto caproMessage = CaproMessage{CaproMessageType::CONNECT, sut.portData.m_serviceDescription};
    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::CONNECT_REQUESTED));
    ASSERT_TRUE(responseCaproMessage.has_value());
    EXPECT_THAT(responseCaproMessage->m_serviceDescription, Eq(sut.portData.m_serviceDescription));
    EXPECT_THAT(responseCaproMessage->m_type, Eq(iox::capro::CaproMessageType::CONNECT));
    EXPECT_THAT(responseCaproMessage->m_chunkQueueData, Eq(&sut.portData.m_chunkReceiverData));
}

TEST_F(ClientPort_test, StateConnectRequestedWithCaProMessageTypeNackTransitionsToStateWaitForOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8921cb0-6a8d-43a4-a6ef-384bd3475aae");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::CONNECT_REQUESTED);

    auto caproMessage = CaproMessage{CaproMessageType::NACK, sut.portData.m_serviceDescription};
    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::WAIT_FOR_OFFER));
    ASSERT_FALSE(responseCaproMessage.has_value());
}

TEST_F(ClientPort_test, StateConnectRequestedWithCaProMessageTypeAckTransitionsToStateConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "3651e440-9d20-48b8-bbf6-ca063f41b767");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::CONNECT_REQUESTED);

    auto caproMessage = CaproMessage{CaproMessageType::ACK, sut.portData.m_serviceDescription};
    caproMessage.m_chunkQueueData = &serverChunkQueueData;

    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::CONNECTED));
    ASSERT_FALSE(responseCaproMessage.has_value());
}

TEST_F(ClientPort_test, StateWaitForOfferWithCaProMessageTypeDisconnetTransitionsToStateNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa9925d1-e867-4155-aa8c-3bfa411b09db");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::WAIT_FOR_OFFER);

    auto caproMessage = CaproMessage{CaproMessageType::DISCONNECT, sut.portData.m_serviceDescription};
    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::NOT_CONNECTED));
    ASSERT_FALSE(responseCaproMessage.has_value());
}

TEST_F(ClientPort_test, StateWaitForOfferWithCaProMessageTypeOfferTransitionsToStateConnectRequested)
{
    ::testing::Test::RecordProperty("TEST_ID", "527a9ca0-f3c7-4bce-8e88-e4ab753358f1");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::WAIT_FOR_OFFER);

    auto caproMessage = CaproMessage{CaproMessageType::OFFER, sut.portData.m_serviceDescription};
    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::CONNECT_REQUESTED));
    ASSERT_TRUE(responseCaproMessage.has_value());
    EXPECT_THAT(responseCaproMessage->m_serviceDescription, Eq(sut.portData.m_serviceDescription));
    EXPECT_THAT(responseCaproMessage->m_type, Eq(iox::capro::CaproMessageType::CONNECT));
    EXPECT_THAT(responseCaproMessage->m_chunkQueueData, Eq(&sut.portData.m_chunkReceiverData));
}

TEST_F(ClientPort_test, StateConnectedWithCaProMessageTypeStopOfferTransitionsToStateWaitForOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c07d376-f316-4805-9a91-575289beae94");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::CONNECTED);

    auto caproMessage = CaproMessage{CaproMessageType::STOP_OFFER, sut.portData.m_serviceDescription};
    caproMessage.m_chunkQueueData = &serverChunkQueueData;

    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::WAIT_FOR_OFFER));
    ASSERT_FALSE(responseCaproMessage.has_value());
}

TEST_F(ClientPort_test, StateConnectedWithCaProMessageTypeDisconnectTransitionsToStateDisconnectRequested)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb3c606e-2ab0-4b76-a7dc-83bec1068171");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::CONNECTED);

    auto caproMessage = CaproMessage{CaproMessageType::DISCONNECT, sut.portData.m_serviceDescription};
    caproMessage.m_chunkQueueData = &serverChunkQueueData;

    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::DISCONNECT_REQUESTED));
    ASSERT_TRUE(responseCaproMessage.has_value());
    EXPECT_THAT(responseCaproMessage->m_serviceDescription, Eq(sut.portData.m_serviceDescription));
    EXPECT_THAT(responseCaproMessage->m_type, Eq(iox::capro::CaproMessageType::DISCONNECT));
    EXPECT_THAT(responseCaproMessage->m_chunkQueueData, Eq(&sut.portData.m_chunkReceiverData));
}

TEST_F(ClientPort_test, StateDisconnectRequestedWithCaProMessageTypeAckTransitionsToStateNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c5f2052-7397-4e23-b53a-8127cce62063");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::DISCONNECT_REQUESTED);

    auto caproMessage = CaproMessage{CaproMessageType::ACK, sut.portData.m_serviceDescription};
    caproMessage.m_chunkQueueData = &serverChunkQueueData;

    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::NOT_CONNECTED));
    ASSERT_FALSE(responseCaproMessage.has_value());
}

TEST_F(ClientPort_test, StateDisconnectRequestedWithCaProMessageTypeNackTransitionsToStateNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d24f15e-5ff3-4c96-8e74-a404cd7f3605");
    auto& sut = initAndGetClientPortForStateTransitionTests();
    sut.portUser.connect();
    tryAdvanceToState(sut, iox::ConnectionState::DISCONNECT_REQUESTED);

    auto caproMessage = CaproMessage{CaproMessageType::NACK, sut.portData.m_serviceDescription};
    caproMessage.m_chunkQueueData = &serverChunkQueueData;

    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    EXPECT_THAT(sut.portUser.getConnectionState(), Eq(iox::ConnectionState::NOT_CONNECTED));
    ASSERT_FALSE(responseCaproMessage.has_value());
}

// END Valid transitions


// BEGIN Invalid transitions

TEST_F(ClientPort_test, InvalidStateTransitionsCallErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "465258d2-b58d-41fe-bc18-e7fd43dd233d");
    constexpr iox::ConnectionState ALL_STATES[]{iox::ConnectionState::NOT_CONNECTED,
                                                iox::ConnectionState::CONNECT_REQUESTED,
                                                iox::ConnectionState::WAIT_FOR_OFFER,
                                                iox::ConnectionState::CONNECTED,
                                                iox::ConnectionState::DISCONNECT_REQUESTED};

    for (auto targetState : ALL_STATES)
    {
        for (int32_t i = 0; i < static_cast<int32_t>(CaproMessageType::MESSGAGE_TYPE_END); ++i)
        {
            auto caproMessageType = static_cast<CaproMessageType>(i);
            SCOPED_TRACE(std::string("Invalid transition test from ")
                         + iox::convert::toString(asStringLiteral(targetState)) + std::string(" with ")
                         + iox::convert::toString(asStringLiteral(caproMessageType)));

            // skip for valid transitions
            switch (targetState)
            {
            case iox::ConnectionState::NOT_CONNECTED:
                if (caproMessageType == CaproMessageType::CONNECT || caproMessageType == CaproMessageType::OFFER)
                {
                    continue;
                }
                break;
            case iox::ConnectionState::CONNECT_REQUESTED:
                if (caproMessageType == CaproMessageType::ACK || caproMessageType == CaproMessageType::NACK)
                {
                    continue;
                }
                break;
            case iox::ConnectionState::WAIT_FOR_OFFER:
                if (caproMessageType == CaproMessageType::DISCONNECT || caproMessageType == CaproMessageType::OFFER)
                {
                    continue;
                }
                break;
            case iox::ConnectionState::CONNECTED:
                if (caproMessageType == CaproMessageType::STOP_OFFER
                    || caproMessageType == CaproMessageType::DISCONNECT)
                {
                    continue;
                }
                break;
            case iox::ConnectionState::DISCONNECT_REQUESTED:
                if (caproMessageType == CaproMessageType::ACK || caproMessageType == CaproMessageType::NACK)
                {
                    continue;
                }
                break;
            }

            auto& sut = initAndGetClientPortForStateTransitionTests();
            if (targetState != iox::ConnectionState::NOT_CONNECTED)
            {
                sut.portUser.connect();
                tryAdvanceToState(sut, targetState);
            }

            IOX_EXPECT_FATAL_FAILURE(
                [&] {
                    auto caproMessage = CaproMessage{caproMessageType, sut.portData.m_serviceDescription};
                    auto responseCaproMessage = sut.portRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
                    ASSERT_FALSE(responseCaproMessage.has_value());
                },
                iox::PoshError::POPO__CAPRO_PROTOCOL_ERROR);
        }
    }
}

// END Invalid transitions

// END ClientPortRouDi tests

} // namespace
