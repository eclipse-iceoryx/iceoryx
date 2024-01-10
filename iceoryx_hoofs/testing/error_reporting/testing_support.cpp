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

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"

namespace iox
{
namespace testing
{

bool hasPanicked()
{
    return ErrorHandler::instance().hasPanicked();
}

bool hasError()
{
    return ErrorHandler::instance().hasError();
}

bool hasAssertViolation()
{
    auto code = iox::er::Violation(iox::er::ViolationErrorCode::ASSERT_VIOLATION).code();
    return ErrorHandler::instance().hasViolation(code);
}

bool hasEnforceViolation()
{
    auto code = iox::er::Violation(iox::er::ViolationErrorCode::ENFORCE_VIOLATION).code();
    return ErrorHandler::instance().hasViolation(code);
}

bool hasViolation()
{
    return hasEnforceViolation() || hasAssertViolation();
}

bool isInNormalState()
{
    return !(hasPanicked() || hasError() || hasViolation());
}

void runInTestThread(const function_ref<void()> testFunction)
{
    auto t = std::thread([&]() {
        auto successfullRun = ErrorHandler::instance().fatalFailureTestContext(testFunction);
        if (!successfullRun)
        {
            GTEST_FAIL() << "This should not fail! Incorrect usage!";
        }
    });

    if (t.joinable())
    {
        t.join();
    }
}

} // namespace testing
} // namespace iox
