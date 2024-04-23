// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/log/building_blocks/console_logger.hpp"

#include "iceoryx_platform/fcntl.hpp"
#include "iox/logging.hpp"
#include "test.hpp"

#include <cstdio>
#include <iostream>

namespace
{
using namespace ::testing;

class LoggerSUT : public iox::log::ConsoleLogger
{
  public:
    using iox::log::ConsoleLogger::flush;
    using iox::log::ConsoleLogger::logString;
};

TEST(ConsoleLogger_test, TestOutput)
{
    ::testing::Test::RecordProperty("TEST_ID", "67f1dac5-b425-414a-9690-268ecb06c1ee");

    GTEST_SKIP() << "This is tested via the integration tests by launch testing waiting for the 'RouDi is ready for "
                    "clients' string";
}

/// @note the actual log API is tested via the LogStream tests

TEST(ConsoleLogger_test, SettingTheLogLevelWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e8225d29-ee35-4864-8528-b1e290a83311");
    constexpr auto LOG_LEVEL{iox::log::LogLevel::INFO};
    EXPECT_THAT(LoggerSUT::getLogLevel(), Ne(LOG_LEVEL));

    LoggerSUT::setLogLevel(LOG_LEVEL);
    EXPECT_THAT(LoggerSUT::getLogLevel(), Eq(LOG_LEVEL));
}

} // namespace
