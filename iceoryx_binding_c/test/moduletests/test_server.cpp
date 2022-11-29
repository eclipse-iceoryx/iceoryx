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
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"

using namespace iox::popo;
using namespace iox::capro;
using namespace iox;
using namespace iox::cxx;

extern "C" {
#include "iceoryx_binding_c/server.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;

class iox_server_test : public Test
{
  public:
    static constexpr const char RUNTIME_NAME[] = "sven_shwiddzler";

    std::unique_ptr<PoshRuntimeMock> runtimeMock = PoshRuntimeMock::create(RUNTIME_NAME);
    static constexpr uint64_t MAX_REQUESTS_HOLD_IN_PARALLEL = iox::MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY + 1U;

    void SetUp() override
    {
        memoryConfig.addMemPool({128, 2});
        memoryConfig.addMemPool({1024, MAX_REQUESTS_HOLD_IN_PARALLEL + 1});
        memoryManager.configureMemoryManager(memoryConfig, mgmtAllocator, dataAllocator);
    }

    ServerPortData* createServerPortData(const ServerOptions& options)
    {
        sutPort.emplace(ServiceDescription{IdString_t(TruncateToCapacity, SERVICE),
                                           IdString_t(TruncateToCapacity, INSTANCE),
                                           IdString_t(TruncateToCapacity, EVENT)},
                        RUNTIME_NAME,
                        options,
                        &memoryManager);
        return &*sutPort;
    }

    void receiveRequest(const int64_t requestValue = 0, const uint32_t chunkSize = sizeof(int64_t))
    {
        auto chunk = memoryManager.getChunk(*iox::mepoo::ChunkSettings::create(
            chunkSize, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT, sizeof(RequestHeader)));
        ASSERT_FALSE(chunk.has_error());
        new (chunk->getChunkHeader()->userHeader())
            RequestHeader(clientResponseQueueData.m_uniqueId, RpcBaseHeader::UNKNOWN_CLIENT_QUEUE_INDEX);
        *static_cast<int64_t*>(chunk->getUserPayload()) = requestValue;
        iox::popo::ChunkQueuePusher<ServerChunkQueueData_t> pusher{&sutPort->m_chunkReceiverData};
        if (!pusher.push(*chunk))
        {
            sutPort->m_chunkReceiverData.m_queueHasLostChunks = true;
        }
    }

    void connectClient()
    {
        sutPort->m_chunkSenderData.m_queues.emplace_back(&clientResponseQueueData);
    }

    void prepareServerInit(const ServerOptions& options = ServerOptions())
    {
        EXPECT_CALL(*runtimeMock,
                    getMiddlewareServer(ServiceDescription{IdString_t(TruncateToCapacity, SERVICE),
                                                           IdString_t(TruncateToCapacity, INSTANCE),
                                                           IdString_t(TruncateToCapacity, EVENT)},
                                        options,
                                        _))
            .WillOnce(Return(createServerPortData(options)));
    }

    static constexpr uint64_t MANAGEMENT_MEMORY_SIZE = 1024 * 1024;
    char managementMemory[MANAGEMENT_MEMORY_SIZE];
    iox::BumpAllocator mgmtAllocator{managementMemory, MANAGEMENT_MEMORY_SIZE};
    static constexpr uint64_t DATA_MEMORY_SIZE = 1024 * 1024;
    char dataMemory[DATA_MEMORY_SIZE];
    iox::BumpAllocator dataAllocator{dataMemory, DATA_MEMORY_SIZE};
    iox::mepoo::MemoryManager memoryManager;
    iox::mepoo::MePooConfig memoryConfig;

    iox::optional<ServerPortData> sutPort;
    iox_server_storage_t sutStorage;

    ClientChunkQueueData_t clientResponseQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                   iox::cxx::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};
    ChunkQueuePopper<ClientChunkQueueData_t> clientResponseQueue{&clientResponseQueueData};

    static constexpr const char SERVICE[] = "TheHoff";
    static constexpr const char INSTANCE[] = "IsAll";
    static constexpr const char EVENT[] = "YouNeed";
};
constexpr const char iox_server_test::RUNTIME_NAME[];
constexpr const char iox_server_test::SERVICE[];
constexpr const char iox_server_test::INSTANCE[];
constexpr const char iox_server_test::EVENT[];

TEST_F(iox_server_test, notInitializedOptionsAreUninitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "e227f280-3725-44e4-ba08-c9d2a43d5a1b");
#if !defined(__clang__)
    iox_server_options_t uninitializedOptions;
    // ignore the warning since we would like to test the behavior of an uninitialized option
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    EXPECT_FALSE(iox_server_options_is_initialized(&uninitializedOptions));
#pragma GCC diagnostic pop
#endif
}

TEST_F(iox_server_test, initializedOptionsAreInitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "f92d5196-d527-4504-8131-5e5304091068");
    iox_server_options_t initializedOptions;
    iox_server_options_init(&initializedOptions);
    EXPECT_TRUE(iox_server_options_is_initialized(&initializedOptions));
}

TEST_F(iox_server_test, initializedOptionsToCPPDefaults)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b202536-31e3-4439-836f-ba83235048fa");
    iox_server_options_t initializedOptions;
    iox_server_options_init(&initializedOptions);

    ServerOptions cppOptions;
    EXPECT_THAT(initializedOptions.requestQueueCapacity, Eq(cppOptions.requestQueueCapacity));
    EXPECT_THAT(initializedOptions.nodeName, StrEq(cppOptions.nodeName.c_str()));
    EXPECT_THAT(initializedOptions.offerOnCreate, Eq(cppOptions.offerOnCreate));
    EXPECT_THAT(initializedOptions.requestQueueFullPolicy,
                Eq(cpp2c::queueFullPolicy(cppOptions.requestQueueFullPolicy)));
    EXPECT_THAT(initializedOptions.clientTooSlowPolicy,
                Eq(cpp2c::consumerTooSlowPolicy(cppOptions.clientTooSlowPolicy)));
}

TEST_F(iox_server_test, InitializingServerWithNullptrOptionsGetsMiddlewareServerWithDefaultOptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "10091749-4c07-442a-b42b-aef0d8d06ceb");
    ServerOptions defaultOptions;
    prepareServerInit(defaultOptions);

    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    ASSERT_THAT(sut, Ne(nullptr));
    iox_server_deinit(sut);
}

TEST_F(iox_server_test, InitializingServerWithCustomOptionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e21f9257-cf3b-4e18-ad98-126a295e780f");
    iox_server_options_t options;
    iox_server_options_init(&options);
    options.requestQueueCapacity = 32;
    strncpy(options.nodeName, "do not hassel with the hoff", IOX_CONFIG_NODE_NAME_SIZE);
    options.offerOnCreate = false;
    options.requestQueueFullPolicy = QueueFullPolicy_BLOCK_PRODUCER;
    options.clientTooSlowPolicy = ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER;

    ServerOptions cppOptions;
    cppOptions.requestQueueCapacity = options.requestQueueCapacity;
    cppOptions.nodeName = iox::NodeName_t(TruncateToCapacity, options.nodeName);
    cppOptions.offerOnCreate = options.offerOnCreate;
    cppOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    cppOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    prepareServerInit(cppOptions);

    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, &options);
    ASSERT_THAT(sut, Ne(nullptr));
    iox_server_deinit(sut);
}

TEST_F(iox_server_test, DeinitReleasesServer)
{
    ::testing::Test::RecordProperty("TEST_ID", "a50eb082-bc40-4c84-95dc-cefcdfcdb70d");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_deinit(sut);
    EXPECT_THAT(sutPort->m_toBeDestroyed.load(), Eq(true));
}

TEST_F(iox_server_test, WhenNotOfferedTakeRequestFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "48763e8a-c192-4baf-88c0-fb5377d89127");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_stop_offer(sut);

    const void* payload;
    EXPECT_THAT(iox_server_take_request(sut, &payload),
                Eq(ServerRequestResult_NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, WhenOfferedAndNoRequestsPresentTakeFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "409ab402-2452-4250-a6b4-4c90489cdd42");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);

    const void* payload;
    EXPECT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_NO_PENDING_REQUESTS));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, WhenOfferedAndRequestsPresentTakeSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "42443d18-5a8f-40ee-938d-2b0a345f829b");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    constexpr int64_t REQUEST_VALUE = 64461001;
    receiveRequest(REQUEST_VALUE);

    const void* payload;
    ASSERT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_SUCCESS));
    ASSERT_THAT(payload, Ne(nullptr));
    EXPECT_THAT(*static_cast<const int64_t*>(payload), Eq(REQUEST_VALUE));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, TakingToMuchRequestsInParallelLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1175d14-0268-42d9-a174-97713e622200");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);

    constexpr int64_t REQUEST_VALUE = 0;
    constexpr int64_t PAYLOAD_SIZE = 512;
    const void* payload;
    for (uint64_t i = 0U; i < MAX_REQUESTS_HOLD_IN_PARALLEL; ++i)
    {
        receiveRequest(REQUEST_VALUE, PAYLOAD_SIZE);

        payload = nullptr;
        ASSERT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_SUCCESS));
        ASSERT_THAT(payload, Ne(nullptr));
    }

    receiveRequest(REQUEST_VALUE, PAYLOAD_SIZE);
    payload = nullptr;
    ASSERT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_TOO_MANY_REQUESTS_HELD_IN_PARALLEL));
    ASSERT_THAT(payload, Eq(nullptr));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, ReleaseRequestWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "98a7df98-5adf-4d16-81ea-ac5ebbdc7576");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    receiveRequest();

    const void* payload;
    ASSERT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_SUCCESS));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    iox_server_release_request(sut, payload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, ReleaseQueuedRequestsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d245c9e4-a18e-42ae-ab0a-375e1ebf24e7");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    receiveRequest();
    receiveRequest();

    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));

    iox_server_release_queued_requests(sut);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, HasClientsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7235e98-43b2-4c81-9939-5d774958bd56");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);

    EXPECT_FALSE(iox_server_has_clients(sut));
    connectClient();
    EXPECT_TRUE(iox_server_has_clients(sut));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, HasRequestWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d833165-8b7d-4364-9268-a0a38a31590d");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    EXPECT_FALSE(iox_server_has_requests(sut));
    receiveRequest();
    EXPECT_TRUE(iox_server_has_requests(sut));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, HasMissedRequestWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3edbf3d6-ef45-49f5-98c9-57b7d1b6c987");
    iox_server_options_t options;
    iox_server_options_init(&options);
    options.requestQueueCapacity = 1;

    ServerOptions cppOptions;
    cppOptions.requestQueueCapacity = 1;

    prepareServerInit(cppOptions);
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, &options);
    iox_server_offer(sut);
    EXPECT_FALSE(iox_server_has_missed_requests(sut));
    receiveRequest();
    receiveRequest();
    EXPECT_TRUE(iox_server_has_missed_requests(sut));
    EXPECT_FALSE(iox_server_has_missed_requests(sut));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, OfferReturnsCorrectOfferState)
{
    ::testing::Test::RecordProperty("TEST_ID", "9537e4b5-27af-4646-83c5-90d5799011bc");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    EXPECT_TRUE(iox_server_is_offered(sut));
    iox_server_stop_offer(sut);
    EXPECT_FALSE(iox_server_is_offered(sut));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, GetServiceDescriptionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2542a9b-e49f-47ea-9fb9-6306208ca987");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    auto serviceDescription = iox_server_get_service_description(sut);

    EXPECT_THAT(serviceDescription.serviceString, StrEq(SERVICE));
    EXPECT_THAT(serviceDescription.instanceString, StrEq(INSTANCE));
    EXPECT_THAT(serviceDescription.eventString, StrEq(EVENT));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, LoanWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "600a78ea-ee00-4179-a62e-b0a4adf9d54a");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest();

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(payload, Ne(nullptr));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, LoanFailsWhenNoMoreChunksAreAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "dac40907-aafb-40cd-9bbe-eeddf8916322");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest();

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)),
                Eq(AllocationResult_RUNNING_OUT_OF_CHUNKS));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, LoanAlignedWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "dfc4cbb1-e72d-470a-8e56-182446a16976");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest();

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_aligned_response(sut, requestPayload, &payload, sizeof(int64_t), 16),
                Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(payload, Ne(nullptr));
    EXPECT_THAT(reinterpret_cast<uint64_t>(payload) % 16, Eq(0U));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, LoanAlignedFailsWhenNoChunksAreAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "2fad0479-1e87-4a0a-a52c-cb437091018d");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest();

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_aligned_response(sut, requestPayload, &payload, sizeof(int64_t), 16),
                Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(iox_server_loan_aligned_response(sut, requestPayload, &payload, sizeof(int64_t), 16),
                Eq(AllocationResult_RUNNING_OUT_OF_CHUNKS));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, ReleaseResponseWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "06d1e165-eb60-4bf7-94b5-290535c6541e");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest();

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));
    iox_server_release_response(sut, payload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
    iox_server_release_request(sut, requestPayload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, SendWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c7fe56ff-1678-449f-a3d3-fe5f9dc57d1b");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest();

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    ASSERT_THAT(payload, Ne(nullptr));
    *static_cast<int64_t*>(payload) = 42424242;

    EXPECT_THAT(iox_server_send(sut, payload), Eq(ServerSendResult_SUCCESS));
    clientResponseQueue.tryPop()
        .and_then(
            [&](auto& sharedChunk) { EXPECT_THAT(*static_cast<int64_t*>(sharedChunk.getUserPayload()), Eq(42424242)); })
        .or_else([&] { GTEST_FAIL() << "Expected response but got nothing"; });

    iox_server_deinit(sut);
}

TEST_F(iox_server_test, SendWithNullptrReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "af538f0b-ce37-48d2-8940-28d39d7a411e");
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();

    EXPECT_THAT(iox_server_send(sut, nullptr), Eq(ServerSendResult_INVALID_RESPONSE));

    iox_server_deinit(sut);
}
} // namespace
