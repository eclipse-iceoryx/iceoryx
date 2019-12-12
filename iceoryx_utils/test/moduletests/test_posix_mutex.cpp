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

#include "iceoryx_utils/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_utils/posix_wrapper/timer.hpp"
#include "test.hpp"

#include <thread>

using namespace ::testing;
using namespace iox::units::duration_literals;

class Mutex_test : public Test
{
  public:
    class MutexMock : public iox::posix::mutex
    {
      public:
        MutexMock(const bool isRecursive)
            : iox::posix::mutex(isRecursive)
        {
        }

        MutexMock(MutexMock&& rhs)
            : iox::posix::mutex(std::forward<iox::posix::mutex>(rhs))
        {
        }

        bool isInitialized() const noexcept
        {
            return m_isInitialized;
        }
    };

    void SetUp() override
    {
        ASSERT_THAT(sut.has_value(), Eq(true));
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

    iox::cxx::optional<iox::posix::mutex> sut = iox::posix::mutex::CreateMutex(false);
};

TEST_F(Mutex_test, TryLockWithNoLock)
{
    EXPECT_THAT(sut->try_lock(), Eq(true));
    EXPECT_THAT(sut->unlock(), Eq(true));
}

TEST_F(Mutex_test, TryLockWithLock)
{
    EXPECT_THAT(sut->lock(), Eq(true));
    EXPECT_THAT(sut->try_lock(), Eq(false));
    EXPECT_THAT(sut->unlock(), Eq(true));
}

TEST_F(Mutex_test, LockAndUnlock)
{
    EXPECT_THAT(sut->lock(), Eq(true));
    EXPECT_THAT(sut->unlock(), Eq(true));
}

TEST_F(Mutex_test, MoveConstructorInvalidatesOrigin)
{
    MutexMock mutexOrigin(true);
    MutexMock sut(std::move(mutexOrigin));

    EXPECT_THAT(mutexOrigin.isInitialized(), Eq(false));
}

// in qnx you can destroy a locked mutex, without error if the thread holding the lock is destructing it.
TEST_F(Mutex_test, DestructorFailsOnLockedMutex)
{
    std::string output = internal::GetCapturedStderr();
    std::set_terminate([]() { std::cout << "", std::abort(); });

    EXPECT_DEATH(
        {
            std::thread* t;
            {
                auto mtx = iox::posix::mutex::CreateMutex(false);
                iox::posix::Timer hold(1000_ms);
                t = new std::thread([&] {
                    mtx->lock();
                    iox::posix::Timer ct(5000_ms);
                    while (!ct.hasExpiredComparedToCreationTime()) // come back in any case!
                        ;
                });

                while (!hold.hasExpiredComparedToCreationTime())
                    ;
            }
            t->join();
            delete t;
        },
        ".*");

    internal::CaptureStderr();
}
