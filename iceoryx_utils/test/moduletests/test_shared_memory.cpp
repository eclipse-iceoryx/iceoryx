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
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/shared_memory.hpp"
#undef private

using namespace testing;

class SharedMemory_Test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(SharedMemory_Test, CTorWithValidArguments)
{
    auto sut = iox::posix::SharedMemory::create("/ignatz",
                                                iox::posix::AccessMode::readWrite,
                                                iox::posix::OwnerShip::mine,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    EXPECT_THAT(sut.has_value(), Eq(true));
}

TEST_F(SharedMemory_Test, CTorWithInvalidMessageQueueNames)
{
    EXPECT_DEATH(
        {
            iox::posix::SharedMemory::create(nullptr,
                                             iox::posix::AccessMode::readWrite,
                                             iox::posix::OwnerShip::mine,
                                             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                             128);
        },
        ".*");

    EXPECT_DEATH(
        {
            iox::posix::SharedMemory::create("",
                                             iox::posix::AccessMode::readWrite,
                                             iox::posix::OwnerShip::mine,
                                             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                             128);
        },
        ".*");

    EXPECT_DEATH(
        {
            iox::posix::SharedMemory::create("ignatz",
                                             iox::posix::AccessMode::readWrite,
                                             iox::posix::OwnerShip::mine,
                                             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                             128);
        },
        ".*");
}

TEST_F(SharedMemory_Test, CTorWithInvalidArguments)
{
    internal::CaptureStderr();
    auto sut = iox::posix::SharedMemory::create("/schlomo",
                                                iox::posix::AccessMode::readWrite,
                                                iox::posix::OwnerShip::openExisting,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    std::string output = internal::GetCapturedStderr();
    EXPECT_THAT(output.empty(), Eq(false));
    EXPECT_THAT(sut.has_value(), Eq(false));
}

TEST_F(SharedMemory_Test, MoveCTorWithValidValues)
{
    int handle;

    iox::posix::SharedMemory sut("/ignatz",
                                 iox::posix::AccessMode::readWrite,
                                 iox::posix::OwnerShip::mine,
                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                 128);
    handle = sut.getHandle();
    {
        iox::posix::SharedMemory sut2(std::move(sut));
        EXPECT_THAT(handle, Eq(sut2.getHandle()));
        EXPECT_THAT(sut.isInitialized(), Eq(false));
    }
}

TEST_F(SharedMemory_Test, MoveCTorWithInvalidValues)
{
    internal::CaptureStderr();
    iox::posix::SharedMemory sut("/fuu11",
                                 iox::posix::AccessMode::readWrite,
                                 iox::posix::OwnerShip::openExisting,
                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                 128);
    std::string output = internal::GetCapturedStderr();
    EXPECT_THAT(output.empty(), Eq(false));
    {
        iox::posix::SharedMemory sut2(std::move(sut));
        EXPECT_THAT(sut2.isInitialized(), Eq(false));
        EXPECT_THAT(sut.isInitialized(), Eq(false));
    }
}

TEST_F(SharedMemory_Test, getHandleOfValidObject)
{
    auto sut = iox::posix::SharedMemory::create("/ignatz",
                                                iox::posix::AccessMode::readWrite,
                                                iox::posix::OwnerShip::mine,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                128);
    EXPECT_THAT(sut->getHandle(), Ne(-1));
}

TEST_F(SharedMemory_Test, getHandleOfInvalidObject)
{
    internal::CaptureStderr();
    iox::posix::SharedMemory sut("/fuu11",
                                 iox::posix::AccessMode::readWrite,
                                 iox::posix::OwnerShip::openExisting,
                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                 128);
    std::string output = internal::GetCapturedStderr();
    EXPECT_THAT(output.empty(), Eq(false));
    EXPECT_THAT(sut.getHandle(), Eq(-1));
}
