// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"

/// @todo Shall RouDiEnvironment be used?
#include "roudi_gtest.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;
using ::testing::Return;

class ChunkBuildingBlocks_IntegrationTest : public RouDi_GTest
{
  public:
    ChunkBuildingBlocks_IntegrationTest()
    {
    }
    virtual ~ChunkBuildingBlocks_IntegrationTest()
    {
    }

    void SetUp()
    {
        // auto& senderRuntime = iox::runtime::PoshRuntime::getInstance("/sender");
        // senderPort = iox::popo::SenderPort(senderRuntime.getMiddlewareSender(m_service_description));

        // auto& receiverRuntime = iox::runtime::PoshRuntime::getInstance("/receiver");
        // receiverPort = iox::popo::ReceiverPort(receiverRuntime.getMiddlewareReceiver(m_service_description));
    }


    void TearDown()
    {
        // senderPort.deactivate();
        // receiverPort.unsubscribe();
    }

    static constexpr uint64_t HISTORY_CAPACITY = 4;
    static constexpr uint32_t MAX_NUMBER_QUEUES = 128;

    iox::mepoo::MemoryManager m_memoryManager;

    using ChunkDistributorData_t = iox::popo::ChunkDistributorData<MAX_NUMBER_QUEUES, iox::popo::ThreadSafePolicy>;
    iox::popo::ChunkSenderData<ChunkDistributorData_t> m_chunkSenderData{&m_memoryManager, 0}; // must be 0 for test
    iox::popo::ChunkSenderData<ChunkDistributorData_t> m_chunkSenderDataWithHistory{&m_memoryManager, HISTORY_CAPACITY};

    using ChunkDistributor_t = iox::popo::ChunkDistributor<MAX_NUMBER_QUEUES, iox::popo::ThreadSafePolicy>;
    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSender{&m_chunkSenderData};
    iox::popo::ChunkSender<ChunkDistributor_t> m_chunkSenderWithHistory{&m_chunkSenderDataWithHistory};
};

TEST_F(ChunkBuildingBlocks_IntegrationTest, SendWithoutConnection)
{
}

TEST_F(ChunkBuildingBlocks_IntegrationTest, SendAndReceive)
{
}
