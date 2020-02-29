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

#if !(defined(QNX) || defined(QNX__) || defined(__QNX__))

#include "iceoryx_utils/platform/time.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "test.hpp"

#include <atomic>
#include <chrono>
#include <thread>


using namespace ::testing;

typedef iox::posix::Semaphore* CreateSemaphore();

iox::posix::Semaphore* CreateNamedSemaphore()
{
    static int i = 10;
    auto semaphore =
        iox::posix::Semaphore::create(std::string("/fuuSem" + std::to_string(i++)).c_str(), S_IRUSR | S_IWUSR, 0);
    return (semaphore.has_error()) ? nullptr : new iox::posix::Semaphore(std::move(*semaphore));
}

iox::posix::Semaphore* CreateUnnamedSemaphore()
{
    auto semaphore = iox::posix::Semaphore::create(0);
    return (semaphore.has_error()) ? nullptr : new iox::posix::Semaphore(std::move(*semaphore));
}

class Semaphore_test : public TestWithParam<CreateSemaphore*>
{
  public:
    Semaphore_test()
        : sut((*GetParam())())
    {
    }

    ~Semaphore_test()
    {
        delete sut;
    }

    void SetUp()
    {
        internal::CaptureStderr();
        ASSERT_THAT(sut, Ne(nullptr));
    }

    void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    iox::posix::Semaphore* sut{nullptr};
};

class SemaphoreCreate_test : public Test
{
  public:
    void SetUp()
    {
        internal::CaptureStderr();
    }

    void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

INSTANTIATE_TEST_CASE_P(Semaphore_test, Semaphore_test, Values(&CreateNamedSemaphore, &CreateUnnamedSemaphore));

TEST_F(SemaphoreCreate_test, CreateNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, CreateExistingNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSem1", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 = iox::posix::Semaphore::create("/fuuSem1", S_IRUSR | S_IWUSR, 10);
    ASSERT_EQ(semaphore.has_error(), false);
    ASSERT_EQ(semaphore2.has_error(), true);
}

TEST_F(SemaphoreCreate_test, CreateLocalUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, OpenNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
    EXPECT_THAT(semaphore2.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, OpenNonExistingNamedSemaphore)
{
    auto semaphore2 = iox::posix::Semaphore::create("/fuuSem", S_IRUSR | S_IWUSR);
    EXPECT_THAT(semaphore2.has_error(), Eq(true));
}

TEST_P(Semaphore_test, PostIncreasesSemaphoreValue)
{
    for (int i = 0; i < 12; ++i)
        sut->post();

    int value;
    ASSERT_THAT(sut->getValue(value), Eq(true));
    EXPECT_THAT(value, Eq(12));
}

TEST_P(Semaphore_test, WaitDecreasesSemaphoreValue)
{
    for (int i = 0; i < 18; ++i)
        sut->post();
    for (int i = 0; i < 7; ++i)
        sut->wait();

    int value;
    ASSERT_THAT(sut->getValue(value), Eq(true));
    EXPECT_THAT(value, Eq(11));
}

TEST_P(Semaphore_test, SuccessfulTryWaitDecreasesSemaphoreValue)
{
    for (int i = 0; i < 15; ++i)
        sut->post();
    for (int i = 0; i < 9; ++i)
        ASSERT_THAT(sut->tryWait(), Eq(true));

    int value;
    ASSERT_THAT(sut->getValue(value), Eq(true));
    EXPECT_THAT(value, Eq(6));
}

TEST_P(Semaphore_test, FailingTryWaitDoesNotChangeSemaphoreValue)
{
    for (int i = 0; i < 4; ++i)
        ASSERT_THAT(sut->tryWait(), Eq(false));

    int value;
    ASSERT_THAT(sut->getValue(value), Eq(true));
    EXPECT_THAT(value, Eq(0));
}

TEST_P(Semaphore_test, SuccessfulTimedWaitDecreasesSemaphoreValue)
{
    for (int i = 0; i < 19; ++i)
        sut->post();

    for (int i = 0; i < 12; ++i)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        constexpr long TWO_MILLISECONDS{2000000};
        ts.tv_nsec += TWO_MILLISECONDS;
        ASSERT_THAT(sut->timedWait(&ts, false), Eq(true));
    }

    int value;
    ASSERT_THAT(sut->getValue(value), Eq(true));
    EXPECT_THAT(value, Eq(7));
}

TEST_P(Semaphore_test, FailingTimedWaitDoesNotChangeSemaphoreValue)
{
    for (int i = 0; i < 4; ++i)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        constexpr long TWO_MICROSECONDS{2000};
        ts.tv_nsec += TWO_MICROSECONDS;
        ASSERT_THAT(sut->timedWait(&ts, false), Eq(false));
    }

    int value;
    ASSERT_THAT(sut->getValue(value), Eq(true));
    EXPECT_THAT(value, Eq(0));
}


TEST_P(Semaphore_test, TryWaitAfterPostIsSuccessful)
{
    sut->post();
    EXPECT_THAT(sut->tryWait(), Eq(true));
}

TEST_P(Semaphore_test, TryWaitWithNoPostIsNotSuccessful)
{
    sut->post();
    EXPECT_THAT(sut->tryWait(), Eq(true));
}

TEST_P(Semaphore_test, WaitValidAfterPostIsNonBlocking)
{
    sut->post();
    // this call should not block and should be successful
    EXPECT_THAT(sut->wait(), Eq(true));
}

TEST_P(Semaphore_test, WaitIsBlocking)
{
    std::atomic<int> counter{0};
    std::thread t1([&] {
        sut->wait();
        counter++;
        sut->post();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_THAT(counter, Eq(0));
    sut->post();
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    sut->wait();
    EXPECT_THAT(counter, Eq(1));
    t1.join();
}

TEST_P(Semaphore_test, MoveAssignment)
{
    iox::posix::Semaphore b;
    {
        b = std::move(*sut);
        EXPECT_THAT(sut->post(), Eq(false));
    }

    EXPECT_THAT(b.post(), Eq(true));
}

TEST_P(Semaphore_test, MoveCTor)
{
    iox::posix::Semaphore b(std::move(*sut));

    EXPECT_THAT(b.post(), Eq(true));
    EXPECT_THAT(sut->post(), Eq(false));
}

TEST_P(Semaphore_test, TimedWaitWithTimeout)
{
    std::atomic<bool> timedWaitFinish{false};

    std::thread t([&] {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        constexpr long TWO_MILLISECONDS{2000000};
        ts.tv_nsec += TWO_MILLISECONDS;
        sut->post();
        EXPECT_THAT(sut->timedWait(&ts, false), Eq(false));
        timedWaitFinish.store(true);
    });

    sut->wait();
    EXPECT_THAT(timedWaitFinish.load(), Eq(false));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    EXPECT_THAT(timedWaitFinish.load(), Eq(true));

    t.join();
}

TEST_P(Semaphore_test, TimedWaitWithoutTimeout)
{
    std::atomic<bool> timedWaitFinish{false};

    std::thread t([&] {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        constexpr long TWO_MILLISECONDS{2000000};
        ts.tv_nsec += TWO_MILLISECONDS;
        EXPECT_THAT(sut->timedWait(&ts, false), Eq(true));
        timedWaitFinish.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_THAT(timedWaitFinish.load(), Eq(false));
    sut->post();
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_THAT(timedWaitFinish.load(), Eq(true));

    t.join();
}


#endif
