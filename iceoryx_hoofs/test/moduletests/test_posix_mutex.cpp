// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by ApexAI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/deadline_timer.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_hoofs/testing/test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"

#include <atomic>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox::units::duration_literals;

class Mutex_test : public Test
{
  public:
    void SetUp() override
    {
        internal::CaptureStderr();
        deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    void signalThreadReady()
    {
        doWaitForThread.store(false, std::memory_order_relaxed);
    }

    void waitForThread()
    {
        while (doWaitForThread.load(std::memory_order_relaxed))
        {
            std::this_thread::yield();
        }
    }

    template <typename T>
    int64_t getDuration(const T& start, const T& end)
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }

    std::atomic_bool doWaitForThread{true};
    iox::posix::mutex sutNonRecursive{false};
    iox::posix::mutex sutRecursive{true};
    iox::units::Duration watchdogTimeout = 5_s;
    Watchdog deadlockWatchdog{watchdogTimeout};
};

TEST_F(Mutex_test, TryLockAndUnlockWithNonRecursiveMutexReturnTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ed2c3f1-6c91-465e-a702-9ea25b5434bb");
    EXPECT_THAT(sutNonRecursive.try_lock(), Eq(true));
    EXPECT_THAT(sutNonRecursive.unlock(), Eq(true));
}


#ifndef _WIN32
TEST_F(Mutex_test, TryLockWithNonRecursiveMutexReturnsFalseAfterLock)
{
    ::testing::Test::RecordProperty("TEST_ID", "910b16e1-53ea-46c6-ad9a-9dcaa0bf7821");
    EXPECT_THAT(sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(sutNonRecursive.try_lock(), Eq(false));
    EXPECT_THAT(sutNonRecursive.unlock(), Eq(true));
}
#endif

TEST_F(Mutex_test, LockAndUnlockWithNonRecursiveMutexReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "b83e4491-50cc-40ca-a6d0-5ad8baf346b9");
    EXPECT_THAT(sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(sutNonRecursive.unlock(), Eq(true));
}

TEST_F(Mutex_test, RepeatedLockAndUnlockWithNonRecursiveMutexReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c01c8cc-8cb2-4869-8ff3-c52e385a2289");
    EXPECT_THAT(sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(sutNonRecursive.unlock(), Eq(true));
    EXPECT_THAT(sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(sutNonRecursive.unlock(), Eq(true));
}

#if !defined(_WIN32) && !defined(QNX) && !defined(QNX__) && !defined(__QNX__) && !defined(__APPLE__)
// in qnx you can destroy a locked mutex, without error if the thread holding the lock is destructing it.
TEST_F(Mutex_test, CallingDestructorOnLockedMutexLeadsToTermination)
{
    ::testing::Test::RecordProperty("TEST_ID", "4bb77e54-4d1e-4d1e-9138-a284638aab8c");
    std::string output = internal::GetCapturedStderr();
    std::set_terminate([]() { std::cout << "", std::abort(); });

    EXPECT_DEATH(
        {
            iox::posix::mutex mtx{false};
            mtx.lock();
        },
        ".*");

    internal::CaptureStderr();
}
#endif

void tryLockReturnsFalseWhenMutexLockedInOtherThread(iox::posix::mutex& mutex)
{
    std::atomic_bool isTryLockSuccessful{true};
    mutex.lock();

    std::thread lockThread([&] { isTryLockSuccessful = mutex.try_lock(); });

    lockThread.join();
    EXPECT_THAT(isTryLockSuccessful.load(), Eq(false));

    mutex.unlock();
}

TEST_F(Mutex_test, TryLockReturnsFalseWhenMutexLockedInOtherThreadNonRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bf2397b-e068-4883-870d-050d7338663f");
    tryLockReturnsFalseWhenMutexLockedInOtherThread(sutNonRecursive);
}

TEST_F(Mutex_test, TryLockReturnsFalseWhenMutexLockedInOtherThreadRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "88f89346-dc69-491e-ad16-081dc29022b7");
    tryLockReturnsFalseWhenMutexLockedInOtherThread(sutRecursive);
}

void lockedMutexBlocks(Mutex_test* test, iox::posix::mutex& mutex)
{
    const std::chrono::milliseconds WAIT_IN_MS(100);

    mutex.lock();

    std::thread lockThread([&] {
        test->signalThreadReady();

        auto start = std::chrono::steady_clock::now();
        EXPECT_TRUE(mutex.lock());
        EXPECT_TRUE(mutex.unlock());
        auto end = std::chrono::steady_clock::now();
        EXPECT_THAT(end - start, Gt(WAIT_IN_MS));
    });

    test->waitForThread();

    std::this_thread::sleep_for(WAIT_IN_MS);
    mutex.unlock();
    lockThread.join();
}

TEST_F(Mutex_test, LockedMutexBlocksNonRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "de50bda2-c94e-413b-ab32-b255a04a8d8a");
    lockedMutexBlocks(this, sutNonRecursive);
}

TEST_F(Mutex_test, LockedMutexBlocksRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "59d4e6e0-d3c7-4d11-a131-01a2637883eb");
    lockedMutexBlocks(this, sutRecursive);
}


} // namespace
