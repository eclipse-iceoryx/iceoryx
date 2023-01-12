// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"

namespace
{
using namespace ::testing;

using iox::roudi::IceOryxRouDiMemoryManager;

/// @brief This test file verifies that the BaseClass IceoryxRouDiMemoryManager is tested
class IceoryxRoudiMemoryManager_test : public Test
{
  public:
    std::unique_ptr<IceOryxRouDiMemoryManager> m_roudiMemoryManagerTest;

    void SetUp() override
    {
        auto config = iox::RouDiConfig_t().setDefaults();
        m_roudiMemoryManagerTest = std::unique_ptr<IceOryxRouDiMemoryManager>(new IceOryxRouDiMemoryManager(config));
    }

    void TearDown() override
    {
    }
};

TEST_F(IceoryxRoudiMemoryManager_test, ConstructorSuccess)
{
    ::testing::Test::RecordProperty("TEST_ID", "f435abe6-07da-44e8-9f1a-074fbcb66209");
    EXPECT_THAT(m_roudiMemoryManagerTest, Not(Eq(nullptr)));
}

TEST_F(IceoryxRoudiMemoryManager_test, IntrospectionMemoryManagerNulloptWhenNotPresent)
{
    ::testing::Test::RecordProperty("TEST_ID", "49daf0d5-41d3-46f8-a1c5-88b37534d38e");
    auto result = m_roudiMemoryManagerTest->introspectionMemoryManager();
    EXPECT_THAT(result, Eq(iox::nullopt_t()));
}

TEST_F(IceoryxRoudiMemoryManager_test, segmentManagerNulloptWhenNotPresent)
{
    ::testing::Test::RecordProperty("TEST_ID", "951b828b-a99c-4eb6-8ea0-34cb34cf7d28");
    auto resultTest = m_roudiMemoryManagerTest->segmentManager();
    EXPECT_THAT(resultTest, Eq(iox::nullopt_t()));
}

TEST_F(IceoryxRoudiMemoryManager_test, portPoolNulloptWhenNotPresent)
{
    ::testing::Test::RecordProperty("TEST_ID", "67677e21-cc46-4734-b2ae-b3ad0a8aa5e2");
    auto testResult = m_roudiMemoryManagerTest->portPool();
    EXPECT_THAT(testResult, Eq(iox::nullopt_t()));
}

TEST_F(IceoryxRoudiMemoryManager_test, CreateAndAnnouceMemoryHasNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ce16e0f-1194-4544-84a2-446f8f8b06a6");
    auto testResult = m_roudiMemoryManagerTest->createAndAnnounceMemory();
    EXPECT_THAT(testResult.has_error(), Eq(false));
}

TEST_F(IceoryxRoudiMemoryManager_test, MgmtMemoryProviderReturnNonNullPtr)
{
    ::testing::Test::RecordProperty("TEST_ID", "e89c1ba8-34ca-410f-bb23-45fb41e24e77");
    auto testResult = m_roudiMemoryManagerTest->mgmtMemoryProvider();
    EXPECT_THAT(testResult, Not(Eq(nullptr)));
}

TEST_F(IceoryxRoudiMemoryManager_test, AcquiringIntrospectionMemoryManagerAfterCreateAndAnnounceMemoryIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b4d5286-288d-4bbc-b134-15437bafbbcd");
    auto tr = m_roudiMemoryManagerTest->createAndAnnounceMemory();

    EXPECT_THAT(tr.has_error(), Eq(false));

    auto result = m_roudiMemoryManagerTest->introspectionMemoryManager();
    EXPECT_THAT(result, Not(Eq(iox::nullopt_t())));
}

TEST_F(IceoryxRoudiMemoryManager_test, AcquiringSegmentManagerAfterCreateAndAnnounceMemoryIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c7fa6c4-d6db-41d8-9111-b6991fac0802");
    auto tr = m_roudiMemoryManagerTest->createAndAnnounceMemory();

    EXPECT_THAT(tr.has_error(), Eq(false));

    auto resultTest = m_roudiMemoryManagerTest->segmentManager();
    EXPECT_THAT(resultTest, Not(Eq(iox::nullopt_t())));
}

TEST_F(IceoryxRoudiMemoryManager_test, AcquiringPortPoolAfterCreateAndAnnounceMemoryIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "7131820d-950f-493a-8884-282380d80d05");
    auto tr = m_roudiMemoryManagerTest->createAndAnnounceMemory();

    EXPECT_THAT(tr.has_error(), Eq(false));

    auto testResult = m_roudiMemoryManagerTest->portPool();
    EXPECT_THAT(testResult, Not(Eq(iox::nullopt_t())));
}

TEST_F(IceoryxRoudiMemoryManager_test, DestroyMemoryReturnNoError)
{
    ::testing::Test::RecordProperty("TEST_ID", "9928be24-88c1-4296-92a1-fd8e2cd860d8");
    auto testResult = m_roudiMemoryManagerTest->createAndAnnounceMemory();
    EXPECT_FALSE(testResult.has_error());

    auto result = m_roudiMemoryManagerTest->destroyMemory();

    EXPECT_THAT(result.has_error(), Eq(false));
}

TEST_F(IceoryxRoudiMemoryManager_test, DestroyMemoryIntrospectionMemoryManagerReturnNullOpt)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8085b0d-7557-4032-874e-dbd327e7db39");
    auto testResult = m_roudiMemoryManagerTest->createAndAnnounceMemory();
    ASSERT_FALSE(testResult.has_error());

    auto result = m_roudiMemoryManagerTest->destroyMemory();
    ASSERT_FALSE(result.has_error());

    auto res = m_roudiMemoryManagerTest->introspectionMemoryManager();
    EXPECT_THAT(res, Eq(iox::nullopt_t()));
}

TEST_F(IceoryxRoudiMemoryManager_test, DestroyMemorySegmentManagerReturnNullOpt)
{
    ::testing::Test::RecordProperty("TEST_ID", "105509a6-21fd-4503-b431-1fdf069ac767");
    auto testResult = m_roudiMemoryManagerTest->createAndAnnounceMemory();
    ASSERT_FALSE(testResult.has_error());

    auto result = m_roudiMemoryManagerTest->destroyMemory();
    ASSERT_FALSE(result.has_error());

    auto resultTest = m_roudiMemoryManagerTest->segmentManager();
    EXPECT_THAT(resultTest, Eq(iox::nullopt_t()));
}

TEST_F(IceoryxRoudiMemoryManager_test, CreateAndAnnouceMemoryFailingAfterCalledTwoTimes)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ac68454-77f3-4764-8198-e2ddb9c301fa");
    auto testResult = m_roudiMemoryManagerTest->createAndAnnounceMemory();
    ASSERT_FALSE(testResult.has_error());

    auto result = m_roudiMemoryManagerTest->createAndAnnounceMemory();

    EXPECT_THAT(result.has_error(), Eq(true));
    EXPECT_THAT(result.get_error(), Eq(iox::roudi::RouDiMemoryManagerError::MEMORY_CREATION_FAILED));
}

TEST_F(IceoryxRoudiMemoryManager_test, DestroyMemoryNotFailingAfterCalledTwoTimes)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe5bd74e-632d-47b6-ad77-92f2d97fa4ff");
    auto testResult = m_roudiMemoryManagerTest->createAndAnnounceMemory();
    ASSERT_FALSE(testResult.has_error());

    auto result = m_roudiMemoryManagerTest->destroyMemory();
    ASSERT_FALSE(result.has_error());

    auto res = m_roudiMemoryManagerTest->destroyMemory();

    EXPECT_THAT(res.has_error(), Eq(false));
}

} // namespace
