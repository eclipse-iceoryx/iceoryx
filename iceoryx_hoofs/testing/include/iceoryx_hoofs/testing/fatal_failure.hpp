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

#include "iceoryx_hoofs/testing/mocks/error_handler_mock.hpp"
#include "test.hpp"

#include <csetjmp>
#include <thread>

namespace iox
{
namespace testing
{
template <typename ErrorType>
void EXPECT_FATAL_FAILURE(const std::function<void()>& testFunction,
                          const ErrorType expectedError,
                          const iox::ErrorLevel expectedErrorLevel)
{
    auto th = std::thread([&] {
        constexpr int JMP_VALUE{1};
        std::jmp_buf jmpBuffer;

        auto errorHandlerGuard =
            iox::ErrorHandlerMock::setTemporaryErrorHandler<ErrorType>([&](const auto error, const auto errorLevel) {
                EXPECT_THAT(error, ::testing::Eq(expectedError));
                EXPECT_THAT(errorLevel, ::testing::Eq(expectedErrorLevel));

                // NOLINTNEXTLINE(cert-err52-cpp) exception cannot be used and longjmp/setjmp is a working fallback
                std::longjmp(&jmpBuffer[0], JMP_VALUE);
            });

        // NOLINTNEXTLINE(cert-err52-cpp) exception cannot be used and longjmp/setjmp is a working fallback
        if (setjmp(&jmpBuffer[0]) == JMP_VALUE)
        {
            return;
        }

        testFunction();

        GTEST_FAIL() << "Expected fatal failure but execution continued!";
    });

    th.join();
}
} // namespace testing
} // namespace iox
