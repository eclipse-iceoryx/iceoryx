// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_roudi.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/atomic.hpp"
#include "iox/scope_guard.hpp"
#include "iox/smart_lock.hpp"
#include "test.hpp"

#include <chrono>
#include <sstream>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox;
using namespace iox::mepoo;

struct DummySample
{
    uint64_t m_dummy{42U};
};


constexpr iox::units::Duration DEADLOCK_TIMEOUT{15_s};

const ServiceDescription TEST_SERVICE_DESCRIPTION("x", "y", "z");
const iox::RuntimeName_t TEST_SUBSCRIBER_RUNTIME_NAME("mySubscriberApp");
const iox::RuntimeName_t TEST_PUBLISHER_RUNTIME_NAME("myPublisherApp");

constexpr uint32_t NUMBER_OF_PUBLISHERS = 17U;
constexpr uint32_t ITERATIONS = 1000U;

constexpr uint32_t NUM_CHUNKS_IN_POOL = NUMBER_OF_PUBLISHERS * ITERATIONS;
constexpr uint64_t SMALL_CHUNK = 128U;
constexpr uint32_t CHUNK_META_INFO_SIZE = 256U;
constexpr size_t MEMORY_SIZE = NUM_CHUNKS_IN_POOL * (SMALL_CHUNK + CHUNK_META_INFO_SIZE);
alignas(64) static uint8_t g_memory[MEMORY_SIZE];

class PortUser_IntegrationTest : public Test
{
  public:
    PortUser_IntegrationTest()
    {
        m_mempoolConfig.addMemPool({SMALL_CHUNK, NUM_CHUNKS_IN_POOL});
        m_memoryManager.configureMemoryManager(m_mempoolConfig, m_memoryAllocator, m_memoryAllocator);
    }

    ~PortUser_IntegrationTest()
    {
    }

    void SetUp()
    {
        for (uint32_t i = 0U; i < NUMBER_OF_PUBLISHERS; i++)
        {
            std::stringstream publisherRuntimeName;
            publisherRuntimeName << TEST_PUBLISHER_RUNTIME_NAME << i;

            iox::RuntimeName_t runtimeName(TruncateToCapacity, publisherRuntimeName.str().c_str());

            m_publisherPortDataVector.emplace_back(TEST_SERVICE_DESCRIPTION,
                                                   runtimeName,
                                                   roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                   &m_memoryManager,
                                                   PublisherOptions());
            m_publisherPortUserVector.emplace_back(&m_publisherPortDataVector.back());
            m_publisherPortRouDiVector.emplace_back(&m_publisherPortDataVector.back());
        }

        m_deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown()
    {
        for (uint32_t i = 0U; i < NUMBER_OF_PUBLISHERS; i++)
        {
            m_publisherPortUserVector[i].stopOffer();
            static_cast<void>(m_publisherPortRouDiVector[i].tryGetCaProMessage());
        }

        m_subscriberPortUserSingleProducer.unsubscribe();
        m_subscriberPortUserMultiProducer.unsubscribe();

        static_cast<void>(m_subscriberPortRouDiSingleProducer.tryGetCaProMessage());
        static_cast<void>(m_subscriberPortRouDiMultiProducer.tryGetCaProMessage());
    }

    Watchdog m_deadlockWatchdog{DEADLOCK_TIMEOUT};

    iox::concurrent::Atomic<uint64_t> m_receiveCounter{0U};
    iox::concurrent::Atomic<uint64_t> m_sendCounter{0U};
    iox::concurrent::Atomic<bool> m_publisherRunFinished{false};

    // Memory objects
    iox::BumpAllocator m_memoryAllocator{g_memory, MEMORY_SIZE};
    MePooConfig m_mempoolConfig;
    MemoryManager m_memoryManager;

    using ConcurrentCaproMessageVector_t = iox::concurrent::smart_lock<vector<CaproMessage, 1>>;
    ConcurrentCaproMessageVector_t m_concurrentCaproMessageExchange;
    ConcurrentCaproMessageVector_t m_concurrentCaproMessageRx;

    // subscriber port for single producer
    SubscriberPortData m_subscriberPortDataSingleProducer{TEST_SERVICE_DESCRIPTION,
                                                          TEST_SUBSCRIBER_RUNTIME_NAME,
                                                          roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                          VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
                                                          SubscriberOptions()};
    SubscriberPortUser m_subscriberPortUserSingleProducer{&m_subscriberPortDataSingleProducer};
    SubscriberPortSingleProducer m_subscriberPortRouDiSingleProducer{&m_subscriberPortDataSingleProducer};

    // subscriber port for multi producer
    SubscriberPortData m_subscriberPortDataMultiProducer{TEST_SERVICE_DESCRIPTION,
                                                         TEST_SUBSCRIBER_RUNTIME_NAME,
                                                         roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                                         VariantQueueTypes::SoFi_MultiProducerSingleConsumer,
                                                         SubscriberOptions()};
    SubscriberPortUser m_subscriberPortUserMultiProducer{&m_subscriberPortDataMultiProducer};
    SubscriberPortMultiProducer m_subscriberPortRouDiMultiProducer{&m_subscriberPortDataMultiProducer};

    // publisher port
    vector<PublisherPortData, NUMBER_OF_PUBLISHERS> m_publisherPortDataVector;
    vector<PublisherPortUser, NUMBER_OF_PUBLISHERS> m_publisherPortUserVector;
    vector<PublisherPortRouDi, NUMBER_OF_PUBLISHERS> m_publisherPortRouDiVector;

    vector<std::thread, NUMBER_OF_PUBLISHERS> m_publisherThreadVector;

    inline CaproMessage waitForCaproMessage(const ConcurrentCaproMessageVector_t& concurrentCaproMessageVector,
                                            const CaproMessageType& caproMessageType)
    {
        bool finished{false};
        CaproMessage caproMessage;

        // Wait until the expected CaPro message has arrived in the shared message vector between subscriber and the
        // first publisher thread
        do
        {
            // Add delay to allow other thread accessing the shared resource
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            {
                auto guardedVector = concurrentCaproMessageVector.get_scope_guard();
                if (guardedVector->size() != 0U)
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

    template <typename SubscriberPortType>
    void subscriberThread(SubscriberPortType& subscriberPortRouDi, SubscriberPortUser& subscriberPortUser)
    {
        bool finished{false};

        // Wait for publisher to be ready
        auto caproMessage = waitForCaproMessage(m_concurrentCaproMessageExchange, CaproMessageType::OFFER);

        // Subscribe to publisher
        subscriberPortUser.subscribe();
        auto maybeCaproMessage = subscriberPortRouDi.tryGetCaProMessage();
        if (maybeCaproMessage.has_value())
        {
            caproMessage = maybeCaproMessage.value();
            m_concurrentCaproMessageExchange->push_back(caproMessage);
        }
        else
        {
            // Error shall never occur
            GTEST_FAIL() << "Error in subscriber SUB CaPro message!";
        }

        // Wait for subscription ACK from publisher
        caproMessage = waitForCaproMessage(m_concurrentCaproMessageExchange, CaproMessageType::ACK);

        // Let RouDi change state to finish subscription
        static_cast<void>(subscriberPortRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessage));

        // Subscription done and ready to receive samples
        while (!finished || subscriberPortUser.hasNewChunks())
        {
            // Try to receive chunk
            subscriberPortUser.tryGetChunk()
                .and_then([&](auto& chunkHeader) {
                    m_receiveCounter.fetch_add(1, std::memory_order_relaxed);
                    subscriberPortUser.releaseChunk(chunkHeader);
                })
                .or_else([&](auto& result) {
                    if (result == ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                    {
                        // Nothing received -> check if publisher(s) still running
                        finished = m_publisherRunFinished.load(std::memory_order_relaxed);
                    }
                    else
                    {
                        // Errors shall never occur
                        GTEST_FAIL() << "Error in tryGetChunk(): " << static_cast<uint32_t>(result);
                    }
                });
        }
    }

    void publisherThread(uint32_t publisherThreadIndex,
                         PublisherPortRouDi& publisherPortRouDi,
                         PublisherPortUser& publisherPortUser)
    {
        CaproMessage caproMessage;

        // Publisher offers its service
        publisherPortUser.offer();

        // Let RouDi change state and send OFFER to subscriber
        auto maybeCaproMessage = publisherPortRouDi.tryGetCaProMessage();

        if (publisherThreadIndex == 0U)
        {
            // First publisher thread will sync with subscriber
            if (maybeCaproMessage.has_value())
            {
                caproMessage = maybeCaproMessage.value();
                m_concurrentCaproMessageExchange->push_back(caproMessage);
            }
            else
            {
                // Error shall never occur
                GTEST_FAIL() << "Error in publisher OFFER CaPro message!";
            }

            // Wait for subscriber to subscribe
            caproMessage = waitForCaproMessage(m_concurrentCaproMessageExchange, CaproMessageType::SUB);
            m_concurrentCaproMessageRx->push_back(caproMessage);

            // Send ACK to subscriber
            maybeCaproMessage =
                publisherPortRouDi.dispatchCaProMessageAndGetPossibleResponse(m_concurrentCaproMessageRx->back());
            if (maybeCaproMessage.has_value())
            {
                caproMessage = maybeCaproMessage.value();
                m_concurrentCaproMessageExchange->push_back(caproMessage);
            }
            else
            {
                // Error shall never occur
                GTEST_FAIL() << "Error in publisher ACK CaPro message!";
            }
        }
        else
        {
            // All other publisher threads wait for the first thread to be synced with subscriber (indicated by
            // receiving a SUB message) to continue
            CaproMessage caproMessageRouDi(CaproMessageType::UNSUB, TEST_SERVICE_DESCRIPTION);
            do
            {
                std::this_thread::sleep_for(std::chrono::microseconds(100));

                if (m_concurrentCaproMessageRx->size() != 0U)
                {
                    caproMessageRouDi = m_concurrentCaproMessageRx->back();
                }

            } while (caproMessageRouDi.m_type != CaproMessageType::SUB);

            static_cast<void>(publisherPortRouDi.dispatchCaProMessageAndGetPossibleResponse(caproMessageRouDi));
        }

        // Subscriber is ready to receive -> start sending samples
        size_t i = 0U;
        while (i < ITERATIONS)
        {
            // slow down to ensure there is no overflow
            auto receiveCounter = m_receiveCounter.load(std::memory_order_relaxed);
            auto sendCounter = m_sendCounter.load(std::memory_order_seq_cst);
            if (sendCounter - receiveCounter > 100)
            {
                std::this_thread::yield();
                continue;
            }
            publisherPortUser
                .tryAllocateChunk(sizeof(DummySample),
                                  alignof(DummySample),
                                  iox::CHUNK_NO_USER_HEADER_SIZE,
                                  iox::CHUNK_NO_USER_HEADER_ALIGNMENT)
                .and_then([&](auto chunkHeader) {
                    auto sample = chunkHeader->userPayload();
                    new (sample) DummySample();
                    static_cast<DummySample*>(sample)->m_dummy = i;
                    publisherPortUser.sendChunk(chunkHeader);
                    m_sendCounter.fetch_add(1, std::memory_order_relaxed);
                })
                .or_else([](auto error) {
                    // Errors shall never occur
                    GTEST_FAIL() << "Error in tryAllocateChunk(): " << static_cast<uint32_t>(error);
                });

            ++i;

            /// Add some jitter to make thread breathe
            /// On Windows even when asked for short sleeps the OS suspends the execution for multiple milliseconds;
            /// therefore lets sleep only every second iteration
            if (i % 2 == 0)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(100 + rand() % 50));
            }
        }
    }
};

TEST_F(PortUser_IntegrationTest, SingleProducer)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb62ac02-2b7d-4d1c-8699-9f5ba4d9bd5a");

    std::thread subscribingThread(
        [this] { subscriberThread(m_subscriberPortRouDiSingleProducer, m_subscriberPortUserSingleProducer); });
    std::thread publishingThread([this] {
        constexpr uint32_t INDEX_OF_PUBLISHER_SINGLE_PRODUCER = 0U;
        publisherThread(
            INDEX_OF_PUBLISHER_SINGLE_PRODUCER, m_publisherPortRouDiVector.front(), m_publisherPortUserVector.front());
    });

    if (publishingThread.joinable())
    {
        publishingThread.join();
    }
    m_publisherRunFinished.store(true, std::memory_order_relaxed);

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    EXPECT_THAT(m_receiveCounter.load(std::memory_order_relaxed), Eq(m_sendCounter.load(std::memory_order_relaxed)));
    EXPECT_FALSE(PortUser_IntegrationTest::m_subscriberPortUserMultiProducer.hasLostChunksSinceLastCall());
}

TEST_F(PortUser_IntegrationTest, MultiProducer)
{
    ::testing::Test::RecordProperty("TEST_ID", "d27279d3-26c0-4489-9208-bd361120525a");

    std::thread subscribingThread(
        [this] { subscriberThread(m_subscriberPortRouDiMultiProducer, m_subscriberPortUserMultiProducer); });

    for (uint32_t i = 0U; i < NUMBER_OF_PUBLISHERS; i++)
    {
        m_publisherThreadVector.emplace_back(
            [i, this] { publisherThread(i, m_publisherPortRouDiVector[i], m_publisherPortUserVector[i]); });
    }

    for (uint32_t i = 0U; i < NUMBER_OF_PUBLISHERS; i++)
    {
        if (m_publisherThreadVector[i].joinable())
        {
            m_publisherThreadVector[i].join();
        }
    }
    m_publisherRunFinished.store(true, std::memory_order_relaxed);

    if (subscribingThread.joinable())
    {
        subscribingThread.join();
    }

    EXPECT_THAT(m_receiveCounter.load(std::memory_order_relaxed), Eq(m_sendCounter.load(std::memory_order_relaxed)));
    EXPECT_FALSE(PortUser_IntegrationTest::m_subscriberPortUserMultiProducer.hasLostChunksSinceLastCall());
}

} // namespace
