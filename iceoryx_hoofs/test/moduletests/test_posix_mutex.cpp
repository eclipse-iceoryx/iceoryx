// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by ApexAI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iox/atomic.hpp"
#include "iox/deadline_timer.hpp"
#include "iox/mutex.hpp"

#include <thread>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::units::duration_literals;

class Mutex_test : public Test
{
  public:
    void SetUp() override
    {
        deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });

        ASSERT_FALSE(MutexBuilder().mutexType(MutexType::RECURSIVE).create(sutRecursive).has_error());
        ASSERT_FALSE(MutexBuilder().mutexType(MutexType::NORMAL).create(sutNonRecursive).has_error());
    }

    void TearDown() override
    {
    }

    void signalThreadReady()
    {
        doWaitForThread.store(false, std::memory_order_relaxed);
    }

    void waitForThread() const
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

    iox::concurrent::Atomic<bool> doWaitForThread{true};
    iox::optional<mutex> sutNonRecursive;
    iox::optional<mutex> sutRecursive;
    iox::units::Duration watchdogTimeout = 5_s;
    Watchdog deadlockWatchdog{watchdogTimeout};
};

TEST_F(Mutex_test, TryLockAndUnlockWithNonRecursiveMutexWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ed2c3f1-6c91-465e-a702-9ea25b5434bb");
    auto tryLockResult = sutNonRecursive->try_lock();
    ASSERT_FALSE(tryLockResult.has_error());
    EXPECT_THAT(*tryLockResult, Eq(MutexTryLock::LOCK_SUCCEEDED));
    EXPECT_FALSE(sutNonRecursive->unlock().has_error());
}

#ifndef _WIN32
TEST_F(Mutex_test, TryLockWithNonRecursiveMutexReturnsFailsWhenLocked)
{
    ::testing::Test::RecordProperty("TEST_ID", "910b16e1-53ea-46c6-ad9a-9dcaa0bf7821");
    EXPECT_FALSE(sutNonRecursive->lock().has_error());
    auto tryLockResult = sutNonRecursive->try_lock();
    ASSERT_FALSE(tryLockResult.has_error());
    EXPECT_THAT(*tryLockResult, Eq(MutexTryLock::FAILED_TO_ACQUIRE_LOCK));
    EXPECT_FALSE(sutNonRecursive->unlock().has_error());
}
#endif

TEST_F(Mutex_test, LockAndUnlockWithNonRecursiveMutexWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b83e4491-50cc-40ca-a6d0-5ad8baf346b9");
    EXPECT_FALSE(sutNonRecursive->lock().has_error());
    EXPECT_FALSE(sutNonRecursive->unlock().has_error());
}

TEST_F(Mutex_test, RepeatedLockAndUnlockWithNonRecursiveMutexWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c01c8cc-8cb2-4869-8ff3-c52e385a2289");
    EXPECT_FALSE(sutNonRecursive->lock().has_error());
    EXPECT_FALSE(sutNonRecursive->unlock().has_error());
    EXPECT_FALSE(sutNonRecursive->lock().has_error());
    EXPECT_FALSE(sutNonRecursive->unlock().has_error());
}

void tryLockReturnsFalseWhenMutexLockedInOtherThread(mutex& mutex)
{
    iox::concurrent::Atomic<MutexTryLock> tryLockState{MutexTryLock::LOCK_SUCCEEDED};
    ASSERT_FALSE(mutex.lock().has_error());
    std::thread lockThread([&] {
        auto tryLockResult = mutex.try_lock();
        ASSERT_FALSE(tryLockResult.has_error());
        tryLockState = *tryLockResult;
    });

    lockThread.join();
    EXPECT_THAT(tryLockState.load(), Eq(MutexTryLock::FAILED_TO_ACQUIRE_LOCK));

    ASSERT_FALSE(mutex.unlock().has_error());
}

TEST_F(Mutex_test, TryLockReturnsFalseWhenMutexLockedInOtherThreadNonRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bf2397b-e068-4883-870d-050d7338663f");
    tryLockReturnsFalseWhenMutexLockedInOtherThread(*sutNonRecursive);
}

TEST_F(Mutex_test, TryLockReturnsFalseWhenMutexLockedInOtherThreadRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "88f89346-dc69-491e-ad16-081dc29022b7");
    tryLockReturnsFalseWhenMutexLockedInOtherThread(*sutRecursive);
}

void lockedMutexBlocks(Mutex_test* test, mutex& mutex)
{
    const std::chrono::milliseconds WAIT_IN_MS(100);
    std::chrono::milliseconds blockingDuration{0};

    ASSERT_FALSE(mutex.lock().has_error());

    std::thread lockThread([&] {
        auto start = std::chrono::steady_clock::now();
        test->signalThreadReady();

        EXPECT_FALSE(mutex.lock().has_error());
        EXPECT_FALSE(mutex.unlock().has_error());
        auto end = std::chrono::steady_clock::now();
        blockingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    });

    test->waitForThread();

    auto start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(WAIT_IN_MS);
    auto end = std::chrono::steady_clock::now();
    auto realWaitDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    ASSERT_FALSE(mutex.unlock().has_error());
    lockThread.join();

    EXPECT_THAT(blockingDuration.count(), Ge(realWaitDuration.count()));
}

TEST_F(Mutex_test, LockedMutexBlocksNonRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "de50bda2-c94e-413b-ab32-b255a04a8d8a");
    lockedMutexBlocks(this, *sutNonRecursive);
}

TEST_F(Mutex_test, LockedMutexBlocksRecursiveMutex)
{
    ::testing::Test::RecordProperty("TEST_ID", "59d4e6e0-d3c7-4d11-a131-01a2637883eb");
    lockedMutexBlocks(this, *sutRecursive);
}

#ifndef _WIN32
TEST_F(Mutex_test, MutexWithDeadlockDetectionsFailsOnDeadlock)
{
    ::testing::Test::RecordProperty("TEST_ID", "feb07935-674d-4ebc-abaa-66664751719a");
    iox::optional<mutex> sut;
    ASSERT_FALSE(MutexBuilder().mutexType(MutexType::WITH_DEADLOCK_DETECTION).create(sut).has_error());
    EXPECT_FALSE(sut->lock().has_error());
    auto result = sut->lock();
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(MutexLockError::DEADLOCK_CONDITION));

    EXPECT_FALSE(sut->unlock().has_error());
}
#endif

TEST_F(Mutex_test, MutexWithDeadlockDetectionsFailsWhenSameThreadTriesToUnlockItTwice)
{
    ::testing::Test::RecordProperty("TEST_ID", "062e411e-a5d3-4759-9faf-db6f4129d395");
    iox::optional<mutex> sut;
    ASSERT_FALSE(MutexBuilder().mutexType(MutexType::WITH_DEADLOCK_DETECTION).create(sut).has_error());
    EXPECT_FALSE(sut->lock().has_error());
    EXPECT_FALSE(sut->unlock().has_error());

    auto result = sut->unlock();
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(MutexUnlockError::NOT_OWNED_BY_THREAD));
}

TEST_F(Mutex_test, MutexWithDeadlockDetectionsFailsWhenAnotherThreadTriesToUnlock)
{
    ::testing::Test::RecordProperty("TEST_ID", "4dcea981-2259-48c6-bf27-7839ad9013b4");
    iox::optional<mutex> sut;
    ASSERT_FALSE(MutexBuilder().mutexType(MutexType::WITH_DEADLOCK_DETECTION).create(sut).has_error());
    EXPECT_FALSE(sut->lock().has_error());

    std::thread t([&] {
        auto result = sut->unlock();
        ASSERT_TRUE(result.has_error());
        EXPECT_THAT(result.error(), Eq(MutexUnlockError::NOT_OWNED_BY_THREAD));
    });
    t.join();
    EXPECT_FALSE(sut->unlock().has_error());
}

#if !defined(__APPLE__) && !defined(_WIN32)
TEST_F(Mutex_test,
       MutexWithOnReleaseWhenLockedBehaviorUnlocksLockedMutexWhenThreadTerminatesAndSetsItIntoInconsistentState)
{
    ::testing::Test::RecordProperty("TEST_ID", "4da7b1fb-23f1-421c-acf3-2a3d9e26b1a1");
#if defined(QNX) || defined(__QNX) || defined(__QNX__) || defined(QNX__)
    GTEST_SKIP() << "iox-#1683 QNX supports robust mutex not like the posix standard describes them.";
#endif
    iox::optional<mutex> sut;
    ASSERT_FALSE(MutexBuilder()
                     .threadTerminationBehavior(MutexThreadTerminationBehavior::RELEASE_WHEN_LOCKED)
                     .create(sut)
                     .has_error());

    std::thread t([&] { EXPECT_FALSE(sut->lock().has_error()); });
    t.join();

    auto result = sut->try_lock();
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), MutexTryLockError::LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
    sut->make_consistent();
    EXPECT_FALSE(sut->unlock().has_error());
}

#if !defined(__FreeBSD__)
TEST_F(Mutex_test, MutexWithStallWhenLockedBehaviorDoesntUnlockMutexWhenThreadTerminates)
{
    ::testing::Test::RecordProperty("TEST_ID", "9beae890-f18e-4878-a957-312920eb1833");
#if defined(QNX) || defined(__QNX) || defined(__QNX__) || defined(QNX__)
    GTEST_SKIP() << "iox-#1683 QNX supports robust mutex not like the posix standard describes them.";
#endif
    iox::optional<mutex> sut;
    ASSERT_FALSE(MutexBuilder()
                     .threadTerminationBehavior(MutexThreadTerminationBehavior::STALL_WHEN_LOCKED)
                     .create(sut)
                     .has_error());

    std::thread t([&] { EXPECT_FALSE(sut->lock().has_error()); });
    t.join();

    auto result = sut->try_lock();
    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, MutexTryLock::FAILED_TO_ACQUIRE_LOCK);
}
#endif
#endif

TEST_F(Mutex_test, InitializingMutexTwiceResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f26c05f-08e5-481f-8a6e-2ceca3067cf0");
    auto result = MutexBuilder().create(sutRecursive);

    ASSERT_THAT(result.has_error(), Eq(true));
    EXPECT_THAT(result.error(), Eq(MutexCreationError::MUTEX_ALREADY_INITIALIZED));
}
} // namespace
