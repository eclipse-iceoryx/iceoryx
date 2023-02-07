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

#ifndef IOX_HOOFS_TESTING_FATAL_FAILURE_HPP
#define IOX_HOOFS_TESTING_FATAL_FAILURE_HPP

#include "iceoryx_hoofs/testing/mocks/error_handler_mock.hpp"
#include "iox/optional.hpp"
#include "test.hpp"

#include <atomic>
#include <csetjmp>
#include <thread>

namespace iox
{
namespace testing
{
namespace detail
{
/// @brief This function is the base for 'IOX_EXPECT_FATAL_FAILURE' and 'IOX_EXPECT_NO_FATAL_FAILURE' and should not be
/// used by its own. The function only works in combination with the iceoryx error handler.
/// @tparam[in] ErrorType The error type which is expected, e.g. 'iox::HoofsError'
/// @param[in] testFunction This function will be executed as SUT and might call the error handler with a 'FATAL' error
/// level
/// @param[in] onFatalFailurePath This function will be executed on the failure path after the failure was detected
/// @param[in] onNonFatalFailurePath This function will be executed on the non-failure path if no failure was detected
/// @return true if a fatal failure occurs, false otherwise
template <typename ErrorType>
bool FATAL_FAILURE_TEST(const std::function<void()>& testFunction,
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

/// @brief This function is used in cases a fatal failure is expected. The function only works in combination with the
/// iceoryx error handler.
/// @tparam[in] ErrorType The error type which is expected, e.g. 'iox::HoofsError'
/// @param[in] testFunction This function will be executed as SUT and is not expected to call the error handler
/// @param[in] expectedError The error value which triggered the fatal failure
/// @return true if a fatal failure occurs, false otherwise
template <typename ErrorType>
bool EXPECT_FATAL_FAILURE(const std::function<void()>& testFunction,
                          const ErrorType expectedError,
                          const iox::ErrorLevel)
{
    return detail::FATAL_FAILURE_TEST<ErrorType>(
        testFunction,
        [&](const auto error, const auto errorLevel) {
            EXPECT_THAT(error, ::testing::Eq(expectedError));
            EXPECT_THAT(errorLevel, ::testing::Eq(iox::ErrorLevel::FATAL));
        },
        [&] { GTEST_FAIL() << "Expected fatal failure but execution continued!"; });
}

/// @brief This function is used in cases no fatal failure is expected but could potentially occur. The function only
/// works in combination with the iceoryx error handler.
/// @tparam[in] ErrorType The error type which is expected if the test fails, e.g. 'iox::HoofsError'
/// @param[in] testFunction This function will be executed as SUT and is not expected to call the error handler
/// @return true if no fatal failure occurs, false otherwise
template <typename ErrorType>
bool IOX_EXPECT_NO_FATAL_FAILURE(const std::function<void()>& testFunction)
{
    return !detail::FATAL_FAILURE_TEST<ErrorType>(
        testFunction,
        [&](const auto error, const auto errorLevel) {
            GTEST_FAIL() << "Expected no fatal failure but execution failed! Error code: " << error
                         << "; Error level: " << errorLevel;
        },
        [&] {});
    return false;
}

} // namespace testing
} // namespace iox

#endif // IOX_HOOFS_TESTING_FATAL_FAILURE_HPP
