// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/internal/units/duration.hpp"
#include "iceoryx_hoofs/platform/time.hpp"
#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"
#include "iceoryx_hoofs/testing/test.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"

#include <atomic>
#include <chrono>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox::units::duration_literals;

typedef iox::posix::Semaphore* CreateSemaphore();

iox::posix::Semaphore* createNamedSemaphore()
{
    static int i = 10;
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore,
                                                   std::string("/fuuSem" + iox::cxx::convert::toString(i++)).c_str(),
                                                   S_IRUSR | S_IWUSR,
                                                   0);
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

    static constexpr unsigned long long TIMING_TEST_TIMEOUT{(100_ms).toNanoseconds()};

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

INSTANTIATE_TEST_SUITE_P(Semaphore_test, Semaphore_test, Values(&createNamedSemaphore, &createUnnamedSemaphore));

TEST_F(SemaphoreCreate_test, CreateNamedSemaphore)
{
    ::testing::Test::RecordProperty("TEST_ID", "80f5fba8-c6db-4948-86e1-9e23d413d1ac");
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem", S_IRUSR | S_IWUSR, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, CreateExistingNamedSemaphore)
{
    ::testing::Test::RecordProperty("TEST_ID", "bee46586-bcf2-42e6-9dda-ab2611fc973f");
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem1", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 =
        iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem1", S_IRUSR | S_IWUSR, 10);
    ASSERT_EQ(semaphore.has_error(), false);
    ASSERT_EQ(semaphore2.has_error(), true);
}

TEST_F(SemaphoreCreate_test, CreateLocalUnnamedSemaphore)
{
    ::testing::Test::RecordProperty("TEST_ID", "42099e77-9ac9-425f-8e53-056dc3b73d71");
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, OpenNamedSemaphore)
{
    ::testing::Test::RecordProperty("TEST_ID", "349cdf0d-987e-4e2f-aa35-98a40fdf979b");
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "/fuuSem", S_IRUSR | S_IWUSR, 10);
    auto semaphore2 = iox::posix::Semaphore::create(iox::posix::OpenNamedSemaphore, "/fuuSem", 0);
    EXPECT_THAT(semaphore.has_error(), Eq(false));
    EXPECT_THAT(semaphore2.has_error(), Eq(false));
}

TEST_F(SemaphoreCreate_test, OpenNamedSemaphoreWithEmptyNameFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "5dab9f61-8b27-4684-8e9d-bbd50430b9fa");
    auto semaphore = iox::posix::Semaphore::create(iox::posix::CreateNamedSemaphore, "", S_IRUSR | S_IWUSR, 10);
    EXPECT_THAT(semaphore.has_error(), Eq(true));
}

TEST_F(SemaphoreCreate_test, OpenNonExistingNamedSemaphore)
{
    ::testing::Test::RecordProperty("TEST_ID", "0034b274-5a3b-49dc-a5d2-1715d068809f");
    auto semaphore2 = iox::posix::Semaphore::create(iox::posix::OpenNamedSemaphore, "/fuuSem", S_IRUSR | S_IWUSR);
    EXPECT_THAT(semaphore2.has_error(), Eq(true));
}

TEST_P(Semaphore_test, PostIncreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "af3013ef-683e-4ad4-874f-5c7e1a3f41fd");
    for (int i = 0; i < 12; ++i)
    {
        ASSERT_FALSE(sut->post().has_error());
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(12));
}

TEST_P(Semaphore_test, WaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "29a63157-caee-4d28-a477-c4fa7048911c");
    for (int i = 0; i < 18; ++i)
    {
        ASSERT_FALSE(sut->post().has_error());
    }
    for (int i = 0; i < 7; ++i)
    {
        ASSERT_FALSE(sut->wait().has_error());
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(11));
}

TEST_P(Semaphore_test, SuccessfulTryWaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a98d1c45-d538-4abc-9633-f4868163785d");
    for (int i = 0; i < 15; ++i)
    {
        ASSERT_FALSE(sut->post().has_error());
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
    ::testing::Test::RecordProperty("TEST_ID", "8bad3784-888b-44cd-ad5d-1c4662b26b17");
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
    ::testing::Test::RecordProperty("TEST_ID", "e3bd3f5d-967c-4a5b-9f22-a8f92f73b3c3");
    const iox::units::Duration timeToWait = 2_ms;
    for (int i = 0; i < 19; ++i)
    {
        ASSERT_FALSE(sut->post().has_error());
    }

    for (int i = 0; i < 12; ++i)
    {
        auto call = sut->timedWait(timeToWait);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.value() == iox::posix::SemaphoreWaitState::NO_TIMEOUT);
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(7));
}

TEST_P(Semaphore_test, FailingTimedWaitDoesNotChangeSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a630be5-aef6-493e-a8ee-4dfabe642258");
    const iox::units::Duration timeToWait = 2_us;
    for (int i = 0; i < 4; ++i)
    {
        auto call = sut->timedWait(timeToWait);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.value() == iox::posix::SemaphoreWaitState::TIMEOUT);
    }

    auto result = sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(0));
}


TEST_P(Semaphore_test, TryWaitAfterPostIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "22354447-c44d-443c-97a5-f5fdffb09748");
    ASSERT_FALSE(sut->post().has_error());
    auto call = sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TEST_P(Semaphore_test, TryWaitWithNoPostIsNotSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e5d6817-88a9-4fca-889e-4dbfe2c30e48");
    ASSERT_FALSE(sut->post().has_error());
    auto call = sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TEST_P(Semaphore_test, WaitValidAfterPostIsNonBlocking)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4b1de28-89c4-4dfa-a465-8c7cfc652d67");
    ASSERT_FALSE(sut->post().has_error());
    // this call should not block and should be successful
    EXPECT_THAT(sut->wait().has_error(), Eq(false));
}

TEST_P(Semaphore_test, WaitIsBlocking)
{
    ::testing::Test::RecordProperty("TEST_ID", "5869a475-6be3-4b55-aa58-2ebf11d46081");
    std::atomic<int> counter{0};
    std::thread t1([&] {
        ASSERT_FALSE(syncSemaphore->wait().has_error());
        ASSERT_FALSE(sut->post().has_error());
        ASSERT_FALSE(syncSemaphore->wait().has_error());
        counter++;
        ASSERT_FALSE(sut->post().has_error());
    });

    EXPECT_THAT(counter.load(), Eq(0));

    ASSERT_FALSE(syncSemaphore->post().has_error());
    ASSERT_FALSE(sut->wait().has_error());
    EXPECT_THAT(counter.load(), Eq(0));

    ASSERT_FALSE(syncSemaphore->post().has_error());
    ASSERT_FALSE(sut->wait().has_error());
    EXPECT_THAT(counter.load(), Eq(1));

    t1.join();
}

TEST_P(Semaphore_test, MoveAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf7277fd-4e5c-49dd-b48b-f5d1ed6e4c01");
    iox::posix::Semaphore b;
    {
        b = std::move(*sut);
    }

    EXPECT_THAT(b.post().has_error(), Eq(false));
}

TEST_P(Semaphore_test, MoveCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "e8083f97-c3c0-4e79-9948-395a837bde84");
    iox::posix::Semaphore b(std::move(*sut));

    EXPECT_THAT(b.post().has_error(), Eq(false));
}

TIMING_TEST_P(Semaphore_test, TimedWaitWithTimeout, Repeat(3), [&] {
    using namespace iox::units;
    std::atomic_bool timedWaitFinish{false};

    std::thread t([&] {
        auto timeout = Duration::fromNanoseconds(TIMING_TEST_TIMEOUT);
        ASSERT_FALSE(syncSemaphore->post().has_error());
        ASSERT_FALSE(sut->wait().has_error());
        auto call = sut->timedWait(timeout);
        TIMING_TEST_ASSERT_FALSE(call.has_error());
        TIMING_TEST_EXPECT_TRUE(call.value() == iox::posix::SemaphoreWaitState::TIMEOUT);
        timedWaitFinish.store(true);
    });

    ASSERT_FALSE(syncSemaphore->wait().has_error());
    ASSERT_FALSE(sut->post().has_error());
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
        auto timeout = Duration::fromNanoseconds(TIMING_TEST_TIMEOUT);
        ASSERT_FALSE(syncSemaphore->post().has_error());
        ASSERT_FALSE(sut->wait().has_error());
        auto call = sut->timedWait(timeout);
        TIMING_TEST_ASSERT_FALSE(call.has_error());
        TIMING_TEST_EXPECT_TRUE(call.value() == iox::posix::SemaphoreWaitState::NO_TIMEOUT);
        timedWaitFinish.store(true);
    });

    ASSERT_FALSE(syncSemaphore->wait().has_error());
    ASSERT_FALSE(sut->post().has_error());
    std::this_thread::sleep_for(std::chrono::nanoseconds(TIMING_TEST_TIMEOUT / 3 * 2));
    TIMING_TEST_EXPECT_FALSE(timedWaitFinish.load());

    ASSERT_FALSE(sut->post().has_error());
    std::this_thread::sleep_for(std::chrono::nanoseconds(TIMING_TEST_TIMEOUT / 3 * 2));
    TIMING_TEST_EXPECT_TRUE(timedWaitFinish.load());

    t.join();
});
} // namespace
