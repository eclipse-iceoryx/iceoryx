// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/error_handling/error_handler.hpp"
#include "iceoryx_hoofs/testing/mocks/error_handler_mock.hpp"

#include "test.hpp"
#include <gtest/gtest-spi.h>

namespace
{
using namespace ::testing;

constexpr uint16_t MODULE_IDENTIFIER_OFFSET{42};

// clang-format off
#define TEST_ERRORS(error) \
    error(TEST__FOOBAR) \
    error(TEST__BARFOO)
// clang-format on

const char* TEST_ERROR_NAMES[] = {TEST_ERRORS(CREATE_ICEORYX_ERROR_STRING)};

enum class KnownError : uint32_t
{
    NO_ERROR = iox::USER_DEFINED_MODULE_IDENTIFIER << iox::ERROR_ENUM_OFFSET_IN_BITS,
    TEST_ERRORS(CREATE_ICEORYX_ERROR_ENUM)
};

enum class UnknownError : uint32_t
{
    NO_ERROR = (iox::USER_DEFINED_MODULE_IDENTIFIER + MODULE_IDENTIFIER_OFFSET) << iox::ERROR_ENUM_OFFSET_IN_BITS,
    TEST_ERRORS(CREATE_ICEORYX_ERROR_ENUM)
};

const char* asStringLiteral(const KnownError error) noexcept
{
    return TEST_ERROR_NAMES[iox::errorToStringIndex(error)];
}

const char* asStringLiteral(const UnknownError error) noexcept
{
    return TEST_ERROR_NAMES[iox::errorToStringIndex(error)];
}

TEST(ErrorHandlerMock_test, UnsettingTemporaryErrorHandlerWithKnownModuleWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b8e3974-42d3-45d9-88bd-07b5213c5b57");
    {
        auto errorHandlerGuard = iox::ErrorHandlerMock::setTemporaryErrorHandler<KnownError>(
            [&](const auto, const iox::ErrorLevel) { GTEST_FAIL() << "Temporary ErrorHandler shall not be called"; });
    }
    iox::errorHandler(KnownError::TEST__FOOBAR, iox::ErrorLevel::MODERATE);
}

TEST(ErrorHandlerMock_test, CallingErrorHandlerWithErrorOfKnownModuleAndDefaultLevelIsCaught)
{
    ::testing::Test::RecordProperty("TEST_ID", "988401f2-9eb0-4246-868c-3057dd6c2560");
    iox::optional<KnownError> detectedError;
    iox::optional<iox::ErrorLevel> detectedLevel;
    auto errorHandlerGuard =
        iox::ErrorHandlerMock::setTemporaryErrorHandler<KnownError>([&](const auto error, const iox::ErrorLevel level) {
            detectedError.emplace(error);
            detectedLevel.emplace(level);
        });

    iox::errorHandler(KnownError::TEST__FOOBAR);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(), Eq(KnownError::TEST__FOOBAR));
    ASSERT_TRUE(detectedLevel.has_value());
    EXPECT_THAT(detectedLevel.value(), Eq(iox::ErrorLevel::FATAL));
}

TEST(ErrorHandlerMock_test, CallingErrorHandlerWithErrorOfKnownModuleAndNonDefaultLevelIsCaught)
{
    ::testing::Test::RecordProperty("TEST_ID", "df123c54-a089-4515-87ff-cc16206d45af");
    iox::optional<KnownError> detectedError;
    iox::optional<iox::ErrorLevel> detectedLevel;
    auto errorHandlerGuard =
        iox::ErrorHandlerMock::setTemporaryErrorHandler<KnownError>([&](const auto error, const iox::ErrorLevel level) {
            detectedError.emplace(error);
            detectedLevel.emplace(level);
        });

    iox::errorHandler(KnownError::TEST__FOOBAR, iox::ErrorLevel::MODERATE);

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(), Eq(KnownError::TEST__FOOBAR));
    ASSERT_TRUE(detectedLevel.has_value());
    EXPECT_THAT(detectedLevel.value(), Eq(iox::ErrorLevel::MODERATE));
}

TEST(ErrorHandlerMock_test, CallingErrorHandlerWithErrorOfUnknownModuleCallsGTestFail)
{
    ::testing::Test::RecordProperty("TEST_ID", "10a50c0c-e21b-4c4f-ab0e-5ed2a4a14b3f");
    auto errorHandlerGuard =
        iox::ErrorHandlerMock::setTemporaryErrorHandler<KnownError>([&](const auto error, const iox::ErrorLevel level) {
            EXPECT_THAT(error, Eq(KnownError::TEST__FOOBAR));
            EXPECT_THAT(level, Eq(iox::ErrorLevel::FATAL));
        });
    EXPECT_FATAL_FAILURE({ iox::errorHandler(UnknownError::TEST__FOOBAR); },
                         "errorName: TEST__FOOBAR, expected error enum type: "
                             + std::to_string(iox::USER_DEFINED_MODULE_IDENTIFIER) + ", actual error enum type: "
                             + std::to_string(iox::USER_DEFINED_MODULE_IDENTIFIER + MODULE_IDENTIFIER_OFFSET));
}
} // namespace
