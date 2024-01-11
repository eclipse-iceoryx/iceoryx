// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iox/duration.hpp"
#include "iox/thread.hpp"
#include "test.hpp"

#include <thread>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::units;
using namespace iox::units::duration_literals;

struct Thread_test : public Test
{
    optional<Thread> sut;
};

TEST_F(Thread_test, CreateThreadWithNonEmptyCallableSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d1e439d-c84e-4a46-ac45-dc8be7530c32");
    bool callableWasCalled = false;
    Thread::callable_t callable = [&] { callableWasCalled = true; };
    ASSERT_FALSE(ThreadBuilder().create(sut, callable).has_error());
    sut.reset();
    EXPECT_TRUE(callableWasCalled);
}

TEST_F(Thread_test, DtorOfThreadBlocksUntilCallbackHasFinished)
{
    ::testing::Test::RecordProperty("TEST_ID", "1062a036-e825-4f30-bfb8-00d5de47fdfd");

    constexpr Duration TEST_WAIT_TIME = 100_ms;
    std::chrono::milliseconds realWaitDuration{0};
    Barrier threadSync;

    ASSERT_FALSE(ThreadBuilder()
                     .create(sut,
                             [&] {
                                 threadSync.wait();
                                 auto start = std::chrono::steady_clock::now();
                                 std::this_thread::sleep_for(std::chrono::nanoseconds(TEST_WAIT_TIME.toNanoseconds()));
                                 auto end = std::chrono::steady_clock::now();
                                 realWaitDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                             })
                     .has_error());

    auto start = std::chrono::steady_clock::now();
    threadSync.notify();
    sut.reset();
    auto end = std::chrono::steady_clock::now();
    auto blockingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_THAT(blockingDuration.count(), Ge(realWaitDuration.count()));
}

TEST_F(Thread_test, SetAndGetWithEmptyThreadNameIsWorking)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba2ed4d9-f051-4ad1-a2df-6741134c494f");
    ThreadName_t emptyString = "";
    ASSERT_FALSE(ThreadBuilder()
                     .name(emptyString)
                     .create(sut, [] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); })
                     .has_error());

    auto getResult = sut->getName();

    EXPECT_THAT(getResult, Eq(emptyString));
}

TEST_F(Thread_test, SetAndGetWithThreadNameCapacityIsWorking)
{
    ::testing::Test::RecordProperty("TEST_ID", "a67128fe-a779-4bdb-a849-3bcbfed4b20f");
    ThreadName_t stringEqualToThreadNameCapacitiy = "123456789ABCDEF";
    EXPECT_THAT(stringEqualToThreadNameCapacitiy.capacity(), Eq(stringEqualToThreadNameCapacitiy.size()));
    ASSERT_FALSE(ThreadBuilder()
                     .name(stringEqualToThreadNameCapacitiy)
                     .create(sut, [] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); })
                     .has_error());

    auto getResult = sut->getName();

    EXPECT_THAT(getResult.c_str(), StrEq(stringEqualToThreadNameCapacitiy.c_str()));
}

TEST_F(Thread_test, SetAndGetSmallStringIsWorking)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5141d3c-2721-478c-b3d1-f35fb3321117");
    ThreadName_t stringShorterThanThreadNameCapacitiy = "I'm short";
    ASSERT_FALSE(ThreadBuilder()
                     .name(stringShorterThanThreadNameCapacitiy)
                     .create(sut, [] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); })
                     .has_error());

    auto getResult = sut->getName();

    EXPECT_THAT(getResult.c_str(), StrEq(stringShorterThanThreadNameCapacitiy.c_str()));
}
} // namespace
