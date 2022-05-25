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
    ::testing::Test::RecordProperty("TEST_ID", "728c70a7-dca3-4f09-a7bf-c347106dab0c");
    constexpr uint64_t NUMBER_OF_INCREMENTS = 12U;

    for (uint64_t i = 0U; i < NUMBER_OF_INCREMENTS; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }

    auto result = this->sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(NUMBER_OF_INCREMENTS));
}

TYPED_TEST(SemaphoreInterfaceTest, WaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae40f4b2-1997-41ab-afe1-7927d5269a36");
    constexpr uint64_t NUMBER_OF_INCREMENTS = 18U;
    constexpr uint64_t NUMBER_OF_DECREMENTS = 7U;

    for (uint64_t i = 0; i < NUMBER_OF_INCREMENTS; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }
    for (uint64_t i = 0; i < NUMBER_OF_DECREMENTS; ++i)
    {
        ASSERT_FALSE(this->sut->wait().has_error());
    }

    auto result = this->sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(NUMBER_OF_INCREMENTS - NUMBER_OF_DECREMENTS));
}

TYPED_TEST(SemaphoreInterfaceTest, SuccessfulTryWaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "977ee162-a233-4e42-9abd-8faa996b6a5c");
    constexpr uint64_t NUMBER_OF_INCREMENTS = 15U;
    constexpr uint64_t NUMBER_OF_DECREMENTS = 9U;

    for (uint64_t i = 0; i < NUMBER_OF_INCREMENTS; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }
    for (uint64_t i = 0; i < NUMBER_OF_DECREMENTS; ++i)
    {
        auto call = this->sut->tryWait();
        ASSERT_THAT(call.has_error(), Eq(false));
        ASSERT_THAT(*call, Eq(true));
    }

    auto result = this->sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(NUMBER_OF_INCREMENTS - NUMBER_OF_DECREMENTS));
}

TYPED_TEST(SemaphoreInterfaceTest, FailingTryWaitDoesNotChangeSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5eff306-7b73-425a-a4d8-9d1af4d5d345");
    constexpr uint64_t NUMBER_OF_DECREMENTS = 4U;

    for (uint64_t i = 0; i < NUMBER_OF_DECREMENTS; ++i)
    {
        auto call = this->sut->tryWait();
        ASSERT_THAT(call.has_error(), Eq(false));
        ASSERT_THAT(*call, Eq(false));
    }

    auto result = this->sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(0));
}

TYPED_TEST(SemaphoreInterfaceTest, SuccessfulTimedWaitDecreasesSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "6bee4652-4eac-4d70-908a-f3be954185de");
    constexpr uint64_t NUMBER_OF_INCREMENTS = 19U;
    constexpr uint64_t NUMBER_OF_DECREMENTS = 12U;
    constexpr iox::units::Duration timeToWait = 2_ms;

    for (uint64_t i = 0; i < NUMBER_OF_INCREMENTS; ++i)
    {
        ASSERT_FALSE(this->sut->post().has_error());
    }

    for (uint64_t i = 0; i < NUMBER_OF_DECREMENTS; ++i)
    {
        auto call = this->sut->timedWait(timeToWait);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.value() == iox::posix::SemaphoreWaitState::NO_TIMEOUT);
    }

    auto result = this->sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(NUMBER_OF_INCREMENTS - NUMBER_OF_DECREMENTS));
}

TYPED_TEST(SemaphoreInterfaceTest, FailingTimedWaitDoesNotChangeSemaphoreValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb2f5ded-7c04-4e2d-ab41-bbe231a520c7");
    constexpr uint64_t NUMBER_OF_DECREMENTS = 4U;
    constexpr iox::units::Duration timeToWait = 2_us;

    for (uint64_t i = 0; i < NUMBER_OF_DECREMENTS; ++i)
    {
        auto call = this->sut->timedWait(timeToWait);
        ASSERT_FALSE(call.has_error());
        ASSERT_TRUE(call.value() == iox::posix::SemaphoreWaitState::TIMEOUT);
    }

    auto result = this->sut->getValue();
    ASSERT_THAT(result.has_error(), Eq(false));
    EXPECT_THAT(*result, Eq(0));
}


TYPED_TEST(SemaphoreInterfaceTest, TryWaitAfterPostIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "df5d32f5-9417-419e-b36e-93a0502ed6e7");
    ASSERT_FALSE(this->sut->post().has_error());
    auto call = this->sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TYPED_TEST(SemaphoreInterfaceTest, TryWaitWithNoPostIsNotSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d0eebcd-11e3-456f-8d19-3fdb6fc70241");
    ASSERT_FALSE(this->sut->post().has_error());
    auto call = this->sut->tryWait();
    ASSERT_THAT(call.has_error(), Eq(false));
    ASSERT_THAT(*call, Eq(true));
}

TYPED_TEST(SemaphoreInterfaceTest, WaitValidAfterPostIsNonBlocking)
{
    ::testing::Test::RecordProperty("TEST_ID", "c08220e3-c368-43d5-85d2-cbf7e1659017");
    ASSERT_FALSE(this->sut->post().has_error());
    // this call should not block and should be successful
    EXPECT_THAT(this->sut->wait().has_error(), Eq(false));
}

TYPED_TEST(SemaphoreInterfaceTest, WaitBlocksAtLeastDefinedSleepTime)
{
    ::testing::Test::RecordProperty("TEST_ID", "9fb454db-2d1d-43a5-b0a5-d9de7f1886e0");

    std::chrono::steady_clock::time_point start;
    std::thread t1([&] {
        start = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::nanoseconds(this->TIMING_TEST_WAIT_TIME.toNanoseconds()));
        ASSERT_FALSE(this->sut->post().has_error());
    });


    ASSERT_FALSE(this->sut->wait().has_error());
    auto end = std::chrono::steady_clock::now();

    t1.join();
    EXPECT_THAT(std::chrono::nanoseconds(end - start).count(), Ge(this->TIMING_TEST_WAIT_TIME.toNanoseconds()));
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
