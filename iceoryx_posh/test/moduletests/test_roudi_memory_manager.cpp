// Copyright (c) 2020, 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "mocks/roudi_memory_block_mock.hpp"
#include "mocks/roudi_memory_provider_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::roudi;

class RouDiMemoryManager_Test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    static const int32_t nbTestCase = 4;

    RouDiMemoryManagerError m_testCombinationRoudiMemoryManagerError[nbTestCase] = {
        RouDiMemoryManagerError::MEMORY_PROVIDER_EXHAUSTED,
        RouDiMemoryManagerError::NO_MEMORY_PROVIDER_PRESENT,
        RouDiMemoryManagerError::MEMORY_CREATION_FAILED,
        RouDiMemoryManagerError::MEMORY_DESTRUCTION_FAILED,
    };

    const char* m_testResultOperatorMethod[nbTestCase] = {"MEMORY_PROVIDER_EXHAUSTED",
                                                          "NO_MEMORY_PROVIDER_PRESENT",
                                                          "MEMORY_CREATION_FAILED",
                                                          "MEMORY_DESTRUCTION_FAILED"};

    MemoryBlockMock memoryBlock1;
    MemoryBlockMock memoryBlock2;
    MemoryProviderTestImpl memoryProvider1;
    MemoryProviderTestImpl memoryProvider2;

    RouDiMemoryManager sut;
};

TEST_F(RouDiMemoryManager_Test, CallingCreateAndAnnounceMemoryWithoutMemoryProviderFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "8048cd15-3786-4eaf-9c26-e1cd6dce753c");
    auto expectError = sut.createAndAnnounceMemory();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(RouDiMemoryManagerError::NO_MEMORY_PROVIDER_PRESENT));
}

TEST_F(RouDiMemoryManager_Test, CallingCreateMemoryWithMemoryProviderSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "0634d8d5-5ab9-448b-a7c6-031b58374366");
    uint64_t MEMORY_SIZE_1{16};
    uint64_t MEMORY_ALIGNMENT_1{8};
    uint64_t MEMORY_SIZE_2{32};
    uint64_t MEMORY_ALIGNMENT_2{16};
    EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE_1));
    EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT_1));
    EXPECT_CALL(memoryBlock1, onMemoryAvailable(_));
    EXPECT_CALL(memoryBlock2, size()).WillRepeatedly(Return(MEMORY_SIZE_2));
    EXPECT_CALL(memoryBlock2, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT_2));
    EXPECT_CALL(memoryBlock2, onMemoryAvailable(_));

    IOX_DISCARD_RESULT(memoryProvider1.addMemoryBlock(&memoryBlock1));
    IOX_DISCARD_RESULT(memoryProvider2.addMemoryBlock(&memoryBlock2));

    ASSERT_FALSE(sut.addMemoryProvider(&memoryProvider1).has_error());
    ASSERT_FALSE(sut.addMemoryProvider(&memoryProvider2).has_error());

    EXPECT_THAT(sut.createAndAnnounceMemory().has_error(), Eq(false));

    EXPECT_CALL(memoryBlock1, destroy());
    EXPECT_CALL(memoryBlock2, destroy());
}

TEST_F(RouDiMemoryManager_Test, CallingCreateMemoryWithMemoryProviderError)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3d5a955-8dd3-40cb-9ac1-88021fbc52e1");
    ASSERT_FALSE(sut.addMemoryProvider(&memoryProvider1).has_error());

    // If no memory block is added to memory provider, Create and Announce Memory will return a error
    ASSERT_THAT(sut.createAndAnnounceMemory().has_error(), Eq(true));
    EXPECT_THAT(sut.createAndAnnounceMemory().error(), Eq(RouDiMemoryManagerError::MEMORY_CREATION_FAILED));

    ASSERT_FALSE(sut.destroyMemory().has_error());
}

TEST_F(RouDiMemoryManager_Test, RouDiMemoryManagerDTorTriggersMemoryProviderDestroy)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb14b892-9f78-4494-a269-0c361b6a88bd");
    uint64_t MEMORY_SIZE_1{16};
    uint64_t MEMORY_ALIGNMENT_1{8};
    EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE_1));
    EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT_1));
    EXPECT_CALL(memoryBlock1, onMemoryAvailable(_));

    IOX_DISCARD_RESULT(memoryProvider1.addMemoryBlock(&memoryBlock1));

    {
        RouDiMemoryManager sutDestroy;
        ASSERT_FALSE(sutDestroy.addMemoryProvider(&memoryProvider1).has_error());
        ASSERT_FALSE(sutDestroy.createAndAnnounceMemory().has_error());
        EXPECT_CALL(memoryBlock1, destroy()).Times(1);
    }
    EXPECT_CALL(memoryBlock1, destroy()).Times(0);
}

TEST_F(RouDiMemoryManager_Test, AddMemoryProviderExceedsCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "d80b71b8-7120-49f2-a77b-0f44a8abadde");
    MemoryProviderTestImpl memoryProvider[iox::MAX_NUMBER_OF_MEMORY_PROVIDER + 1];
    RouDiMemoryManager sutExhausting;

    for (uint32_t i = 0; i < iox::MAX_NUMBER_OF_MEMORY_PROVIDER; ++i)
    {
        EXPECT_THAT(sutExhausting.addMemoryProvider(&memoryProvider[i]).has_error(), Eq(false));
    }

    auto expectError = sutExhausting.addMemoryProvider(&memoryProvider[iox::MAX_NUMBER_OF_MEMORY_PROVIDER]);
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(RouDiMemoryManagerError::MEMORY_PROVIDER_EXHAUSTED));
}

TEST_F(RouDiMemoryManager_Test, OperatorTest)
{
    ::testing::Test::RecordProperty("TEST_ID", "67167a98-5ac2-498d-8062-47a61102a130");
    iox::testing::Logger_Mock loggerMock;
    for (uint16_t i = 0; i < nbTestCase; i++)
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << m_testCombinationRoudiMemoryManagerError[i];
        ASSERT_THAT(loggerMock.logs.size(), Eq(i + 1U));
        EXPECT_THAT(loggerMock.logs[i].message, Eq(m_testResultOperatorMethod[i]));
    }
}

} // namespace
