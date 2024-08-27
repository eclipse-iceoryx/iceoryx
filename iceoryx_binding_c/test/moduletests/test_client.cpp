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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"

using namespace iox::popo;
using namespace iox::capro;
using namespace iox;
using namespace iox::testing;

extern "C" {
#include "iceoryx_binding_c/client.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;

class iox_client_test : public Test
{
  public:
    static constexpr const char RUNTIME_NAME[] = "spongebob_floodler";

    std::unique_ptr<PoshRuntimeMock> runtimeMock = PoshRuntimeMock::create(RUNTIME_NAME);

    void SetUp() override
    {
        memoryConfig.addMemPool({1024, 2});
        memoryManager.configureMemoryManager(memoryConfig, mgmtAllocator, dataAllocator);
    }

    ClientPortData* createClientPortData(const ClientOptions& options)
    {
        sutPort.emplace(ServiceDescription{IdString_t(TruncateToCapacity, SERVICE),
                                           IdString_t(TruncateToCapacity, INSTANCE),
                                           IdString_t(TruncateToCapacity, EVENT)},
                        RUNTIME_NAME,
                        roudi::DEFAULT_UNIQUE_ROUDI_ID,
                        options,
                        &memoryManager);
        return &*sutPort;
    }

    void connect()
    {
        sutPort->m_connectRequested.store(true);
        sutPort->m_connectionState = iox::ConnectionState::CONNECTED;

        sutPort->m_chunkSenderData.m_queues.emplace_back(&serverChunkQueueData);
    }

    void receiveChunk(const int64_t chunkValue = 0)
    {
        auto chunk = memoryManager.getChunk(*iox::mepoo::ChunkSettings::create(
            sizeof(int64_t), iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT, sizeof(ResponseHeader)));
        ASSERT_FALSE(chunk.has_error());
        new (chunk->getChunkHeader()->userHeader())
            ResponseHeader(iox::UniqueId(), RpcBaseHeader::UNKNOWN_CLIENT_QUEUE_INDEX, 0U);
        *static_cast<int64_t*>(chunk->getUserPayload()) = chunkValue;
        iox::popo::ChunkQueuePusher<ClientChunkQueueData_t> pusher{&sutPort->m_chunkReceiverData};
        pusher.push(*chunk);
    }

    void prepareClientInit(const ClientOptions& options = ClientOptions())
    {
        EXPECT_CALL(*runtimeMock,
                    getMiddlewareClient(ServiceDescription{IdString_t(TruncateToCapacity, SERVICE),
                                                           IdString_t(TruncateToCapacity, INSTANCE),
                                                           IdString_t(TruncateToCapacity, EVENT)},
                                        options,
                                        _))
            .WillOnce(Return(createClientPortData(options)));
    }

    bool isPayloadInDataSegment(const void* payload)
    {
        uint64_t startDataSegment = reinterpret_cast<uint64_t>(&dataMemory[0]);
        uint64_t payloadPosition = reinterpret_cast<uint64_t>(payload);

        return (startDataSegment <= payloadPosition && payloadPosition <= startDataSegment + DATA_MEMORY_SIZE);
    }

    static constexpr uint64_t MANAGEMENT_MEMORY_SIZE = 1024 * 1024;
    char managementMemory[MANAGEMENT_MEMORY_SIZE];
    iox::BumpAllocator mgmtAllocator{managementMemory, MANAGEMENT_MEMORY_SIZE};
    static constexpr uint64_t DATA_MEMORY_SIZE = 1024 * 1024;
    alignas(8) char dataMemory[DATA_MEMORY_SIZE];
    iox::BumpAllocator dataAllocator{dataMemory, DATA_MEMORY_SIZE};
    iox::mepoo::MemoryManager memoryManager;
    iox::mepoo::MePooConfig memoryConfig;

    iox::optional<ClientPortData> sutPort;
    iox_client_storage_t sutStorage;

    ServerChunkQueueData_t serverChunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                iox::popo::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};
    ChunkQueuePopper<ServerChunkQueueData_t> serverRequestQueue{&serverChunkQueueData};

    static constexpr const char SERVICE[] = "allGlory";
    static constexpr const char INSTANCE[] = "ToThe";
    static constexpr const char EVENT[] = "HYPNOTOAD";
};
constexpr const char iox_client_test::RUNTIME_NAME[];
constexpr const char iox_client_test::SERVICE[];
constexpr const char iox_client_test::INSTANCE[];
constexpr const char iox_client_test::EVENT[];

TEST_F(iox_client_test, NotInitializedOptionsAreUninitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "347f3a6d-8659-4ac3-81be-720e8a444d5e");
#if !defined(__clang__)
    iox_client_options_t uninitializedOptions;
    // ignore the warning since we would like to test the behavior of an uninitialized option
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    EXPECT_FALSE(iox_client_options_is_initialized(&uninitializedOptions));
#pragma GCC diagnostic pop
#endif
}

TEST_F(iox_client_test, InitializedOptionsAreInitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "b512741e-9c1f-410f-a40b-68fec4a72bc5");
    iox_client_options_t initializedOptions;
    iox_client_options_init(&initializedOptions);
    EXPECT_TRUE(iox_client_options_is_initialized(&initializedOptions));
}

TEST_F(iox_client_test, InitializedOptionsWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ae62644-5fb2-45cf-af99-b4daba43d044");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_options_init(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, CheckInitializedOptionsWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "9a3b7845-170f-4b7f-a0a4-f5b43d96059f");
    iox_client_options_t initializedOptions;
    iox_client_options_init(&initializedOptions);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_options_is_initialized(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, InitializingClientWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ce04604b-ae5b-451f-842b-3c3d3f41ebb7");
    iox_client_options_t options;
    iox_client_options_init(&options);
    options.responseQueueCapacity = 456;
    strncpy(options.nodeName, "hypnotoad is all you need", IOX_CONFIG_NODE_NAME_SIZE);
    options.connectOnCreate = false;
    options.responseQueueFullPolicy = QueueFullPolicy_BLOCK_PRODUCER;
    options.serverTooSlowPolicy = ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER;

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_init(nullptr, SERVICE, INSTANCE, EVENT, &options); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_init(&sutStorage, nullptr, INSTANCE, EVENT, &options); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_init(&sutStorage, SERVICE, nullptr, EVENT, &options); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_init(&sutStorage, SERVICE, INSTANCE, nullptr, &options); },
                             iox::er::ENFORCE_VIOLATION);
    iox_client_options_t emptyOpts;
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, &emptyOpts); },
                             iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, DeinitClientWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f055b75-3cdd-4a55-b292-86b1ffb7a32d");

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_deinit(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, LoanAlignedChunkWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "18eb8cf9-59a4-4e53-beaf-a174e372efff");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    constexpr uint64_t ALIGNMENT = 128;
    void* payload = nullptr;
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_loan_aligned_request(nullptr, &payload, 32, ALIGNMENT); },
                             iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_loan_aligned_request(sut, nullptr, 32, ALIGNMENT); },
                             iox::er::ENFORCE_VIOLATION);
    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ReleaseClientWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "09e9ee2d-e9b3-4791-8f9f-979f2d75f7c9");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, 32), Eq(AllocationResult_SUCCESS));

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_release_request(nullptr, payload); }, iox::er::ENFORCE_VIOLATION);

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_release_request(sut, nullptr); }, iox::er::ENFORCE_VIOLATION);
    iox_client_deinit(sut);
}

TEST_F(iox_client_test, SendWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c7d9b5e9-ed49-4a67-b5fc-12aaf21447b9");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    *static_cast<int64_t*>(payload) = 8912389;

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_send(nullptr, payload); }, iox::er::ENFORCE_VIOLATION);
    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ClientConnectWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "f778de64-e153-4fb7-9535-9bd288979cc9");

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_connect(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, ClientDisconnectWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "69e69ebc-f8bd-4d70-9eee-de593acc5019");

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_disconnect(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, ClientGetConnectStateWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "cdf21827-47c8-49d8-bf07-b375dab74a70");

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_get_connection_state(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, ClientTakeResponseWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "2cdd5a14-bd66-48a7-847c-e9c9ddcfc882");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    receiveChunk(800131);
    const void* payload = nullptr;

    EXPECT_THAT(iox_client_take_response(sut, &payload), Eq(ChunkReceiveResult_SUCCESS));
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_take_response(nullptr, &payload); }, iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_take_response(sut, nullptr); }, iox::er::ENFORCE_VIOLATION);
    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ClientReleasingResponseWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6cde4e4b-4b4c-4200-a660-aa2eb8c687ee");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    receiveChunk();
    const void* payload = nullptr;

    iox_client_take_response(sut, &payload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_release_response(nullptr, payload); }, iox::er::ENFORCE_VIOLATION);
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_release_response(sut, nullptr); }, iox::er::ENFORCE_VIOLATION);

    iox_client_release_response(sut, payload);
    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ReleasingQueuedResponsesWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d36d962-96af-4b82-a19c-4d4dc34f8c37");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_release_queued_responses(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, CheckClientHasResponseWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2b81347-ce89-4d24-bd18-1cbdd716940e");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_has_responses(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, CheckClientHasMissedResponseWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "516b27af-5f78-4988-9886-726c414b6b31");
    IOX_EXPECT_FATAL_FAILURE([&] { iox_client_has_missed_responses(nullptr); }, iox::er::ENFORCE_VIOLATION);
}

TEST_F(iox_client_test, InitializedOptionsAreToCPPDefaults)
{
    ::testing::Test::RecordProperty("TEST_ID", "a48477c1-7762-4790-acd1-5b13db486cac");
    iox_client_options_t initializedOptions;
    iox_client_options_init(&initializedOptions);

    ClientOptions cppOptions;

    EXPECT_THAT(initializedOptions.responseQueueCapacity, Eq(cppOptions.responseQueueCapacity));
    EXPECT_THAT(initializedOptions.nodeName, StrEq(cppOptions.nodeName.c_str()));
    EXPECT_THAT(initializedOptions.connectOnCreate, Eq(cppOptions.connectOnCreate));
    EXPECT_THAT(initializedOptions.responseQueueFullPolicy,
                Eq(cpp2c::queueFullPolicy(cppOptions.responseQueueFullPolicy)));
    EXPECT_THAT(initializedOptions.serverTooSlowPolicy,
                Eq(cpp2c::consumerTooSlowPolicy(cppOptions.serverTooSlowPolicy)));
}

TEST_F(iox_client_test, InitializingClientWithNullptrOptionsGetMiddlewareClientWithDefaultOptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0775190-5672-479b-afe9-8e127abc1bc2");
    ClientOptions defaultOptions;
    prepareClientInit(defaultOptions);

    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    ASSERT_THAT(sut, Ne(nullptr));
    iox_client_deinit(sut);
}

TEST_F(iox_client_test, InitializingClientWithCustomOptionsWork)
{
    ::testing::Test::RecordProperty("TEST_ID", "69b2da3d-fc4f-48cf-86e7-4e4365557391");
    iox_client_options_t options;
    iox_client_options_init(&options);
    options.responseQueueCapacity = 456;
    strncpy(options.nodeName, "hypnotoad is all you need", IOX_CONFIG_NODE_NAME_SIZE);
    options.connectOnCreate = false;
    options.responseQueueFullPolicy = QueueFullPolicy_BLOCK_PRODUCER;
    options.serverTooSlowPolicy = ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER;

    ClientOptions cppOptions;
    cppOptions.responseQueueCapacity = options.responseQueueCapacity;
    cppOptions.connectOnCreate = options.connectOnCreate;
    cppOptions.nodeName = options.nodeName;
    cppOptions.responseQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    cppOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    prepareClientInit(cppOptions);

    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, &options);
    ASSERT_THAT(sut, Ne(nullptr));
    iox_client_deinit(sut);
}

TEST_F(iox_client_test, DeinitReleasesClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "91311811-6741-4bca-839d-326c375e9b8c");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    ASSERT_THAT(sut, Ne(nullptr));

    iox_client_deinit(sut);
    EXPECT_THAT(sutPort->m_toBeDestroyed.load(), Eq(true));
}

TEST_F(iox_client_test, LoanWithValidArgumentsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f7ae32e-8a43-479b-beb7-6d174b791010");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, 32), Eq(AllocationResult_SUCCESS));
    EXPECT_TRUE(isPayloadInDataSegment(payload));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, LoanAlignedChunkWithValidArgumentsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff897354-8213-4f13-88fe-530e29830d79");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    constexpr uint64_t ALIGNMENT = 128;
    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_aligned_request(sut, &payload, 32, ALIGNMENT), Eq(AllocationResult_SUCCESS));
    EXPECT_TRUE(isPayloadInDataSegment(payload));
    EXPECT_THAT(reinterpret_cast<uint64_t>(payload) % ALIGNMENT, Eq(0U));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, LoanFailsWhenNoMoreChunksAreAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "5ad2c0a6-1f39-44e7-ba7a-c286a1d2d40b");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, 32), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(iox_client_loan_request(sut, &payload, 32), Eq(AllocationResult_SUCCESS));

    payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, 322), Eq(AllocationResult_RUNNING_OUT_OF_CHUNKS));
    EXPECT_THAT(payload, Eq(nullptr));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, LoanAlignedFailsWhenNoMoreChunksAreAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "7720afdd-b106-4081-a79d-0f0edfc1edcb");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_aligned_request(sut, &payload, 32, 32), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(iox_client_loan_aligned_request(sut, &payload, 32, 32), Eq(AllocationResult_SUCCESS));

    payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, 322), Eq(AllocationResult_RUNNING_OUT_OF_CHUNKS));
    EXPECT_THAT(payload, Eq(nullptr));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ReleaseWorksOnValidPayload)
{
    ::testing::Test::RecordProperty("TEST_ID", "159e9b42-4f43-41df-8449-5891950eb592");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, 32), Eq(AllocationResult_SUCCESS));

    iox_client_release_request(sut, payload);

    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, LoanAndSendWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fd83a4cd-6f86-47f8-aa46-e20f34959461");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan_request(sut, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    *static_cast<int64_t*>(payload) = 8912389;

    EXPECT_THAT(iox_client_send(sut, payload), Eq(ClientSendResult_SUCCESS));

    serverRequestQueue.tryPop()
        .and_then([&](auto& sharedChunk) {
            auto msg = static_cast<int64_t*>(sharedChunk.getUserPayload());
            EXPECT_THAT(*msg, Eq(8912389));
        })
        .or_else([&] { GTEST_FAIL() << "Expected request but got none"; });

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, SendWithNullptrReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "927583a2-5b26-47ba-b05c-95729b7af8f1");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();

    EXPECT_THAT(iox_client_send(sut, nullptr), Eq(ClientSendResult_INVALID_REQUEST));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ConnectWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4809d1ce-3a53-4981-bded-4709599f62b5");
    iox_client_options_t options;
    iox_client_options_init(&options);
    options.connectOnCreate = false;

    ClientOptions cppOptions;
    cppOptions.connectOnCreate = false;
    prepareClientInit(cppOptions);
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, &options);
    iox_client_connect(sut);

    EXPECT_THAT(sutPort->m_connectRequested.load(), Eq(true));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, DisconnectWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ebb07dc5-1aa2-4dbe-8b86-7378e6bd2ed2");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_client_disconnect(sut);

    EXPECT_THAT(sutPort->m_connectRequested.load(), Eq(false));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, GetConnectionIsNotConnectedWhenCreatedWithoutAutoConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "8689930e-5ce8-4d1e-8863-01689d6d0fbd");
    iox_client_options_t options;
    iox_client_options_init(&options);
    options.connectOnCreate = false;

    ClientOptions cppOptions;
    cppOptions.connectOnCreate = false;
    prepareClientInit(cppOptions);
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, &options);

    EXPECT_THAT(iox_client_get_connection_state(sut), Eq(ConnectionState_NOT_CONNECTED));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, GetConnectionReturnsConnectRequested)
{
    ::testing::Test::RecordProperty("TEST_ID", "adeca842-72ba-406e-890b-1124f8bfcee5");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    sutPort->m_connectRequested = true;
    sutPort->m_connectionState = iox::ConnectionState::CONNECT_REQUESTED;

    EXPECT_THAT(iox_client_get_connection_state(sut), Eq(ConnectionState_CONNECT_REQUESTED));

    sutPort->m_connectionState = iox::ConnectionState::CONNECTED;
    EXPECT_THAT(iox_client_get_connection_state(sut), Eq(ConnectionState_CONNECTED));

    sutPort->m_connectRequested = false;
    sutPort->m_connectionState = iox::ConnectionState::DISCONNECT_REQUESTED;
    EXPECT_THAT(iox_client_get_connection_state(sut), Eq(ConnectionState_DISCONNECT_REQUESTED));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, GetConnectionReturnsWaitForOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "941ab168-895d-43ee-af8f-0ac2650dea51");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    sutPort->m_connectRequested = true;
    sutPort->m_connectionState = iox::ConnectionState::WAIT_FOR_OFFER;

    EXPECT_THAT(iox_client_get_connection_state(sut), Eq(ConnectionState_WAIT_FOR_OFFER));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, TakeReturnsNoChunkAvailableWhenNothingWasReceived)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f367fca-322c-4246-9349-5519f22c118e");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    const void* payload = nullptr;

    EXPECT_THAT(iox_client_take_response(sut, &payload), Eq(ChunkReceiveResult_NO_CHUNK_AVAILABLE));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, TakeAcquiresChunkWhenOneIsAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e991175-2b7e-4940-b099-94a4e648a0a4");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    receiveChunk(800131);
    const void* payload = nullptr;

    EXPECT_THAT(iox_client_take_response(sut, &payload), Eq(ChunkReceiveResult_SUCCESS));
    ASSERT_THAT(payload, Ne(nullptr));
    EXPECT_THAT(*static_cast<const int64_t*>(payload), Eq(800131));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ReleasingResponseReleasesChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "49a608c3-a5e7-46d8-b1eb-851c3d4dbfd9");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    receiveChunk();
    const void* payload = nullptr;

    iox_client_take_response(sut, &payload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
    iox_client_release_response(sut, payload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, ReleasingQueuedResponsesReleasesEverything)
{
    ::testing::Test::RecordProperty("TEST_ID", "45f34faf-dc39-4658-adf5-936e2a33c5df");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    receiveChunk();
    receiveChunk();

    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));
    iox_client_release_queued_responses(sut);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, HasResponsesIsFalseWhenThereIsNoResponse)
{
    ::testing::Test::RecordProperty("TEST_ID", "40a108ef-a5ed-47b7-9729-8e2dc0ceabd3");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    EXPECT_FALSE(iox_client_has_responses(sut));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, HasResponsesIsTrueWhenThereIsAreResponses)
{
    ::testing::Test::RecordProperty("TEST_ID", "15d14dc9-fef6-4c6f-b423-13946b633848");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();
    receiveChunk();

    EXPECT_TRUE(iox_client_has_responses(sut));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, HasMissedResponsesOnOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8bb1562-0bd9-4ea9-98a6-eeabcfc6d4cc");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();

    sutPort->m_chunkReceiverData.m_queueHasLostChunks = true;
    EXPECT_TRUE(iox_client_has_missed_responses(sut));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, HasNoMissedResponses)
{
    ::testing::Test::RecordProperty("TEST_ID", "97e52ac8-f9a4-4f81-ba37-c41b6e750b55");
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connect();

    sutPort->m_chunkReceiverData.m_queueHasLostChunks = false;
    EXPECT_FALSE(iox_client_has_missed_responses(sut));

    iox_client_deinit(sut);
}

TEST_F(iox_client_test, GetServiceDescriptionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d456aa37-4c28-4de7-9adc-2c5c4108f588");

    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    auto serviceDescription = iox_client_get_service_description(sut);

    EXPECT_THAT(serviceDescription.serviceString, StrEq(SERVICE));
    EXPECT_THAT(serviceDescription.instanceString, StrEq(INSTANCE));
    EXPECT_THAT(serviceDescription.eventString, StrEq(EVENT));

    iox_client_deinit(sut);
}
} // namespace
