// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/memory.hpp"
#include "iox/posix_group.hpp"
#include "iox/posix_shared_memory_object.hpp"
#include "iox/posix_user.hpp"
#include "test.hpp"

namespace
{
using namespace testing;
using namespace iox;

class SharedMemoryObject_Test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(SharedMemoryObject_Test, CTorWithValidArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbda60d2-d741-407e-9a9f-f0ca74d985a8");
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("validShmMem")
                   .memorySizeInBytes(100)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .create();

    EXPECT_THAT(sut.has_error(), Eq(false));
}

TEST_F(SharedMemoryObject_Test, CTorOpenNonExistingSharedMemoryObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "d80278c3-1dd8-409d-9162-f7f900892526");
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("pummeluff")
                   .memorySizeInBytes(100)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::OPEN_EXISTING)
                   .create();

    EXPECT_THAT(sut.has_error(), Eq(true));
}

TEST_F(SharedMemoryObject_Test, AllocateMemoryInSharedMemoryAndReadIt)
{
    ::testing::Test::RecordProperty("TEST_ID", "6169ac70-a08e-4a19-80e4-57f0d5f89233");
    const uint64_t MEMORY_SIZE = 16;
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(MEMORY_SIZE)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .permissions(perms::owner_all)
                   .create()
                   .expect("failed to create sut");

    auto* data_ptr = static_cast<uint8_t*>(sut.getBaseAddress());

    for (uint64_t i = 0; i < MEMORY_SIZE; ++i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        data_ptr[i] = static_cast<uint8_t>(i * 2 + 1);
    }

    auto sut2 = PosixSharedMemoryObjectBuilder()
                    .name("shmAllocate")
                    .memorySizeInBytes(MEMORY_SIZE)
                    .openMode(iox::OpenMode::OPEN_EXISTING)
                    .create()
                    .expect("failed to create sut");

    auto* data_ptr2 = static_cast<uint8_t*>(sut2.getBaseAddress());

    for (uint64_t i = 0; i < MEMORY_SIZE; ++i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        EXPECT_THAT(data_ptr2[i], Eq(static_cast<uint8_t>(i) * 2 + 1));
    }
}

TEST_F(SharedMemoryObject_Test, OpenFailsWhenActualMemorySizeIsSmallerThanRequestedSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb58b45e-8366-42ae-bd30-8d7415791dd4");
    const uint64_t MEMORY_SIZE = 16U << 20U; // 16 MB
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(1)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .permissions(perms::owner_all)
                   .create()
                   .expect("failed to create sut");

    auto sut2 = PosixSharedMemoryObjectBuilder()
                    .name("shmAllocate")
                    .memorySizeInBytes(MEMORY_SIZE)
                    .openMode(iox::OpenMode::OPEN_EXISTING)
                    .create();

    ASSERT_TRUE(sut2.has_error());
    EXPECT_THAT(sut2.error(), Eq(PosixSharedMemoryObjectError::REQUESTED_SIZE_EXCEEDS_ACTUAL_SIZE));
}

TEST_F(SharedMemoryObject_Test, OpenSutMapsAllMemoryIntoProcess)
{
    ::testing::Test::RecordProperty("TEST_ID", "0c8b41eb-74fd-4796-9e5e-fe6707f3c46c");
    const uint64_t MEMORY_SIZE = 1024;
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(MEMORY_SIZE * sizeof(uint64_t))
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .permissions(perms::owner_all)
                   .create()
                   .expect("failed to create sut");

    auto* data_ptr = static_cast<uint64_t*>(sut.getBaseAddress());

    for (uint64_t i = 0; i < MEMORY_SIZE; ++i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        data_ptr[i] = i * 2 + 1;
    }

    auto sut2 = PosixSharedMemoryObjectBuilder()
                    .name("shmAllocate")
                    .memorySizeInBytes(1)
                    .openMode(iox::OpenMode::OPEN_EXISTING)
                    .create()
                    .expect("failed to create sut");

    ASSERT_THAT(*sut2.get_size(), Ge(MEMORY_SIZE * sizeof(uint64_t)));

    auto* data_ptr2 = static_cast<uint64_t*>(sut2.getBaseAddress());

    for (uint64_t i = 0; i < MEMORY_SIZE; ++i)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        EXPECT_THAT(data_ptr2[i], Eq(i * 2 + 1));
    }
}

#if !defined(_WIN32) && !defined(__APPLE__)
TEST_F(SharedMemoryObject_Test, AcquiringOwnerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a9859b5e-555b-4cff-b418-74168a9fd85a");
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(8)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .permissions(perms::owner_all)
                   .create();

    auto owner = sut->get_ownership();
    ASSERT_FALSE(owner.has_error());

    EXPECT_THAT(owner->uid(), PosixUser::getUserOfCurrentProcess().getID());
    EXPECT_THAT(owner->gid(), PosixGroup::getGroupOfCurrentProcess().getID());
}

TEST_F(SharedMemoryObject_Test, AcquiringPermissionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b36bc3b-16a0-4c18-a1cb-6815812c6616");
    const auto permissions = perms::owner_all | perms::group_write | perms::group_read | perms::others_exec;
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(8)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .permissions(permissions)
                   .create();

    auto sut_perm = sut->get_permissions();
    ASSERT_FALSE(sut_perm.has_error());
    EXPECT_THAT(*sut_perm, Eq(permissions));
}

TEST_F(SharedMemoryObject_Test, SettingOwnerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "da85be28-7e21-4207-9077-698a2ec188d6");
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(8)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .permissions(perms::owner_all)
                   .create();

    auto owner = sut->get_ownership();
    ASSERT_FALSE(owner.has_error());

    // It is a slight stub test since we must be root to change the owner of a
    // file. But changing the owner from self to self is legal.
    ASSERT_FALSE(sut->set_ownership(*owner).has_error());
}

TEST_F(SharedMemoryObject_Test, SettingPermissionsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "412abc8a-d1f8-4ceb-86db-f2790d2da58f");
    auto sut = PosixSharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(8)
                   .accessMode(iox::AccessMode::READ_WRITE)
                   .openMode(iox::OpenMode::PURGE_AND_CREATE)
                   .permissions(perms::owner_all)
                   .create();

    ASSERT_FALSE(sut->set_permissions(perms::none).has_error());
    auto result = sut->get_permissions();
    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, Eq(perms::none));
}
#endif


} // namespace
