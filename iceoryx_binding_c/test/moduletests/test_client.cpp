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

#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"

using namespace iox::popo;
using namespace iox::capro;
using namespace iox::capro;
using namespace iox::cxx;

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
                        options,
                        &memoryManager);
        return &*sutPort;
    }

    void connect()
    {
        sutPort->m_connectRequested.store(true);
        sutPort->m_connectionState = iox::ConnectionState::CONNECTED;

        sutPort->m_chunkSenderData.m_queues.emplace_back(&sutServerQueue);
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
    char managmentMemory[MANAGEMENT_MEMORY_SIZE];
    iox::posix::Allocator mgmtAllocator{managmentMemory, MANAGEMENT_MEMORY_SIZE};
    static constexpr uint64_t DATA_MEMORY_SIZE = 1024 * 1024;
    char dataMemory[DATA_MEMORY_SIZE];
    iox::posix::Allocator dataAllocator{dataMemory, DATA_MEMORY_SIZE};
    iox::mepoo::MemoryManager memoryManager;
    iox::mepoo::MePooConfig memoryConfig;

    iox::cxx::optional<ClientPortData> sutPort;
    iox_client_storage_t sutStorage;
    iox::cxx::VariantQueue<int64_t, 2> sutServerQueue{iox::cxx::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};

    static constexpr const char SERVICE[] = "allGlory";
    static constexpr const char INSTANCE[] = "ToThe";
    static constexpr const char EVENT[] = "HYPNOTOAD";
};
constexpr const char iox_client_test::RUNTIME_NAME[];
constexpr const char iox_client_test::SERVICE[];
constexpr const char iox_client_test::INSTANCE[];
constexpr const char iox_client_test::EVENT[];

TEST_F(iox_client_test, notInitializedOptionsAreUninitialized)
{
    iox_client_options_t uninitializedOptions;
    // ignore the warning since we would like to test the behavior of an uninitialized option
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    EXPECT_FALSE(iox_client_options_is_initialized(&uninitializedOptions));
#pragma GCC diagnostic pop
}

TEST_F(iox_client_test, initializedOptionsAreInitialized)
{
    iox_client_options_t initializedOptions;
    iox_client_options_init(&initializedOptions);
    EXPECT_TRUE(iox_client_options_is_initialized(&initializedOptions));
}

TEST_F(iox_client_test, InitializingClientWithNullptrOptionsGetMiddlewareClientWithDefaultOptions)
{
    ClientOptions defaultOptions;
    prepareClientInit(defaultOptions);

    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    ASSERT_THAT(sut, Ne(nullptr));
}

/// @todo enable and adjust it when iox-#1032 is implemented
TEST_F(iox_client_test, DISABLED_InitializingClientWithInitializedOptionsFails)
{
    iox_client_options_t uninitializedOptions;

    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    ASSERT_THAT(sut, Eq(nullptr));
}

TEST_F(iox_client_test, InitializingClientWithCustomOptionsWork)
{
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
}

TEST_F(iox_client_test, DeinitReleasesClient)
{
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);
    ASSERT_THAT(sut, Ne(nullptr));

    iox_client_deinit(sut);
    EXPECT_THAT(sutPort->m_toBeDestroyed.load(), Eq(true));
}

TEST_F(iox_client_test, LoanWithValidArgumentsWorks)
{
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    EXPECT_THAT(iox_client_loan(sut, &payload, 32, 32), Eq(AllocationResult_SUCCESS));
    EXPECT_TRUE(isPayloadInDataSegment(payload));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(1U));
}

TEST_F(iox_client_test, LoanFailsWhenNoMoreChunksAreAvailable)
{
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    iox_client_loan(sut, &payload, 32, 32);
    iox_client_loan(sut, &payload, 32, 32);

    payload = nullptr;
    EXPECT_THAT(iox_client_loan(sut, &payload, 32, 32), Eq(AllocationResult_RUNNING_OUT_OF_CHUNKS));
    EXPECT_THAT(payload, Eq(nullptr));
    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(2U));
}

TEST_F(iox_client_test, ReleaseWorksOnValidPayload)
{
    prepareClientInit();
    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    void* payload = nullptr;
    iox_client_loan(sut, &payload, 32, 32);

    iox_client_release_request(sut, payload);

    EXPECT_THAT(memoryManager.getMemPoolInfo(0).m_usedChunks, Eq(0U));
}


} // namespace
