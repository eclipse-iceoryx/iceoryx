// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_TESTING_ERROR_REPORTING_TESTING_SUPPORT_HPP
#define IOX_HOOFS_TESTING_ERROR_REPORTING_TESTING_SUPPORT_HPP

#include <gtest/gtest.h>

#include "iceoryx_hoofs/error_reporting/custom/default/error_handler.hpp"
#include "iceoryx_hoofs/testing/error_reporting/testing_error_handler.hpp"
#include "iox/static_lifetime_guard.hpp"

#include <thread>
#include <utility>

// NOLINTNEXTLINE(hicpp-deprecated-headers) required to work on some platforms
#include <setjmp.h>

namespace iox
{
namespace testing
{

using ErrorHandler = iox::StaticLifetimeGuard<iox::testing::TestErrorHandler>;

/// @brief indicates whether the test error handler registered a specific error
template <typename Code>
inline bool hasError(Code&& code)
{
    auto e = iox::er::toError(std::forward<Code>(code));
    return ErrorHandler::instance().hasError(e.code(), e.module());
}

/// @brief indicates whether the test error handler invoked panic
bool hasPanicked();

/// @brief indicates whether the test error handler registered any error
bool hasError();

/// @brief indicates whether the test error handler registered a precondition violation
bool hasPreconditionViolation();

/// @brief indicates whether the test error handler registered an assumption violation
bool hasAssumptionViolation();

/// @brief indicates whether the test error handler registered  violation (there are only two kinds).
bool hasViolation();

/// @brief indicates there is no error, violation or panic.
bool isInNormalState();

/// @brief runs testFunction in a testContext that can detect fatal failures;
/// runs in the same thread
/// @note uses setjmp/longjmp
template <typename Function, typename... Args>
inline void testContext(Function&& testFunction, Args&&... args)
{
    jmp_buf* buf = ErrorHandler::instance().prepareJump();

    if (buf == nullptr)
    {
        GTEST_FAIL() << "This should not fail! Incorrect usage!";
    };

    // setjmp must be called in a stackframe that still exists when longjmp is called
    // Therefore there cannot be a convenient abstraction that does not also
    // know the test function that is being called.
    // NOLINTNEXTLINE(cert-err52-cpp) exception cannot be used, required for testing to jump in case of failure
    if (setjmp(&(*buf)[0]) != ErrorHandler::instance().jumpIndicator())
    {
        testFunction(std::forward<Args>(args)...);
    }
}

/// @brief runs testFunction in a testContext that can detect fatal failures;
/// runs in a separate thread
/// @note uses a longjump inside the thread it runs the function in
template <typename Function, typename... Args>
inline void runInTestThread(Function&& testFunction, Args&&... args)
{
    // needed to infer the testContext arguments
    auto f = [&]() { testContext(std::forward<Function>(testFunction), std::forward<Args>(args)...); };

    std::thread t(f);
    if (t.joinable())
    {
        t.join();
    }
}

} // namespace testing
} // namespace iox

// Use macros to preserve line numbers in tests (failure case).

// ASSERT_* aborts test if the check fails.

// NOLINTBEGIN(cppcoreguidelines-macro-usage) macro required for source location in tests

#define IOX_TESTING_ASSERT_OK() ASSERT_TRUE(iox::testing::isInNormalState())

#define IOX_TESTING_ASSERT_NO_PANIC() ASSERT_FALSE(iox::testing::hasPanicked())

#define IOX_TESTING_ASSERT_PANIC() ASSERT_TRUE(iox::testing::hasPanicked())

#define IOX_TESTING_ASSERT_ERROR(code) ASSERT_TRUE(iox::testing::hasError(code))

#define IOX_TESTING_ASSERT_NO_ERROR() ASSERT_FALSE(iox::testing::hasError())

#define IOX_TESTING_ASSERT_VIOLATION()                                                                                 \
    ASSERT_TRUE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation())

#define IOX_TESTING_ASSERT_NO_VIOLATION()                                                                              \
    ASSERT_FALSE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation())

#define IOX_TESTING_ASSERT_PRECONDITION_VIOLATION() ASSERT_TRUE(iox::testing::hasPreconditionViolation())

#define IOX_TESTING_ASSERT_ASSUMPTION_VIOLATION() ASSERT_TRUE(iox::testing::hasAssumptionViolation())

// EXPECT_* continues with test if the check fails.

#define IOX_TESTING_EXPECT_OK() EXPECT_TRUE(iox::testing::isInNormalState())

#define IOX_TESTING_EXPECT_NO_PANIC() EXPECT_FALSE(iox::testing::hasPanicked())

#define IOX_TESTING_EXPECT_PANIC() EXPECT_TRUE(iox::testing::hasPanicked())

#define IOX_TESTING_EXPECT_ERROR(code) EXPECT_TRUE(iox::testing::hasError(code))

#define IOX_TESTING_EXPECT_NO_ERROR() EXPECT_FALSE(iox::testing::hasError())

#define IOX_TESTING_EXPECT_VIOLATION()                                                                                 \
    EXPECT_TRUE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation())

#define IOX_TESTING_EXPECT_NO_VIOLATION()                                                                              \
    EXPECT_FALSE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation())

#define IOX_TESTING_EXPECT_PRECONDITION_VIOLATION() EXPECT_TRUE(iox::testing::hasPreconditionViolation())

#define IOX_TESTING_EXPECT_ASSUMPTION_VIOLATION() EXPECT_TRUE(iox::testing::hasAssumptionViolation())

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
