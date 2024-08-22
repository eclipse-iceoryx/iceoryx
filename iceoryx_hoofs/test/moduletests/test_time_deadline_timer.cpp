// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iox/atomic.hpp"
#include "iox/deadline_timer.hpp"
#include "iox/duration.hpp"
#include "test.hpp"

#include <chrono>
#include <thread>

namespace
{
using namespace ::testing;

using namespace iox::units;
using namespace iox::units::duration_literals;

using Timer = iox::deadline_timer;

class DeadlineTimer_test : public Test
{
  public:
    void SetUp() override
    {
        numberOfCalls = 0;
    }

    void TearDown() override
    {
    }

    Duration second{1_s};

    iox::concurrent::Atomic<int> numberOfCalls{0};
    static const Duration TIMEOUT;
    static const uint64_t SLEEPTIME;
};

const Duration DeadlineTimer_test::TIMEOUT{10_ms};
const uint64_t DeadlineTimer_test::SLEEPTIME = DeadlineTimer_test::TIMEOUT.toMilliseconds();

TIMING_TEST_F(DeadlineTimer_test, ZeroTimeoutTest, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "eb956212-5565-45d6-8f2a-64f79a0709f0");
    Timer sut(0_s);

    TIMING_TEST_EXPECT_TRUE(sut.hasExpired());
})

TIMING_TEST_F(DeadlineTimer_test, DurationOfNonZeroIsExpiresAfterTimeout, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "bc7c63b2-b55f-4731-8677-f8794d2676d9");
    Timer sut(TIMEOUT);

    TIMING_TEST_EXPECT_FALSE(sut.hasExpired());
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME / 3));
    TIMING_TEST_EXPECT_FALSE(sut.hasExpired());
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME / 3));
    TIMING_TEST_EXPECT_TRUE(sut.hasExpired());
})

TIMING_TEST_F(DeadlineTimer_test, ResetWithDurationIsExpired, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "a0af948d-31f1-4a18-b9a7-7c81f6e19bb6");
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME));
    TIMING_TEST_EXPECT_TRUE(sut.hasExpired());
    sut.reset();
    TIMING_TEST_EXPECT_FALSE(sut.hasExpired());
})

TIMING_TEST_F(DeadlineTimer_test, ResetWhenNotExpiredIsStillNotExpired, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "1ce05dc0-04fd-497f-8f8d-b3d5ba9cd3fa");
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME / 3));
    sut.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME / 3));
    TIMING_TEST_EXPECT_FALSE(sut.hasExpired());
})

TIMING_TEST_F(DeadlineTimer_test, ResetAfterBeingExpiredIsNotExpired, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "08cfd0df-4bd6-4944-b070-e7538aa4464d");
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME));

    TIMING_TEST_ASSERT_TRUE(sut.hasExpired());
    sut.reset();
    TIMING_TEST_EXPECT_FALSE(sut.hasExpired());
})

TIMING_TEST_F(DeadlineTimer_test, ResetWithCustomizedTimeAfterBeingExpiredIsNotExpired, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "9ceb9355-4013-4d51-a51f-65399f25b14f");
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME));

    TIMING_TEST_ASSERT_TRUE(sut.hasExpired());

    sut.reset(20_s);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME));

    TIMING_TEST_EXPECT_FALSE(sut.hasExpired());
})

TIMING_TEST_F(DeadlineTimer_test, ResetWithCustomizedTimeAfterBeingExpiredIsExpired, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "03c71362-4cba-46be-b056-ffdc9f811f8a");
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME));

    TIMING_TEST_ASSERT_TRUE(sut.hasExpired());

    sut.reset(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME));

    TIMING_TEST_ASSERT_TRUE(sut.hasExpired());
})

TIMING_TEST_F(DeadlineTimer_test, RemainingTimeCheckIfExpired, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "8c8af30c-104c-41ef-b89f-f40b50823d12");
    Timer sut(TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * SLEEPTIME));

    TIMING_TEST_ASSERT_TRUE(sut.hasExpired());

    uint64_t remainingTime = sut.remainingTime().toMilliseconds();
    const uint64_t EXPECTED_REMAINING_TIME = 0; // the timer is expired the remaining wait time is Zero
    TIMING_TEST_EXPECT_TRUE(remainingTime == EXPECTED_REMAINING_TIME);
})

TIMING_TEST_F(DeadlineTimer_test, RemainingTimeCheckIfNotExpired, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "0d0f56d4-8e6a-435d-9026-c15f472c1d0d");
    Timer sut(2 * TIMEOUT);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEPTIME));

    TIMING_TEST_ASSERT_FALSE(sut.hasExpired());

    uint64_t remainingTime = sut.remainingTime().toMilliseconds();
    const int PASSED_TIMER_TIME = SLEEPTIME; // Already 10ms passed in sleeping out of 20ms
    const int RANGE_APPROX = 2;              // 2ms arppoximation. This may be lost after arming the timer in execution.
    const int EXPECTED_REMAINING_TIME = PASSED_TIMER_TIME - RANGE_APPROX;

    TIMING_TEST_EXPECT_TRUE(remainingTime >= EXPECTED_REMAINING_TIME && remainingTime <= PASSED_TIMER_TIME);
})
} // namespace
