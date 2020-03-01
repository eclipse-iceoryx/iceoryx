// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

/// @brief This headers provides TIMING_TEST unit test infrastructure. The idea
///         is that a timing test is running multiple times and if in one of the
///         repetitions all results of the test are successful then the timing
///         test itself is successful.
///
///        You can get to know them in praxis in the test_posix_timer.cpp unit test
///        for instance.
///
/// @code
/// class MyClass_test : public Test {};
///
/// TIMING_TEST_F(MyClass_test, WaitForSleep, Repeat(3), [&]{
///   std::atomic_bool threadFinished{false};
///   std::thread t([&]{ sleep(2); threadFinished = true; });
///
///   TIMING_TEST_EXPECT_FALSE(threadFinished.load());
///   sleep(1);
///   TIMING_TEST_EXPECT_FALSE(threadFinished.load());
///   sleep(2);
///   TIMING_TEST_EXPECT_TRUE(threadFinished.load());
///   t.join();
///
///   TIMING_TEST_END();
/// });
/// @endcode
///
/// @details
///     Available testing verificators
///        TIMING_TEST_EXPECT_TRUE(value);
///        TIMING_TEST_EXPECT_FALSE(value);
///        TIMING_TEST_ASSERT_TRUE(value);
///        TIMING_TEST_ASSERT_FALSE(value);
///
///     Available test types
///         TIMING_TEST_F -> maps to TEST_F
///         TIMING_TEST_P -> maps to TEST_P
///
///     If you would like to disable timing tests you can start your unit
///     test like:
///      # ./myUnitTest --gtest_filter="-*TimingTest*"
///
///     Or if you would like only timing test you can use the following
///     approach:
///      # ./myUnitTest --gtest_filter="*TimingTest*"

#define TIMING_TEST_CONSTRUCT(Name, Case, Repetitions, Test, GTestType)                                                \
    GTestType(Name, TimingTest_##Case)                                                                                 \
    {                                                                                                                  \
        std::atomic_bool __timingTestResult__{true};                                                                   \
        std::string __errorMessages__;                                                                                 \
        __PerformingTimingTest__(Test, Repetitions);                                                                   \
        EXPECT_TRUE(__timingTestResult__.load());                                                                      \
        if (!__timingTestResult__.load())                                                                              \
        {                                                                                                              \
            std::cout << "\n" << __errorMessages__ << std::endl;                                                       \
        }                                                                                                              \
    }

#define TIMING_TEST_F(Name, Case, Repetitions, Test) TIMING_TEST_CONSTRUCT(Name, Case, Repetitions, Test, TEST_F)
#define TIMING_TEST_P(Name, Case, Repetitions, Test) TIMING_TEST_CONSTRUCT(Name, Case, Repetitions, Test, TEST_P)

#define TIMING_TEST_END() return __timingTestResult__.load();

#define TIMING_TEST_EXPECT_TRUE(value)                                                                                 \
    __errorMessages__ += __VerifyTimingTestResult__(__FILE__, __LINE__, #value, value, true, __timingTestResult__)
#define TIMING_TEST_EXPECT_FALSE(value)                                                                                \
    __errorMessages__ += __VerifyTimingTestResult__(__FILE__, __LINE__, #value, value, false, __timingTestResult__)
#define TIMING_TEST_ASSERT_TRUE(value)                                                                                 \
    TIMING_TEST_EXPECT_TRUE(value);                                                                                    \
    if (!__timingTestResult__.load())                                                                                  \
    {                                                                                                                  \
        return false;                                                                                                  \
    }
#define TIMING_TEST_ASSERT_FALSE(value)                                                                                \
    TIMING_TEST_EXPECT_FALSE(value);                                                                                   \
    if (!__timingTestResult__.load())                                                                                  \
    {                                                                                                                  \
        return false;                                                                                                  \
    }


#define Repeat(n) n

inline bool __PerformingTimingTest__(const std::function<bool()>& testCallback, const uint64_t repetitions) noexcept
{
    for (uint64_t i = 0; i < repetitions; ++i)
    {
        if (testCallback())
        {
            return true;
        }
    }
    return false;
}

inline std::string __VerifyTimingTestResult__(const char* file,
                                              const int line,
                                              const char* valueStr,
                                              const bool value,
                                              const bool expected,
                                              std::atomic_bool& result) noexcept
{
    std::string errorMessage;
    if (value != expected)
    {
        errorMessage += "Timing Test failure in:\n";
        errorMessage += std::string(file) + ":" + std::to_string(line) + "\n";
        errorMessage += "Value of: " + std::string(valueStr) + " should be true\n";
        result.store(false);
    }
    return errorMessage;
}

