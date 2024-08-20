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
#include "test_roudi_portmanager_fixture.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"

namespace iox_test_roudi_portmanager
{
using namespace iox::popo;

constexpr uint64_t RESPONSE_QUEUE_CAPACITY{2U};
constexpr uint64_t REQUEST_QUEUE_CAPACITY{2U};

ClientOptions createTestClientOptions()
{
    return ClientOptions{RESPONSE_QUEUE_CAPACITY, iox::NodeName_t("node")};
}

ServerOptions createTestServerOptions()
{
    return ServerOptions{REQUEST_QUEUE_CAPACITY, iox::NodeName_t("node")};
}

// BEGIN aquireClientPortData tests

TEST_F(PortManager_test, AcquireClientPortDataReturnsPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "92225f2c-619a-425b-bba0-6a014822c4c3");
    const ServiceDescription sd{"hyp", "no", "toad"};
    const RuntimeName_t runtimeName{"hypnotoad"};
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = false;
    clientOptions.responseQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    clientOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    m_portManager->acquireClientPortData(sd, clientOptions, runtimeName, m_payloadDataSegmentMemoryManager, {})
        .and_then([&](const auto& clientPortData) {
            EXPECT_THAT(clientPortData->m_serviceDescription, Eq(sd));
            EXPECT_THAT(clientPortData->m_runtimeName, Eq(runtimeName));
            EXPECT_THAT(clientPortData->m_toBeDestroyed.load(), Eq(false));
            EXPECT_THAT(clientPortData->m_chunkReceiverData.m_queue.capacity(),
                        Eq(clientOptions.responseQueueCapacity));
            EXPECT_THAT(clientPortData->m_connectRequested.load(), Eq(clientOptions.connectOnCreate));
            EXPECT_THAT(clientPortData->m_chunkReceiverData.m_queueFullPolicy,
                        Eq(clientOptions.responseQueueFullPolicy));
            EXPECT_THAT(clientPortData->m_chunkSenderData.m_consumerTooSlowPolicy,
                        Eq(clientOptions.serverTooSlowPolicy));
        })
        .or_else([&](const auto& error) {
            GTEST_FAIL() << "Expected ClientPortData but got PortPoolError: " << static_cast<uint8_t>(error);
        });
}

// END aquireClientPortData tests

// BEGIN aquireServerPortData tests

TEST_F(PortManager_test, AcquireServerPortDataReturnsPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "776c51c4-074a-4404-b6a7-ed08f59f05a0");
    const ServiceDescription sd{"hyp", "no", "toad"};
    const RuntimeName_t runtimeName{"hypnotoad"};
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = false;
    serverOptions.requestQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    serverOptions.clientTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    m_portManager->acquireServerPortData(sd, serverOptions, runtimeName, m_payloadDataSegmentMemoryManager, {})
        .and_then([&](const auto& serverPortData) {
            EXPECT_THAT(serverPortData->m_serviceDescription, Eq(sd));
            EXPECT_THAT(serverPortData->m_runtimeName, Eq(runtimeName));
            EXPECT_THAT(serverPortData->m_toBeDestroyed.load(), Eq(false));
            EXPECT_THAT(serverPortData->m_chunkReceiverData.m_queue.capacity(), Eq(serverOptions.requestQueueCapacity));
            EXPECT_THAT(serverPortData->m_offeringRequested.load(), Eq(serverOptions.offerOnCreate));
            EXPECT_THAT(serverPortData->m_chunkReceiverData.m_queueFullPolicy,
                        Eq(serverOptions.requestQueueFullPolicy));
            EXPECT_THAT(serverPortData->m_chunkSenderData.m_consumerTooSlowPolicy,
                        Eq(serverOptions.clientTooSlowPolicy));
        })
        .or_else([&](const auto& error) {
            GTEST_FAIL() << "Expected ClientPortData but got PortPoolError: " << static_cast<uint8_t>(error);
        });
}

TEST_F(PortManager_test, AcquireServerPortDataWithSameServiceDescriptionTwiceCallsErrorHandlerAndReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f2c24ba-192d-4ce8-a61a-fe40b42c655b");
    const ServiceDescription sd{"hyp", "no", "toad"};
    const RuntimeName_t runtimeName{"hypnotoad"};
    auto serverOptions = createTestServerOptions();

    // first call must be successful
    m_portManager->acquireServerPortData(sd, serverOptions, runtimeName, m_payloadDataSegmentMemoryManager, {})
        .or_else([&](const auto& error) {
            GTEST_FAIL() << "Expected ClientPortData but got PortPoolError: " << static_cast<uint8_t>(error);
        });

    // second call must fail
    m_portManager->acquireServerPortData(sd, serverOptions, runtimeName, m_payloadDataSegmentMemoryManager, {})
        .and_then([&](const auto&) {
            GTEST_FAIL() << "Expected PortPoolError::UNIQUE_SERVER_PORT_ALREADY_EXISTS but got ServerPortData";
        })
        .or_else([&](const auto& error) { EXPECT_THAT(error, Eq(PortPoolError::UNIQUE_SERVER_PORT_ALREADY_EXISTS)); });

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__PORT_MANAGER_SERVERPORT_NOT_UNIQUE);
}

TEST_F(PortManager_test, AcquireServerPortDataWithSameServiceDescriptionTwiceAndFirstPortMarkedToBeDestroyedReturnsPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7f2815d-f1ea-403d-9355-69470d92a10f");
    const ServiceDescription sd{"hyp", "no", "toad"};
    const RuntimeName_t runtimeName{"hypnotoad"};
    auto serverOptions = createTestServerOptions();

    // first call must be successful
    auto serverPortDataResult =
        m_portManager->acquireServerPortData(sd, serverOptions, runtimeName, m_payloadDataSegmentMemoryManager, {});

    ASSERT_FALSE(serverPortDataResult.has_error());

    serverPortDataResult.value()->m_toBeDestroyed = true;

    // second call must now also succeed
    m_portManager->acquireServerPortData(sd, serverOptions, runtimeName, m_payloadDataSegmentMemoryManager, {})
        .or_else([&](const auto& error) {
            GTEST_FAIL() << "Expected ClientPortData but got PortPoolError: " << static_cast<uint8_t>(error);
        });

    IOX_TESTING_EXPECT_OK();
}

// END aquireServerPortData tests

// BEGIN discovery tests

TEST_F(PortManager_test, CreateClientWithConnectOnCreateAndNoServerResultsInWaitForOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "14070d7b-d8e1-4df5-84fc-119e5e126cde");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;

    auto clientPortUser = createClient(clientOptions);

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::WAIT_FOR_OFFER));
}

TEST_F(PortManager_test, DoDiscoveryWithClientConnectOnCreateAndNoServerResultsInClientNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "6829e506-9f58-4253-bc42-469f2970a2c7");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;

    auto clientPortUser = createClient(clientOptions);
    m_portManager->doDiscovery();

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::WAIT_FOR_OFFER));
}

TEST_F(PortManager_test, CreateClientWithConnectOnCreateAndNotOfferingServerResultsInWaitForOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f7098d0-2646-4c10-b347-9b57b0f593ce");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = false;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::WAIT_FOR_OFFER));
}

TEST_F(PortManager_test, CreateClientWithConnectOnCreateAndOfferingServerResultsInClientConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "108170d4-786b-4266-ad2a-ef922188f70b");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, CreateServerWithOfferOnCreateAndClientWaitingToConnectResultsInClientConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5bb10b2-bf9b-400e-ab5c-aa3a1e0e826f");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto clientPortUser = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, CreateClientWithNotConnectOnCreateAndNoServerResultsInClientNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "fde662f1-f9e1-4302-be41-59a7a0bfa4e7");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = false;

    auto clientPortUser = createClient(clientOptions);

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::NOT_CONNECTED));
}

TEST_F(PortManager_test, DoDiscoveryWithClientNotConnectOnCreateAndNoServerResultsInClientNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "c59b7343-6277-4a4b-8204-506048726be4");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = false;

    auto clientPortUser = createClient(clientOptions);
    m_portManager->doDiscovery();

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::NOT_CONNECTED));
}

TEST_F(PortManager_test, CreateClientWithNotConnectOnCreateAndOfferingServerResultsInClientNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "17cf22ba-066a-418a-8366-1c6b75177b9a");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = false;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::NOT_CONNECTED));
}

TEST_F(PortManager_test, DoDiscoveryWithClientNotConnectOnCreateAndServerResultsInConnectedWhenCallingConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "87bbb991-4aaf-49c1-b238-d9b0bb18d699");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = false;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    clientPortUser.connect();

    m_portManager->doDiscovery();

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, DoDiscoveryWithClientConnectResultsInClientNotConnectedWhenCallingDisconnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "b6826f93-096d-473d-b846-ab824efff1ee");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    clientPortUser.disconnect();

    m_portManager->doDiscovery();

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::NOT_CONNECTED));
}

TEST_F(PortManager_test, DoDiscoveryWithClientConnectResultsInWaitForOfferWhenCallingStopOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "45c9cc27-4198-4539-943f-2111ae2d1368");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    serverPortUser.stopOffer();

    m_portManager->doDiscovery();

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::WAIT_FOR_OFFER));
}

TEST_F(PortManager_test, DoDiscoveryWithClientConnectResultsInWaitForOfferWhenServerIsDestroyed)
{
    ::testing::Test::RecordProperty("TEST_ID", "585ad47d-1a03-4599-a4dc-57ea1fb6eac7");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;


    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    serverPortUser.destroy();

    m_portManager->doDiscovery();

    EXPECT_THAT(clientPortUser.getConnectionState(), Eq(ConnectionState::WAIT_FOR_OFFER));
}

TEST_F(PortManager_test, DoDiscoveryWithClientConnectResultsInNoClientsWhenClientIsDestroyed)
{
    ::testing::Test::RecordProperty("TEST_ID", "3be2f7b5-7e22-4676-a25b-c8a93a4aaa7d");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;


    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    EXPECT_TRUE(serverPortUser.hasClients());

    clientPortUser.destroy();

    m_portManager->doDiscovery();

    EXPECT_FALSE(serverPortUser.hasClients());
}

TEST_F(PortManager_test, CreateMultipleClientsWithConnectOnCreateAndOfferingServerResultsInAllClientsConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "08f9981f-2585-4574-b0fc-c16cf0eef7d4");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser1 = createClient(clientOptions);
    auto clientPortUser2 = createClient(clientOptions);

    EXPECT_THAT(clientPortUser1.getConnectionState(), Eq(ConnectionState::CONNECTED));
    EXPECT_THAT(clientPortUser2.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test,
       DoDiscoveryWithMultipleClientsNotConnectedAndOfferingServerResultsSomeClientsConnectedWhenSomeClientsCallConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d210259-7c50-479e-b108-bf9747ceb0ef");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = false;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser1 = createClient(clientOptions);
    auto clientPortUser2 = createClient(clientOptions);

    clientPortUser2.connect();
    m_portManager->doDiscovery();

    EXPECT_THAT(clientPortUser1.getConnectionState(), Eq(ConnectionState::NOT_CONNECTED));
    EXPECT_THAT(clientPortUser2.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

// END discovery tests

// BEGIN forwarding to InterfacePort tests

TEST_F(PortManager_test, ServerStateIsForwardedToInterfacePortWhenOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "e51d6f8b-55dd-43b6-977a-da08cfed7be1");
    auto interfacePort = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::DDS, "penguin");
    auto serverOptions = createTestServerOptions();
    m_portManager->doDiscovery();

    serverOptions.offerOnCreate = true;
    auto serverPortUser = createServer(serverOptions);

    m_portManager->doDiscovery();

    interfacePort->m_caproMessageFiFo.pop()
        .and_then([&](const auto& caproMessage) {
            EXPECT_THAT(caproMessage.m_type, Eq(CaproMessageType::OFFER));
            EXPECT_THAT(caproMessage.m_serviceType, Eq(CaproServiceType::SERVER));
        })
        .or_else([&]() { GTEST_FAIL() << "Expected OFFER message but got none"; });
    EXPECT_FALSE(interfacePort->m_caproMessageFiFo.pop().has_value());
}

TEST_F(PortManager_test, ServerStateIsForwardedToInterfacePortWhenStopOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "70692935-82da-4694-a2b0-8307ab2c167c");
    auto interfacePort = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::DDS, "penguin");
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;
    auto serverPortUser = createServer(serverOptions);
    m_portManager->doDiscovery();

    // empty fifo
    while (interfacePort->m_caproMessageFiFo.pop().has_value())
    {
    }

    serverPortUser.stopOffer();
    m_portManager->doDiscovery();

    interfacePort->m_caproMessageFiFo.pop()
        .and_then([&](const auto& caproMessage) {
            EXPECT_THAT(caproMessage.m_type, Eq(CaproMessageType::STOP_OFFER));
            EXPECT_THAT(caproMessage.m_serviceType, Eq(CaproServiceType::SERVER));
        })
        .or_else([&]() { GTEST_FAIL() << "Expected STOP_OFFER message but got none"; });
    EXPECT_FALSE(interfacePort->m_caproMessageFiFo.pop().has_value());
}

TEST_F(PortManager_test, ServerStateIsForwardedToInterfacePortWhenDestroyed)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e9660f8-046c-4e3a-acfd-bad33a6f999c");
    auto interfacePort = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::DDS, "penguin");
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;
    auto serverPortUser = createServer(serverOptions);
    m_portManager->doDiscovery();

    // empty fifo
    while (interfacePort->m_caproMessageFiFo.pop().has_value())
    {
    }

    serverPortUser.destroy();
    m_portManager->doDiscovery();

    interfacePort->m_caproMessageFiFo.pop()
        .and_then([&](const auto& caproMessage) {
            EXPECT_THAT(caproMessage.m_type, Eq(CaproMessageType::STOP_OFFER));
            EXPECT_THAT(caproMessage.m_serviceType, Eq(CaproServiceType::SERVER));
        })
        .or_else([&]() { GTEST_FAIL() << "Expected STOP_OFFER message but got none"; });
    EXPECT_FALSE(interfacePort->m_caproMessageFiFo.pop().has_value());
}

TEST_F(PortManager_test, ServerStateIsForwardedToInterfacePortWhenAlreadyOfferAndInterfacePortIsNewlyCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "31563bb9-561c-43ee-8e3e-b6676cfc9547");
    auto serverOptions = createTestServerOptions();

    serverOptions.offerOnCreate = true;
    auto serverPortUser = createServer(serverOptions);

    m_portManager->doDiscovery();

    auto interfacePort = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::DDS, "penguin");
    m_portManager->doDiscovery();

    interfacePort->m_caproMessageFiFo.pop()
        .and_then([&](const auto& caproMessage) {
            EXPECT_THAT(caproMessage.m_type, Eq(CaproMessageType::OFFER));
            EXPECT_THAT(caproMessage.m_serviceType, Eq(CaproServiceType::SERVER));
        })
        .or_else([&]() { GTEST_FAIL() << "Expected OFFER message but got none"; });
    EXPECT_FALSE(interfacePort->m_caproMessageFiFo.pop().has_value());
}

// END forwarding to InterfacePort tests

// BEGIN service registry tests

TEST_F(PortManager_test, CreateServerWithNotOfferOnCreateDoesNotAddServerToServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "df05ce4d-a1f2-46f2-8224-34b0dbc237ad");
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = false;

    auto serverPortUser = createServer(serverOptions);
    m_portManager->doDiscovery();

    uint64_t serverCount{0U};
    m_portManager->serviceRegistry().find(nullopt, nullopt, nullopt, [&](const auto& entry) {
        EXPECT_THAT(entry.serverCount, Eq(1U));
        serverCount += entry.serverCount;
    });
    EXPECT_THAT(serverCount, Eq(0U));
}

TEST_F(PortManager_test, CreateServerWithOfferOnCreateAddsServerToServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ac876e9-f460-4d1c-97c9-995f3a603317");
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    m_portManager->doDiscovery();

    uint64_t serverCount{0U};
    m_portManager->serviceRegistry().find(nullopt, nullopt, nullopt, [&](const auto& entry) {
        EXPECT_THAT(entry.serverCount, Eq(1U));
        serverCount += entry.serverCount;
    });
    EXPECT_THAT(serverCount, Eq(1U));
}

TEST_F(PortManager_test, StopOfferRemovesServerFromServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "5cb255ec-446c-4c68-99b4-c99d0f8abdc5");
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    m_portManager->doDiscovery();

    serverPortUser.stopOffer();
    m_portManager->doDiscovery();

    uint64_t serverCount{0U};
    m_portManager->serviceRegistry().find(nullopt, nullopt, nullopt, [&](const auto& entry) {
        EXPECT_THAT(entry.serverCount, Eq(1U));
        serverCount += entry.serverCount;
    });
    EXPECT_THAT(serverCount, Eq(0U));
}

TEST_F(PortManager_test, OfferAddsServerToServiceRegistry)
{
    ::testing::Test::RecordProperty("TEST_ID", "60beb1df-a806-4b3a-9e2f-6f6bf352ea1b");
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = false;

    auto serverPortUser = createServer(serverOptions);
    m_portManager->doDiscovery();

    serverPortUser.offer();
    m_portManager->doDiscovery();

    uint64_t serverCount{0U};
    m_portManager->serviceRegistry().find(nullopt, nullopt, nullopt, [&](const auto& entry) {
        EXPECT_THAT(entry.serverCount, Eq(1U));
        serverCount += entry.serverCount;
    });
    EXPECT_THAT(serverCount, Eq(1U));
}

// END service registry tests

// BEGIN policy based connection tests

// NOTE: there is a client/server sandwich to test both code paths where the client and
// the server initiates the state machine ping pong

TEST_F(PortManager_test, ClientWithDiscardOldestDataAndServerWithDiscardOldestDataAreConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "56871f9d-d7c1-4c3c-b86c-9a1e1dc9fd74");
    auto clientOptions = createTestClientOptions();
    clientOptions.responseQueueFullPolicy = QueueFullPolicy::DISCARD_OLDEST_DATA;
    auto serverOptions = createTestServerOptions();
    serverOptions.clientTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, ClientWithDiscardOldestDataAndServerWithWaitForConsumerAreConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "4767b263-1ca4-4e54-b489-5e486f40f4db");
    auto clientOptions = createTestClientOptions();
    clientOptions.responseQueueFullPolicy = QueueFullPolicy::DISCARD_OLDEST_DATA;
    auto serverOptions = createTestServerOptions();
    serverOptions.clientTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, ClientWithBlockProducerAndServerWithWaitForConsumerAreConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "c118ce87-25bf-4f53-b157-7414b9f10193");
    auto clientOptions = createTestClientOptions();
    clientOptions.responseQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    auto serverOptions = createTestServerOptions();
    serverOptions.clientTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, ClientWithBlockProducerAndServerWithDiscardOldestDataAreNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "f5c6213a-b875-42bd-b55b-17bc04179e6d");
    auto clientOptions = createTestClientOptions();
    clientOptions.responseQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    auto serverOptions = createTestServerOptions();
    serverOptions.clientTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Ne(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Ne(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, ServerWithDiscardOldestDataAndClientWithDiscardOldestDataAreConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "53d4ee50-5799-4405-8505-4b7ac3037310");
    auto clientOptions = createTestClientOptions();
    clientOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;
    auto serverOptions = createTestServerOptions();
    serverOptions.requestQueueFullPolicy = QueueFullPolicy::DISCARD_OLDEST_DATA;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, ServerWithDiscardOldestDataAndClientWithWaitForConsumerAreConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d7a8819-3e33-478e-a13b-844b83fe92ae");
    auto clientOptions = createTestClientOptions();
    clientOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    auto serverOptions = createTestServerOptions();
    serverOptions.requestQueueFullPolicy = QueueFullPolicy::DISCARD_OLDEST_DATA;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, ServerWithBlockProducerAndClientWithWaitForConsumerAreConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c3b7770-13e6-4003-aa9f-b04a34df67c9");
    auto clientOptions = createTestClientOptions();
    clientOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    auto serverOptions = createTestServerOptions();
    serverOptions.requestQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Eq(ConnectionState::CONNECTED));
}

TEST_F(PortManager_test, ServerWithBlockProducerAndClientWithDiscardOldestDataAreNotConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d89fa87-3628-4645-9147-82f4223e878a");
    auto clientOptions = createTestClientOptions();
    clientOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;
    auto serverOptions = createTestServerOptions();
    serverOptions.requestQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;

    auto clientBeforeServerOffer = createClient(clientOptions);
    auto serverPortUser = createServer(serverOptions);
    auto clientAfterServerOffer = createClient(clientOptions);

    EXPECT_THAT(clientBeforeServerOffer.getConnectionState(), Ne(ConnectionState::CONNECTED));
    EXPECT_THAT(clientAfterServerOffer.getConnectionState(), Ne(ConnectionState::CONNECTED));
}

// END policy based connection tests

// BEGIN communication tests

TEST_F(PortManager_test, ConnectedClientCanCommunicateWithServer)
{
    ::testing::Test::RecordProperty("TEST_ID", "6376b58d-a796-4cc4-9c40-0c5a117b53f5");
    auto clientOptions = createTestClientOptions();
    clientOptions.connectOnCreate = true;
    auto serverOptions = createTestServerOptions();
    serverOptions.offerOnCreate = true;

    auto serverPortUser = createServer(serverOptions);
    auto clientPortUser = createClient(clientOptions);

    using DataType = uint64_t;
    constexpr int64_t SEQUENCE_ID{42};

    auto allocateRequestResult = clientPortUser.allocateRequest(sizeof(DataType), alignof(DataType));
    ASSERT_FALSE(allocateRequestResult.has_error());
    auto requestHeader = allocateRequestResult.value();
    requestHeader->setSequenceId(SEQUENCE_ID);
    EXPECT_FALSE(clientPortUser.sendRequest(requestHeader).has_error());

    auto getRequestResult = serverPortUser.getRequest();
    ASSERT_FALSE(getRequestResult.has_error());
    auto receivedRequestHeader = getRequestResult.value();
    EXPECT_THAT(receivedRequestHeader->getSequenceId(), Eq(SEQUENCE_ID));

    auto allocateResponseResult =
        serverPortUser.allocateResponse(receivedRequestHeader, sizeof(DataType), alignof(DataType));
    ASSERT_FALSE(allocateResponseResult.has_error());
    auto responseHeader = allocateResponseResult.value();
    EXPECT_FALSE(serverPortUser.sendResponse(responseHeader).has_error());

    auto getResponseResult = clientPortUser.getResponse();
    ASSERT_FALSE(getResponseResult.has_error());
    auto receivedResponseHeader = getResponseResult.value();
    EXPECT_THAT(receivedResponseHeader->getSequenceId(), Eq(SEQUENCE_ID));
}

// END communication tests

} // namespace iox_test_roudi_portmanager
