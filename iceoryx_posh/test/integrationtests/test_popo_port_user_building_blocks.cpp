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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "test.hpp"

#include <chrono>
#include <stdlib.h>
#include <thread>

using namespace ::testing;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;
using namespace iox::posix;
using ::testing::Return;

struct DummySample
{
    uint64_t m_dummy{42};
};

static const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION("x", "y", "z");
static const iox::ProcessName_t TEST_SUBSCRIBER_APP_NAME("mySubscriberApp");
static const iox::ProcessName_t TEST_PUBLISHER_APP_NAME("myPublisherApp");

static constexpr uint32_t NUM_CHUNKS_IN_POOL = 3 * iox::MAX_RECEIVER_QUEUE_CAPACITY;
static constexpr uint32_t SMALL_CHUNK = 128;
static constexpr uint32_t CHUNK_META_INFO_SIZE = 256;
static constexpr size_t MEMORY_SIZE = NUM_CHUNKS_IN_POOL * (SMALL_CHUNK + CHUNK_META_INFO_SIZE);
alignas(64) static uint8_t g_memory[MEMORY_SIZE];
static constexpr uint32_t ITERATIONS = 10;
static constexpr uint32_t MAX_NUMBER_QUEUES = 128;

struct ChunkDistributorConfig
{
    static constexpr uint32_t MAX_QUEUES = MAX_NUMBER_QUEUES;
    static constexpr uint64_t MAX_HISTORY_CAPACITY = iox::MAX_HISTORY_CAPACITY_OF_CHUNK_DISTRIBUTOR;
};

struct ChunkQueueConfig
{
    static constexpr uint64_t MAX_QUEUE_CAPACITY = NUM_CHUNKS_IN_POOL;
};

using ChunkQueueData_t = ChunkQueueData<ChunkQueueConfig, ThreadSafePolicy>;
using ChunkDistributorData_t =
    ChunkDistributorData<ChunkDistributorConfig, ThreadSafePolicy, ChunkQueuePusher<ChunkQueueData_t>>;
using ChunkDistributor_t = ChunkDistributor<ChunkDistributorData_t>;
using ChunkQueuePopper_t = ChunkQueuePopper<ChunkQueueData_t>;

class PortUser_SingleProducer_IntegrationTest : public Test
{
  public:
    PortUser_SingleProducer_IntegrationTest()
    {
        m_mempoolConfig.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolConfig, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~PortUser_SingleProducer_IntegrationTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
        m_publisherUserSide.stopOffer();
        m_publisherUserSide.destroy();

        m_subscriberPortUserSingleProducer.unsubscribe();
        m_subscriberPortUserSingleProducer.detachConditionVaribale();

        m_waiter.reset();
    }

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
    uint64_t m_sendCounter{0};
    uint64_t m_receiveCounter{0};
    std::atomic<bool> m_publisherRun{true};

    // Memory objects
    Allocator m_memoryAllocator{g_memory, MEMORY_SIZE};
    MePooConfig m_mempoolConfig;
    MemoryManager m_memoryManager;

    ConditionVariableData m_condVarData;
    ConditionVariableWaiter m_waiter{&m_condVarData};

    ChunkQueueData_t m_chunkQueueData{iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

    // subscriber port
    iox::popo::SubscriberPortData m_subscriberPortDataSingleProducer{
        TEST_SERVICE_DESCRIPTION,
        TEST_SUBSCRIBER_APP_NAME,
        iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::popo::SubscriberPortUser m_subscriberPortUserSingleProducer{&m_subscriberPortDataSingleProducer};

    // publisher port w/o history
    iox::popo::PublisherPortData m_publisherPortData{
        TEST_SERVICE_DESCRIPTION, TEST_PUBLISHER_APP_NAME, &m_memoryManager};
    iox::popo::PublisherPortUser m_publisherUserSide{&m_publisherPortData};

    void subscriberThread()
    {
        bool finished{false};
        // this is to prevent a race condition on thread shutdown; there must be two consecutive empty pops after the
        // forward thread finished
        bool newChunkReceivedInLastIteration{true};

        m_subscriberPortUserSingleProducer.attachConditionVariable(&m_condVarData);
        m_subscriberPortUserSingleProducer.subscribe();

        while (!finished)
        {
            if (m_waiter.timedWait(1_ms))
            {
                m_subscriberPortUserSingleProducer.getChunk()
                    .and_then([&](iox::cxx::optional<const iox::mepoo::ChunkHeader*>& maybeChunkHeader) {
                        if (maybeChunkHeader.has_value())
                        {
                            auto chunkHeader = maybeChunkHeader.value();
                            auto dummySample = *reinterpret_cast<DummySample*>(chunkHeader->payload());
                            // Check if monotonically increasing
                            EXPECT_THAT(dummySample.m_dummy, Eq(m_receiveCounter));
                            m_receiveCounter++;
                            m_subscriberPortUserSingleProducer.releaseChunk(chunkHeader);
                            newChunkReceivedInLastIteration = true;
                        }
                        else if (!m_publisherRun.load(std::memory_order_relaxed))
                        {
                            if (newChunkReceivedInLastIteration)
                            {
                                newChunkReceivedInLastIteration = false;
                            }
                            else
                            {
                                finished = true;
                            }
                        }
                    })
                    .or_else([](ChunkReceiveError) {
                        // Errors shall never occur
                        FAIL();
                    });
            }
            else
            {
            }
        }
    }

    void publisherThread()
    {
        m_publisherUserSide.offer();

        for (size_t i = 0; i < ITERATIONS; i++)
        {
            m_publisherUserSide.allocateChunk(sizeof(DummySample))
                .and_then([&](iox::mepoo::ChunkHeader* chunkHeader) {
                    auto sample = chunkHeader->payload();
                    new (sample) DummySample();
                    static_cast<DummySample*>(sample)->m_dummy = i;
                    m_publisherUserSide.sendChunk(chunkHeader);
                    m_sendCounter++;
                })
                .or_else([](AllocationError) {
                    // Errors shall never occur
                    FAIL();
                });

            /// Add some jitter to make thread breathe
            std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));
        }
        // Signal the next threads we're done
        m_publisherRun = false;
    }
};

TEST_F(PortUser_SingleProducer_IntegrationTest, test1)
{
    std::thread subscribingThread(&PortUser_SingleProducer_IntegrationTest::subscriberThread, this);
    std::thread publishingThread(&PortUser_SingleProducer_IntegrationTest::publisherThread, this);

    if (publishingThread.joinable())
    {
        publishingThread.join();
    }

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    EXPECT_EQ(m_sendCounter, m_receiveCounter);
}
