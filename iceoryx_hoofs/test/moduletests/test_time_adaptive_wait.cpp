// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "test.hpp"
using namespace ::testing;

#include "iox/atomic.hpp"
#include "iox/detail/adaptive_wait.hpp"

#include <thread>

using namespace iox::detail;

namespace
{
TEST(AdaptiveWaitTest, repeatCallingWaitUntilItSaturatesTakesAtLeastCorrectAmountOfTime)
{
    ::testing::Test::RecordProperty("TEST_ID", "5bf15f13-dc22-45f4-a957-d07c5beeb31c");
    class AdaptiveWaitSut : public adaptive_wait
    {
      public:
        using adaptive_wait::INITIAL_REPETITIONS;
        using adaptive_wait::INITIAL_WAITING_TIME;
        using adaptive_wait::YIELD_REPETITIONS;
    };

    adaptive_wait sut;

    auto start = std::chrono::steady_clock::now();
    for (uint64_t i = 0U; i < AdaptiveWaitSut::INITIAL_REPETITIONS; ++i)
    {
        sut.wait();
    }
    auto end = std::chrono::steady_clock::now();

    iox::units::Duration waitingTime = iox::units::Duration::fromMicroseconds(
        (AdaptiveWaitSut::INITIAL_REPETITIONS - AdaptiveWaitSut::YIELD_REPETITIONS)
        * AdaptiveWaitSut::INITIAL_WAITING_TIME.count());
    EXPECT_THAT((end - start).count(), Ge(waitingTime.toNanoseconds()));
}

TEST(AdaptiveWaitTest, waitWaitsAtLeastFINAL_WAITING_TIMEafterINITIAL_REPETITIONS)
{
    ::testing::Test::RecordProperty("TEST_ID", "34bc814c-0565-40ef-a5ef-93f994d024d1");
    class AdaptiveWaitSut : public adaptive_wait
    {
      public:
        using adaptive_wait::FINAL_WAITING_TIME;
        using adaptive_wait::INITIAL_REPETITIONS;
    };

    adaptive_wait sut;

    for (uint64_t i = 0U; i < AdaptiveWaitSut::INITIAL_REPETITIONS; ++i)
    {
        sut.wait();
    }

    auto start = std::chrono::steady_clock::now();
    sut.wait();
    auto end = std::chrono::steady_clock::now();

    EXPECT_THAT(
        (end - start).count(),
        Ge(iox::units::Duration::fromMilliseconds(AdaptiveWaitSut::FINAL_WAITING_TIME.count()).toNanoseconds()));
}

TEST(AdaptiveWaitTest, wait_loopWaitsAtLeastAsLongAsTheConditionsReturnsTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "c44e9315-fed4-4681-ba0c-2d25bce4459b");
    class AdaptiveWaitSut : public adaptive_wait
    {
      public:
        using adaptive_wait::FINAL_WAITING_TIME;
        using adaptive_wait::INITIAL_REPETITIONS;
    };

    iox::concurrent::Atomic<bool> continueToWait{true};
    iox::concurrent::Atomic<bool> threadIsStarted{false};
    std::thread waitThread{[&] {
        threadIsStarted = true;
        AdaptiveWaitSut().wait_loop([&] { return continueToWait.load(); });
    }};

    while (!threadIsStarted.load())
    {
        std::this_thread::yield();
    }

    auto start = std::chrono::steady_clock::now();
    const std::chrono::milliseconds waitTime(100);
    std::this_thread::sleep_for(waitTime);
    auto end = std::chrono::steady_clock::now();

    continueToWait.store(false);

    EXPECT_THAT((end - start).count(), Ge(iox::units::Duration::fromMilliseconds(waitTime.count()).toNanoseconds()));

    waitThread.join();
}
} // namespace
