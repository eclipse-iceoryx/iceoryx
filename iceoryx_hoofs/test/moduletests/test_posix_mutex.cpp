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
        m_deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });
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
        m_waitForThread.store(false);
    }

    void waitForThread()
    {
        while (m_waitForThread.load())
        {
        }
    }

    template <typename T>
    int64_t getDuration(const T& start, const T& end)
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }

    static constexpr int64_t WAIT_IN_MS = 100;
    static constexpr int64_t WAIT_IN_NS = WAIT_IN_MS * 1000000;

    std::atomic_bool m_waitForThread{true};
    iox::posix::mutex m_sutNonRecursive{false};
    iox::posix::mutex m_sutRecursive{true};
    iox::units::Duration m_watchdogTimeout = 5_s;
    Watchdog m_deadlockWatchdog{m_watchdogTimeout};
};

TEST_F(Mutex_test, TryLockAndUnlockWithNonRecursiveMutexReturnTrue)
{
    EXPECT_THAT(m_sutNonRecursive.try_lock(), Eq(true));
    EXPECT_THAT(m_sutNonRecursive.unlock(), Eq(true));
}


#ifndef _WIN32
TEST_F(Mutex_test, TryLockWithNonRecursiveMutexReturnsFalseAfterLock)
{
    EXPECT_THAT(m_sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(m_sutNonRecursive.try_lock(), Eq(false));
    EXPECT_THAT(m_sutNonRecursive.unlock(), Eq(true));
}
#endif

TEST_F(Mutex_test, LockAndUnlockWithNonRecursiveMutexReturnsTrue)
{
    EXPECT_THAT(m_sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(m_sutNonRecursive.unlock(), Eq(true));
}

TEST_F(Mutex_test, RepeatedLockAndUnlockWithNonRecursiveMutexReturnsTrue)
{
    EXPECT_THAT(m_sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(m_sutNonRecursive.unlock(), Eq(true));
    EXPECT_THAT(m_sutNonRecursive.lock(), Eq(true));
    EXPECT_THAT(m_sutNonRecursive.unlock(), Eq(true));
}

#if !defined(_WIN32) && !defined(QNX) && !defined(QNX__) && !defined(__QNX__) && !defined(__APPLE__)
// in qnx you can destroy a locked mutex, without error if the thread holding the lock is destructing it.
TEST_F(Mutex_test, CallingDestructorOnLockedMutexLeadsToTermination)
{
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

void tryLockReturnsFalseWhenMutexLockedInOtherThread(Mutex_test* test, iox::posix::mutex& mutex)
{
    std::atomic_bool isTryLockSuccessful{true};
    mutex.lock();

    std::thread lockThread([&] {
        isTryLockSuccessful = mutex.try_lock();
        test->signalThreadReady();
    });

    test->waitForThread();
    EXPECT_THAT(isTryLockSuccessful.load(), Eq(false));

    mutex.unlock();
    lockThread.join();
}

TEST_F(Mutex_test, TryLockReturnsFalseWhenMutexLockedInOtherThreadNonRecursiveMutex)
{
    tryLockReturnsFalseWhenMutexLockedInOtherThread(this, m_sutNonRecursive);
}

TEST_F(Mutex_test, TryLockReturnsFalseWhenMutexLockedInOtherThreadRecursiveMutex)
{
    tryLockReturnsFalseWhenMutexLockedInOtherThread(this, m_sutRecursive);
}

void lockedMutexBlocks(Mutex_test* test, iox::posix::mutex& mutex)
{
    mutex.lock();

    std::thread lockThread([&] {
        test->signalThreadReady();

        auto start = std::chrono::high_resolution_clock::now();
        mutex.lock();
        mutex.unlock();
        auto end = std::chrono::high_resolution_clock::now();
        EXPECT_THAT(test->getDuration(start, end), Gt(test->WAIT_IN_NS));
    });

    test->waitForThread();

    std::this_thread::sleep_for(std::chrono::milliseconds(test->WAIT_IN_MS));
    mutex.unlock();
    lockThread.join();
}

TEST_F(Mutex_test, LockedMutexBlocksNonRecursiveMutex)
{
    lockedMutexBlocks(this, m_sutNonRecursive);
}

TEST_F(Mutex_test, LockedMutexBlocksRecursiveMutex)
{
    lockedMutexBlocks(this, m_sutRecursive);
}


} // namespace
