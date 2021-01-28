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

#include "iceoryx_utils/cxx/deadline_timer.hpp"
#include "iceoryx_utils/internal/posix_wrapper/mutex.hpp"
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
    };

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

    iox::posix::mutex sut{false};
};

TEST_F(Mutex_test, TryLockWithNoLock)
{
    EXPECT_THAT(sut.try_lock(), Eq(true));
    EXPECT_THAT(sut.unlock(), Eq(true));
}

TEST_F(Mutex_test, TryLockWithLock)
{
    EXPECT_THAT(sut.lock(), Eq(true));
    EXPECT_THAT(sut.try_lock(), Eq(false));
    EXPECT_THAT(sut.unlock(), Eq(true));
}

TEST_F(Mutex_test, LockAndUnlock)
{
    EXPECT_THAT(sut.lock(), Eq(true));
    EXPECT_THAT(sut.unlock(), Eq(true));
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
                iox::posix::mutex mtx{false};
                iox::cxx::DeadlineTimer mutexTimer(1000_ms);
                t = new std::thread([&] {
                    mtx.lock();
                    iox::cxx::DeadlineTimer ct(5000_ms);
                    while (!ct.hasExpired()) // come back in any case!
                        ;
                });

                while (!mutexTimer.hasExpired())
                    ;
            }
            t->join();
            delete t;
        },
        ".*");

    internal::CaptureStderr();
}
