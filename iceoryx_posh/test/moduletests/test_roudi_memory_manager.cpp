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

#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"

#include "mocks/roudi_memory_block_mock.hpp"
#include "mocks/roudi_memory_provider_mock.hpp"

#include "test.hpp"

using namespace ::testing;

using namespace iox::roudi;
/// @todo the RouDiMemoryManager changed quite much from the initial idea, check which tests makes sense
#if 0
class RouDiMemoryManager_Test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    MemoryBlockMock memoryBlock1;
    MemoryBlockMock memoryBlock2;
    MemoryProviderTestImpl memoryProvider1;
    MemoryProviderTestImpl memoryProvider2;

    RouDiMemoryManager sut;
};

TEST_F(RouDiMemoryManager_Test, CallingCreateAndAnnounceMemoryWithoutMemoryProviderFails)
{
    auto expectError = sut.createAndAnnounceMemory();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(RouDiMemoryManagerError::NO_MEMORY_PROVIDER_PRESENT));
}

TEST_F(RouDiMemoryManager_Test, CallingCreateMemoryWithMemoryProviderSucceeds)
{
    uint64_t MEMORY_SIZE_1{16};
    uint64_t MEMORY_ALIGNMENT_1{8};
    uint64_t MEMORY_SIZE_2{32};
    uint64_t MEMORY_ALIGNMENT_2{16};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE_1));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT_1));
    EXPECT_CALL(memoryBlock1, memoryAvailableMock(_));
    EXPECT_CALL(memoryBlock2, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE_2));
    EXPECT_CALL(memoryBlock2, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT_2));
    EXPECT_CALL(memoryBlock2, memoryAvailableMock(_));

    memoryProvider1.addMemoryBlock(&memoryBlock1);
    memoryProvider2.addMemoryBlock(&memoryBlock2);

    sut.addMemoryProvider(&memoryProvider1);
    sut.addMemoryProvider(&memoryProvider2);

    EXPECT_THAT(sut.createAndAnnounceMemory().has_error(), Eq(false));

    EXPECT_CALL(memoryBlock1, destroyMock());
    EXPECT_CALL(memoryBlock2, destroyMock());
}

TEST_F(RouDiMemoryManager_Test, RouDiMemoryManagerDTorTriggersMemoryProviderDestroy)
{
    uint64_t MEMORY_SIZE_1{16};
    uint64_t MEMORY_ALIGNMENT_1{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE_1));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT_1));
    EXPECT_CALL(memoryBlock1, memoryAvailableMock(_));

    memoryProvider1.addMemoryBlock(&memoryBlock1);

    {
        RouDiMemoryManager sutDestroy;
        sutDestroy.addMemoryProvider(&memoryProvider1);
        sutDestroy.createAndAnnounceMemory();
        EXPECT_CALL(memoryBlock1, destroyMock()).Times(1);
    }
    EXPECT_CALL(memoryBlock1, destroyMock()).Times(0);
}

TEST_F(RouDiMemoryManager_Test, AddMemoryProviderExceedsCapacity)
{
    MemoryProviderTestImpl memoryProvider[iox::MAX_NUMBER_OF_MEMORY_PROVIDER + 1];
    RouDiMemoryManager sutExhausting;

    for (uint32_t i = 0; i < iox::MAX_NUMBER_OF_MEMORY_PROVIDER; ++i)
    {
        EXPECT_THAT(sutExhausting.addMemoryProvider(&memoryProvider[i]).has_error(), Eq(false));
    }

    auto expectError = sutExhausting.addMemoryProvider(&memoryProvider[iox::MAX_NUMBER_OF_MEMORY_PROVIDER]);
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(RouDiMemoryManagerError::MEMORY_PROVIDER_EXHAUSTED));
}
#endif
