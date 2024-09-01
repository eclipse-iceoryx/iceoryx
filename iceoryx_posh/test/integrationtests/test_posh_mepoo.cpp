// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_posh/roudi/roudi_app.hpp"
#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/duration.hpp"
#include "iox/optional.hpp"
#include "iox/std_chrono_support.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

namespace
{
using namespace ::testing;
using namespace iox::units::duration_literals;
using namespace iox::roudi_env;
using iox::mepoo::MePooConfig;

class Mepoo_IntegrationTest : public Test
{
  public:
    using MemPoolInfoContainer = iox::roudi::MemPoolInfoContainer;
    using MePooConfig = iox::mepoo::MePooConfig;

    struct TestMemPoolConfig
    {
        TestMemPoolConfig(uint32_t chunkSize, uint32_t chunkCount)
            : chunkSize(chunkSize)
            , chunkCount(chunkCount)
        {
        }
        uint32_t chunkSize;
        uint32_t chunkCount;
    };

    void SetUp() override
    {
    }

    void TearDown() override
    {
        publisherPort->stopOffer();
        subscriberPort->unsubscribe();
    }

    enum class configType
    {
        /// Default RouDi Config
        DEFAULT,
        /// Custom defined Mempool Config
        CUSTOM,
    };

    iox::IceoryxConfig createIceoryxConfig(MemPoolInfoContainer& memPoolTestContainer,
                                           std::vector<TestMemPoolConfig>& testMempoolConfig,
                                           const configType defaultconf = configType::DEFAULT)
    {
        if (defaultconf == configType::CUSTOM)
        {
            iox::mepoo::MePooConfig mempoolConfig;

            // create actual config
            for (size_t i = 0; i < testMempoolConfig.size() && i < memPoolTestContainer.capacity(); ++i)
            {
                iox::roudi::MemPoolInfo mempoolInfo;
                mempoolInfo.m_minFreeChunks = testMempoolConfig[i].chunkCount;
                mempoolInfo.m_usedChunks = 0;
                mempoolInfo.m_chunkSize = testMempoolConfig[i].chunkSize;
                mempoolInfo.m_numChunks = testMempoolConfig[i].chunkCount;
                memPoolTestContainer.push_back(mempoolInfo);

                mempoolConfig.m_mempoolConfig.push_back(
                    {testMempoolConfig[i].chunkSize, testMempoolConfig[i].chunkCount});
            }

            auto currentGroup = iox::PosixGroup::getGroupOfCurrentProcess();
            iox::IceoryxConfig config;
            config.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mempoolConfig});
            return config;
        }
        else
        {
            return iox::IceoryxConfig().setDefaults();
        }
    }

    void SetUp(MemPoolInfoContainer& memPoolTestContainer,
               std::vector<TestMemPoolConfig>& testMempoolConfig,
               const configType defaultconf = configType::DEFAULT)
    {
        auto config = createIceoryxConfig(memPoolTestContainer, testMempoolConfig, defaultconf);
        m_roudiEnv.emplace(config);

        ASSERT_THAT(m_roudiEnv.has_value(), Eq(true));

        iox::capro::ServiceDescription m_service_description{"99", "1", "20"};

        auto& senderRuntime = iox::runtime::PoshRuntime::initRuntime("sender");
        publisherPort.emplace(senderRuntime.getMiddlewarePublisher(m_service_description));

        auto& receiverRuntime = iox::runtime::PoshRuntime::initRuntime("receiver");
        subscriberPort.emplace(receiverRuntime.getMiddlewareSubscriber(m_service_description));
    }

    void SetUpRouDiOnly(MemPoolInfoContainer& memPoolTestContainer,
                        std::vector<TestMemPoolConfig>& testMempoolConfig,
                        const configType defaultconf = configType::DEFAULT)
    {
        auto config = createIceoryxConfig(memPoolTestContainer, testMempoolConfig, defaultconf);
        m_roudiEnv.emplace(config);

        ASSERT_THAT(m_roudiEnv.has_value(), Eq(true));
    }

    void PrintTiming(iox::units::Duration start)
    {
        auto totalMillisconds = start.toMilliseconds();
        auto milliseconds = totalMillisconds % 1000U;
        auto totalSeconds = totalMillisconds / 1000U;
        auto seconds = totalSeconds % 60U;
        auto minutes = totalSeconds / 60U;
        std::cerr << "RouDi startup took " << minutes << " minutes " << seconds << " seconds and " << milliseconds
                  << " milliseconds" << std::endl;
    }

    template <uint32_t size>
    struct MemPoolTestTopic
    {
        uint8_t testtopic[size] = {0};
    };

    enum class Log
    {
        On,
        Off
    };

    bool compareMemPoolInfo(MemPoolInfoContainer& first, MemPoolInfoContainer& second, Log doLog = Log::On)
    {
        // check containersize is equal
        if (first.size() != second.size())
        {
            return false;
        }

        size_t index = 0;
        for (auto& info : first)
        {
            // check only for mempool which are not used by the introspection; sendreceivesample takes care to not use
            // this mempools
            bool isInIntrospectionMempools =
                std::find(m_introspectionChunkSizes.begin(), m_introspectionChunkSizes.end(), info.m_chunkSize)
                != m_introspectionChunkSizes.end();
            if (!isInIntrospectionMempools)
            {
                if (info.m_chunkSize != second[index].m_chunkSize)
                {
                    if (doLog == Log::On)
                    {
                        std::cerr << "chunksize must: " << info.m_chunkSize
                                  << " is not equal to chunksize is: " << second[index].m_chunkSize << std::endl;
                    }
                    return false;
                }
                if (info.m_minFreeChunks != second[index].m_minFreeChunks)
                {
                    if (doLog == Log::On)
                    {
                        std::cerr << "m_minFreeChunks must: " << info.m_minFreeChunks
                                  << " is not equal to m_minFreeChunks is: " << second[index].m_minFreeChunks
                                  << std::endl;
                    }
                    return false;
                }
                if (info.m_numChunks != second[index].m_numChunks)
                {
                    if (doLog == Log::On)
                    {
                        std::cerr << "m_numChunks must: " << info.m_numChunks
                                  << " is not equal to m_numChunks is: " << second[index].m_numChunks << std::endl;
                    }
                    return false;
                }
                if (info.m_usedChunks != second[index].m_usedChunks)
                {
                    if (doLog == Log::On)
                    {
                        std::cerr << "m_usedChunks must: " << info.m_usedChunks
                                  << " is not equal to m_usedChunks is: " << second[index].m_usedChunks << std::endl;
                    }
                    return false;
                }
            }
            index++;
        }
        return true;
    }

    void getMempoolInfoFromIntrospection(MemPoolInfoContainer& memPoolInfoContainer)
    {
        iox::runtime::PoshRuntime::initRuntime("hypnotoad");

        iox::popo::SubscriberOptions options;
        options.queueCapacity = 1U;
        options.historyRequest = 1U;

        iox::popo::Subscriber<iox::roudi::MemPoolIntrospectionInfoContainer> subscriber(
            iox::roudi::IntrospectionMempoolService, options);
        ASSERT_THAT(subscriber.getSubscriptionState(), iox::SubscribeState::SUBSCRIBED);

        iox::popo::WaitSet<1> waitset;
        waitset.attachState(subscriber, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
            std::cerr << "failed to attach subscriber" << std::endl;
            std::exit(EXIT_FAILURE);
        });

        auto notifications = waitset.wait();

        ASSERT_THAT(notifications.size(), Eq(1U));
        if (notifications[0]->doesOriginateFrom(&subscriber))
        {
            subscriber.take().and_then([&](auto& sample) {
                ASSERT_THAT(sample->size(), Eq(2U)); // internal and user mempools
                memPoolInfoContainer = sample->at(1).m_mempoolInfo;
                // internally, the chunks are adjusted to the additional management information;
                // this needs to be subtracted to be able to compare to the configured sizes
                for (auto& info : memPoolInfoContainer)
                {
                    if (info.m_chunkSize != 0)
                    {
                        info.m_chunkSize = info.m_chunkSize - sizeof(iox::mepoo::ChunkHeader);
                    }
                }
            });
        }
    }

    template <uint32_t size>
    uint32_t indexOfMempool(std::vector<TestMemPoolConfig>& testMempoolConfig) const
    {
        using Topic = MemPoolTestTopic<size>;
        constexpr auto TOPIC_SIZE = sizeof(Topic);

        uint32_t index = 0;
        for (const auto& mempoolConfig : testMempoolConfig)
        {
            if (TOPIC_SIZE <= mempoolConfig.chunkSize)
            {
                break;
            }
            index++;
        }
        return index;
    }

    template <uint32_t size>
    bool sendReceiveSample(const uint32_t& times,
                           iox::optional<iox::popo::AllocationError> expectedAllocationError = iox::nullopt)
    {
        using Topic = MemPoolTestTopic<size>;
        constexpr auto TOPIC_SIZE = sizeof(Topic);
        constexpr auto TOPIC_ALIGNMENT = alignof(Topic);

        bool hasRunAsExpected = true;
        for (uint32_t idx = 0; idx < times; ++idx)
        {
            auto allocationResult = publisherPort
                                        ->tryAllocateChunk(TOPIC_SIZE,
                                                           TOPIC_ALIGNMENT,
                                                           iox::CHUNK_NO_USER_HEADER_SIZE,
                                                           iox::CHUNK_NO_USER_HEADER_ALIGNMENT)
                                        .and_then([&](auto sample) {
                                            new (sample->userPayload()) Topic;
                                            publisherPort->sendChunk(sample);
                                        });

            if (allocationResult.has_value())
            {
                hasRunAsExpected &= !expectedAllocationError.has_value();
                EXPECT_FALSE(expectedAllocationError.has_value());
            }
            else if (!expectedAllocationError.has_value())
            {
                hasRunAsExpected = false;
                std::cout << "Did not expect an error but got: " << static_cast<uint32_t>(allocationResult.error())
                          << std::endl;
                EXPECT_TRUE(hasRunAsExpected);
            }
            else if (allocationResult.error() != expectedAllocationError.value())
            {
                hasRunAsExpected = false;
                std::cout << "Expected error: " << static_cast<uint32_t>(expectedAllocationError.value()) << std::endl;
                std::cout << "But got: " << static_cast<uint32_t>(allocationResult.error()) << std::endl;
                EXPECT_TRUE(hasRunAsExpected);
            }
        }

        return hasRunAsExpected;
    }

    static constexpr uint32_t DEFAULT_NUMBER_OF_CHUNKS{10U};

    std::vector<TestMemPoolConfig> defaultMemPoolConfig()
    {
        constexpr uint32_t MEMPOOL_COUNT = 6U;
        std::vector<TestMemPoolConfig> defaultConfig;
        uint32_t power = 5;
        for (uint32_t i = 0; i < MEMPOOL_COUNT; ++i)
        {
            defaultConfig.push_back({1u << power, DEFAULT_NUMBER_OF_CHUNKS});
            ++power;
        }

        return defaultConfig;
    }

    std::vector<uint32_t> m_introspectionChunkSizes;

    MePooConfig memconf;

    iox::optional<iox::popo::PublisherPortUser> publisherPort;
    iox::optional<iox::popo::SubscriberPortUser> subscriberPort;

    iox::optional<RouDiEnv> m_roudiEnv;
};

constexpr uint32_t Mepoo_IntegrationTest::DEFAULT_NUMBER_OF_CHUNKS;


TEST_F(Mepoo_IntegrationTest, MempoolConfigCheck)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa78a873-ee8d-445c-a42a-6548bd7c2c6b");
    MemPoolInfoContainer memPoolTestContainer;

    auto testMempoolConfig = defaultMemPoolConfig();

    SetUp(memPoolTestContainer, testMempoolConfig, configType::CUSTOM);

    constexpr uint32_t SAMPLE_SIZE_1 = 200U;
    constexpr uint32_t REPETITION_1 = 1U;
    ASSERT_TRUE(sendReceiveSample<SAMPLE_SIZE_1>(REPETITION_1));
    auto mempolIndex1 = indexOfMempool<SAMPLE_SIZE_1>(testMempoolConfig);
    memPoolTestContainer[mempolIndex1].m_usedChunks = REPETITION_1;
    memPoolTestContainer[mempolIndex1].m_minFreeChunks =
        memPoolTestContainer[mempolIndex1].m_minFreeChunks - REPETITION_1;

    constexpr uint32_t SAMPLE_SIZE_2 = 450U;
    constexpr uint32_t REPETITION_2 = 3U;
    ASSERT_TRUE(sendReceiveSample<SAMPLE_SIZE_2>(REPETITION_2));
    auto mempolIndex2 = indexOfMempool<SAMPLE_SIZE_2>(testMempoolConfig);
    memPoolTestContainer[mempolIndex2].m_usedChunks = REPETITION_2;
    memPoolTestContainer[mempolIndex2].m_minFreeChunks =
        memPoolTestContainer[mempolIndex2].m_minFreeChunks - REPETITION_2;

    // get mempoolconfig from introspection
    MemPoolInfoContainer memPoolInfoContainer;
    EXPECT_THAT(compareMemPoolInfo(memPoolInfoContainer, memPoolTestContainer, Log::Off), Eq(false));
    getMempoolInfoFromIntrospection(memPoolInfoContainer);

    EXPECT_THAT(compareMemPoolInfo(memPoolInfoContainer, memPoolTestContainer), Eq(true));
}

TEST_F(Mepoo_IntegrationTest, WrongSampleSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "f03bfe1c-5892-4638-979c-2532097347c1");
    MemPoolInfoContainer memPoolTestContainer;
    auto testMempoolConfig = defaultMemPoolConfig();
    SetUp(memPoolTestContainer, testMempoolConfig, configType::CUSTOM);
    constexpr uint32_t SAMPLE_SIZE = 2048U;
    constexpr uint32_t REPETITION = 1U;

    EXPECT_TRUE(sendReceiveSample<SAMPLE_SIZE>(REPETITION, {iox::popo::AllocationError::NO_MEMPOOLS_AVAILABLE}));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE);
}

TEST_F(Mepoo_IntegrationTest, SampleOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "62fcd41b-426a-4dbb-b69f-24288044deff");
    MemPoolInfoContainer memPoolTestContainer;
    auto testMempoolConfig = defaultMemPoolConfig();
    SetUp(memPoolTestContainer, testMempoolConfig, configType::CUSTOM);
    constexpr uint32_t SAMPLE_SIZE_1 = 200U;
    constexpr uint32_t REPETITION = 1U;

    // make the mempool empty
    EXPECT_TRUE(sendReceiveSample<SAMPLE_SIZE_1>(DEFAULT_NUMBER_OF_CHUNKS));

    // trigger out of chunk error
    EXPECT_TRUE(sendReceiveSample<SAMPLE_SIZE_1>(REPETITION, {iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS}));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_POOL_IS_RUNNING_OUT_OF_CHUNKS);
}

TIMING_TEST_F(Mepoo_IntegrationTest, MempoolCreationTimeDefaultConfig, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "0e76509d-d7af-4c8c-9de6-77e5b0dc9575");
    MemPoolInfoContainer memPoolTestContainer;
    auto testMempoolConfig = defaultMemPoolConfig();

    auto start = std::chrono::steady_clock::now();

    SetUp(memPoolTestContainer, testMempoolConfig, configType::DEFAULT);

    auto stop = std::chrono::steady_clock::now();

    // Calc the difference
    auto timediff = iox::into<iox::units::Duration>(stop - start);

    PrintTiming(timediff);

    // Currently we expect that the RouDi is ready at least after 2 seconds
    auto maxtime = 2000_ms;
    EXPECT_THAT(timediff, Le(maxtime));
})

} // namespace
