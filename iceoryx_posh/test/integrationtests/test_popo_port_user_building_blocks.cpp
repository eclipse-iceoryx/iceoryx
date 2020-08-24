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
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/concurrent/smart_lock.hpp"
#include "test.hpp"

#include <chrono>
#include <sstream>
#include <thread>

using namespace ::testing;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::cxx;
using namespace iox::mepoo;
using namespace iox::posix;
using ::testing::Return;

struct DummySample
{
    uint64_t m_dummy{42};
};

static const ServiceDescription TEST_SERVICE_DESCRIPTION("x", "y", "z");
static const iox::ProcessName_t TEST_SUBSCRIBER_APP_NAME("mySubscriberApp");
static const iox::ProcessName_t TEST_PUBLISHER_APP_NAME("myPublisherApp");

static constexpr uint32_t NUMBER_OF_PUBLISHERS = 42;
static constexpr uint32_t ITERATIONS = 1000;

static constexpr uint32_t NUM_CHUNKS_IN_POOL = NUMBER_OF_PUBLISHERS * 3 * iox::MAX_RECEIVER_QUEUE_CAPACITY;
static constexpr uint32_t SMALL_CHUNK = 128;
static constexpr uint32_t CHUNK_META_INFO_SIZE = 256;
static constexpr size_t MEMORY_SIZE = NUM_CHUNKS_IN_POOL * (SMALL_CHUNK + CHUNK_META_INFO_SIZE);
alignas(64) static uint8_t g_memory[MEMORY_SIZE];

class PortUser_IntegrationTest : public Test
{
  public:
    PortUser_IntegrationTest()
    {
        m_mempoolConfig.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolConfig, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~PortUser_IntegrationTest()
    {
    }

    void SetUp()
    {
        for (uint32_t i = 0; i < NUMBER_OF_PUBLISHERS; i++)
        {
            std::stringstream publisherAppName;
            publisherAppName << TEST_PUBLISHER_APP_NAME << i;

            iox::cxx::string<100> processName(TruncateToCapacity, publisherAppName.str().c_str());

            m_publisherPortDataVector.emplace_back(TEST_SERVICE_DESCRIPTION, processName, &m_memoryManager);
            m_publisherUserSideVector.emplace_back(&m_publisherPortDataVector.back());
            m_publisherRouDiSideVector.emplace_back(&m_publisherPortDataVector.back());
        }
    }

    void TearDown()
    {
        m_publisherUserSide.stopOffer();

        m_subscriberPortUserSingleProducer.unsubscribe();
        m_subscriberPortUserMultiProducer.unsubscribe();

        static_cast<void>(m_publisherRouDiSide.getCaProMessage());
        static_cast<void>(m_subscriberPortRouDiSideSingleProducer.getCaProMessage());
        static_cast<void>(m_subscriberPortRouDiMultiSingleProducer.getCaProMessage());

        if (m_subscriberPortUserSingleProducer.isConditionVariableAttached())
        {
            static_cast<void>(m_subscriberPortUserSingleProducer.detachConditionVariable());
        }

        if (m_subscriberPortUserMultiProducer.isConditionVariableAttached())
        {
            static_cast<void>(m_subscriberPortUserMultiProducer.detachConditionVariable());
        }

        m_waiter.reset();
    }

    GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                [] { iox::popo::internal::unsetUniqueRouDiId(); }};
    std::atomic<uint64_t> m_sendCounter{0};
    uint64_t m_receiveCounter{0};
    std::atomic<bool> m_publisherRun{true};

    // Memory objects
    Allocator m_memoryAllocator{g_memory, MEMORY_SIZE};
    MePooConfig m_mempoolConfig;
    MemoryManager m_memoryManager;

    ConditionVariableData m_condVarData;
    ConditionVariableWaiter m_waiter{&m_condVarData};

    using ConcurrentCaproMessageVector_t = iox::concurrent::smart_lock<vector<CaproMessage, 1>>;
    ConcurrentCaproMessageVector_t m_concurrentCaproMessageVector;
    iox::concurrent::smart_lock<vector<CaproMessage, 1>> m_caproMessageRx;

    // subscriber port for single producer
    SubscriberPortData m_subscriberPortDataSingleProducer{
        TEST_SERVICE_DESCRIPTION, TEST_SUBSCRIBER_APP_NAME, VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    SubscriberPortUser m_subscriberPortUserSingleProducer{&m_subscriberPortDataSingleProducer};
    SubscriberPortSingleProducer m_subscriberPortRouDiSideSingleProducer{&m_subscriberPortDataSingleProducer};

    // subscriber port for multi producer
    SubscriberPortData m_subscriberPortDataMultiProducer{
        TEST_SERVICE_DESCRIPTION, TEST_SUBSCRIBER_APP_NAME, VariantQueueTypes::SoFi_MultiProducerSingleConsumer};
    SubscriberPortUser m_subscriberPortUserMultiProducer{&m_subscriberPortDataMultiProducer};
    SubscriberPortMultiProducer m_subscriberPortRouDiMultiSingleProducer{&m_subscriberPortDataMultiProducer};

    // publisher port w/o history
    PublisherPortData m_publisherPortData{TEST_SERVICE_DESCRIPTION, TEST_PUBLISHER_APP_NAME, &m_memoryManager};
    PublisherPortUser m_publisherUserSide{&m_publisherPortData};
    PublisherPortRouDi m_publisherRouDiSide{&m_publisherPortData};

    vector<PublisherPortData, NUMBER_OF_PUBLISHERS> m_publisherPortDataVector;
    vector<PublisherPortUser, NUMBER_OF_PUBLISHERS> m_publisherUserSideVector;
    vector<PublisherPortRouDi, NUMBER_OF_PUBLISHERS> m_publisherRouDiSideVector;

    inline CaproMessage waitForCaproMessage(const ConcurrentCaproMessageVector_t& concurrentCaproMessageVector,
                                            const CaproMessageType& caproMessageType)
    {
        bool finished{false};
        CaproMessage caproMessage;

        do
        {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
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
    void subscriberThread(SubscriberPortProducerType& subscriberPortProducer, SubscriberPortUser& subscriberPortUser)
    {
        bool finished{false};
        optional<CaproMessage> maybeCaproMessage;
        CaproMessage caproMessage;

        subscriberPortUser.attachConditionVariable(&m_condVarData);

        // Wait for publisher to be ready
        caproMessage = waitForCaproMessage(m_concurrentCaproMessageVector, CaproMessageType::OFFER);

        // Subscribe to publisher
        subscriberPortUser.subscribe();
        maybeCaproMessage = subscriberPortProducer.getCaProMessage();
        if (maybeCaproMessage.has_value())
        {
            caproMessage = maybeCaproMessage.value();
            m_concurrentCaproMessageVector->push_back(caproMessage);
        }

        // Wait for subscription ACK from publisher
        caproMessage = waitForCaproMessage(m_concurrentCaproMessageVector, CaproMessageType::ACK);

        // Let RouDi change state to finish subscription
        static_cast<void>(subscriberPortProducer.dispatchCaProMessage(caproMessage));

        // Subscription done and ready to receive samples
        while (!finished)
        {
            if (m_waiter.timedWait(1000_ms))
            {
                // Condition variable triggered
                subscriberPortUser.getChunk()
                    .and_then([&](optional<const ChunkHeader*>& maybeChunkHeader) {
                        if (maybeChunkHeader.has_value())
                        {
                            auto chunkHeader = maybeChunkHeader.value();
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

    void publisherThread(uint32_t publisherThreadIndex,
                         PublisherPortRouDi& publisherPortRouDi,
                         PublisherPortUser& publisherPortUser)
    {
        optional<CaproMessage> maybeCaproMessage;
        CaproMessage caproMessage;

        // Publisher offers its service
        publisherPortUser.offer();

        // Let RouDi change state and send OFFER to subscriber
        maybeCaproMessage = publisherPortRouDi.getCaProMessage();

        if (publisherThreadIndex == 0)
        {
            if (maybeCaproMessage.has_value())
            {
                caproMessage = maybeCaproMessage.value();
                auto guardedVector = m_concurrentCaproMessageVector.GetScopeGuard();
                guardedVector->push_back(caproMessage);
            }

            // Wait for subscriber to subscribe
            caproMessage = waitForCaproMessage(m_concurrentCaproMessageVector, CaproMessageType::SUB);
            m_caproMessageRx->push_back(caproMessage);

            // Send ACK to subscriber
            maybeCaproMessage = publisherPortRouDi.dispatchCaProMessage(m_caproMessageRx->back());
            if (maybeCaproMessage.has_value())
            {
                caproMessage = maybeCaproMessage.value();
                m_concurrentCaproMessageVector->push_back(caproMessage);
            }
        }
        else
        {
            CaproMessage caproMessageRouDi(CaproMessageType::UNSUB, TEST_SERVICE_DESCRIPTION);
            do
            {
                std::this_thread::sleep_for(std::chrono::microseconds(10));

                if (m_caproMessageRx->size() != 0)
                {
                    caproMessageRouDi = m_caproMessageRx->back();
                }

            } while (caproMessageRouDi.m_type != CaproMessageType::SUB);

            static_cast<void>(publisherPortRouDi.dispatchCaProMessage(caproMessageRouDi));
        }

        // Subscriber is ready to receive -> start sending samples
        for (size_t i = 0; i < ITERATIONS; i++)
        {
            publisherPortUser.allocateChunk(sizeof(DummySample))
                .and_then([&](ChunkHeader* chunkHeader) {
                    auto sample = chunkHeader->payload();
                    new (sample) DummySample();
                    static_cast<DummySample*>(sample)->m_dummy = i;
                    publisherPortUser.sendChunk(chunkHeader);
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

TEST_F(PortUser_IntegrationTest, SingleProducer)
{
    std::thread subscribingThread(std::bind(&PortUser_IntegrationTest::subscriberThread<SubscriberPortSingleProducer>,
                                            this,
                                            std::ref(PortUser_IntegrationTest::m_subscriberPortRouDiSideSingleProducer),
                                            std::ref(PortUser_IntegrationTest::m_subscriberPortUserSingleProducer)));
    std::thread publishingThread(std::bind(&PortUser_IntegrationTest::publisherThread,
                                           this,
                                           0,
                                           std::ref(PortUser_IntegrationTest::m_publisherRouDiSide),
                                           std::ref(PortUser_IntegrationTest::m_publisherUserSide)));

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    if (publishingThread.joinable())
    {
        publishingThread.join();
    }

    EXPECT_EQ(m_sendCounter.load(std::memory_order_relaxed), m_receiveCounter);
}

TEST_F(PortUser_IntegrationTest, MultiProducer)
{
    std::thread subscribingThread(
        std::bind(&PortUser_IntegrationTest::subscriberThread<SubscriberPortMultiProducer>,
                  this,
                  std::ref(PortUser_IntegrationTest::m_subscriberPortRouDiMultiSingleProducer),
                  std::ref(PortUser_IntegrationTest::m_subscriberPortUserMultiProducer)));

    vector<std::thread, NUMBER_OF_PUBLISHERS> publisherThreadVector;
    for (uint32_t i = 0; i < NUMBER_OF_PUBLISHERS; i++)
    {
        publisherThreadVector.emplace_back(std::bind(&PortUser_IntegrationTest::publisherThread,
                                                     this,
                                                     i,
                                                     std::ref(PortUser_IntegrationTest::m_publisherRouDiSideVector[i]),
                                                     std::ref(PortUser_IntegrationTest::m_publisherUserSideVector[i])));
    }

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    for (uint32_t i = 0; i < NUMBER_OF_PUBLISHERS; i++)
    {
        if (publisherThreadVector[i].joinable())
        {
            publisherThreadVector[i].join();
        }
    }

    EXPECT_EQ(m_sendCounter.load(std::memory_order_relaxed), m_receiveCounter);
}
