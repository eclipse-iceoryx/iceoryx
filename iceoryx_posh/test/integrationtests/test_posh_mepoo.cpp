// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_posh/roudi/roudi_app.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include <algorithm>

#define private public
#define protected public

#include "iceoryx_posh/internal/roudi/roudi_multi_process.hpp"
#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"

#undef private
#undef protected

#include "test.hpp"

using namespace ::testing;
using ::testing::Return;

using iox::mepoo::MePooConfig;
using iox::roudi::RouDiEnvironment;

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

    Mepoo_IntegrationTest()
    {
    }
    virtual ~Mepoo_IntegrationTest()
    {
    }

    // just to prevent the "hides overloaded virtual function" warning
    virtual void SetUp()
    {
    }

    iox::RouDiConfig_t createRouDiConfig(MemPoolInfoContainer& memPoolTestContainer,
                                         std::vector<TestMemPoolConfig>& testMempoolConfig)
    {
        iox::mepoo::MePooConfig mempoolConfig;

        uint64_t mempoolSize{0};
        // create actual config
        for (size_t i = 0; i < testMempoolConfig.size() && i < memPoolTestContainer.capacity(); ++i)
        {
            iox::roudi::MemPoolInfo mempoolInfo;
            mempoolInfo.m_minFreeChunks = testMempoolConfig[i].chunkCount;
            mempoolInfo.m_usedChunks = 0;
            mempoolInfo.m_chunkSize = testMempoolConfig[i].chunkSize;
            mempoolInfo.m_numChunks = testMempoolConfig[i].chunkCount;
            memPoolTestContainer.push_back(mempoolInfo);

            mempoolConfig.m_mempoolConfig.push_back({testMempoolConfig[i].chunkSize, testMempoolConfig[i].chunkCount});
            mempoolSize += static_cast<uint64_t>(testMempoolConfig[i].chunkSize) * testMempoolConfig[i].chunkCount;
        }

        auto currentGroup = iox::posix::PosixGroup::getGroupOfCurrentProcess();
        iox::RouDiConfig_t roudiConfig;
        roudiConfig.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mempoolConfig});
        return roudiConfig;
    }

    void SetUp(MemPoolInfoContainer& memPoolTestContainer, std::vector<TestMemPoolConfig>& testMempoolConfig)
    {
        auto config = createRouDiConfig(memPoolTestContainer, testMempoolConfig);
        m_roudiEnv = iox::cxx::optional<RouDiEnvironment>(config);

        ASSERT_THAT(m_roudiEnv.has_value(), Eq(true));

        iox::capro::ServiceDescription m_service_description{99, 1, 20};

        auto& senderRuntime = iox::runtime::PoshRuntime::getInstance("/sender");
        senderPort = iox::popo::SenderPort(senderRuntime.getMiddlewareSender(m_service_description));

        auto& receiverRuntime = iox::runtime::PoshRuntime::getInstance("/receiver");
        receiverPort = iox::popo::ReceiverPort(receiverRuntime.getMiddlewareReceiver(m_service_description));
    }

    virtual void TearDown()
    {
        senderPort.deactivate();
        receiverPort.unsubscribe();
    }

    template <uint32_t size>
    struct MemPoolTestTopic
    {
        char testtopic[size] = {0};
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

    void getMempoolInfoFromIntrospection(MemPoolInfoContainer& mempoolInfo)
    {
        auto currentUser = iox::posix::PosixUser::getUserOfCurrentProcess();
        auto memoryManager = m_roudiEnv->m_roudiApp->m_mempoolIntrospection.m_segmentManager
                                 ->getSegmentInformationForUser(currentUser.getName())
                                 .m_memoryManager;
        m_roudiEnv->m_roudiApp->m_mempoolIntrospection.copyMemPoolInfo(*memoryManager, mempoolInfo);

        // internally, the chunks are adjusted to the additional management information;
        // this needs to be substracted to be able to compare to the configured sizes
        for (auto& mempool : mempoolInfo)
        {
            if (mempool.m_chunkSize != 0)
            {
                mempool.m_chunkSize = mempool.m_chunkSize - static_cast<uint32_t>(sizeof(iox::mepoo::ChunkHeader));
            }
        }
    }

    template <uint32_t size>
    uint32_t indexOfMempool(std::vector<TestMemPoolConfig>& testMempoolConfig) const
    {
        using Topic = MemPoolTestTopic<size>;
        constexpr auto topicSize = sizeof(Topic);

        uint32_t index = 0;
        for (const auto& mempoolConfig : testMempoolConfig)
        {
            if (topicSize <= mempoolConfig.chunkSize)
            {
                break;
            }
            index++;
        }
        return index;
    }

    template <uint32_t size>
    bool sendreceivesample(const int& times)
    {
        using Topic = MemPoolTestTopic<size>;
        constexpr auto topicSize = sizeof(Topic);

        if (!(senderPort.isPortActive()))
        {
            senderPort.activate();
        }

        if (receiverPort.isSubscribed())
        {
            receiverPort.unsubscribe();
        }

        m_roudiEnv->InterOpWait();
        receiverPort.subscribe(true, topicSize);
        m_roudiEnv->InterOpWait();

        for (int idx = 0; idx < times; ++idx)
        {
            auto sample = senderPort.reserveChunk(topicSize);
            new (sample->payload()) Topic;
            sample->m_info.m_payloadSize = topicSize;
            senderPort.deliverChunk(sample);
            m_roudiEnv->InterOpWait();
        }
        senderPort.deactivate();

        return true;
    }

    static constexpr uint32_t DefaultNumChunks = 10;

    std::vector<TestMemPoolConfig> defaultMemPoolConfig()
    {
        constexpr int MempoolCount = 6;
        std::vector<TestMemPoolConfig> defaultConfig;
        uint32_t power = 5;
        for (int i = 0; i < MempoolCount; ++i)
        {
            defaultConfig.push_back({1u << power, DefaultNumChunks});
            ++power;
        }

        return defaultConfig;
    }

    std::vector<uint32_t> m_introspectionChunkSizes;

    MePooConfig memconf;

    iox::popo::SenderPort senderPort{nullptr};
    iox::popo::ReceiverPort receiverPort{nullptr};

    iox::cxx::optional<RouDiEnvironment> m_roudiEnv;
};


TEST_F(Mepoo_IntegrationTest, MempoolConfigCheck)
{
    MemPoolInfoContainer memPoolTestContainer;

    auto testMempoolConfig = defaultMemPoolConfig();

    SetUp(memPoolTestContainer, testMempoolConfig);

    constexpr int samplesize1 = 200;
    const int repetition1 = 1;
    ASSERT_TRUE(sendreceivesample<samplesize1>(repetition1));
    auto mempolIndex1 = indexOfMempool<samplesize1>(testMempoolConfig);
    memPoolTestContainer[mempolIndex1].m_usedChunks = repetition1;
    memPoolTestContainer[mempolIndex1].m_minFreeChunks =
        memPoolTestContainer[mempolIndex1].m_minFreeChunks - repetition1;

    m_roudiEnv->InterOpWait();

    constexpr int samplesize2 = 450;
    const int repetition2 = 3;
    ASSERT_TRUE(sendreceivesample<samplesize2>(repetition2));
    auto mempolIndex2 = indexOfMempool<samplesize2>(testMempoolConfig);
    memPoolTestContainer[mempolIndex2].m_usedChunks = repetition2;
    memPoolTestContainer[mempolIndex2].m_minFreeChunks =
        memPoolTestContainer[mempolIndex2].m_minFreeChunks - repetition2;

    m_roudiEnv->InterOpWait();


    // get mempoolconfig from introspection
    MemPoolInfoContainer memPoolInfoContainer;
    EXPECT_THAT(compareMemPoolInfo(memPoolInfoContainer, memPoolTestContainer, Log::Off), Eq(false));
    getMempoolInfoFromIntrospection(memPoolInfoContainer);

    EXPECT_THAT(compareMemPoolInfo(memPoolInfoContainer, memPoolTestContainer), Eq(true));
}

/// @todo jenkins doesn't have enough RAM for this test; needs to be moved to a nightly test on a powerfull machine
TEST_F(Mepoo_IntegrationTest, DISABLED_MempoolOver4GB)
{
    constexpr uint32_t chunkSize = 16777216;
    constexpr uint32_t m_numChunks = 350;

    MemPoolInfoContainer memPoolTestContainer;

    std::vector<TestMemPoolConfig> testMempoolConfig;
    testMempoolConfig.push_back({chunkSize, m_numChunks});
    auto mempoolSize = 1234; // TODO get from createRouDiConfig();
    SetUp(memPoolTestContainer, testMempoolConfig);

    uint64_t size = 666; // this->m_roudiEnv->m_roudiApp->m_shmMgr.m_mePooConfig.getSharedMemorySize();
    EXPECT_THAT(size, Eq(mempoolSize));

    constexpr int samplesize2 = chunkSize - 10;
    const int repetition2 = 2;
    ASSERT_TRUE(sendreceivesample<samplesize2>(repetition2));

    auto mempolIndex2 = indexOfMempool<samplesize2>(testMempoolConfig);
    memPoolTestContainer[mempolIndex2].m_usedChunks = repetition2;
    memPoolTestContainer[mempolIndex2].m_minFreeChunks = m_numChunks - repetition2;

    m_roudiEnv->InterOpWait();


    // get mempoolconfig from introspection
    MemPoolInfoContainer memPoolInfoContainer;
    EXPECT_THAT(compareMemPoolInfo(memPoolInfoContainer, memPoolTestContainer, Log::Off), Eq(false));
    getMempoolInfoFromIntrospection(memPoolInfoContainer);

    EXPECT_THAT(compareMemPoolInfo(memPoolInfoContainer, memPoolTestContainer), Eq(true));
}

TEST_F(Mepoo_IntegrationTest, DISABLED_WrongSampleSize)
{
    MemPoolInfoContainer memPoolTestContainer;

    auto testMempoolConfig = defaultMemPoolConfig();
    SetUp(memPoolTestContainer, testMempoolConfig);

    constexpr int samplesize3 = 2048;
    const int repetition3 = 1;

    EXPECT_DEATH({ sendreceivesample<samplesize3>(repetition3); }, ".*");
}

TEST_F(Mepoo_IntegrationTest, DISABLED_SampleOverflow)
{
    MemPoolInfoContainer memPoolTestContainer;

    auto testMempoolConfig = defaultMemPoolConfig();
    SetUp(memPoolTestContainer, testMempoolConfig);

    constexpr int samplesize1 = 200;
    const int repetition1 = DefaultNumChunks;
    EXPECT_DEATH({ sendreceivesample<samplesize1>(repetition1); }, ".*");
}
