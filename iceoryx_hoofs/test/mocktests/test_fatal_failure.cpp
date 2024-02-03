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

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iox/assertions.hpp"
#include "iox/detail/hoofs_error_reporting.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace ::iox::testing;

TEST(FatalFailure, UsingExpectFatalFailureWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "26393210-9738-462f-9d35-dbd53fbae9d2");

    auto hasFatalFailure = IOX_EXPECT_FATAL_FAILURE([&] { IOX_ENFORCE(false, ""); }, iox::er::ENFORCE_VIOLATION);

    EXPECT_TRUE(hasFatalFailure);
}

TEST(FatalFailure, UsingExpectNoFatalFailureWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "80bf8050-bfaa-4482-b69c-d0c80699bd4b");

    auto hasNoFatalFailure = IOX_EXPECT_NO_FATAL_FAILURE([&] {});

    EXPECT_TRUE(hasNoFatalFailure);
}
} // namespace
