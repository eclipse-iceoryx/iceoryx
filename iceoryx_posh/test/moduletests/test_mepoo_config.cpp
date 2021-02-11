// Copyright (c) 2019, 2021 by  Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
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
    constexpr uint32_t size{128U};
    constexpr uint32_t chunkCount{100U};

    sut.addMemPool({size, chunkCount});

    EXPECT_EQ(sut.m_mempoolConfig.size(), 1U);
    EXPECT_EQ(sut.m_mempoolConfig[0].m_size, size);
    EXPECT_EQ(sut.m_mempoolConfig[0].m_chunkCount, chunkCount);
}

TEST_F(MePooConfig_Test, AddingMempoolWhenTheMemPoolConfigContainerIsFullReturnsError)
{
    MePooConfig sut;
    constexpr uint32_t size{128U};
    constexpr uint32_t chunkCount{100U};

    for (size_t i = 0; i < iox::MAX_NUMBER_OF_MEMPOOLS; i++)
    {
        sut.addMemPool({size, chunkCount});
    }
    EXPECT_DEATH({ sut.addMemPool({size, chunkCount}); }, ".*");
}

TEST_F(MePooConfig_Test, SetDefaultMethodAddsTheDefaultMemPoolConfigurationToTheMemPoolConfigContainer)
{
    MePooConfig sut;
    MePooConfig::Entry defaultEntry[7] = {{128, 10000},
                                          {1024, 5000},
                                          {1024 * 16, 1000},
                                          {1024 * 128, 200},
                                          {1024 * 512, 50},
                                          {1024 * 1024, 30},
                                          {1024 * 1024 * 4, 10}};

    sut.setDefaults();

    EXPECT_EQ(sut.m_mempoolConfig.size(), 7U);
    for (size_t i = 0; i < 7; i++)
    {
        EXPECT_THAT((sut.m_mempoolConfig[i].m_chunkCount), Eq(defaultEntry[i].m_chunkCount));
        EXPECT_THAT((sut.m_mempoolConfig[i].m_size), Eq(defaultEntry[i].m_size));
    }
}

TEST_F(MePooConfig_Test, GetMemoryConfigMethodReturnsTheMemPoolConfigContainerWithAddedMempools)
{
    MePooConfig sut;
    constexpr uint32_t chunkCount{100U};
    constexpr uint32_t size{128U};
    sut.addMemPool({size, chunkCount});

    const iox::mepoo::MePooConfig::MePooConfigContainerType* mempoolconfptr = sut.getMemPoolConfig();

    EXPECT_THAT(mempoolconfptr->size(), Eq(1U));
    EXPECT_THAT((mempoolconfptr[0].data()->m_chunkCount), Eq(chunkCount));
    EXPECT_THAT((mempoolconfptr[0].data()->m_size), Eq(size));
}

TEST_F(MePooConfig_Test, OptimizeMethodCombinesTwoMempoolWithSameSizeAndDoublesTheChunkCountInTheMemPoolConfigContainer)
{
    MePooConfig sut;
    constexpr uint32_t chunkCount{100U};
    constexpr uint32_t size{100U};
    sut.addMemPool({size, chunkCount});
    sut.addMemPool({size, chunkCount});

    sut.optimize();

    EXPECT_THAT(sut.m_mempoolConfig.size(), Eq(1U));
    EXPECT_THAT(sut.m_mempoolConfig[0].m_chunkCount, Eq(chunkCount * 2));
}

TEST_F(MePooConfig_Test, OptimizeMethodRemovesTheMempoolWithSizeZeroInTheMemPoolConfigContainer)
{
    MePooConfig sut;
    constexpr uint32_t chunkCount{100U};
    constexpr uint32_t size1{64U};
    constexpr uint32_t size2{0U};
    constexpr uint32_t size3{128U};
    sut.addMemPool({size1, chunkCount});
    sut.addMemPool({size2, chunkCount});
    sut.addMemPool({size3, chunkCount});

    sut.optimize();

    EXPECT_THAT(sut.m_mempoolConfig.size(), Eq(2U));
}

TEST_F(MePooConfig_Test, OptimizeMethodSortsTheAddedMempoolsInTheMemPoolConfigContainerInIncreasingOrderOfSize)
{
    MePooConfig sut;
    constexpr uint32_t chunkCount{100U};
    constexpr uint32_t size1{512U};
    constexpr uint32_t size2{128U};
    constexpr uint32_t size3{256U};
    sut.addMemPool({size1, chunkCount});
    sut.addMemPool({size2, chunkCount});
    sut.addMemPool({size3, chunkCount});

    sut.optimize();

    EXPECT_THAT((sut.m_mempoolConfig[0].m_size), Eq(size2));
    EXPECT_THAT((sut.m_mempoolConfig[1].m_size), Eq(size3));
    EXPECT_THAT((sut.m_mempoolConfig[2].m_size), Eq(size1));
}

TEST_F(MePooConfig_Test, VerifyOptimizeMethodOnMePooConfigWithNoAddedMemPools)
{
    MePooConfig sut;

    sut.optimize();

    EXPECT_THAT(sut.m_mempoolConfig.size(), Eq(0U));
}