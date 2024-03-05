// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"

#include "iox/detail/system_configuration.hpp"

#include "mocks/roudi_memory_block_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox;
using namespace iox::roudi;

using iox::ShmName_t;
static const ShmName_t TEST_SHM_NAME = ShmName_t("FuManchu");


class PosixShmMemoryProvider_Test : public Test
{
  public:
    void SetUp() override
    {
        /// @note just in the case a test left something behind we remove the shared memory if it exists
        IOX_DISCARD_RESULT(iox::detail::PosixSharedMemory::unlinkIfExist(resourceName(TEST_SHM_NAME)));
    }

    void TearDown() override
    {
    }

    iox::detail::PosixSharedMemory::Name_t resourceName(ShmName_t name)
    {
        return concatenate(iceoryxResourcePrefix(DEFAULT_DOMAIN_ID, ResourceType::ICEORYX_DEFINED), name);
    }

    bool shmExists()
    {
        return !iox::PosixSharedMemoryObjectBuilder()
                    .name(resourceName(TEST_SHM_NAME))
                    .memorySizeInBytes(8)
                    .accessMode(iox::AccessMode::READ_ONLY)
                    .openMode(iox::OpenMode::OPEN_EXISTING)
                    .permissions(iox::perms::owner_all)
                    .create()
                    .has_error();
    }

    MemoryBlockMock memoryBlock1;
    MemoryBlockMock memoryBlock2;
};

TEST_F(PosixShmMemoryProvider_Test, CreateMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "9808f2a5-4cd3-49fe-9a19-6e747183141d");
    PosixShmMemoryProvider sut(
        TEST_SHM_NAME, DEFAULT_DOMAIN_ID, iox::AccessMode::READ_WRITE, iox::OpenMode::PURGE_AND_CREATE);
    ASSERT_FALSE(sut.addMemoryBlock(&memoryBlock1).has_error());
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    EXPECT_THAT(sut.create().has_error(), Eq(false));

    EXPECT_THAT(shmExists(), Eq(true));

    EXPECT_CALL(memoryBlock1, destroy());
}

TEST_F(PosixShmMemoryProvider_Test, DestroyMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "f864b99c-373d-4954-ac8b-61acc3c9c555");
    PosixShmMemoryProvider sut(
        TEST_SHM_NAME, DEFAULT_DOMAIN_ID, iox::AccessMode::READ_WRITE, iox::OpenMode::PURGE_AND_CREATE);
    ASSERT_FALSE(sut.addMemoryBlock(&memoryBlock1).has_error());
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{8};
    EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    ASSERT_FALSE(sut.create().has_error());

    EXPECT_CALL(memoryBlock1, destroy());

    ASSERT_FALSE(sut.destroy().has_error());

    EXPECT_THAT(shmExists(), Eq(false));
}

TEST_F(PosixShmMemoryProvider_Test, CreationFailedWithAlignmentExceedingPageSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "6614de7e-0f4c-48ea-bd3c-dd500fa231f2");
    PosixShmMemoryProvider sut(
        TEST_SHM_NAME, DEFAULT_DOMAIN_ID, iox::AccessMode::READ_WRITE, iox::OpenMode::PURGE_AND_CREATE);
    ASSERT_FALSE(sut.addMemoryBlock(&memoryBlock1).has_error());
    uint64_t MEMORY_SIZE{16};
    uint64_t MEMORY_ALIGNMENT{iox::detail::pageSize() + 8U};
    EXPECT_CALL(memoryBlock1, size()).WillRepeatedly(Return(MEMORY_SIZE));
    EXPECT_CALL(memoryBlock1, alignment()).WillRepeatedly(Return(MEMORY_ALIGNMENT));

    auto expectFailed = sut.create();
    ASSERT_THAT(expectFailed.has_error(), Eq(true));
    ASSERT_THAT(expectFailed.error(), Eq(MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE));

    EXPECT_THAT(shmExists(), Eq(false));
}

} // namespace
