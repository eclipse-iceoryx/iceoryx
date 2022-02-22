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
#include "iceoryx_hoofs/testing/watch_dog.hpp"
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

        sutPort->m_chunkSenderData.m_queues.emplace_back(&serverChunkQueueData);
    }

    void receiveChunk(const int64_t chunkValue = 0)
    {
        auto chunk = memoryManager.getChunk(*iox::mepoo::ChunkSettings::create(
            sizeof(int64_t), iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT, sizeof(ResponseHeader)));
        ASSERT_FALSE(chunk.has_error());
        new (chunk->getChunkHeader()->userHeader())
            ResponseHeader(iox::cxx::UniqueId(), RpcBaseHeader::UNKNOWN_CLIENT_QUEUE_INDEX, 0U);
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
    iox::posix::Allocator mgmtAllocator{managementMemory, MANAGEMENT_MEMORY_SIZE};
    static constexpr uint64_t DATA_MEMORY_SIZE = 1024 * 1024;
    char dataMemory[DATA_MEMORY_SIZE];
    iox::posix::Allocator dataAllocator{dataMemory, DATA_MEMORY_SIZE};
    iox::mepoo::MemoryManager memoryManager;
    iox::mepoo::MePooConfig memoryConfig;

    iox::cxx::optional<ClientPortData> sutPort;
    iox_client_storage_t sutStorage;

    ServerChunkQueueData_t serverChunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                iox::cxx::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};
    ChunkQueuePopper<ServerChunkQueueData_t> serverRequestQueue{&serverChunkQueueData};

    static constexpr const char SERVICE[] = "allGlory";
    static constexpr const char INSTANCE[] = "ToThe";
    static constexpr const char EVENT[] = "HYPNOTOAD";
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

} // namespace
