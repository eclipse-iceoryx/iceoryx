// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
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
        internal::CaptureStderr();
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    static void PerformDeathTest(const std::function<void()>& deathTest)
    {
        std::set_terminate([]() { std::cout << "", std::abort(); });

        internal::GetCapturedStderr();
        // @todo iox-#1613 remove EXPECT_DEATH
        // NOLINTBEGIN(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
        // hiccpp-vararg)
        EXPECT_DEATH({ deathTest(); }, ".*");
        // NOLINTEND(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
        // hiccpp-vararg)
        internal::CaptureStderr();
    }
};

TEST_F(SharedMemoryObject_Test, CTorWithValidArguments)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbda60d2-d741-407e-9a9f-f0ca74d985a8");
    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("validShmMem")
                   .memorySizeInBytes(100)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                   .create();

    EXPECT_THAT(sut.has_error(), Eq(false));
}

TEST_F(SharedMemoryObject_Test, CTorOpenNonExistingSharedMemoryObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "d80278c3-1dd8-409d-9162-f7f900892526");
    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("pummeluff")
                   .memorySizeInBytes(100)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::OPEN_EXISTING)
                   .create();

    EXPECT_THAT(sut.has_error(), Eq(true));
}

TEST_F(SharedMemoryObject_Test, AllocateMemoryInSharedMemoryAndReadIt)
{
    ::testing::Test::RecordProperty("TEST_ID", "6169ac70-a08e-4a19-80e4-57f0d5f89233");
    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(16)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                   .permissions(cxx::perms::owner_all)
                   .create();

    ASSERT_THAT(sut.has_error(), Eq(false));
    int* test = static_cast<int*>(sut->allocate(sizeof(int), 1));
    ASSERT_THAT(test, Ne(nullptr));
    *test = 123;
    EXPECT_THAT(*test, Eq(123));
}

TEST_F(SharedMemoryObject_Test, AllocateWholeSharedMemoryWithOneChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "2def907e-683d-4aaa-a969-47b5468d5383");
    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(8)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                   .permissions(cxx::perms::owner_all)
                   .create();

    ASSERT_THAT(sut.has_error(), Eq(false));
    void* test = sut->allocate(8, 1);
    ASSERT_THAT(test, Ne(nullptr));
}

TEST_F(SharedMemoryObject_Test, AllocateWholeSharedMemoryWithMultipleChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd70c0aa-fef5-49ed-875c-4bb768894ae5");
    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(8)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                   .permissions(cxx::perms::owner_all)
                   .create();

    ASSERT_THAT(sut.has_error(), Eq(false));

    for (uint64_t i = 0; i < 8; ++i)
    {
        void* test = sut->allocate(1, 1);
        ASSERT_THAT(test, Ne(nullptr));
    }
}

TEST_F(SharedMemoryObject_Test, AllocateTooMuchMemoryInSharedMemoryWithOneChunk)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b054aac-1d49-4260-afc0-908b184e0b12");
    uint64_t memorySize{8U};

    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(memorySize)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                   .permissions(cxx::perms::owner_all)
                   .create();

    ASSERT_THAT(sut.has_error(), Eq(false));

    PerformDeathTest([&] { sut->allocate(cxx::align(memorySize, posix::Allocator::MEMORY_ALIGNMENT) + 1, 1); });
}

TEST_F(SharedMemoryObject_Test, AllocateTooMuchSharedMemoryWithMultipleChunks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5bb3c7fc-0f15-4487-8479-b27d1d4a17d3");
    uint64_t memorySize{8U};
    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(memorySize)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                   .permissions(cxx::perms::owner_all)
                   .create();

    ASSERT_THAT(sut.has_error(), Eq(false));

    for (uint64_t i = 0; i < cxx::align(memorySize, posix::Allocator::MEMORY_ALIGNMENT); ++i)
    {
        void* test = sut->allocate(1, 1);
        ASSERT_THAT(test, Ne(nullptr));
    }

    PerformDeathTest([&] { sut->allocate(1, 1); });
}

TEST_F(SharedMemoryObject_Test, AllocateAfterFinalizeAllocation)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4711eaa-e811-41d4-927a-63384cdcb984");
    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("shmAllocate")
                   .memorySizeInBytes(8)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                   .permissions(cxx::perms::owner_all)
                   .create();

    ASSERT_THAT(sut.has_error(), Eq(false));
    sut->finalizeAllocation();
    PerformDeathTest([&] { sut->allocate(2, 1); });
}

TEST_F(SharedMemoryObject_Test, OpeningSharedMemoryAndReadMultipleContents)
{
    ::testing::Test::RecordProperty("TEST_ID", "14f77425-34aa-43d0-82dd-e05efd93464b");
    uint64_t memorySize = 128;

    auto shmMemory = iox::posix::SharedMemoryObjectBuilder()
                         .name("shmSut")
                         .memorySizeInBytes(memorySize)
                         .accessMode(iox::posix::AccessMode::READ_WRITE)
                         .openMode(iox::posix::OpenMode::PURGE_AND_CREATE)
                         .permissions(cxx::perms::owner_all)
                         .create();

    ASSERT_THAT(shmMemory.has_error(), Eq(false));

    int* test = static_cast<int*>(shmMemory->allocate(sizeof(int), 1));
    *test = 4557;
    int* test2 = static_cast<int*>(shmMemory->allocate(sizeof(int), 1));
    *test2 = 8912;


    auto sut = iox::posix::SharedMemoryObjectBuilder()
                   .name("shmSut")
                   .memorySizeInBytes(memorySize)
                   .accessMode(iox::posix::AccessMode::READ_WRITE)
                   .openMode(iox::posix::OpenMode::OPEN_EXISTING)
                   .permissions(cxx::perms::owner_all)
                   .create();

    int* sutValue1 = static_cast<int*>(sut->allocate(sizeof(int), 1));
    int* sutValue2 = static_cast<int*>(sut->allocate(sizeof(int), 1));

    EXPECT_THAT(*sutValue1, Eq(4557));
    EXPECT_THAT(*sutValue2, Eq(8912));
}
} // namespace
