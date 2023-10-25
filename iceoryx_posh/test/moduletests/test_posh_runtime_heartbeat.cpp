// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iox/fixed_position_container.hpp"

#include "iceoryx_posh/internal/runtime/heartbeat.hpp"

#include "test.hpp"

#include <chrono>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::runtime;

constexpr uint64_t ALLOWED_NON_SLEEP_JITTER_MS{5};
// NOTE: this needs to be quite high due to the thread sanitizer on macOS slows the application down quite a lot
constexpr uint64_t ALLOWED_SLEEP_JITTER_MS{150};

TEST(Heartbeat_test, ElapsedMillisecondsSinceLastBeatOnNewlyCreatedInstanceIsCloseToZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8640277-c179-4adf-a7f1-5ba70fd39854");

    constexpr uint64_t EXPECTED_MS{0};

    Heartbeat sut;
    auto elapsed_ms = sut.elapsed_milliseconds_since_last_beat();

    EXPECT_THAT(elapsed_ms, Le(EXPECTED_MS + ALLOWED_NON_SLEEP_JITTER_MS));
}

TEST(Heartbeat_test, ElapsedMillisecondsSinceLastBeatIsLargerOrEqualToSleepTimeAfterInstanceCreation)
{
    ::testing::Test::RecordProperty("TEST_ID", "d076c96b-59ad-4241-a024-20d65667c404");

    constexpr uint64_t EXPECTED_MS{100};

    Heartbeat sut;
    std::this_thread::sleep_for(std::chrono::milliseconds(EXPECTED_MS));
    auto elapsed_ms = sut.elapsed_milliseconds_since_last_beat();

    EXPECT_THAT(elapsed_ms, Ge(EXPECTED_MS));
    EXPECT_THAT(elapsed_ms, Le(EXPECTED_MS + ALLOWED_SLEEP_JITTER_MS));
}

TEST(Heartbeat_test, ElapsedMillisecondsSinceLastBeatAfterBeatCallIsCloseToZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "1197fc96-d3e2-4f32-88dd-209f0647bbdd");

    constexpr uint64_t EXPECTED_MS{0};

    Heartbeat sut;
    std::this_thread::sleep_for(std::chrono::milliseconds(ALLOWED_NON_SLEEP_JITTER_MS * 2));
    sut.beat();
    auto elapsed_ms = sut.elapsed_milliseconds_since_last_beat();

    EXPECT_THAT(elapsed_ms, Le(EXPECTED_MS + ALLOWED_NON_SLEEP_JITTER_MS));
}

TEST(Heartbeat_test, ElapsedMillisecondsSinceLastBeatIsLargerOrEqualToSleepTimeAfterCallToBeat)
{
    ::testing::Test::RecordProperty("TEST_ID", "8891a282-f606-44b4-9bcb-6d99cff4ab71");

    constexpr uint64_t EXPECTED_MS{100};

    Heartbeat sut;
    std::this_thread::sleep_for(std::chrono::milliseconds(ALLOWED_SLEEP_JITTER_MS * 2));
    sut.beat();
    std::this_thread::sleep_for(std::chrono::milliseconds(EXPECTED_MS));
    auto elapsed_ms = sut.elapsed_milliseconds_since_last_beat();

    EXPECT_THAT(elapsed_ms, Ge(EXPECTED_MS));
    EXPECT_THAT(elapsed_ms, Le(EXPECTED_MS + ALLOWED_SLEEP_JITTER_MS));
}
} // namespace
