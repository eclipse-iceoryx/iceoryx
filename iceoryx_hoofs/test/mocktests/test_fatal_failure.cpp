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

#include "iceoryx_hoofs/cxx/requires.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace ::iox::testing;

TEST(FatalFailure, TriggeringFatalFailureIsDetectedAndDoesNotTerminate)
{
    ::testing::Test::RecordProperty("TEST_ID", "5463f1c9-eb30-4fd1-85ce-351f03c37fe0");

    auto hasFatalFailure = detail::IOX_FATAL_FAILURE_TEST<iox::HoofsError>(
        [&] { iox::cxx::Expects(false); },
        [&](const auto error, const auto errorLevel) {
            EXPECT_THAT(error, Eq(iox::HoofsError::EXPECTS_ENSURES_FAILED));
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        },
        [&] { GTEST_FAIL() << "This is the non-fatal path and should therefore not be called"; });

    EXPECT_TRUE(hasFatalFailure);
}

TEST(FatalFailure, ExpectingFatalFailureWhichDoesNotOccurIsDetected)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6c1d4f2-cafe-45e3-bbb7-c1373b2e15a8");

    auto hasFatalFailure = detail::IOX_FATAL_FAILURE_TEST<iox::HoofsError>(
        [&] {},
        [&](const auto, const auto) { GTEST_FAIL() << "This is the fatal path and should therefore not be called"; },
        [&] { GTEST_SUCCEED() << "This is the non-fatal path and should be called"; });

    EXPECT_FALSE(hasFatalFailure);
}

TEST(FatalFailure, UsingExpectFatalFailureWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "26393210-9738-462f-9d35-dbd53fbae9d2");

    auto hasFatalFailure = IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { iox::cxx::Expects(false); },
                                                                     iox::HoofsError::EXPECTS_ENSURES_FAILED);

    EXPECT_TRUE(hasFatalFailure);
}

TEST(FatalFailure, UsingExpectNoFatalFailureWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "80bf8050-bfaa-4482-b69c-d0c80699bd4b");

    auto hasNoFatalFailure = IOX_EXPECT_NO_FATAL_FAILURE<iox::HoofsError>([&] {});

    EXPECT_TRUE(hasNoFatalFailure);
}
} // namespace
