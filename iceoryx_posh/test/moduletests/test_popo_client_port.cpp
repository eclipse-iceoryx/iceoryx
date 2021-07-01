// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;

class ClientPort_test : public Test
{
    // keep this the very first and also private
    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0U); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};

  public:
    ClientPort_test()
    {
        constexpr uint32_t NUM_CHUNKS = 20U;
        constexpr uint32_t CHUNK_SIZE = 128U;
        iox::mepoo::MePooConfig mempoolconf;
        mempoolconf.addMemPool({CHUNK_SIZE, NUM_CHUNKS});
        m_memoryManager.configureMemoryManager(mempoolconf, m_memoryAllocator, m_memoryAllocator);
    }

    void SetUp() override
    {
        deadlockWatchdog.emplace(DEADLOCK_TIMEOUT);
        deadlockWatchdog->watchAndActOnFailure([] { std::terminate(); });

        // this is basically what RouDi does when a client is requested
        tryAdvanceToState(clientPortDataWithConnectOnCreate, iox::ConnectionState::CONNECTED);
        tryAdvanceToState(clientPortDataWithoutConnectOnCreate, iox::ConnectionState::NOT_CONNECTED);
    }

    void TearDown() override
    {
    }

    void tryAdvanceToState(ClientPortData& clientPortData, const iox::ConnectionState state)
    {
        ClientPortRouDi clientPortRouDi{clientPortData};
        auto maybeCaProMessage = clientPortRouDi.tryGetCaProMessage();
        if (state == iox::ConnectionState::NOT_CONNECTED && clientPortData.m_connectionState == state)
        {
            return;
        }

        ASSERT_THAT(clientPortData.m_connectionState, Eq(iox::ConnectionState::CONNECT_REQUESTED));
        if (clientPortData.m_connectionState == state)
        {
            return;
        }

        iox::capro::CaproMessage serverMessage{
            iox::capro::CaproMessageType::ACK, m_serviceDescription, iox::capro::CaproMessageSubType::NOSUBTYPE};
        serverMessage.m_chunkQueueData = &serverChunkQueueData;
        clientPortRouDi.dispatchCaProMessageAndGetPossibleResponse(serverMessage);
        ASSERT_THAT(clientPortData.m_connectionState, Eq(iox::ConnectionState::CONNECTED));
        if (clientPortData.m_connectionState == state)
        {
            return;
        }

        constexpr bool NOT_IMPLEMENTED{true};
        ASSERT_FALSE(NOT_IMPLEMENTED);
    }

    uint32_t getNumberOfUsedChunks() const
    {
        return m_memoryManager.getMemPoolInfo(0U).m_usedChunks;
    }

  private:
    static constexpr size_t MEMORY_SIZE = 1024U * 1024U;
    uint8_t m_memory[MEMORY_SIZE];
    iox::posix::Allocator m_memoryAllocator{m_memory, MEMORY_SIZE};
    iox::mepoo::MemoryManager m_memoryManager;

    iox::capro::ServiceDescription m_serviceDescription{"hyp", "no", "toad"};
    iox::RuntimeName_t m_runtimeName{"hypnotoad"};

    ClientOptions m_withConnectOnCreate = [] {
        ClientOptions options;
        options.connectOnCreate = true;
        return options;
    }();
    ClientOptions m_withoutConnectOnCreate = [] {
        ClientOptions options;
        options.connectOnCreate = false;
        return options;
    }();

  public:
    static constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    iox::cxx::optional<Watchdog> deadlockWatchdog;

    static constexpr uint32_t USER_PAYLOAD_SIZE{32U};
    static constexpr uint32_t USER_PAYLOAD_ALIGNMENT{8U};

    ServerChunkQueueData_t serverChunkQueueData{iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                iox::cxx::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};

    // client port with connect on create
    ClientPortData clientPortDataWithConnectOnCreate{
        m_serviceDescription, m_runtimeName, m_withConnectOnCreate, &m_memoryManager};
    ClientPortUser clientPortUserWithConnectOnCreate{clientPortDataWithConnectOnCreate};
    ClientPortRouDi clientPortRouDiWithConnectOnCreate{clientPortDataWithConnectOnCreate};

    // client port without connect on create
    ClientPortData clientPortDataWithoutConnectOnCreate{
        m_serviceDescription, m_runtimeName, m_withoutConnectOnCreate, &m_memoryManager};
    ClientPortUser clientPortUserWithoutConnectOnCreate{clientPortDataWithoutConnectOnCreate};
    ClientPortRouDi clientPortRouDiWithoutConnectOnCreate{clientPortDataWithoutConnectOnCreate};
};
constexpr iox::units::Duration ClientPort_test::DEADLOCK_TIMEOUT;

// BEGIN ClientPortUser tests

TEST_F(ClientPort_test, InitialConnectionStateOnPortWithConnectOnCreateIs_CONNECTED)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    EXPECT_THAT(sut.getConnectionState(), Eq(iox::ConnectionState::CONNECTED));
}

TEST_F(ClientPort_test, InitialConnectionStateOnPortWithoutConnectOnCreateIs_NOT_CONNECTED)
{
    auto& sut = clientPortUserWithoutConnectOnCreate;
    EXPECT_THAT(sut.getConnectionState(), Eq(iox::ConnectionState::NOT_CONNECTED));
}

TEST_F(ClientPort_test, AllocateRequestDoesNotFailAndUsesTheMempool)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    EXPECT_THAT(getNumberOfUsedChunks(), Eq(0U));

    auto maybeRequest = sut.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT);
    ASSERT_FALSE(maybeRequest.has_error());

    EXPECT_THAT(getNumberOfUsedChunks(), Eq(1U));
}

TEST_F(ClientPort_test, FreeRequestWithNullptrTerminates)
{
    auto& sut = clientPortUserWithConnectOnCreate;

    // stop all threads before death test
    deadlockWatchdog.reset();
    EXPECT_DEATH({ sut.freeRequest(nullptr); }, ".*");
}

TEST_F(ClientPort_test, FreeRequestWithValidRequestWorksAndReleasesTheChunkToTheMempool)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    sut.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](auto& requestHeader) {
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(1U));
            sut.freeRequest(requestHeader);
            EXPECT_THAT(this->getNumberOfUsedChunks(), Eq(0U));
        })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });
}

TEST_F(ClientPort_test, SendRequestWithNullptrOnConnectedClientPortTerminates)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    sut.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](auto&) {
            // stop all threads before death test
            deadlockWatchdog.reset();
            EXPECT_DEATH({ sut.sendRequest(nullptr); }, ".*");
        })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });
}

TEST_F(ClientPort_test, SendRequestOnConnectedClientPortEnqueuesRequestToServerQueue)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    sut.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](auto& requestHeader) { sut.sendRequest(requestHeader); })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });

    EXPECT_TRUE(serverChunkQueueData.m_queue.pop().has_value());
}

/// @todo send to full server queue ... should this be done in an integration test with a real ServerPort?

TEST_F(ClientPort_test, SendRequestOnNotConnectedClientPortDoesNotEnqueuesRequestToServerQueue)
{
    auto& sut = clientPortUserWithoutConnectOnCreate;
    sut.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](auto& requestHeader) { sut.sendRequest(requestHeader); })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });

    EXPECT_FALSE(serverChunkQueueData.m_queue.pop().has_value());
}

TEST_F(ClientPort_test, ConnectAfterPreviousSendRequestCallDoesNotEnqueuesRequestToServerQueue)
{
    auto& sut = clientPortUserWithoutConnectOnCreate;
    sut.allocateRequest(USER_PAYLOAD_SIZE, USER_PAYLOAD_ALIGNMENT)
        .and_then([&](auto& requestHeader) { sut.sendRequest(requestHeader); })
        .or_else([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        });

    sut.connect();
    tryAdvanceToState(clientPortDataWithoutConnectOnCreate, iox::ConnectionState::CONNECTED);

    EXPECT_FALSE(serverChunkQueueData.m_queue.pop().has_value());
}

TEST_F(ClientPort_test, GetResponseOnNotConnectedClientPortHasNoResponse)
{
    auto& sut = clientPortUserWithoutConnectOnCreate;
    sut.getResponse()
        .and_then([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        })
        .or_else([&](auto& err) { EXPECT_THAT(err, Eq(iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)); });
}

TEST_F(ClientPort_test, GetResponseOnConnectedClientPortWithEmptyResponseQueueHasNoResponse)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    sut.getResponse()
        .and_then([&](auto&) {
            constexpr bool UNREACHABLE{false};
            EXPECT_TRUE(UNREACHABLE);
        })
        .or_else([&](auto& err) { EXPECT_THAT(err, Eq(iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)); });
}

TEST_F(ClientPort_test, GetResponseOnConnectedClientPortWithNonEmptyResponseQueueHasResponse)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, ReleaseResponseWithNullptrIsGracefullyHandled)
{
    auto& sut = clientPortUserWithConnectOnCreate;

    // stop all threads before death test
    deadlockWatchdog.reset();
    EXPECT_DEATH({ sut.releaseResponse(nullptr); }, ".*");
}

TEST_F(ClientPort_test, ReleaseResponseWithValidResponseReleasesChunkToTheMempool)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, HasNewResponseOnEmptyResponseQueueReturnsFalse)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    EXPECT_FALSE(sut.hasNewResponses());
}

TEST_F(ClientPort_test, HasNewResponseOnNonEmptyResponseQueueReturnsTrue)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, HasNewResponseOnEmptyResponseQueueAfterPreviouslyNotEmptyReturnsFalse)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, HasLostResponsesSinceLastCallWithoutLosingResponsesReturnsFalse)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    EXPECT_FALSE(sut.hasLostResponsesSinceLastCall());
}

TEST_F(ClientPort_test, HasLostResponsesSinceLastCallWithLosingResponsesReturnsTrue)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, HasLostResponsesSinceLastCallReturnsFalseAfterPreviouslyReturningTrue)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, ConditionVariableInitiallyNotSet)
{
    auto& sut = clientPortUserWithConnectOnCreate;
    EXPECT_FALSE(sut.isConditionVariableSet());
}

TEST_F(ClientPort_test, SettingConditionVariableWithoutConditionVariablePresentWorks)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, SettingConditionVariableWithConditionVariablePresentWorks)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, UnsettingConditionVariableWithConditionVariablePresentWorks)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, UnsettingConditionVariableWithoutConditionVariablePresentIsHandledGracefully)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, ConnectOnNotConnectedClientPortResultsInStateChange)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, ConnectOnConnectedClientPortResultsInNoStateChange)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, DisconnectOnConnectedClientPortResultsInStateChange)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

TEST_F(ClientPort_test, DisconnectOnNotConnectedClientPortResultsInNoStateChange)
{
    constexpr bool UNIMPLEMENTED{true};
    EXPECT_FALSE(UNIMPLEMENTED);
}

// END ClientPortUser tests

// BEGIN ClientPortRouDi tests

/// @todo state machine test

// END ClientPortRouDi tests

} // namespace
