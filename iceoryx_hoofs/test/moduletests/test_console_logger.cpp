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

#include "iceoryx_hoofs/log/platform_building_blocks/console_logger.hpp"

#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "test.hpp"

#include <cstdio>
#include <iostream>

namespace
{
using namespace ::testing;

class LoggerSUT : public iox::pbb::ConsoleLogger
{
  public:
    using iox::pbb::ConsoleLogger::flush;
    using iox::pbb::ConsoleLogger::logString;
};

/// @todo todo iox-#1345 this test will be done via the integration tests with launch testing once "RouDi is ready for
/// clients" will be printed via the logger
#if 0
TEST(ConsoleLogger_test, TestOutput)
{
    ::testing::Test::RecordProperty("TEST_ID", "67f1dac5-b425-414a-9690-268ecb06c1ee");

    constexpr const char* LOG_FILE_NAME{"iceoryx_console_logger_output_test.txt"};
    constexpr const char* LOG_MESSAGE{"All glory to the hypnotoad!"};

    // #### redirect stdout ####
    // to be able to check if the logger writes to stdout we need to redirect the output to a file
    // save original stdout
    auto stdoutOriginal = dup(STDOUT_FILENO);

    int logFd = open(LOG_FILE_NAME, O_CREAT | O_WRONLY | O_TRUNC, 0664);
    ASSERT_GE(logFd, 0);
    ASSERT_GE(dup2(logFd, STDOUT_FILENO), 0);
    // #### redirect stdout ####

    LoggerSUT sut;
    // createLogMessageHeader is intentionally not called;
    // it creates a timestamp which needs to be removed anyway;
    // logString is sufficient to ensure that the output works
    sut.logString(LOG_MESSAGE);
    sut.flush();

    // #### restore stdout ####
    close(logFd);

    ASSERT_GE(dup2(stdoutOriginal, STDOUT_FILENO), 0);
    // #### restore stdout ####

    std::fstream fileStream;
    fileStream.open(LOG_FILE_NAME, std::fstream::in);
    ASSERT_TRUE(fileStream.is_open());
    std::string logString;
    ASSERT_TRUE(std::getline(fileStream, logString));

    // assure that there a no further strings
    EXPECT_THAT(logString, StrEq(LOG_MESSAGE));
    ASSERT_FALSE(std::getline(fileStream, logString));
}
#endif

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
