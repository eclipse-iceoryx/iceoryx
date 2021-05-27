// Copyright (c) 2021 by  Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "test.hpp"

using namespace ::testing;

using namespace iox::mepoo;

class MePooConfig_Test : public Test
{
  public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_F(MePooConfig_Test, AddMemPoolMethodAddsTheCorrespondingMempoolInTheMemPoolConfigContainer)
{
    MePooConfig sut;
    constexpr uint32_t SIZE{128U};
    constexpr uint32_t CHUNK_COUNT{100U};

    sut.addMemPool({SIZE, CHUNK_COUNT});

    ASSERT_EQ(sut.m_mempoolConfig.size(), 1U);
    EXPECT_EQ(sut.m_mempoolConfig[0].m_size, SIZE);
    EXPECT_EQ(sut.m_mempoolConfig[0].m_chunkCount, CHUNK_COUNT);
}

TEST_F(MePooConfig_Test, AddingMempoolWhenTheMemPoolConfigContainerIsFullReturnsError)
{
    MePooConfig sut;
    constexpr uint32_t SIZE{128U};
    constexpr uint32_t CHUNK_COUNT{100U};

    for (size_t i = 0U; i < iox::MAX_NUMBER_OF_MEMPOOLS; i++)
    {
        sut.addMemPool({SIZE, CHUNK_COUNT});
    }
    EXPECT_DEATH({ sut.addMemPool({SIZE, CHUNK_COUNT}); }, ".*");
}

TEST_F(MePooConfig_Test, SetDefaultMethodAddsTheDefaultMemPoolConfigurationToTheMemPoolConfigContainer)
{
    MePooConfig sut;
    MePooConfig::Entry defaultEntry[7] = {{128U, 10000U},
                                          {1024U, 5000U},
                                          {1024U * 16U, 1000U},
                                          {1024U * 128U, 200U},
                                          {1024U * 512U, 50U},
                                          {1024 * 1024, 30U},
                                          {1024U * 1024U * 4U, 10U}};

    sut.setDefaults();

    ASSERT_EQ(sut.m_mempoolConfig.size(), 7U);
    for (size_t i = 0; i < 7U; i++)
    {
        EXPECT_EQ((sut.m_mempoolConfig[i].m_chunkCount), defaultEntry[i].m_chunkCount);
        EXPECT_EQ((sut.m_mempoolConfig[i].m_size), defaultEntry[i].m_size);
    }
}

TEST_F(MePooConfig_Test, GetMemoryConfigMethodReturnsTheMemPoolConfigContainerWithAddedMempools)
{
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint32_t SIZE{128U};
    sut.addMemPool({SIZE, CHUNK_COUNT});

    const MePooConfig::MePooConfigContainerType* mempoolconfptr = sut.getMemPoolConfig();

    ASSERT_THAT(mempoolconfptr->size(), Eq(1U));
    EXPECT_THAT((mempoolconfptr[0].data()->m_chunkCount), Eq(CHUNK_COUNT));
    EXPECT_THAT((mempoolconfptr[0].data()->m_size), Eq(SIZE));
}

TEST_F(MePooConfig_Test, OptimizeMethodCombinesTwoMempoolWithSameSizeAndDoublesTheChunkCountInTheMemPoolConfigContainer)
{
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint32_t SIZE{100U};
    sut.addMemPool({SIZE, CHUNK_COUNT});
    sut.addMemPool({SIZE, CHUNK_COUNT});

    sut.optimize();

    ASSERT_THAT(sut.m_mempoolConfig.size(), Eq(1U));
    EXPECT_THAT(sut.m_mempoolConfig[0].m_chunkCount, Eq(CHUNK_COUNT * 2U));
}

TEST_F(MePooConfig_Test, OptimizeMethodRemovesTheMempoolWithSizeZeroInTheMemPoolConfigContainer)
{
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint32_t SIZE_1{64U};
    constexpr uint32_t SIZE_2{0U};
    constexpr uint32_t SIZE_3{128U};
    sut.addMemPool({SIZE_1, CHUNK_COUNT});
    sut.addMemPool({SIZE_2, CHUNK_COUNT});
    sut.addMemPool({SIZE_3, CHUNK_COUNT});

    sut.optimize();

    ASSERT_THAT(sut.m_mempoolConfig.size(), Eq(2U));
    EXPECT_THAT(sut.m_mempoolConfig[0].m_size, Eq(SIZE_1));
    EXPECT_THAT(sut.m_mempoolConfig[1].m_size, Eq(SIZE_3));
}

TEST_F(MePooConfig_Test, OptimizeMethodSortsTheAddedMempoolsInTheMemPoolConfigContainerInIncreasingOrderOfSize)
{
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint32_t SIZE_1{512U};
    constexpr uint32_t SIZE_2{128U};
    constexpr uint32_t SIZE_3{256U};
    sut.addMemPool({SIZE_1, CHUNK_COUNT});
    sut.addMemPool({SIZE_2, CHUNK_COUNT});
    sut.addMemPool({SIZE_3, CHUNK_COUNT});

    sut.optimize();

    ASSERT_THAT(sut.m_mempoolConfig.size(), Eq(3U));
    EXPECT_THAT((sut.m_mempoolConfig[0].m_size), Eq(SIZE_2));
    EXPECT_THAT((sut.m_mempoolConfig[1].m_size), Eq(SIZE_3));
    EXPECT_THAT((sut.m_mempoolConfig[2].m_size), Eq(SIZE_1));
}

TEST_F(MePooConfig_Test, VerifyOptimizeMethodOnMePooConfigWithNoAddedMemPools)
{
    MePooConfig sut;

    sut.optimize();

    ASSERT_THAT(sut.m_mempoolConfig.size(), Eq(0U));
}
