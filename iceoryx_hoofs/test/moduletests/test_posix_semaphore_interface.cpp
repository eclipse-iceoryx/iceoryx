// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/semaphore_interface.hpp"
#include "iceoryx_hoofs/internal/units/duration.hpp"
#include "iceoryx_hoofs/platform/time.hpp"
#include "iceoryx_hoofs/posix_wrapper/unnamed_semaphore.hpp"
#include "iceoryx_hoofs/testing/test.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"

#include <atomic>
#include <chrono>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox::units::duration_literals;

template <typename T>
class SemaphoreInterfaceTest : public Test
{
  public:
    using SutFactory = T;
    using SutType = typename SutFactory::SutType;

    SemaphoreInterfaceTest()
    {
    }

    ~SemaphoreInterfaceTest()
    {
    }

    void SetUp()
    {
        deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });
        ASSERT_TRUE(SutFactory::create(sut));
    }

    void TearDown()
    {
    }

    SutType sut;

    static constexpr iox::units::Duration WATCHDOG_TIMEOUT = 5_s;
    static constexpr iox::units::Duration TIMING_TEST_WAIT_TIME = 100_ms;
    Watchdog deadlockWatchdog{WATCHDOG_TIMEOUT};
};
template <typename T>
constexpr iox::units::Duration SemaphoreInterfaceTest<T>::WATCHDOG_TIMEOUT;

template <typename T>
constexpr iox::units::Duration SemaphoreInterfaceTest<T>::TIMING_TEST_WAIT_TIME;

struct UnnamedSemaphoreTest
{
    using SutType = iox::cxx::optional<iox::posix::UnnamedSemaphore>;
    static bool create(SutType& sut)
    {
        return !iox::posix::UnnamedSemaphoreBuilder()
                    .initialValue(0U)
                    .isInterProcessCapable(false)
                    .create(sut)
                    .has_error();
    }
};

using Implementations = Types<UnnamedSemaphoreTest>;
TYPED_TEST_SUITE(SemaphoreInterfaceTest, Implementations);

TYPED_TEST(SemaphoreInterfaceTest, PostIncreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "af3013ef-683e-4ad4-874f-5c7e1a3f41fd");
    for (int i = 0; i < 12; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }

    auto result = this->sut->getState();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result->value, Eq(12));
}

TYPED_TEST(SemaphoreInterfaceTest, WaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "29a63157-caee-4d28-a477-c4fa7048911c");
    for (int i = 0; i < 18; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }
    for (int i = 0; i < 7; ++i)
    {
        ASSERT_FALSE(this->sut->wait().has_error());
    }

    auto result = this->sut->getState();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result->value, Eq(11));
}

TYPED_TEST(SemaphoreInterfaceTest, SuccessfulTryWaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a98d1c45-d538-4abc-9633-f4868163785d");
    for (int i = 0; i < 15; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }
    for (int i = 0; i < 9; ++i)
    {
        auto call = this->sut->tryWait();
        ASSERT_THAT(call.has_error(), Eq(false));
        ASSERT_THAT(*call, Eq(true));
    }

    auto result = this->sut->getState();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result->value, Eq(6));
}

TYPED_TEST(SemaphoreInterfaceTest, FailingTryWaitDoesNotChangeSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "8bad3784-888b-44cd-ad5d-1c4662b26b17");
    for (int i = 0; i < 4; ++i)
    {
        auto call = this->sut->tryWait();
        ASSERT_THAT(call.has_error(), Eq(false));
        ASSERT_THAT(*call, Eq(false));
    }

    auto result = this->sut->getState();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result->value, Eq(0));
}

TYPED_TEST(SemaphoreInterfaceTest, SuccessfulTimedWaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e3bd3f5d-967c-4a5b-9f22-a8f92f73b3c3");
    const iox::units::Duration timeToWait = 2_ms;
    for (int i = 0; i < 19; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }

    for (int i = 0; i < 12; ++i)
    {
        auto call = this->sut->timedWait(timeToWait);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.value() == iox::posix::SemaphoreWaitState::NO_TIMEOUT);
    }

    auto result = this->sut->getState();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result->value, Eq(7));
}

TYPED_TEST(SemaphoreInterfaceTest, FailingTimedWaitDoesNotChangeSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a630be5-aef6-493e-a8ee-4dfabe642258");
    const iox::units::Duration timeToWait = 2_us;
    for (int i = 0; i < 4; ++i)
    {
        auto call = this->sut->timedWait(timeToWait);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.value() == iox::posix::SemaphoreWaitState::TIMEOUT);
    }

    auto result = this->sut->getState();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(result->value, Eq(0));
}


TYPED_TEST(SemaphoreInterfaceTest, TryWaitAfterPostIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "22354447-c44d-443c-97a5-f5fdffb09748");
    ASSERT_FALSE(this->sut->post().has_error());
    auto call = this->sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TYPED_TEST(SemaphoreInterfaceTest, TryWaitWithNoPostIsNotSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e5d6817-88a9-4fca-889e-4dbfe2c30e48");
    ASSERT_FALSE(this->sut->post().has_error());
    auto call = this->sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TYPED_TEST(SemaphoreInterfaceTest, WaitValidAfterPostIsNonBlocking)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4b1de28-89c4-4dfa-a465-8c7cfc652d67");
    ASSERT_FALSE(this->sut->post().has_error());
    // this call should not block and should be successful
    EXPECT_THAT(this->sut->wait().has_error(), Eq(false));
}

TYPED_TEST(SemaphoreInterfaceTest, WaitBlocksAtLeastDefinedSleepTime)
{
    ::testing::Test::RecordProperty("TEST_ID", "5869a475-6be3-4b55-aa58-2ebf11d46081");
    std::atomic<int> counter{0};

    std::thread t1([&] {
        counter = 1;
        while (counter.load() != 2)
        {
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(this->TIMING_TEST_WAIT_TIME.toNanoseconds()));
        ASSERT_FALSE(this->sut->post().has_error());
    });

    while (counter.load() != 1)
    {
    }

    auto start = std::chrono::steady_clock::now();
    counter = 2;

    ASSERT_FALSE(this->sut->wait().has_error());
    auto end = std::chrono::steady_clock::now();

    EXPECT_THAT(std::chrono::nanoseconds(end - start).count(), Ge(this->TIMING_TEST_WAIT_TIME.toNanoseconds()));

    t1.join();
}

TYPED_TEST(SemaphoreInterfaceTest, TimedWaitBlocksAtLeastDefinedSleepTimeAndSignalsTimeout)
{
    ::testing::Test::RecordProperty("TEST_ID", "71ca9773-f724-4625-8550-6c56ef135ad7");

    auto start = std::chrono::steady_clock::now();
    auto result = this->sut->timedWait(this->TIMING_TEST_WAIT_TIME);
    auto end = std::chrono::steady_clock::now();

    ASSERT_FALSE(result.has_error());
    EXPECT_THAT(*result, Eq(iox::posix::SemaphoreWaitState::TIMEOUT));

    EXPECT_THAT(std::chrono::nanoseconds(end - start).count(), Ge(this->TIMING_TEST_WAIT_TIME.toNanoseconds()));
}
} // namespace
