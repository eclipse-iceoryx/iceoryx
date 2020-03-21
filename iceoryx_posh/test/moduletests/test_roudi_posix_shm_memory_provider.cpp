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

#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"

#include "iceoryx_utils/internal/posix_wrapper/system_configuration.hpp"

#include "mocks/roudi_memory_block_mock.hpp"

#include "test.hpp"

using namespace ::testing;

using namespace iox::roudi;

static const ShmNameString SHM_NAME = ShmNameString("/FuManchu");

class PosixShmMemoryProvider_Test : public Test
{
  public:
    void SetUp() override
    {
        /// @note just is case a test left something behind, cleanup the shm by creating a new one with "mine" ownership
        iox::posix::SharedMemoryObject::create(
            SHM_NAME.c_str(), 1024, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, nullptr);
    }

    void TearDown() override
    {
    }

    bool shmExists()
    {
        return iox::posix::SharedMemoryObject::create(
                   SHM_NAME.c_str(), 8, iox::posix::AccessMode::readOnly, iox::posix::OwnerShip::openExisting, nullptr)
            .has_value();
    }

    MemoryBlockMock memoryBlock1;
    MemoryBlockMock memoryBlock2;
};

TEST_F(PosixShmMemoryProvider_Test, CreateMemory)
{
    PosixShmMemoryProvider sut(SHM_NAME, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine);
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    EXPECT_THAT(sut.create().has_error(), Eq(false));

    EXPECT_THAT(shmExists(), Eq(true));

    EXPECT_CALL(memoryBlock1, destroyMock());
}

TEST_F(PosixShmMemoryProvider_Test, DestroyMemory)
{
    PosixShmMemoryProvider sut(SHM_NAME, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine);
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    sut.create();

    EXPECT_CALL(memoryBlock1, destroyMock());

    sut.destroy();

    EXPECT_THAT(shmExists(), Eq(false));
}

TEST_F(PosixShmMemoryProvider_Test, CreationFailedWithAlignmentExceedingPageSize)
{
    PosixShmMemoryProvider sut(SHM_NAME, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine);
    sut.addMemoryBlock(&memoryBlock1);
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{iox::posix::pageSize().value_or(iox::posix::MaxPageSize) + 8};
    EXPECT_CALL(memoryBlock1, sizeMock()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignmentMock()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    auto expectFailed = sut.create();
    ASSERT_THAT(expectFailed.has_error(), Eq(true));
    ASSERT_THAT(expectFailed.get_error(), Eq(MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE));

    EXPECT_THAT(shmExists(), Eq(false));
}
