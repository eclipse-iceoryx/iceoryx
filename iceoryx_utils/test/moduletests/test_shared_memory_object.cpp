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

#include "test.hpp"
#define private public
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"
#undef private

using namespace testing;

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
    auto sut = iox::posix::SharedMemoryObject::create(
        "/validShmMem", 100, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, nullptr);
    EXPECT_THAT(sut.has_value(), Eq(true));
}

TEST_F(SharedMemoryObject_Test, CTorOpenNonExistingSharedMemoryObject)
{
    internal::CaptureStderr();
    auto sut = iox::posix::SharedMemoryObject::create(
        "/pummeluff", 100, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::openExisting, nullptr);
    std::string output = internal::GetCapturedStderr();
    EXPECT_THAT(output.empty(), Eq(false));
    EXPECT_THAT(sut.has_value(), Eq(false));
}

TEST_F(SharedMemoryObject_Test, AllocateMemoryInSharedMemory)
{
    auto sut = iox::posix::SharedMemoryObject::create(
        "/shmAllocate", 16, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, 0);
    int* test = static_cast<int*>(sut->allocate(sizeof(int), 1));
    ASSERT_THAT(test, Ne(nullptr));
    *test = 123;
    EXPECT_THAT(*test, Eq(123));
}

TEST_F(SharedMemoryObject_Test, AllocateWholeSharedMemoryWithOneChunk)
{
    auto sut = iox::posix::SharedMemoryObject::create(
        "/shmAllocate", 8, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, 0);

    void* test = sut->allocate(8, 1);
    ASSERT_THAT(test, Ne(nullptr));
}

TEST_F(SharedMemoryObject_Test, AllocateWholeSharedMemoryWithMultipleChunks)
{
    auto sut = iox::posix::SharedMemoryObject::create(
        "/shmAllocate", 8, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, 0);

    for (uint64_t i = 0; i < 8; ++i)
    {
        void* test = sut->allocate(1, 1);
        ASSERT_THAT(test, Ne(nullptr));
    }
}

TEST_F(SharedMemoryObject_Test, AllocateTooMuchMemoryInSharedMemoryWithOneChunk)
{
    auto sut = iox::posix::SharedMemoryObject::create(
        "/shmAllocate", 8, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, 0);

    std::set_terminate([]() { std::cout << "", std::abort(); });

    EXPECT_DEATH(
        {
            internal::CaptureStderr();
            sut->allocate(9, 1);
            std::string output = internal::GetCapturedStderr();
            EXPECT_THAT(output.empty(), Eq(false));
        },
        ".*");
}

TEST_F(SharedMemoryObject_Test, AllocateTooMuchSharedMemoryWithMultipleChunks)
{
    auto sut = iox::posix::SharedMemoryObject::create(
        "/shmAllocate", 8, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, 0);

    for (uint64_t i = 0; i < 8; ++i)
    {
        void* test = sut->allocate(1, 1);
        ASSERT_THAT(test, Ne(nullptr));
    }

    std::set_terminate([]() { std::cout << "", std::abort(); });
    EXPECT_DEATH(
        {
            internal::CaptureStderr();
            sut->allocate(1, 1);
            std::string output = internal::GetCapturedStderr();
            EXPECT_THAT(output.empty(), Eq(false));
        },
        ".*");
}

TEST_F(SharedMemoryObject_Test, AllocateAfterFinalizeAllocation)
{
    auto sut = iox::posix::SharedMemoryObject::create(
        "/shmAllocate", 8, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, 0);
    sut->finalizeAllocation();

    std::set_terminate([]() { std::cout << "", std::abort(); });
    EXPECT_DEATH({ sut->allocate(2, 1); }, ".*");
}
