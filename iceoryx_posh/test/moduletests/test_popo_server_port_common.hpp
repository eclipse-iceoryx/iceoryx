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

#ifndef IOX_TEST_POPO_SERVER_PORT_COMMON_HPP
#define IOX_TEST_POPO_SERVER_PORT_COMMON_HPP

#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"

#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/assertions.hpp"

#include "test.hpp"

namespace iox_test_popo_server_port
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox::popo;
using namespace iox::mepoo;

class ServerPort_test : public Test
{
    static constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    Watchdog m_deadlockWatchdog{DEADLOCK_TIMEOUT};

    struct SutServerPort
    {
        SutServerPort(const ServiceDescription& serviceDescription,
                      const iox::RuntimeName_t& runtimeName,
                      const ServerOptions& serverOptions,
                      MemoryManager& memoryManager)
            : portData(
                serviceDescription, runtimeName, iox::roudi::DEFAULT_UNIQUE_ROUDI_ID, serverOptions, &memoryManager)
        {
        }

        ServerPortData portData;
        ServerPortUser portUser{portData};
        ServerPortRouDi portRouDi{portData};
        ChunkQueuePusher<ServerChunkQueueData_t> requestQueuePusher{&portData.m_chunkReceiverData};
    };

  public:
    ServerPort_test()
    {
        MePooConfig mempoolconf;
        mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS});
        m_memoryManager.configureMemoryManager(mempoolconf, m_memoryAllocator, m_memoryAllocator);
    }

    void SetUp() override
    {
        m_deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });

        // this is basically what RouDi does when a server is requested
        IOX_DISCARD_RESULT(serverPortWithOfferOnCreate.portRouDi.tryGetCaProMessage());
        IOX_DISCARD_RESULT(serverPortWithoutOfferOnCreate.portRouDi.tryGetCaProMessage());
        IOX_DISCARD_RESULT(serverOptionsWithBlockProducerRequestQueueFullPolicy.portRouDi.tryGetCaProMessage());
        IOX_DISCARD_RESULT(serverOptionsWithWaitForConsumerClientTooSlowPolicy.portRouDi.tryGetCaProMessage());
    }

    void TearDown() override
    {
    }

    void addClientQueue(SutServerPort& serverPort)
    {
        CaproMessage message;
        message.m_chunkQueueData = &clientChunkQueueData;
        message.m_type = CaproMessageType::CONNECT;
        message.m_serviceDescription = m_serviceDescription;
        auto maybeCaproMessage = serverPort.portRouDi.dispatchCaProMessageAndGetPossibleResponse(message);
    }

    void removeClientQueue(SutServerPort& serverPort)
    {
        CaproMessage message;
        message.m_chunkQueueData = &clientChunkQueueData;
        message.m_type = CaproMessageType::DISCONNECT;
        message.m_serviceDescription = m_serviceDescription;
        auto maybeCaproMessage = serverPort.portRouDi.dispatchCaProMessageAndGetPossibleResponse(message);
    }

    uint32_t getNumberOfUsedChunks() const
    {
        return m_memoryManager.getMemPoolInfo(0U).m_usedChunks;
    }

    SharedChunk getChunkFromMemoryManager(uint64_t userPayloadSize, uint32_t userHeaderSize)
    {
        auto chunkSettings = ChunkSettings::create(userPayloadSize,
                                                   iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT,
                                                   userHeaderSize,
                                                   iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT)
                                 .expect("Valid 'ChunkSettings'");

        return m_memoryManager.getChunk(chunkSettings).expect("Obtaining chunk");
    }

    static constexpr uint64_t DUMMY_DATA{0U};

    SharedChunk getChunkWithInitializedRequestHeaderAndData(const uint64_t data = DUMMY_DATA)
    {
        constexpr uint64_t USER_PAYLOAD_SIZE{sizeof(uint64_t)};
        auto sharedChunk = getChunkFromMemoryManager(USER_PAYLOAD_SIZE, sizeof(RequestHeader));
        new (sharedChunk.getChunkHeader()->userHeader())
            RequestHeader(clientChunkQueueData.m_uniqueId, RpcBaseHeader::UNKNOWN_CLIENT_QUEUE_INDEX);
        new (sharedChunk.getUserPayload()) uint64_t(data);
        return sharedChunk;
    }

    uint64_t getRequestData(const RequestHeader* requestHeader)
    {
        IOX_ENFORCE(requestHeader != nullptr, "requestHeader must not be a nullptr");
        auto userPayload = ChunkHeader::fromUserHeader(requestHeader)->userPayload();
        IOX_ENFORCE(userPayload != nullptr, "userPayload must not be a nullptr");

        return *static_cast<const uint64_t*>(userPayload);
    }

    /// @return true if all pushes succeed, false if a push failed and a chunk was lost
    bool pushRequests(ChunkQueuePusher<ServerChunkQueueData_t>& chunkQueuePusher,
                      uint64_t numberOfPushes,
                      uint64_t requestDataBase = DUMMY_DATA,
                      QueueFullPolicy queueFullPolicy = QueueFullPolicy::DISCARD_OLDEST_DATA)
    {
        for (uint64_t i = 0U; i < numberOfPushes; ++i)
        {
            auto sharedChunk = getChunkWithInitializedRequestHeaderAndData(requestDataBase + i);
            if (!chunkQueuePusher.push(sharedChunk))
            {
                // this would actually be done by the ChunkDistributor from the ClientPort
                if (queueFullPolicy == QueueFullPolicy::DISCARD_OLDEST_DATA)
                {
                    chunkQueuePusher.lostAChunk();
                }
                return false;
            }
        }

        return true;
    }

    void allocateResponseWithRequestHeaderAndThen(
        SutServerPort& sut, std::function<void(const RequestHeader* const, ResponseHeader* const)> testFunction)
    {
        constexpr uint64_t USER_PAYLOAD_SIZE{8};
        constexpr uint32_t USER_PAYLOAD_ALIGNMENT{8};

        constexpr uint64_t NUMBER_OF_REQUESTS{1U};
        pushRequests(sut.requestQueuePusher, NUMBER_OF_REQUESTS);
        auto requestResult = sut.portUser.getRequest();
        ASSERT_FALSE(requestResult.has_error());
        auto requestHeader = requestResult.value();

        sut.portUser.allocateResponse(requestHeader, USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
            .and_then([&](auto& responseHeader) { testFunction(requestHeader, responseHeader); })
            .or_else([&](const auto& error) { GTEST_FAIL() << "Expected ResponseHeader but got error: " << error; });
    }

    static constexpr uint64_t QUEUE_CAPACITY{iox::MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY * 2U};

  private:
    static constexpr uint32_t NUM_CHUNKS =
        iox::MAX_REQUESTS_ALLOCATED_SIMULTANEOUSLY + iox::MAX_RESPONSES_ALLOCATED_SIMULTANEOUSLY
        + iox::MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY + iox::MAX_RESPONSES_PROCESSED_SIMULTANEOUSLY + 16U;
    static constexpr uint64_t CHUNK_SIZE = 128U;
    static constexpr size_t MEMORY_SIZE = 1024U * 1024U;
    uint8_t m_memory[MEMORY_SIZE];
    iox::BumpAllocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    MemoryManager m_memoryManager;

    ServiceDescription m_serviceDescription{"hyp", "no", "toad"};
    iox::RuntimeName_t m_runtimeName{"hypnotoad"};

    ServerOptions m_serverOptionsWithOfferOnCreate = [&] {
        ServerOptions options;
        options.offerOnCreate = true;
        options.requestQueueCapacity = QUEUE_CAPACITY;
        return options;
    }();
    ServerOptions m_serverOptionsWithoutOfferOnCreate = [] {
        ServerOptions options;
        options.offerOnCreate = false;
        options.requestQueueCapacity = QUEUE_CAPACITY;
        return options;
    }();

    ServerOptions m_serverOptionsWithBlockProducerRequestQueueFullPolicy = [&] {
        ServerOptions options;
        options.offerOnCreate = true;
        options.requestQueueCapacity = QUEUE_CAPACITY;
        options.requestQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
        return options;
    }();
    ServerOptions m_serverOptionsWithWaitForConsumerClientTooSlowPolicy = [&] {
        ServerOptions options;
        options.offerOnCreate = true;
        options.requestQueueCapacity = QUEUE_CAPACITY;
        options.clientTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
        return options;
    }();

    iox::optional<SutServerPort> clientPortForStateTransitionTests;

  public:
    static constexpr uint64_t USER_PAYLOAD_SIZE{32U};
    static constexpr uint32_t USER_PAYLOAD_ALIGNMENT{8U};

    ClientChunkQueueData_t clientChunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                iox::popo::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};
    ChunkQueuePopper<ClientChunkQueueData_t> clientResponseQueue{&clientChunkQueueData};

    SutServerPort serverPortWithOfferOnCreate{
        m_serviceDescription, m_runtimeName, m_serverOptionsWithOfferOnCreate, m_memoryManager};
    SutServerPort serverPortWithoutOfferOnCreate{
        m_serviceDescription, m_runtimeName, m_serverOptionsWithoutOfferOnCreate, m_memoryManager};
    SutServerPort serverOptionsWithBlockProducerRequestQueueFullPolicy{
        m_serviceDescription, m_runtimeName, m_serverOptionsWithBlockProducerRequestQueueFullPolicy, m_memoryManager};
    SutServerPort serverOptionsWithWaitForConsumerClientTooSlowPolicy{
        m_serviceDescription, m_runtimeName, m_serverOptionsWithWaitForConsumerClientTooSlowPolicy, m_memoryManager};
};

} // namespace iox_test_popo_server_port

#endif // IOX_TEST_POPO_SERVER_PORT_COMMON_HPP
