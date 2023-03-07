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

#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"
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

using TestErrorHandler = iox::StaticLifetimeGuard<iox::testing::TestHandler>;

/// @brief indicates whether the test error handler invoked panic
inline bool hasPanicked()
{
    return TestErrorHandler::instance().hasPanicked();
}

/// @brief indicates whether the test error handler registered a specific error
template <typename Code>
inline bool hasError(Code&& code)
{
    auto e = iox::err::toError(std::forward<Code>(code));
    return TestErrorHandler::instance().hasError(e.code());
}

/// @brief indicates whether the test error handler registered any error
inline bool hasError()
{
    return TestErrorHandler::instance().hasError();
}

/// @brief runs testFunction in a testContext that can detect fatal failures;
/// runs in the same thread
/// @note uses a longjump
template <typename Function, typename... Args>
inline void testContext(Function&& testFunction, Args&&... args)
{
    jmp_buf* buf = TestErrorHandler::instance().prepareJump();

    if (buf == nullptr)
    {
        // should not fail with correct usage
        GTEST_FAIL();
    };

    // setjmp must be called in a stackframe that still exists when longjmp is called
    // Therefore there cannot be a convenient abstraction that does not also
    // know the test function that is being called.
    // NOLINTNEXTLINE
    if (setjmp(*buf) != TestErrorHandler::instance().jumpIndicator())
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
#endif

#define EXPECT_NO_ERROR()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        EXPECT_FALSE(iox::testing::hasError());                                                                        \
    } while (false)
