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

#include "iceoryx_hoofs/internal/cxx/spinator.hpp"

using namespace iox::cxx::internal;

namespace
{
class SpinatorTest : public Test
{
  public:
    static constexpr iox::units::Duration MAX_WAITING_TIME = iox::units::Duration::fromMilliseconds(40);
    static constexpr iox::units::Duration INITIAL_WAITING_TIME = iox::units::Duration::fromMilliseconds(20);
    static constexpr uint64_t STEPS = 1;
    static constexpr uint64_t REPETITIONS_PER_STEP = 1;

    spinator_properties sutProperties{MAX_WAITING_TIME, INITIAL_WAITING_TIME, STEPS, REPETITIONS_PER_STEP};
};

constexpr iox::units::Duration SpinatorTest::MAX_WAITING_TIME;
constexpr iox::units::Duration SpinatorTest::INITIAL_WAITING_TIME;
constexpr uint64_t SpinatorTest::STEPS;
constexpr uint64_t SpinatorTest::REPETITIONS_PER_STEP;

TEST_F(SpinatorTest, yieldWaitsAtLeastTheInitialWaitingTime)
{
    spinator sut{sutProperties};

    auto start = std::chrono::steady_clock::now();
    sut.yield();
    auto end = std::chrono::steady_clock::now();

    EXPECT_THAT(std::chrono::nanoseconds(end - start).count(), Ge(INITIAL_WAITING_TIME.toNanoseconds()));
}

TEST_F(SpinatorTest, secondYieldWaitsAAtLeastMaxWaitingTime)
{
    spinator sut{sutProperties};

    sut.yield();

    auto start = std::chrono::steady_clock::now();
    sut.yield();
    auto end = std::chrono::steady_clock::now();

    EXPECT_THAT(std::chrono::nanoseconds(end - start).count(), Ge(MAX_WAITING_TIME.toNanoseconds()));
}

TEST_F(SpinatorTest, whenStepCountIsZeroWaitAtLeastInitialWaitingTime)
{
    sutProperties.stepCount = 0U;
    spinator sut{sutProperties};

    auto start = std::chrono::steady_clock::now();
    sut.yield();
    auto end = std::chrono::steady_clock::now();

    EXPECT_THAT(std::chrono::nanoseconds(end - start).count(), Ge(INITIAL_WAITING_TIME.toNanoseconds()));
}

TEST_F(SpinatorTest, whenInitialWaitingTimeIsGreaterThenMaxWaitingTimeWaitAtLeastInitialWaitingTime)
{
    sutProperties.initialWaitingTime = sutProperties.maxWaitingTime * 2U;
    spinator sut{sutProperties};

    auto start = std::chrono::steady_clock::now();
    sut.yield();
    auto end = std::chrono::steady_clock::now();

    EXPECT_THAT(std::chrono::nanoseconds(end - start).count(), Ge(sutProperties.initialWaitingTime.toNanoseconds()));
}
} // namespace
