// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/cxx/function_ref.hpp"
#include "iceoryx_utils/cxx/method_callback.hpp"
#include "iceoryx_utils/internal/concurrent/periodic_task.hpp"
#include "timing_test.hpp"

#include "test.hpp"

#include <cstdint>
#include <functional>

using namespace ::testing;
using namespace iox;
using namespace iox::units::duration_literals;

constexpr std::chrono::milliseconds SLEEP_TIME{100};
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
    EXPECT_TRUE(std::is_copy_constructible<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_copy_constructible<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TEST_F(PeriodicTask_test, MoveConstructorIsDeleted)
{
    EXPECT_TRUE(std::is_move_constructible<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_move_constructible<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TEST_F(PeriodicTask_test, CopyAssignmentIsDeleted)
{
    EXPECT_TRUE(std::is_copy_assignable<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_copy_assignable<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TEST_F(PeriodicTask_test, MoveAssignmentIsDeleted)
{
    EXPECT_TRUE(std::is_move_assignable<PeriodicTaskTestType>::value);
    EXPECT_FALSE(std::is_move_assignable<concurrent::PeriodicTask<PeriodicTaskTestType>>::value);
}

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWithObjectWithDefaultConstructor, Repeat(3), [&] {
    {
        using namespace iox::units::duration_literals;
        concurrent::PeriodicTask<PeriodicTaskTestType> sut("Test", 10_ms);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWithObjectWithConstructorWithArguments, Repeat(3), [&] {
    constexpr uint64_t CALL_COUNTER_OFFSET{1000ULL * 1000ULL * 1000ULL * 1000ULL};
    {
        using namespace iox::units::duration_literals;
        concurrent::PeriodicTask<PeriodicTaskTestType> sut("Test", 10_ms, CALL_COUNTER_OFFSET);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter,
                AllOf(Ge(CALL_COUNTER_OFFSET + MIN_RUNS), Le(CALL_COUNTER_OFFSET + MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWithObjectAsReference, Repeat(3), [&] {
    {
        using namespace iox::units::duration_literals;
        PeriodicTaskTestType testType;
        concurrent::PeriodicTask<PeriodicTaskTestType&> sut("Test", 10_ms, testType);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWithCxxFunctionRef, Repeat(3), [&] {
    {
        using namespace iox::units::duration_literals;
        concurrent::PeriodicTask<cxx::function_ref<void()>> sut("Test", 10_ms, PeriodicTaskTestType::increment);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWithStdFunction, Repeat(3), [&] {
    {
        using namespace iox::units::duration_literals;
        concurrent::PeriodicTask<std::function<void()>> sut("Test", 10_ms, PeriodicTaskTestType::increment);

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
});

TIMING_TEST_F(PeriodicTask_test, PeriodicTaskWithMethodCallback, Repeat(3), ([&] {
    {
        using namespace iox::units::duration_literals;
        PeriodicTaskTestType testType;
        concurrent::PeriodicTask<cxx::MethodCallback<void>> sut{
            "Test", 10_ms, cxx::MethodCallback<void>{testType, &PeriodicTaskTestType::incrementMethod}};

        std::this_thread::sleep_for(SLEEP_TIME);
    }

    EXPECT_THAT(PeriodicTaskTestType::callCounter, AllOf(Ge(MIN_RUNS), Le(MAX_RUNS)));
}));
