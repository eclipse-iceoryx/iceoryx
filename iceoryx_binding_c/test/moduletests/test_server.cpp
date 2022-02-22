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
#include "iceoryx_hoofs/internal/relocatable_pointer/atomic_relocatable_pointer.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"

using namespace iox::popo;
using namespace iox::capro;
using namespace iox::capro;
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

    void SetUp() override
    {
        memoryConfig.addMemPool({1024, 2});
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

    void receiveRequest(const int64_t requestValue)
    {
        auto chunk = memoryManager.getChunk(*iox::mepoo::ChunkSettings::create(
            sizeof(int64_t), iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT, sizeof(RequestHeader)));
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
    iox::posix::Allocator mgmtAllocator{managementMemory, MANAGEMENT_MEMORY_SIZE};
    static constexpr uint64_t DATA_MEMORY_SIZE = 1024 * 1024;
    char dataMemory[DATA_MEMORY_SIZE];
    iox::posix::Allocator dataAllocator{dataMemory, DATA_MEMORY_SIZE};
    iox::mepoo::MemoryManager memoryManager;
    iox::mepoo::MePooConfig memoryConfig;

    iox::cxx::optional<ServerPortData> sutPort;
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
    iox_server_options_t uninitializedOptions;
#if !defined(__clang__)
    // ignore the warning since we would like to test the behavior of an uninitialized option
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    EXPECT_FALSE(iox_server_options_is_initialized(&uninitializedOptions));
#pragma GCC diagnostic pop
#endif
}

TEST_F(iox_server_test, initializedOptionsAreInitialized)
{
    iox_server_options_t initializedOptions;
    iox_server_options_init(&initializedOptions);
    EXPECT_TRUE(iox_server_options_is_initialized(&initializedOptions));
}

TEST_F(iox_server_test, initializedOptionsToCPPDefaults)
{
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
    ServerOptions defaultOptions;
    prepareServerInit(defaultOptions);

    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    ASSERT_THAT(sut, Ne(nullptr));
}

TEST_F(iox_server_test, InitializingServerWithCustomOptionsWorks)
{
    iox_server_options_t options;
    iox_server_options_init(&options);
    options.requestQueueCapacity = 32;
    strncpy(options.nodeName, "do not hassel with the hoff", IOX_CONFIG_NODE_NAME_SIZE);
    options.offerOnCreate = false;
    options.requestQueueFullPolicy = QueueFullPolicy_BLOCK_PRODUCER;
    options.clientTooSlowPolicy = ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER;

    ServerOptions cppOptions;
    cppOptions.requestQueueCapacity = 32;
    cppOptions.nodeName = "do not hassel with the hoff";
    cppOptions.offerOnCreate = false;
    cppOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    cppOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    prepareServerInit(cppOptions);

    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, &options);
    ASSERT_THAT(sut, Ne(nullptr));
}

TEST_F(iox_server_test, DeinitReleasesServer)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_deinit(sut);
    EXPECT_THAT(sutPort->m_toBeDestroyed.load(), Eq(true));
}

TEST_F(iox_server_test, WhenNotOfferedTakeRequestFails)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_stop_offer(sut);

    const void* payload;
    EXPECT_THAT(iox_server_take_request(sut, &payload),
                Eq(ServerRequestResult_NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER));
}

TEST_F(iox_server_test, WhenOfferedAndNoRequestsPresentTakeFails)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);

    const void* payload;
    EXPECT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_NO_PENDING_REQUESTS));
}

TEST_F(iox_server_test, WhenOfferedAndRequestsPresentTakeSucceeds)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    receiveRequest(64461001);

    const void* payload;
    ASSERT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_SUCCESS));
    ASSERT_THAT(payload, Ne(nullptr));
    EXPECT_THAT(*static_cast<const int64_t*>(payload), Eq(64461001));
}

TEST_F(iox_server_test, ReleaseRequestWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    receiveRequest(64461001);

    const void* payload;
    ASSERT_THAT(iox_server_take_request(sut, &payload), Eq(ServerRequestResult_SUCCESS));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));

    iox_server_release_request(sut, payload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

TEST_F(iox_server_test, ReleaseQueuedRequestsWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    receiveRequest(64461001);
    receiveRequest(313);

    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));

    iox_server_release_queued_requests(sut);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

TEST_F(iox_server_test, HasClientsWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);

    EXPECT_FALSE(iox_server_has_clients(sut));
    connectClient();
    EXPECT_TRUE(iox_server_has_clients(sut));
}

TEST_F(iox_server_test, HasRequestWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    EXPECT_FALSE(iox_server_has_requests(sut));
    receiveRequest(64461001);
    EXPECT_TRUE(iox_server_has_requests(sut));
}

TEST_F(iox_server_test, HasMissedRequestWorks)
{
    iox_server_options_t options;
    iox_server_options_init(&options);
    options.requestQueueCapacity = 1;

    ServerOptions cppOptions;
    cppOptions.requestQueueCapacity = 1;

    prepareServerInit(cppOptions);
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, &options);
    iox_server_offer(sut);
    EXPECT_FALSE(iox_server_has_missed_requests(sut));
    receiveRequest(64461001);
    receiveRequest(6321);
    EXPECT_TRUE(iox_server_has_missed_requests(sut));
    EXPECT_FALSE(iox_server_has_missed_requests(sut));
}

TEST_F(iox_server_test, OfferReturnsCorrectOfferState)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    iox_server_offer(sut);
    EXPECT_TRUE(iox_server_is_offered(sut));
    iox_server_stop_offer(sut);
    EXPECT_FALSE(iox_server_is_offered(sut));
}

TEST_F(iox_server_test, GetServiceDescriptionWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    auto serviceDescription = iox_server_get_service_description(sut);

    EXPECT_THAT(serviceDescription.serviceString, StrEq(SERVICE));
    EXPECT_THAT(serviceDescription.instanceString, StrEq(INSTANCE));
    EXPECT_THAT(serviceDescription.eventString, StrEq(EVENT));
}

TEST_F(iox_server_test, LoanWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest(31711);

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(payload, Ne(nullptr));
}

TEST_F(iox_server_test, LoanFailsWhenNoMoreChunksAreAvailable)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest(31711);

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)),
                Eq(AllocationResult_RUNNING_OUT_OF_CHUNKS));
}

TEST_F(iox_server_test, LoanAlignedWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest(31711);

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_aligned_response(sut, requestPayload, &payload, sizeof(int64_t), 16),
                Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(payload, Ne(nullptr));
    EXPECT_THAT(reinterpret_cast<uint64_t>(payload) % 16, Eq(0));
}

TEST_F(iox_server_test, ReleaseResponseWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest(31711);

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));
    iox_server_release_response(sut, payload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
    iox_server_release_request(sut, requestPayload);
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}

TEST_F(iox_server_test, SendWorks)
{
    prepareServerInit();
    iox_server_t sut = iox_server_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    connectClient();
    receiveRequest(31711);

    const void* requestPayload;
    EXPECT_THAT(iox_server_take_request(sut, &requestPayload), Eq(ServerRequestResult_SUCCESS));

    void* payload = nullptr;
    EXPECT_THAT(iox_server_loan_response(sut, requestPayload, &payload, sizeof(int64_t)), Eq(AllocationResult_SUCCESS));
    ASSERT_THAT(payload, Ne(nullptr));
    *static_cast<int64_t*>(payload) = 42424242;

    iox_server_send(sut, payload);
    clientResponseQueue.tryPop()
        .and_then(
            [&](auto& sharedChunk) { EXPECT_THAT(*static_cast<int64_t*>(sharedChunk.getUserPayload()), Eq(42424242)); })
        .or_else([&] { GTEST_FAIL() << "Expected response but got nothing"; });
}
} // namespace
