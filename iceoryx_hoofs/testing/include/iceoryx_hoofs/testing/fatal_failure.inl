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

#ifndef IOX_HOOFS_TESTING_FATAL_FAILURE_INL
#define IOX_HOOFS_TESTING_FATAL_FAILURE_INL

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"

namespace iox
{
namespace testing
{
template <typename ErrorType>
inline bool IOX_EXPECT_FATAL_FAILURE(const function_ref<void()> testFunction,
                                     const ErrorType expectedError [[maybe_unused]])
{
    iox::testing::ErrorHandler::instance().reset();
    runInTestThread([&] { testFunction(); });
    IOX_TESTING_EXPECT_PANIC();

    /// @todo iox-#1032 'hasViolation' should not be necessary
    auto hasExpectedError =
        iox::testing::hasError(expectedError) || iox::testing::hasViolation() || iox::testing::hasPanicked();
    EXPECT_TRUE(hasExpectedError);
    return hasExpectedError;
}

inline bool IOX_EXPECT_NO_FATAL_FAILURE(const function_ref<void()> testFunction)
{
    runInTestThread([&] { testFunction(); });
    return !iox::testing::hasPanicked();
}

} // namespace testing
} // namespace iox

#endif // IOX_HOOFS_TESTING_FATAL_FAILURE_INL
