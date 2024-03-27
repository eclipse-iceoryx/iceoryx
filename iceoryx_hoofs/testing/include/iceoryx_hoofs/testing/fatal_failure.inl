// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_TESTING_FATAL_FAILURE_INL
#define IOX_HOOFS_TESTING_FATAL_FAILURE_INL

#include "iceoryx_hoofs/testing/fatal_failure.hpp"

namespace iox
{
namespace testing
{
template <typename ErrorType>
// NOLINTJUSTIFICATION The complexity comes from the expanded macros; without the expansions the function is quite readable
// NOLINTNEXTLINE(readability-function-size, readability-function-cognitive-complexity)
inline bool IOX_EXPECT_FATAL_FAILURE(const function_ref<void()> testFunction,
                                     const ErrorType expectedError [[maybe_unused]])
{
    iox::testing::ErrorHandler::instance().reset();
    runInTestThread([&] { testFunction(); });
    IOX_TESTING_EXPECT_PANIC();
    auto hasPanicked = iox::testing::hasPanicked();

    auto hasExpectedError{false};
    if constexpr (std::is_same_v<ErrorType, iox::er::FatalKind>)
    {
        hasExpectedError = hasPanicked;
        if (!hasExpectedError)
        {
            IOX_LOG(ERROR, "Expected '" << iox::er::FatalKind::name << "' but it did not happen!");
        }
    }
    else if constexpr (std::is_same_v<ErrorType, iox::er::EnforceViolationKind>)
    {
        hasExpectedError = iox::testing::hasEnforceViolation();
        if (!hasExpectedError)
        {
            IOX_LOG(ERROR, "Expected '" << iox::er::EnforceViolationKind::name << "' but it did not happen!");
        }
    }
    else if constexpr (std::is_same_v<ErrorType, iox::er::AssertViolationKind>)
    {
        hasExpectedError = iox::testing::hasAssertViolation();
        if (!hasExpectedError)
        {
            IOX_LOG(ERROR, "Expected '" << iox::er::AssertViolationKind::name << "' but it did not happen!");
        }
    }
    else
    {
        hasExpectedError = iox::testing::hasError(expectedError);
        if (!hasExpectedError)
        {
            IOX_LOG(ERROR, "Expected an '" << expectedError << "' error but it did not happen!");
        }
    }

    EXPECT_TRUE(hasExpectedError);
    return hasExpectedError && hasPanicked;
}

inline bool IOX_EXPECT_NO_FATAL_FAILURE(const function_ref<void()> testFunction)
{
    runInTestThread([&] { testFunction(); });
    return !iox::testing::hasPanicked();
}

} // namespace testing
} // namespace iox

#endif // IOX_HOOFS_TESTING_FATAL_FAILURE_INL
