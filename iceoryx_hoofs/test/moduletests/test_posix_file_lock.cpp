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

#if !defined(__APPLE__)
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/posix_wrapper/file_lock.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::posix;
using namespace iox::cxx;

constexpr char TEST_NAME[] = "TestProcess";
constexpr char ANOTHER_TEST_NAME[] = "AnotherTestProcess";

/// @req
/// @brief This test suite verifies the RAII behaviour of FileLock
/// @pre The file lock for TEST_NAME is acquired
/// @post The file lock for TEST_NAME is released
/// @note This should become a FЯIDA integration test once available, in order to test with two processes
class FileLock_test : public Test
{
  public:
    FileLock_test()
    {
    }

    void SetUp()
    {
        auto maybeFileLock = iox::posix::FileLock::create(TEST_NAME);
        ASSERT_FALSE(maybeFileLock.has_error());
        m_sut.emplace(std::move(maybeFileLock.value()));
        ASSERT_TRUE(m_sut.has_value());
    }

    void TearDown()
    {
        m_sut.reset();
    }

    ~FileLock_test()
    {
    }
    optional<FileLock> m_sut;
};

TEST_F(FileLock_test, EmptyNameLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "dfbcbeba-fe6a-452d-8fb0-3f4c1793c44d");
    auto sut2 = iox::posix::FileLock::create("");
    ASSERT_TRUE(sut2.has_error());
    EXPECT_THAT(sut2.get_error(), Eq(FileLockError::INVALID_FILE_NAME));
}

TEST_F(FileLock_test, MaxStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1cf3418d-51d1-4ead-9001-e0d8e61617f0");
    const FileLock::FileName_t maxString(iox::cxx::TruncateToCapacity,
                                         std::string(FileLock::FileName_t().capacity(), 'x'));
    auto sut2 = iox::posix::FileLock::create(maxString);
    ASSERT_FALSE(sut2.has_error());
}

TEST_F(FileLock_test, SecondLockWithDifferentNameWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "05f8c97a-f29d-40ca-91f4-525fc4e98683");
    auto sut2 = iox::posix::FileLock::create(ANOTHER_TEST_NAME);
    ASSERT_FALSE(sut2.has_error());
}

TEST_F(FileLock_test, LockAndReleaseWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a884cf3f-178d-4711-be9b-6e5260d0e0e7");
    {
        IOX_DISCARD_RESULT(iox::posix::FileLock::create(ANOTHER_TEST_NAME));
    }
    auto sut2 = iox::posix::FileLock::create(ANOTHER_TEST_NAME);
    ASSERT_FALSE(sut2.has_error());
}

TEST_F(FileLock_test, CreatingSameFileLockAgainFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed3af1c8-4a84-4d4f-a267-c4a80481dc42");
    auto sut2 = iox::posix::FileLock::create(TEST_NAME);
    ASSERT_TRUE(sut2.has_error());
    EXPECT_THAT(sut2.get_error(), Eq(FileLockError::LOCKED_BY_OTHER_PROCESS));
}

TEST_F(FileLock_test, MoveCtorInvalidatesRhs)
{
    ::testing::Test::RecordProperty("TEST_ID", "9eb39f39-f5e2-4d43-8945-7e0424610c3e");
    auto movedSut{std::move(m_sut.value())};
    ASSERT_FALSE(m_sut.value().isInitialized());
    ASSERT_TRUE(movedSut.isInitialized());
}

TEST_F(FileLock_test, MoveCtorTransfersLock)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ba1f8d8-3bd5-46ee-aba8-5dff7e712026");
    auto movedSut{std::move(m_sut.value())};
    auto anotherLock = iox::posix::FileLock::create(TEST_NAME);
    ASSERT_TRUE(anotherLock.has_error());
    EXPECT_THAT(anotherLock.get_error(), Eq(FileLockError::LOCKED_BY_OTHER_PROCESS));
}

TEST_F(FileLock_test, MoveAssignInvalidatesRhs)
{
    ::testing::Test::RecordProperty("TEST_ID", "705627aa-9e8e-478e-8086-38fb899a4b70");
    auto movedSut = std::move(m_sut.value());
    ASSERT_FALSE(m_sut.value().isInitialized());
    ASSERT_TRUE(movedSut.isInitialized());
}

TEST_F(FileLock_test, MoveAssignTransfersLock)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd9ee3d0-4f57-44e1-b01c-f892610e805a");
    auto movedSut = std::move(m_sut.value());
    auto anotherLock = iox::posix::FileLock::create(TEST_NAME);
    ASSERT_TRUE(anotherLock.has_error());
    EXPECT_THAT(anotherLock.get_error(), Eq(FileLockError::LOCKED_BY_OTHER_PROCESS));
}
} // namespace
#endif
