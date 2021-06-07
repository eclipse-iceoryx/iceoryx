// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#include "iceoryx_hoofs/platform/stat.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <fcntl.h>

namespace
{
using namespace testing;
using namespace iox;
using namespace iox::posix;

class SharedMemory_Test : public Test
{
  public:
    void SetUp() override
    {
        testing::internal::CaptureStderr();
    }

    void TearDown() override
    {
        std::string output = testing::internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    static constexpr const char SUT_SHM_NAME[] = "/ignatz";

    iox::cxx::expected<iox::posix::SharedMemory, iox::posix::SharedMemoryError>
    createSharedMemory(const iox::posix::Policy policy)
    {
        return iox::posix::SharedMemory::create(SUT_SHM_NAME,
                                                iox::posix::AccessMode::READ_WRITE,
                                                policy,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    }
};

constexpr const char SharedMemory_Test::SUT_SHM_NAME[];

TEST_F(SharedMemory_Test, CTorWithValidArguments)
{
    auto sut = iox::posix::SharedMemory::create("/ignatz",
                                                iox::posix::AccessMode::READ_WRITE,
                                                iox::posix::Policy::PURGE_AND_CREATE,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    EXPECT_THAT(sut.has_error(), Eq(false));
}

TEST_F(SharedMemory_Test, CTorWithInvalidMessageQueueNames)
{
    EXPECT_THAT(iox::posix::SharedMemory::create("",
                                                 iox::posix::AccessMode::READ_WRITE,
                                                 iox::posix::Policy::PURGE_AND_CREATE,
                                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                 128)
                    .has_error(),
                Eq(true));

    EXPECT_THAT(iox::posix::SharedMemory::create("ignatz",
                                                 iox::posix::AccessMode::READ_WRITE,
                                                 iox::posix::Policy::PURGE_AND_CREATE,
                                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                 128)
                    .has_error(),
                Eq(true));
}

TEST_F(SharedMemory_Test, CTorWithInvalidArguments)
{
    auto sut = iox::posix::SharedMemory::create("/schlomo",
                                                iox::posix::AccessMode::READ_WRITE,
                                                iox::posix::Policy::OPEN,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    EXPECT_THAT(sut.has_error(), Eq(true));
}

TEST_F(SharedMemory_Test, MoveCTorWithValidValues)
{
    int handle;

    auto sut = iox::posix::SharedMemory::create(SUT_SHM_NAME,
                                                iox::posix::AccessMode::READ_WRITE,
                                                iox::posix::Policy::PURGE_AND_CREATE,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    ASSERT_FALSE(sut.has_error());
    handle = sut->getHandle();
    {
        iox::posix::SharedMemory sut2(std::move(*sut));
        EXPECT_THAT(handle, Eq(sut2.getHandle()));
        EXPECT_THAT(sut->isInitialized(), Eq(false));
    }
}

TEST_F(SharedMemory_Test, getHandleOfValidObject)
{
    auto sut = iox::posix::SharedMemory::create(SUT_SHM_NAME,
                                                iox::posix::AccessMode::READ_WRITE,
                                                iox::posix::Policy::PURGE_AND_CREATE,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    ASSERT_FALSE(sut.has_error());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

TEST_F(SharedMemory_Test, UnlinkNonExistingShmFails)
{
    EXPECT_FALSE(iox::posix::SharedMemory::unlinkIfExist("/look_over_there_a_dead_pidgin"));
}

TEST_F(SharedMemory_Test, UnlinkExistingShmWorks)
{
    constexpr const char SHM_NAME[] = "/its_a_mee_monukulius";
    ASSERT_FALSE(iox::posix::posixCall(iox_shm_open)(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
                     .failureReturnValue(SharedMemory::INVALID_HANDLE)
                     .evaluate()
                     .has_error());
    EXPECT_TRUE(iox::posix::SharedMemory::unlinkIfExist(SHM_NAME));
}

TEST_F(SharedMemory_Test, ExclusiveCreateWorksWhenShmDoesNotExist)
{
    auto sut = createSharedMemory(Policy::EXCLUSIVE_CREATE);
    ASSERT_FALSE(sut.has_error());
    EXPECT_TRUE(sut->hasOwnership());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

TEST_F(SharedMemory_Test, ExclusiveCreateFailsWhenShmExists)
{
    ASSERT_FALSE(
        iox::posix::posixCall(iox_shm_open)(SUT_SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
            .failureReturnValue(SharedMemory::INVALID_HANDLE)
            .evaluate()
            .has_error());
    auto sut = createSharedMemory(Policy::EXCLUSIVE_CREATE);
    ASSERT_TRUE(sut.has_error());
    iox::posix::SharedMemory::unlinkIfExist(SUT_SHM_NAME);
}

TEST_F(SharedMemory_Test, PurgeAndCreateWorksWhenShmDoesNotExist)
{
    auto sut = createSharedMemory(Policy::PURGE_AND_CREATE);
    ASSERT_FALSE(sut.has_error());
    EXPECT_TRUE(sut->hasOwnership());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

TEST_F(SharedMemory_Test, PurgeAndCreateWorksWhenShmExists)
{
    ASSERT_FALSE(
        iox::posix::posixCall(iox_shm_open)(SUT_SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
            .failureReturnValue(SharedMemory::INVALID_HANDLE)
            .evaluate()
            .has_error());
    auto sut = createSharedMemory(Policy::PURGE_AND_CREATE);
    ASSERT_FALSE(sut.has_error());
    EXPECT_TRUE(sut->hasOwnership());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

TEST_F(SharedMemory_Test, CreateOrOpenCreatesShmWhenShmDoesNotExist)
{
    auto sut = createSharedMemory(Policy::CREATE_OR_OPEN);
    ASSERT_FALSE(sut.has_error());
    EXPECT_TRUE(sut->hasOwnership());
    EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
}

TEST_F(SharedMemory_Test, CreateOrOpenOpensShmWhenShmDoesExist)
{
    ASSERT_FALSE(
        iox::posix::posixCall(iox_shm_open)(SUT_SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
            .failureReturnValue(SharedMemory::INVALID_HANDLE)
            .evaluate()
            .has_error());
    {
        auto sut = createSharedMemory(Policy::CREATE_OR_OPEN);
        ASSERT_FALSE(sut.has_error());
        EXPECT_FALSE(sut->hasOwnership());
        EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
    }
    EXPECT_TRUE(iox::posix::SharedMemory::unlinkIfExist(SUT_SHM_NAME));
}

TEST_F(SharedMemory_Test, OpenWorksWhenShmExist)
{
    ASSERT_FALSE(
        iox::posix::posixCall(iox_shm_open)(SUT_SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
            .failureReturnValue(SharedMemory::INVALID_HANDLE)
            .evaluate()
            .has_error());
    {
        auto sut = createSharedMemory(Policy::OPEN);
        ASSERT_FALSE(sut.has_error());
        EXPECT_FALSE(sut->hasOwnership());
        EXPECT_THAT(sut->getHandle(), Ne(SharedMemory::INVALID_HANDLE));
    }
    EXPECT_TRUE(iox::posix::SharedMemory::unlinkIfExist(SUT_SHM_NAME));
}

TEST_F(SharedMemory_Test, OpenFailsWhenShmDoesNotExist)
{
    auto sut = createSharedMemory(Policy::OPEN);
    ASSERT_TRUE(sut.has_error());
}


} // namespace
