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

#include "iceoryx_hoofs/testing/fatal_failure.hpp"

namespace iox
{
namespace testing
{
namespace detail
{
template <typename ErrorType>
inline bool
IOX_FATAL_FAILURE_TEST(const std::function<void()>& testFunction,
                       const std::function<void(const ErrorType, const iox::ErrorLevel)>& onFatalFailurePath,
                       const std::function<void()>& onNonFatalFailurePath)
{
    std::atomic<bool> hasFatalFailure{false};
    auto th = std::thread([&] {
        constexpr int JMP_VALUE{1};
        std::jmp_buf jmpBuffer;

        optional<ErrorType> detectedError;
        optional<iox::ErrorLevel> detectedErrorLevel;

        auto errorHandlerGuard =
            iox::ErrorHandlerMock::setTemporaryErrorHandler<ErrorType>([&](const auto error, const auto errorLevel) {
                detectedError.emplace(error);
                detectedErrorLevel.emplace(errorLevel);

                // NOLINTNEXTLINE(cert-err52-cpp) exception cannot be used and longjmp/setjmp is a working fallback
                std::longjmp(&jmpBuffer[0], JMP_VALUE);
            });

        // NOLINTNEXTLINE(cert-err52-cpp) exception cannot be used and longjmp/setjmp is a working fallback
        if (setjmp(&jmpBuffer[0]) == JMP_VALUE)
        {
            hasFatalFailure = true;
            // using value directly is save since this path is only executed if the error handler was called and the
            // respective values were set
            onFatalFailurePath(detectedError.value(), detectedErrorLevel.value());
            return;
        }

        testFunction();

        onNonFatalFailurePath();
    });

    th.join();

    return hasFatalFailure.load(std::memory_order_relaxed);
}
} // namespace detail

template <typename ErrorType>
inline bool IOX_EXPECT_FATAL_FAILURE(const std::function<void()>& testFunction, const ErrorType expectedError)
{
    return detail::IOX_FATAL_FAILURE_TEST<ErrorType>(
        testFunction,
        [&](const auto error, const auto errorLevel) {
            EXPECT_THAT(error, ::testing::Eq(expectedError));
            EXPECT_THAT(errorLevel, ::testing::Eq(iox::ErrorLevel::FATAL));
        },
        [&] { GTEST_FAIL() << "Expected fatal failure but execution continued!"; });
}

template <typename ErrorType>
inline bool IOX_EXPECT_NO_FATAL_FAILURE(const std::function<void()>& testFunction)
{
    return !detail::IOX_FATAL_FAILURE_TEST<ErrorType>(
        testFunction,
        [&](const auto error, const auto errorLevel) {
            GTEST_FAIL() << "Expected no fatal failure but execution failed! Error code: "
                         << static_cast<uint64_t>(error) << "; Error level: " << static_cast<uint64_t>(errorLevel);
        },
        [&] { GTEST_SUCCEED() << "Non-fatal path taken!"; });
    return false;
}

} // namespace testing
} // namespace iox

#endif // IOX_HOOFS_TESTING_FATAL_FAILURE_INL
