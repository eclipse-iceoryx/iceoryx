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

#include "iceoryx_utils/posix_wrapper/file_lock.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::posix;
using namespace iox::cxx;

constexpr char TEST_NAME[] = "TestProcess";
constexpr char ANOTHER_TEST_NAME[] = "AnotherTestProcess";

/// @req
/// @brief This test suite verifies the behaviour of the class FileLock
/// @pre the file lock for TEST_NAME is acquired
/// @post None
/// @note This should become a FЯIDA integration test once available, in order to test with two processes
class FileLock_test : public Test
{
  public:
    FileLock_test()
    {
    }

    void SetUp()
    {
        auto createResult = iox::posix::FileLock::create(TEST_NAME);
        ASSERT_FALSE(createResult.has_error());
        m_sut = std::move(createResult.value());
    }

    void TearDown()
    {
    }

    ~FileLock_test()
    {
    }
    iox::posix::FileLock m_sut;
};

TEST_F(FileLock_test, SecondLockWithDifferentNameWorks)
{
    auto sut2 = iox::posix::FileLock::create(ANOTHER_TEST_NAME);
    ASSERT_FALSE(sut2.has_error());
}

TEST_F(FileLock_test, LockAndReleaseWorks)
{
    {
        auto sut1 = iox::posix::FileLock::create(ANOTHER_TEST_NAME);
    }
    auto sut2 = iox::posix::FileLock::create(ANOTHER_TEST_NAME);
    ASSERT_FALSE(sut2.has_error());
}

TEST_F(FileLock_test, LockAndNoReleaseLeadsToError)
{
    auto sut2 = iox::posix::FileLock::create(TEST_NAME);
    ASSERT_TRUE(sut2.has_error());
    EXPECT_THAT(sut2.get_error(), Eq(FileLockError::LOCKED_BY_OTHER_PROCESS));
}
