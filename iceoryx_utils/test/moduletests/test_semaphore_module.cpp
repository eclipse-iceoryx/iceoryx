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

#include "timing_test.hpp"
#if !(defined(QNX) || defined(QNX__) || defined(__QNX__))

#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/platform/time.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "test.hpp"

#include <atomic>
#include <chrono>
#include <thread>


using namespace ::testing;
using namespace iox::units::duration_literals;

typedef iox::posix::Semaphore* CreateSemaphore();

iox::posix::Semaphore* createNamedSemaphore()
{
    static int i = 10;
    auto semaphore = iox::posix::Semaphore::create(
        iox::posix::CreateNamedSemaphore, std::string("/fuuSem" + std::to_string(i++)).c_str(), S_IRUSR | S_IWUSR, 0);
    return (semaphore.has_error()) ? nullptr : new iox::posix::Semaphore(std::move(*semaphore));
}

iox::posix::Semaphore* createUnnamedSemaphore()
{
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0);
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
        delete syncSemaphore;
    }

    void SetUp()
    {
        internal::CaptureStderr();
        ASSERT_THAT(sut, Ne(nullptr));
        ASSERT_THAT(syncSemaphore, Ne(nullptr));
    }

    void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    static constexpr unsigned long long TIMING_TEST_TIMEOUT{(100_ms).nanoSeconds<long>()};

    iox::posix::Semaphore* sut{nullptr};
    iox::posix::Semaphore* syncSemaphore = [] {
        auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0);
        return (semaphore.has_error()) ? nullptr : new iox::posix::Semaphore(std::move(*semaphore));
    }();
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

/// we require INSTANTIATE_TEST_CASE since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
INSTANTIATE_TEST_CASE_P(Semaphore_test, Semaphore_test, Values(&createNamedSemaphore, &createUnnamedSemaphore));
#pragma GCC diagnostic pop

TEST_F(SemaphoreCreate_test, CreateNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem", S_IRUSR | S_IWUSR, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, CreateExistingNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem1", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 =
        iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem1", S_IRUSR | S_IWUSR, 10);
    ASSERT_EQ(semaphore.has_error(), false);
    ASSERT_EQ(semaphore2.has_error(), true);
}

TEST_F(SemaphoreCreate_test, CreateLocalUnnamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, OpenNamedSemaphore)
{
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 = iox::posix::Semaphore::create(iox::posix::OpenNamedSemaphore, "/fuuSem", S_IRUSR | S_IWUSR);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
    EXPECT_THAT(semaphore2.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, OpenNamedSemaphoreWithEmptyNameFails)
{
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "", S_IRUSR | S_IWUSR, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(true));
}

TEST_F(SemaphoreCreate_test, OpenNonExistingNamedSemaphore)
{
    auto semaphore2 = iox::posix::Semaphore::create(iox::posix::OpenNamedSemaphore, "/fuuSem", S_IRUSR | S_IWUSR);
    EXPECT_THAT(semaphore2.has_error(), Eq(true));
}

TEST_P(Semaphore_test, PostIncreasesSemaphoreValue)
{
    for (int i = 0; i < 12; ++i)
    {
        sut->post();
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(12));
}

TEST_P(Semaphore_test, WaitDecreasesSemaphoreValue)
{
    for (int i = 0; i < 18; ++i)
    {
        sut->post();
    }
    for (int i = 0; i < 7; ++i)
    {
        sut->wait();
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(11));
}

TEST_P(Semaphore_test, SuccessfulTryWaitDecreasesSemaphoreValue)
{
    for (int i = 0; i < 15; ++i)
    {
        sut->post();
    }
    for (int i = 0; i < 9; ++i)
    {
        auto call = sut->tryWait();
        ASSERT_THAT(call.has_error(), Eq(false));
        ASSERT_THAT(*call, Eq(true));
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(6));
}

TEST_P(Semaphore_test, FailingTryWaitDoesNotChangeSemaphoreValue)
{
    for (int i = 0; i < 4; ++i)
    {
        auto call = sut->tryWait();
        ASSERT_THAT(call.has_error(), Eq(false));
        ASSERT_THAT(*call, Eq(false));
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(0));
}

TEST_P(Semaphore_test, SuccessfulTimedWaitDecreasesSemaphoreValue)
{
    for (int i = 0; i < 19; ++i)
    {
        sut->post();
    }

    for (int i = 0; i < 12; ++i)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        constexpr long TWO_MILLISECONDS{2000000};
        ts.tv_nsec += TWO_MILLISECONDS;

        auto call = sut->timedWait(&ts, false);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.get_value() == iox::posix::SemaphoreWaitState::NO_TIMEOUT);
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(7));
}

TEST_P(Semaphore_test, FailingTimedWaitDoesNotChangeSemaphoreValue)
{
    for (int i = 0; i < 4; ++i)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        constexpr long TWO_MICROSECONDS{2000};
        ts.tv_nsec += TWO_MICROSECONDS;

        auto call = sut->timedWait(&ts, false);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.get_value() == iox::posix::SemaphoreWaitState::TIMEOUT);
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(0));
}


TEST_P(Semaphore_test, TryWaitAfterPostIsSuccessful)
{
    sut->post();
    auto call = sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TEST_P(Semaphore_test, TryWaitWithNoPostIsNotSuccessful)
{
    sut->post();
    auto call = sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TEST_P(Semaphore_test, WaitValidAfterPostIsNonBlocking)
{
    sut->post();
    // this call should not block and should be successful
    EXPECT_THAT(sut->wait().has_error(), Eq(false));
}

TEST_P(Semaphore_test, WaitIsBlocking)
{
    std::atomic<int> counter{0};
    std::thread t1([&] {
        syncSemaphore->wait();
        sut->post();
        syncSemaphore->wait();
        counter++;
        sut->post();
    });

    EXPECT_THAT(counter.load(), Eq(0));

    syncSemaphore->post();
    sut->wait();
    EXPECT_THAT(counter.load(), Eq(0));

    syncSemaphore->post();
    sut->wait();
    EXPECT_THAT(counter.load(), Eq(1));

    t1.join();
}

TEST_P(Semaphore_test, MoveAssignment)
{
    iox::posix::Semaphore b;
    {
        b = std::move(*sut);
    }

    EXPECT_THAT(b.post().has_error(), Eq(false));
}

TEST_P(Semaphore_test, MoveCTor)
{
    iox::posix::Semaphore b(std::move(*sut));

    EXPECT_THAT(b.post().has_error(), Eq(false));
}

TIMING_TEST_P(Semaphore_test, TimedWaitWithTimeout, Repeat(3), [&] {
    using namespace iox::units;
    std::atomic_bool timedWaitFinish{false};

    std::thread t([&] {
        auto ts = Duration::nanoseconds(TIMING_TEST_TIMEOUT).timespec(TimeSpecReference::Epoch);
        syncSemaphore->post();
        sut->wait();
        auto call = sut->timedWait(&ts, false);
        TIMING_TEST_ASSERT_FALSE(call.has_error());
        TIMING_TEST_EXPECT_TRUE(call.get_value() == iox::posix::SemaphoreWaitState::TIMEOUT);
        timedWaitFinish.store(true);
    });

    syncSemaphore->wait();
    sut->post();
    std::this_thread::sleep_for(std::chrono::nanoseconds(TIMING_TEST_TIMEOUT / 3 * 2));
    TIMING_TEST_EXPECT_FALSE(timedWaitFinish.load());

    std::this_thread::sleep_for(std::chrono::nanoseconds(TIMING_TEST_TIMEOUT / 3 * 2));
    TIMING_TEST_EXPECT_TRUE(timedWaitFinish.load());

    t.join();
});


TIMING_TEST_P(Semaphore_test, TimedWaitWithoutTimeout, Repeat(3), [&] {
    using namespace iox::units;
    std::atomic_bool timedWaitFinish{false};

    std::thread t([&] {
        auto ts = Duration::nanoseconds(TIMING_TEST_TIMEOUT).timespec(TimeSpecReference::Epoch);
        syncSemaphore->post();
        sut->wait();
        auto call = sut->timedWait(&ts, false);
        TIMING_TEST_ASSERT_FALSE(call.has_error());
        TIMING_TEST_EXPECT_TRUE(call.get_value() == iox::posix::SemaphoreWaitState::NO_TIMEOUT);
        timedWaitFinish.store(true);
    });

    syncSemaphore->wait();
    sut->post();
    std::this_thread::sleep_for(std::chrono::nanoseconds(TIMING_TEST_TIMEOUT / 3 * 2));
    TIMING_TEST_EXPECT_FALSE(timedWaitFinish.load());

    sut->post();
    std::this_thread::sleep_for(std::chrono::nanoseconds(TIMING_TEST_TIMEOUT / 3 * 2));
    TIMING_TEST_EXPECT_TRUE(timedWaitFinish.load());

    t.join();
});

#endif // not defined QNX
