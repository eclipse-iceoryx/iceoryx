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

#ifndef IOX_HOOFS_TESTING_ERROR_REPORTING_TEST_SUPPORT_HPP
#define IOX_HOOFS_TESTING_ERROR_REPORTING_TEST_SUPPORT_HPP

#include <gtest/gtest.h>

#include "iceoryx_hoofs/error_reporting/custom/default/error_handler.hpp"
#include "iceoryx_hoofs/testing/error_reporting/test_error_handler.hpp"
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
    auto e = iox::err::toError(std::forward<Code>(code));
    return ErrorHandler::instance().hasError(e.code(), e.module());
}

/// @brief indicates whether the test error handler invoked panic
bool hasPanicked();

/// @brief indicates whether the test error handler registered any error
bool hasError();

/// @brief indicates whether the test error handler registered a precondition violation
bool hasPreconditionViolation();

/// @brief indicates whether the test error handler registered a precondition violation
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
        // should not fail with correct usage
        GTEST_FAIL();
    };

    // setjmp must be called in a stackframe that still exists when longjmp is called
    // Therefore there cannot be a convenient abstraction that does not also
    // know the test function that is being called.
    // NOLINTNEXTLINE(cert-err52-cpp) required for testing to jump in case of failure
    if (setjmp(*buf) != ErrorHandler::instance().jumpIndicator())
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

#define ASSERT_IOX_OK()                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::isInNormalState());                                                                  \
    } while (false)

#define ASSERT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(iox::testing::hasPanicked());                                                                     \
    } while (false)

#define ASSERT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::hasPanicked());                                                                      \
    } while (false)

#define ASSERT_ERROR(code)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::hasError(code));                                                                     \
    } while (false)

#define ASSERT_NO_ERROR()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(iox::testing::hasError());                                                                        \
    } while (false)

#define EXPECT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_FALSE(iox::testing::hasPanicked());                                                                     \
    } while (false)

#define ASSERT_VIOLATION()                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation());               \
    } while (false)

#define ASSERT_NO_VIOLATION()                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation());              \
    } while (false)

#define ASSERT_PRECONDITION_VIOLATION()                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::hasPreconditionViolation());                                                         \
    } while (false)

#define ASSERT_ASSUMPTION_VIOLATION()                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::testing::hasAssumptionViolation());                                                           \
    } while (false)

// EXPECT_* continues with test if the check fails.

#define EXPECT_IOX_OK()                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::isInNormalState());                                                                  \
    } while (false)

#define EXPECT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::hasPanicked());                                                                      \
    } while (false)

#define EXPECT_ERROR(code)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::hasError(code));                                                                     \
    } while (false)

#define EXPECT_NO_ERROR()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_FALSE(iox::testing::hasError());                                                                        \
    } while (false)

#define EXPECT_VIOLATION()                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation());               \
    } while (false)

#define EXPECT_NO_VIOLATION()                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_FALSE(iox::testing::hasPreconditionViolation() || iox::testing::hasAssumptionViolation());              \
    } while (false)

#define EXPECT_PRECONDITION_VIOLATION()                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::hasPreconditionViolation());                                                         \
    } while (false)

#define EXPECT_ASSUMPTION_VIOLATION()                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_TRUE(iox::testing::hasAssumptionViolation());                                                           \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
