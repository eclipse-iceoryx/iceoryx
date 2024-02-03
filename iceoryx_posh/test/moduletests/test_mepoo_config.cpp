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
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::testing;

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
    ::testing::Test::RecordProperty("TEST_ID", "e2e10dcf-039a-4865-a8fa-5a716994f213");
    MePooConfig sut;
    constexpr uint64_t SIZE{128U};
    constexpr uint32_t CHUNK_COUNT{100U};

    sut.addMemPool({SIZE, CHUNK_COUNT});

    ASSERT_EQ(sut.m_mempoolConfig.size(), 1U);
    EXPECT_EQ(sut.m_mempoolConfig[0].m_size, SIZE);
    EXPECT_EQ(sut.m_mempoolConfig[0].m_chunkCount, CHUNK_COUNT);
}

TEST_F(MePooConfig_Test, AddingMempoolWhenTheMemPoolConfigContainerIsFullReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "67227cee-44e2-445f-ba6b-5066e7348757");
    MePooConfig sut;
    constexpr uint64_t SIZE{128U};
    constexpr uint32_t CHUNK_COUNT{100U};

    for (size_t i = 0U; i < iox::MAX_NUMBER_OF_MEMPOOLS; i++)
    {
        sut.addMemPool({SIZE, CHUNK_COUNT});
    }
    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            sut.addMemPool({SIZE, CHUNK_COUNT});
        },
        iox::PoshError::MEPOO__MAXIMUM_NUMBER_OF_MEMPOOLS_REACHED);
}

TEST_F(MePooConfig_Test, SetDefaultMethodAddsTheDefaultMemPoolConfigurationToTheMemPoolConfigContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "744b4d55-9782-421d-aafb-4464ec00d2a1");
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
    ::testing::Test::RecordProperty("TEST_ID", "45abbd55-9d30-4fdc-abf8-4dd85c2d48b4");
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint64_t SIZE{128U};
    sut.addMemPool({SIZE, CHUNK_COUNT});

    const MePooConfig::MePooConfigContainerType* mempoolconfptr = sut.getMemPoolConfig();

    ASSERT_THAT(mempoolconfptr->size(), Eq(1U));
    EXPECT_THAT((mempoolconfptr[0].data()->m_chunkCount), Eq(CHUNK_COUNT));
    EXPECT_THAT((mempoolconfptr[0].data()->m_size), Eq(SIZE));
}

TEST_F(MePooConfig_Test, OptimizeMethodCombinesTwoMempoolWithSameSizeAndDoublesTheChunkCountInTheMemPoolConfigContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "b39e5bc5-4352-4427-be72-deeed45d937d");
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint64_t SIZE{100U};
    sut.addMemPool({SIZE, CHUNK_COUNT});
    sut.addMemPool({SIZE, CHUNK_COUNT});

    sut.optimize();

    ASSERT_THAT(sut.m_mempoolConfig.size(), Eq(1U));
    EXPECT_THAT(sut.m_mempoolConfig[0].m_chunkCount, Eq(CHUNK_COUNT * 2U));
}

TEST_F(MePooConfig_Test, OptimizeMethodRemovesTheMempoolWithSizeZeroInTheMemPoolConfigContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "56209c3e-8b69-45cd-8ea5-ef347152ff7c");
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint64_t SIZE_1{64U};
    constexpr uint64_t SIZE_2{0U};
    constexpr uint64_t SIZE_3{128U};
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
    ::testing::Test::RecordProperty("TEST_ID", "c9034d02-9fa9-404f-955c-12f647b3a946");
    MePooConfig sut;
    constexpr uint32_t CHUNK_COUNT{100U};
    constexpr uint64_t SIZE_1{512U};
    constexpr uint64_t SIZE_2{128U};
    constexpr uint64_t SIZE_3{256U};
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
    ::testing::Test::RecordProperty("TEST_ID", "8ee01d92-acb6-4841-b844-2ae151c53200");
    MePooConfig sut;

    sut.optimize();

    ASSERT_THAT(sut.m_mempoolConfig.size(), Eq(0U));
}
