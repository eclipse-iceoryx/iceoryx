// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "iceoryx_hoofs/cxx/method_callback.hpp"
#include "iceoryx_hoofs/internal/concurrent/periodic_task.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"

#include "test.hpp"

#include <cstdint>
#include <functional>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::concurrent;
using namespace iox::units::duration_literals;

constexpr std::chrono::milliseconds SLEEP_TIME{100};
constexpr units::Duration INTERVAL{10_ms};
#if defined(__APPLE__)
constexpr uint64_t MIN_RUNS{3U};
constexpr uint64_t MAX_RUNS{17U};
#else
constexpr uint64_t MIN_RUNS{5U};
constexpr uint64_t MAX_RUNS{15U};
#endif

struct PeriodicTaskTestType
{
  public:
    PeriodicTaskTestType() = default;

    PeriodicTaskTestType(uint64_t callCounterOffset)
    {
        callCounter = callCounterOffset;
    }

    void operator()()
    {
        increment();
    }

    void incrementMethod()
    {
        increment();
    }

    static void increment()
    {
        ++callCounter;
    }

    static uint64_t callCounter;
};

uint64_t PeriodicTaskTestType::callCounter{0};

class PeriodicTask_test : public Test
{
  public:
    virtual void SetUp()
    {
        PeriodicTaskTestType::callCounter = 0;
    }

    virtual void TearDown()
    {
    }
};

TEST_F(PeriodicTask_test, CopyConstructorIsDeleted)
{
    ::testing::Test::RecordProperty("TEST_ID", "b62ff99f-bed3-4c46-8c30-701968f7a217");
    EXPECT_TRUE(std::is_copy_constructible<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_copy_constructible<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TEST_F(PeriodicTask_test, MoveConstructorIsDeleted)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa8f709b-b992-406e-bb40-d9da388fa0e1");
    EXPECT_TRUE(std::is_move_constructible<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_move_constructible<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TEST_F(PeriodicTask_test, CopyAssignmentIsDeleted)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4f6efb3-7440-4929-8a09-ef7e0489a78d");
    EXPECT_TRUE(std::is_copy_assignable<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_copy_assignable<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TEST_F(PeriodicTask_test, MoveAssignmentIsDeleted)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3f8991b-1b97-4d31-93f3-dacb69fd440f");
    EXPECT_TRUE(std::is_move_assignable<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_move_assignable<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TEST_F(PeriodicTask_test, PeriodicTaskConstructedWithoutIntervalIsInactive)
{
    ::testing::Test::RecordProperty("TEST_ID", "37c092b9-d44b-4de2-8599-39ac8234d69e");
    concurrent::PeriodicTask<PeriodicTaskTestType> sut(PeriodicTaskManualStart, "Test");

    EXPECT_THAT(sut.isActive(), Eq(false));
}

TEST_F(PeriodicTask_test, PeriodicTaskConstructedWithoutIntervalIsActiveAfterCallingStart)
{
    ::testing::Test::RecordProperty("TEST_ID", "02e24d7f-c990-4c76-af26-e86e43beb151");
    concurrent::PeriodicTask<PeriodicTaskTestType> sut(PeriodicTaskManualStart, "Test");
    sut.start(INTERVAL);

    EXPECT_THAT(sut.isActive(), Eq(true));
}

TEST_F(PeriodicTask_test, PeriodicTaskConstructedWithIntervalIsActive)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ff6df91-8fff-4edf-a452-d35dfac77a78");
    concurrent::PeriodicTask<PeriodicTaskTestType> sut(PeriodicTaskAutoStart, INTERVAL, "Test");

    EXPECT_THAT(sut.isActive(), Eq(true));
}

TEST_F(PeriodicTask_test, PeriodicTaskConstructedWithIntervalIsInactiveAfterCallingStop)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0311c9a-f605-4517-b118-94d2513ecc26");
    concurrent::PeriodicTask<PeriodicTaskTestType> sut(PeriodicTaskAutoStart, INTERVAL, "Test");
    sut.stop();

    EXPECT_THAT(sut.isActive(), Eq(false));
}

TEST_F(PeriodicTask_test, PeriodicTaskWhichIsInactiveDoesNotExecuteTheCallable)
{
    ::testing::Test::RecordProperty("TEST_ID", "c48d19c8-170f-4714-a294-6fafb4a04a7f");
    {
        concurrent::PeriodicTask<PeriodicTaskTestType> sut(PeriodicTaskManualStart, "Test");

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, Eq(0U));
}

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskRunningWithObjectWithDefaultConstructor, Repeat(3), [&] {
    {
        concurrent::PeriodicTask<PeriodicTaskTestType> sut(PeriodicTaskAutoStart, INTERVAL, "Test");

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskRunningWithObjectWithConstructorWithArguments, Repeat(3), [&] {
    constexpr uint64_t CALL_COUNTER_OFFSET{1000ULL * 1000ULL * 1000ULL * 1000ULL};
    {
        concurrent::PeriodicTask<PeriodicTaskTestType> sut(
            PeriodicTaskAutoStart, INTERVAL, "Test", CALL_COUNTER_OFFSET);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter,
                AllOf(Ge(CALL_COUNTER_OFFSET + MIN_RUNS), Le(CALL_COUNTER_OFFSET + MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskRunningWithObjectAsReference, Repeat(3), [&] {
    {
        PeriodicTaskTestType testType;
        concurrent::PeriodicTask<PeriodicTaskTestType&> sut(PeriodicTaskAutoStart, INTERVAL, "Test", testType);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskRunningWithCxxFunctionRef, Repeat(3), [&] {
    {
        concurrent::PeriodicTask<cxx::function_ref<void()>> sut(
            PeriodicTaskAutoStart, INTERVAL, "Test", PeriodicTaskTestType::increment);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskRunningWithStdFunction, Repeat(3), [&] {
    {
        concurrent::PeriodicTask<std::function<void()>> sut(
            PeriodicTaskAutoStart, INTERVAL, "Test", PeriodicTaskTestType::increment);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

// clang-format off
// due to the `()` enclosing the lambda, clang-format is messing this up
// the `()` are needed since the `TIMING_TEST_F` macro is messing up the forwarding to the MethodCallback c'tor
TIMING_TEST_F(PeriodicTask_test, PeriodicTaskRunningWithMethodCallback, Repeat(3), ([&] {
    {
        PeriodicTaskTestType testType;
        concurrent::PeriodicTask<cxx::MethodCallback<void>> sut{PeriodicTaskAutoStart,
            INTERVAL, "Test", testType, &PeriodicTaskTestType::incrementMethod};

            std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
}));
// clang-format on

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWhichIsActiveAppliesNewIntervalAfterStart, Repeat(3), [&] {
    auto start = std::chrono::steady_clock::now();
    {
        constexpr units::Duration WAY_TOO_LARGE_INTERVAL{10 * MAX_RUNS * INTERVAL};
        concurrent::PeriodicTask<PeriodicTaskTestType> sut(PeriodicTaskAutoStart, WAY_TOO_LARGE_INTERVAL, "Test");

        sut.start(INTERVAL);

        std::this_thread::sleep_for(SLEEP_TIME);
    }
    auto stop = std::chrono::steady_clock::now();
    auto elapsedTime{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)};

    EXPECT_THAT(elapsedTime, Le(2 * SLEEP_TIME));
    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWhichIsExecutingTheCallableIsBlockingOnStop, Repeat(3), [&] {
    auto start = std::chrono::steady_clock::now();
    concurrent::PeriodicTask<cxx::function_ref<void()>> sut(
        PeriodicTaskAutoStart, INTERVAL, "Test", [] { std::this_thread::sleep_for(SLEEP_TIME); });
    sut.stop();
    auto stop = std::chrono::steady_clock::now();
    auto elapsedTime{std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)};

    EXPECT_THAT(elapsedTime, Ge(SLEEP_TIME));
});
} // namespace
