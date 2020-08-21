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
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/internal/concurrent/smart_lock.hpp"

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
static constexpr uint32_t ITERATIONS = 1000;

class PortUserIntegrationTest_SingleProducer : public Test
{
  public:
    PortUserIntegrationTest_SingleProducer()
    {
        m_mempoolConfig.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolConfig, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~PortUserIntegrationTest_SingleProducer()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
        m_publisherUserSide.stopOffer();
        static_cast<void>(m_publisherRouDiSide.getCaProMessage());

        m_subscriberPortUserSingleProducer.unsubscribe();
        static_cast<void>(m_subscriberPortRouDiSideSingleProducer.getCaProMessage());
        EXPECT_THAT(m_subscriberPortUserSingleProducer.detachConditionVariable(), Eq(true));

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

    using ConcurrentCaproMessageVector_t = iox::concurrent::smart_lock<iox::cxx::vector<iox::capro::CaproMessage, 1>>;
    ConcurrentCaproMessageVector_t m_concurrentCaproMessageVector;

    iox::popo::SubscriberPortData::ChunkQueueData_t m_chunkQueueData{
        iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};

    // subscriber port
    iox::popo::SubscriberPortData m_subscriberPortDataSingleProducer{
        TEST_SERVICE_DESCRIPTION,
        TEST_SUBSCRIBER_APP_NAME,
        iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::popo::SubscriberPortUser m_subscriberPortUserSingleProducer{&m_subscriberPortDataSingleProducer};
    iox::popo::SubscriberPortSingleProducer m_subscriberPortRouDiSideSingleProducer{
        &m_subscriberPortDataSingleProducer};

    // publisher port w/o history
    iox::popo::PublisherPortData m_publisherPortData{
        TEST_SERVICE_DESCRIPTION, TEST_PUBLISHER_APP_NAME, &m_memoryManager};
    iox::popo::PublisherPortUser m_publisherUserSide{&m_publisherPortData};
    iox::popo::PublisherPortRouDi m_publisherRouDiSide{&m_publisherPortData};

    inline iox::capro::CaproMessage
    waitForCaproMessage(const ConcurrentCaproMessageVector_t& concurrentCaproMessageVector,
                        const iox::capro::CaproMessageType& caproMessageType)
    {
        bool finished{false};
        iox::capro::CaproMessage caproMessage;

        do
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            {
                auto guardedVector = concurrentCaproMessageVector.GetScopeGuard();
                if (guardedVector->size() != 0)
                {
                    caproMessage = guardedVector->back();

                    if (caproMessage.m_type == caproMessageType)
                    {
                        guardedVector->pop_back();
                        finished = true;
                    }
                }
            }
        } while (!finished);

        return caproMessage;
    }

    template <typename SubscriberPortProducerType>
    void subscriberThread(SubscriberPortProducerType& subscriberPortProducer,
                          iox::popo::SubscriberPortUser& subscriberPortUser)
    {
        bool finished{false};
        iox::cxx::optional<iox::capro::CaproMessage> maybeCaproMessage;
        iox::capro::CaproMessage caproMessage;

        subscriberPortUser.attachConditionVariable(&m_condVarData);

        // Wait for publisher to be ready
        caproMessage = waitForCaproMessage(m_concurrentCaproMessageVector, iox::capro::CaproMessageType::OFFER);

        // Subscribe to publisher
        subscriberPortUser.subscribe();
        maybeCaproMessage = subscriberPortProducer.getCaProMessage();
        if (maybeCaproMessage.has_value())
        {
            caproMessage = maybeCaproMessage.value();
            m_concurrentCaproMessageVector->push_back(caproMessage);
        }

        // Wait for subscription ACK from publisher
        caproMessage = waitForCaproMessage(m_concurrentCaproMessageVector, iox::capro::CaproMessageType::ACK);

        // Let RouDi change state to finish subscription
        static_cast<void>(subscriberPortProducer.dispatchCaProMessage(caproMessage));

        // Subscription done and ready to receive samples
        while (!finished)
        {
            if (m_waiter.timedWait(100_ms))
            {
                // Condition variable triggered
                subscriberPortUser.getChunk()
                    .and_then([&](iox::cxx::optional<const iox::mepoo::ChunkHeader*>& maybeChunkHeader) {
                        if (maybeChunkHeader.has_value())
                        {
                            auto chunkHeader = maybeChunkHeader.value();
                            auto dummySample = *reinterpret_cast<DummySample*>(chunkHeader->payload());
                            // Check if monotonically increasing
                            EXPECT_THAT(dummySample.m_dummy, Eq(m_receiveCounter));
                            m_receiveCounter++;
                            subscriberPortUser.releaseChunk(chunkHeader);
                        }
                    })
                    .or_else([](ChunkReceiveError) {
                        // Errors shall never occur
                        FAIL();
                    });
            }
            else
            {
                // Timeout -> check if publisher is still going
                if (!m_publisherRun.load(std::memory_order_relaxed))
                {
                    finished = true;
                }
            }
        }
    }

    void publisherThread()
    {
        iox::cxx::optional<iox::capro::CaproMessage> maybeCaproMessage;
        iox::capro::CaproMessage caproMessage;

        // Publisher offers its service
        m_publisherUserSide.offer();

        // Let RouDi change state and send OFFER to subscriber
        maybeCaproMessage = m_publisherRouDiSide.getCaProMessage();
        if (maybeCaproMessage.has_value())
        {
            caproMessage = maybeCaproMessage.value();
            auto guardedVector = m_concurrentCaproMessageVector.GetScopeGuard();
            guardedVector->push_back(caproMessage);
        }

        // Wait for subscriber to subscribe
        caproMessage = waitForCaproMessage(m_concurrentCaproMessageVector, iox::capro::CaproMessageType::SUB);

        // Send ACK to subscriber
        maybeCaproMessage = m_publisherRouDiSide.dispatchCaProMessage(caproMessage);
        if (maybeCaproMessage.has_value())
        {
            caproMessage = maybeCaproMessage.value();
            m_concurrentCaproMessageVector->push_back(caproMessage);
        }

        // Subscriber is ready to receive -> start sending samples
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

        // Signal the subscriber thread we're done
        m_publisherRun = false;
    }
};

TEST_F(PortUserIntegrationTest_SingleProducer, test1)
{
    std::thread subscribingThread(
        std::bind(&PortUserIntegrationTest_SingleProducer::subscriberThread<iox::popo::SubscriberPortSingleProducer>,
                  this,
                  std::ref(PortUserIntegrationTest_SingleProducer::m_subscriberPortRouDiSideSingleProducer),
                  std::ref(PortUserIntegrationTest_SingleProducer::m_subscriberPortUserSingleProducer)));
    std::thread publishingThread(&PortUserIntegrationTest_SingleProducer::publisherThread, this);

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
