// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/memory/memory_provider.hpp"

#include "iox/relative_pointer.hpp"

#include "mocks/roudi_memory_block_mock.hpp"
#include "mocks/roudi_memory_provider_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::roudi;

class MemoryProviderFailingCreation : public iox::roudi::MemoryProvider
{
  public:
    using MemoryProvider::getErrorString;

    iox::expected<void*, MemoryProviderError> createMemory(const uint64_t size [[maybe_unused]],
                                                           const uint64_t alignment [[maybe_unused]]) noexcept override
    {
        return iox::err(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    iox::expected<void, MemoryProviderError> destroyMemory() noexcept override
    {
        return iox::err(MemoryProviderError::MEMORY_DESTRUCTION_FAILED);
    }
};

class MemoryProvider_Test : public Test
{
  public:
    void SetUp() override
    {
        // since the MemoryProvider registers for relative pointer, it is necessary to call unregisterAll, to have a
        // clean environment especially for the first test
        iox::UntypedRelativePointer::unregisterAll();
    }

    void TearDown() override
    {
        // unregisterAll is also called to leave a clean environment after the last test
        iox::UntypedRelativePointer::unregisterAll();
    }

    static constexpr uint64_t COMMON_SETUP_MEMORY_SIZE{16};
    static constexpr uint64_t COMMON_SETUP_MEMORY_ALIGNMENT{8};

    iox::expected<void, MemoryProviderError> commonSetup()
    {
        EXPECT_FALSE(sut.addMemoryBlock(&memoryBlock1).has_error());
        EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(COMMON_SETUP_MEMORY_SIZE));
        EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(COMMON_SETUP_MEMORY_ALIGNMENT));
        EXPECT_CALL(sut, createMemoryMock(COMMON_SETUP_MEMORY_SIZE, COMMON_SETUP_MEMORY_ALIGNMENT)).Times(1);

        EXPECT_CALL(sut, destroyMemoryMock());
        EXPECT_CALL(memoryBlock1, destroy());

        return sut.create();
    }

    static constexpr iox::roudi::MemoryProviderError m_testCombinationMemoryProviderError[] = {
        iox::roudi::MemoryProviderError::MEMORY_BLOCKS_EXHAUSTED,
        iox::roudi::MemoryProviderError::NO_MEMORY_BLOCKS_PRESENT,
        iox::roudi::MemoryProviderError::MEMORY_ALREADY_CREATED,
        iox::roudi::MemoryProviderError::MEMORY_CREATION_FAILED,
        iox::roudi::MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE,
        iox::roudi::MemoryProviderError::MEMORY_ALLOCATION_FAILED,
        iox::roudi::MemoryProviderError::MEMORY_MAPPING_FAILED,
        iox::roudi::MemoryProviderError::MEMORY_NOT_AVAILABLE,
        iox::roudi::MemoryProviderError::MEMORY_DESTRUCTION_FAILED,
        iox::roudi::MemoryProviderError::MEMORY_DEALLOCATION_FAILED,
        iox::roudi::MemoryProviderError::MEMORY_UNMAPPING_FAILED,
        iox::roudi::MemoryProviderError::SIGACTION_CALL_FAILED};

    static constexpr const char* m_testResultGetErrorString[] = {"MEMORY_BLOCKS_EXHAUSTED",
                                                                 "NO_MEMORY_BLOCKS_PRESENT",
                                                                 "MEMORY_ALREADY_CREATED",
                                                                 "MEMORY_CREATION_FAILED",
                                                                 "MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE",
                                                                 "MEMORY_ALLOCATION_FAILED",
                                                                 "MEMORY_MAPPING_FAILED",
                                                                 "MEMORY_NOT_AVAILABLE",
                                                                 "MEMORY_DESTRUCTION_FAILED",
                                                                 "MEMORY_DEALLOCATION_FAILED",
                                                                 "MEMORY_UNMAPPING_FAILED",
                                                                 "SIGACTION_CALL_FAILED"};

    MemoryBlockMock memoryBlock1;
    MemoryBlockMock memoryBlock2;

    // the MemoryProvider is a class with a pure virtual member functions s, therefore we need an implementation to
    // instantiate and test non-virtual member functions
    MemoryProviderMock sut;
};
constexpr iox::roudi::MemoryProviderError MemoryProvider_Test::m_testCombinationMemoryProviderError[];
constexpr const char* MemoryProvider_Test::m_testResultGetErrorString[];

TEST_F(MemoryProvider_Test, InitiallyMemoryIsNotAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "25d5dc0c-4999-45b8-a26f-a18c5e2d2644");
    EXPECT_THAT(sut.isAvailable(), Eq(false));
}

TEST_F(MemoryProvider_Test, InitiallyMemoryIsNotAvailableAnnounced)
{
    ::testing::Test::RecordProperty("TEST_ID", "709ea86a-9480-4ef8-a471-982f5343e221");
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));
}

TEST_F(MemoryProvider_Test, AddMemoryBlock)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5588686-68c0-44d2-b637-7b78167aada8");
    EXPECT_THAT(sut.addMemoryBlock(&memoryBlock1).has_error(), Eq(false));
}

TEST_F(MemoryProvider_Test, AddMemoryBlockDoesNotMakeMemoryAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1462366-c357-4929-a4ee-d86e7058dd64");
    ASSERT_FALSE(sut.addMemoryBlock(&memoryBlock1).has_error());
    EXPECT_THAT(sut.isAvailable(), Eq(false));
}

TEST_F(MemoryProvider_Test, AddMemoryBlockExceedsCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "5503e89e-d927-4669-a0ec-1fa048df373e");
    MemoryBlockMock memoryBlocks[iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER + 1];

    for (uint32_t i = 0; i < iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER; ++i)
    {
        EXPECT_THAT(sut.addMemoryBlock(&memoryBlocks[i]).has_error(), Eq(false));
    }

    auto expectError = sut.addMemoryBlock(&memoryBlocks[iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER]);
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(MemoryProviderError::MEMORY_BLOCKS_EXHAUSTED));
}

TEST_F(MemoryProvider_Test, CreateWithoutMemoryBlock)
{
    ::testing::Test::RecordProperty("TEST_ID", "82f4bcac-3d44-4152-8d6a-ad72cb4ec834");
    EXPECT_CALL(sut, createMemoryMock(_, _)).Times(0);
    auto expectError = sut.create();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(MemoryProviderError::NO_MEMORY_BLOCKS_PRESENT));

    EXPECT_THAT(sut.isAvailable(), Eq(false));
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));
}

TEST_F(MemoryProvider_Test, CreateWithCommonSetupOfOneMemoryBlockIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d4a3cba-35c2-4787-b1aa-7c5325fe505c");
    auto expectSuccess = commonSetup();

    EXPECT_THAT(expectSuccess.has_error(), Eq(false));

    EXPECT_THAT(sut.isAvailable(), Eq(true));
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));
}

TEST_F(MemoryProvider_Test, CreationFailed)
{
    ::testing::Test::RecordProperty("TEST_ID", "b47cd296-8fb2-4ae7-ad80-9c962eff687f");
    MemoryProviderFailingCreation sutFailure;
    ASSERT_FALSE(sutFailure.addMemoryBlock(&memoryBlock1).has_error());
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    auto expectError = sutFailure.create();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(MemoryProviderError::MEMORY_CREATION_FAILED));

    EXPECT_THAT(sut.isAvailable(), Eq(false));
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));
}

TEST_F(MemoryProvider_Test, CreateAndAnnounceWithOneMemoryBlock)
{
    ::testing::Test::RecordProperty("TEST_ID", "a090b31e-7bb9-4461-b644-2ae0384824f6");
    ASSERT_FALSE(commonSetup().has_error());

    EXPECT_CALL(memoryBlock1, onMemoryAvailable(_)).Times(1);
    sut.announceMemoryAvailable();

    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(true));
}

TEST_F(MemoryProvider_Test, CreateAndAnnounceWithMultipleMemoryBlocks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4c13cc4-6596-4902-be7b-99b801a89cc0");
    ASSERT_FALSE(sut.addMemoryBlock(&memoryBlock1).has_error());
    ASSERT_FALSE(sut.addMemoryBlock(&memoryBlock2).has_error());
    uint64_t MEMORY_SIZE_1{16};
    uint64_t MEMORY_ALIGNMENT_1{8};
    uint64_t MEMORY_SIZE_2{32};
    uint64_t MEMORY_ALIGNMENT_2{16};
    EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE_1));
    EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT_1));
    EXPECT_CALL(memoryBlock2, size()).WillRepeatedly(Return(MEMORY_SIZE_2));
    EXPECT_CALL(memoryBlock2, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT_2));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE_1 + MEMORY_SIZE_2, std::max(MEMORY_ALIGNMENT_1, MEMORY_ALIGNMENT_2)))
        .Times(1);
    EXPECT_THAT(sut.create().has_error(), Eq(false));

    EXPECT_CALL(memoryBlock1, onMemoryAvailable(_)).Times(1);
    EXPECT_CALL(memoryBlock2, onMemoryAvailable(_)).Times(1);
    sut.announceMemoryAvailable();

    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(true));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroy());
    EXPECT_CALL(memoryBlock2, destroy());
}

TEST_F(MemoryProvider_Test, AddMemoryBlockAfterCreation)
{
    ::testing::Test::RecordProperty("TEST_ID", "04e8514a-9ea5-4027-8415-7aaf1ffc5637");
    ASSERT_FALSE(commonSetup().has_error());

    auto expectError = sut.addMemoryBlock(&memoryBlock2);
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(MemoryProviderError::MEMORY_ALREADY_CREATED));
}

TEST_F(MemoryProvider_Test, MultipleCreates)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e1c1168-da0c-4c20-b027-16b641683f30");
    ASSERT_FALSE(commonSetup().has_error());

    auto expectError = sut.create();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(MemoryProviderError::MEMORY_ALREADY_CREATED));
}

TEST_F(MemoryProvider_Test, MultipleAnnouncesAreSuppressed)
{
    ::testing::Test::RecordProperty("TEST_ID", "cfc04605-ad22-4e97-b587-0dd13db63765");
    ASSERT_FALSE(commonSetup().has_error());

    EXPECT_CALL(memoryBlock1, onMemoryAvailable(_)).Times(1);
    sut.announceMemoryAvailable();
    sut.announceMemoryAvailable(); // this shouldn't trigger a second memoryAvailable call on memoryBlock1

    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(true));
}

TEST_F(MemoryProvider_Test, MultipleDestroys)
{
    ::testing::Test::RecordProperty("TEST_ID", "61f21297-511f-4c09-b560-c6e2a93cb20e");
    ASSERT_FALSE(commonSetup().has_error());

    EXPECT_THAT(sut.destroy().has_error(), Eq(false));

    auto expectError = sut.destroy();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.error(), Eq(MemoryProviderError::MEMORY_NOT_AVAILABLE));
}

TEST_F(MemoryProvider_Test, IntialBaseAddressValueIsUnset)
{
    ::testing::Test::RecordProperty("TEST_ID", "0de67825-644e-49ea-9cbb-48cd22855260");
    EXPECT_THAT(sut.baseAddress().has_value(), Eq(false));
}

TEST_F(MemoryProvider_Test, BaseAddressValueAfterCreationIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "09e1ded2-658c-41fd-a9c1-7a257d30af2e");
    ASSERT_FALSE(commonSetup().has_error());

    auto baseAddress = sut.baseAddress();
    ASSERT_THAT(baseAddress.has_value(), Eq(true));
    EXPECT_THAT(baseAddress.value(), Eq(memoryBlock1.memory().value()));
}

TEST_F(MemoryProvider_Test, BaseAddressValueAfterDestructionIsUnset)
{
    ::testing::Test::RecordProperty("TEST_ID", "22c77eeb-5c27-4690-915e-bf9cd004ff89");
    ASSERT_FALSE(commonSetup().has_error());

    ASSERT_FALSE(sut.destroy().has_error());

    EXPECT_THAT(sut.baseAddress().has_value(), Eq(false));
}

TEST_F(MemoryProvider_Test, InitialSizeValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "4dacac9e-6630-48b6-b050-ad5477586eaf");
    EXPECT_THAT(sut.size(), Eq(0u));
}

TEST_F(MemoryProvider_Test, SizeValueAfterCreationHasExpectedValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "46d325f0-a384-497e-9ca1-991af5348a8b");
    ASSERT_FALSE(commonSetup().has_error());

    EXPECT_THAT(sut.size(), Eq(COMMON_SETUP_MEMORY_SIZE));
}

TEST_F(MemoryProvider_Test, SizeValueAfterDestructionIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "28ef9db3-310f-46ab-88b6-253a1a56eb26");
    ASSERT_FALSE(commonSetup().has_error());

    ASSERT_FALSE(sut.destroy().has_error());

    EXPECT_THAT(sut.size(), Eq(0u));
}

TEST_F(MemoryProvider_Test, InitialSegmentIdValueIsUnset)
{
    ::testing::Test::RecordProperty("TEST_ID", "237e15ad-7b32-4dc6-a447-e74092c4a411");
    EXPECT_THAT(sut.segmentId().has_value(), Eq(false));
}

TEST_F(MemoryProvider_Test, SegmentIdValueAfterCreationIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "56307b8c-724b-4bb2-8619-a127205db184");
    constexpr uint64_t DummyMemorySize{1024};
    uint8_t dummy[DummyMemorySize];
    auto segmentIdOffset = iox::UntypedRelativePointer::registerPtr(dummy, DummyMemorySize);

    ASSERT_TRUE(segmentIdOffset.has_value());
    ASSERT_FALSE(commonSetup().has_error());

    auto segmentId = sut.segmentId();
    ASSERT_THAT(segmentId.has_value(), Eq(true));
    // the segment id being monotonic increasing is an implementation detail, in the case that the implementation
    // changes, just remove this check, since we already check to get a valid result
    EXPECT_THAT(segmentId.value(), Eq(segmentIdOffset.value() + 1U));
}

TEST_F(MemoryProvider_Test, SegmentIdValueAfterDestructionIsUnset)
{
    ::testing::Test::RecordProperty("TEST_ID", "c011594c-1a56-4857-ad23-65e91c5b99fd");
    ASSERT_FALSE(commonSetup().has_error());

    ASSERT_FALSE(sut.destroy().has_error());

    EXPECT_THAT(sut.segmentId().has_value(), Eq(false));
}

TEST_F(MemoryProvider_Test, GetErrorString)
{
    ::testing::Test::RecordProperty("TEST_ID", "68b8d3b6-0d70-4aac-9c92-19f9f27a86d7");
    constexpr int32_t NUMBER_OF_TEST_CASES =
        sizeof(m_testCombinationMemoryProviderError) / sizeof(iox::roudi::MemoryProviderError);

    for (int16_t i = 0; i < NUMBER_OF_TEST_CASES; i++)
    {
        const char* result = MemoryProviderFailingCreation::getErrorString(m_testCombinationMemoryProviderError[i]);
        EXPECT_THAT(*result, Eq(*m_testResultGetErrorString[i]));
    }
}

} // namespace
