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

#include "iceoryx_posh/roudi/memory/memory_provider.hpp"

#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include "mocks/roudi_memory_block_mock.hpp"
#include "mocks/roudi_memory_provider_mock.hpp"

#include "test.hpp"

using namespace ::testing;

using namespace iox::roudi;

class MemoryProviderFailingCreation : public iox::roudi::MemoryProvider
{
  public:
    iox::cxx::expected<void*, MemoryProviderError>
    createMemory(const uint64_t size[[gnu::unused]], const uint64_t alignment[[gnu::unused]]) noexcept override
    {
        return iox::cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    iox::cxx::expected<MemoryProviderError> destroyMemory() noexcept override
    {
        return iox::cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_DESTRUCTION_FAILED);
    }
};

class MemoryProvider_Test : public Test
{
  public:
    void SetUp() override
    {
        // since the MemoryProvider registers for relative pointer, it is necessary to call unregisterAll, to have a
        // clean environment especially for the first test
        iox::RelativePointer::unregisterAll();
    }

    void TearDown() override
    {
        // unregisterAll is also called to leave a clean environment after the last test
        iox::RelativePointer::unregisterAll();
    }

    MemoryBlockMock memoryBlock1;
    MemoryBlockMock memoryBlock2;

    // the MemoryProvider is a class with a pure virtual member functions s, therefore we need an implementation to
    // instantiate and test non-virtual member functions
    MemoryProviderMock sut;
};

TEST_F(MemoryProvider_Test, InitiallyMemoryIsNotAvailable)
{
    EXPECT_THAT(sut.isAvailable(), Eq(false));
}

TEST_F(MemoryProvider_Test, InitiallyMemoryIsNotAvailableAnnounced)
{
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));
}

TEST_F(MemoryProvider_Test, AddMemoryBlock)
{
    EXPECT_THAT(sut.addMemoryBlock(&memoryBlock1).has_error(), Eq(false));
}

TEST_F(MemoryProvider_Test, AddMemoryBlockDoesNotMakeMemoryAvailable)
{
    sut.addMemoryBlock(&memoryBlock1);
    EXPECT_THAT(sut.isAvailable(), Eq(false));
}

TEST_F(MemoryProvider_Test, AddMemoryBlockExceedsCapacity)
{
    MemoryBlockMock memoryBlocks[iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER + 1];

    for (uint32_t i = 0; i < iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER; ++i)
    {
        EXPECT_THAT(sut.addMemoryBlock(&memoryBlocks[i]).has_error(), Eq(false));
    }

    auto expectError = sut.addMemoryBlock(&memoryBlocks[iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER]);
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(MemoryProviderError::MEMORY_BLOCKS_EXHAUSTED));
}

TEST_F(MemoryProvider_Test, CreateWithoutMemoryBlock)
{
    EXPECT_CALL(sut, createMemoryMock(_, _)).Times(0);
    auto expectError = sut.create();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(MemoryProviderError::NO_MEMORY_BLOCKS_PRESENT));

    EXPECT_THAT(sut.isAvailable(), Eq(false));
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));
}

TEST_F(MemoryProvider_Test, CreateWithOneMemoryBlock)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);
    EXPECT_THAT(sut.create().has_error(), Eq(false));

    EXPECT_THAT(sut.isAvailable(), Eq(true));
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, CreationFailed)
{
    MemoryProviderFailingCreation sutFailure;
    sutFailure.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    auto expectError = sutFailure.create();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(MemoryProviderError::MEMORY_CREATION_FAILED));

    EXPECT_THAT(sut.isAvailable(), Eq(false));
    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(false));
}

TEST_F(MemoryProvider_Test, CreateAndAnnounceWithOneMemoryBlock)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);
    EXPECT_THAT(sut.create().has_error(), Eq(false));

    EXPECT_CALL(memoryBlock1, memoryAvailableMock()).Times(1);
    sut.announceMemoryAvailable();

    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(true));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, CreateAndAnnounceWithMultipleMemoryBlocks)
{
    sut.addMemoryBlock(&memoryBlock1);
    sut.addMemoryBlock(&memoryBlock2);
    uint64_t MEMORY_SIZE_1{16};
    uint64_t MEMORY_ALIGNMENT_1{8};
    uint64_t MEMORY_SIZE_2{32};
    uint64_t MEMORY_ALIGNMENT_2{16};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE_1));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT_1));
    EXPECT_CALL(memoryBlock2, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE_2));
    EXPECT_CALL(memoryBlock2, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT_2));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE_1 + MEMORY_SIZE_2, std::max(MEMORY_ALIGNMENT_1, MEMORY_ALIGNMENT_2)))
        .Times(1);
    EXPECT_THAT(sut.create().has_error(), Eq(false));

    EXPECT_CALL(memoryBlock1, memoryAvailableMock()).Times(1);
    EXPECT_CALL(memoryBlock2, memoryAvailableMock()).Times(1);
    sut.announceMemoryAvailable();

    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(true));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
    EXPECT_CALL(memoryBlock2, destroyMock());
}

TEST_F(MemoryProvider_Test, AddMemoryBlockAfterCreation)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(_, _)).Times(1);
    sut.create().has_error(), Eq(false);

    auto expectError = sut.addMemoryBlock(&memoryBlock2);
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(MemoryProviderError::MEMORY_ALREADY_CREATED));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, MultipleCreates)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);

    EXPECT_THAT(sut.create().has_error(), Eq(false));
    auto expectError = sut.create();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(MemoryProviderError::MEMORY_ALREADY_CREATED));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, MultipleAnnouncesAreSuppressed)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);
    sut.create();

    EXPECT_CALL(memoryBlock1, memoryAvailableMock()).Times(1);
    sut.announceMemoryAvailable();
    sut.announceMemoryAvailable(); // this shouldn't trigger a second memoryAvailable call on memoryBlock1

    EXPECT_THAT(sut.isAvailableAnnounced(), Eq(true));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, MultipleDestroys)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);

    sut.create(), Eq(true);

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());

    EXPECT_THAT(sut.destroy().has_error(), Eq(false));

    auto expectError = sut.destroy();
    ASSERT_THAT(expectError.has_error(), Eq(true));
    EXPECT_THAT(expectError.get_error(), Eq(MemoryProviderError::MEMORY_NOT_AVAILABLE));
}

TEST_F(MemoryProvider_Test, IntialBaseAddressValueIsUnset)
{
    EXPECT_THAT(sut.baseAddress().has_value(), Eq(false));
}

TEST_F(MemoryProvider_Test, BaseAddressValueAfterCreationIsValid)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);
    sut.create();

    auto baseAddress = sut.baseAddress();
    ASSERT_THAT(baseAddress.has_value(), Eq(true));
    EXPECT_THAT(baseAddress.value(), Eq(memoryBlock1.memory().value()));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, BaseAddressValueAfterDestructionIsUnset)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);
    sut.create();

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
    sut.destroy();

    EXPECT_THAT(sut.baseAddress().has_value(), Eq(false));
}

TEST_F(MemoryProvider_Test, InitialSizeValueIsZero)
{
    EXPECT_THAT(sut.size(), Eq(0u));
}

TEST_F(MemoryProvider_Test, SizeValueAfterCreationHasExpectedValue)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);

    sut.create();

    EXPECT_THAT(sut.size(), Eq(MEMORY_SIZE));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, SizeValueAfterDestructionIsZero)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);

    sut.create();

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
    sut.destroy();

    EXPECT_THAT(sut.size(), Eq(0u));
}

TEST_F(MemoryProvider_Test, InitialSegmentIdValueIsUnset)
{
    EXPECT_THAT(sut.segmentId().has_value(), Eq(false));
}

TEST_F(MemoryProvider_Test, SegmentIdValueAfterCreationIsValid)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);

    constexpr uint64_t DummyMemorySize{1024};
    uint8_t dummy[DummyMemorySize];
    auto segmentIdOffset = iox::RelativePointer::registerPtr(dummy, DummyMemorySize);

    sut.create();

    auto segmentId = sut.segmentId();
    ASSERT_THAT(segmentId.has_value(), Eq(true));
    EXPECT_THAT(segmentId.value(), Eq(segmentIdOffset + 1));

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(MemoryProvider_Test, SegmentIdValueAfterDestructionIsUnset)
{
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));
    EXPECT_CALL(sut, createMemoryMock(MEMORY_SIZE, MEMORY_ALIGNMENT)).Times(1);
    sut.create();

    EXPECT_CALL(sut, destroyMemoryMock());
    EXPECT_CALL(memoryBlock1, destroyMock());
    sut.destroy();

    EXPECT_THAT(sut.segmentId().has_value(), Eq(false));
}
