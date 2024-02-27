// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI. All rights reserved.
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

#include "iceoryx_hoofs/testing/testing_logger.hpp"
#include "iceoryx_platform/getopt.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iceoryx_posh/roudi/roudi_config_toml_file_provider.hpp"
#include "iox/detail/convert.hpp"
#include "iox/logging.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "test.hpp"

#include <regex>

namespace
{
using namespace ::testing;

using iox::roudi::IceOryxRouDiApp;
using namespace iox::testing;
using namespace iox::config;
using namespace iox;

class IceoryxRoudiApp_Child : public IceOryxRouDiApp
{
  public:
    IceoryxRoudiApp_Child(const IceoryxConfig& config)
        : IceOryxRouDiApp(config)
    {
    }

    bool getVariableRun()
    {
        return m_run;
    }

    iox::log::LogLevel getLogLevel()
    {
        return m_config.logLevel;
    }

    roudi::MonitoringMode getMonitoringMode()
    {
        return m_config.monitoringMode;
    }

    void setVariableRun(bool condition)
    {
        m_run = condition;
    }

    uint8_t callRun() noexcept
    {
        return run();
    }
};

/// @brief Test goal: This file tests class roudi app
class IceoryxRoudiApp_test : public Test
{
  public:
    CmdLineParserConfigFileOption cmdLineParser;

    void TearDown() override
    {
        // Reset optind to be able to parse again
        optind = 0;
    };

    const std::regex colorCode{"\\x1B\\[([0-9]*;?)*m"};
};

TEST_F(IceoryxRoudiApp_test, VerifyConstructorIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "530346f1-7405-4640-9f5f-37e45073f95d");
    constexpr uint8_t NUMBER_OF_ARGS{1U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    args[0] = &appName[0];

    auto cmdLineArgs = cmdLineParser.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(cmdLineArgs.has_error());

    IceoryxRoudiApp_Child roudi(iox::IceoryxConfig().setDefaults());

    EXPECT_TRUE(roudi.getVariableRun());
    EXPECT_EQ(roudi.getLogLevel(), iox::log::LogLevel::INFO);
    EXPECT_EQ(roudi.getMonitoringMode(), roudi::MonitoringMode::OFF);
}

TEST_F(IceoryxRoudiApp_test, CreateTwoRoudiAppIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "a095ea92-be03-4157-959a-72b1cb285b46");

    IceoryxRoudiApp_Child roudi(iox::IceoryxConfig().setDefaults());

    IceoryxRoudiApp_Child roudiTest(iox::IceoryxConfig().setDefaults());

    EXPECT_TRUE(roudiTest.getVariableRun());
}

TEST_F(IceoryxRoudiApp_test, VerifyRunMethodWithFalseConditionReturnExitSuccess)
{
    ::testing::Test::RecordProperty("TEST_ID", "e25e69a5-4d41-4020-85ca-9f585ac09919");

    IceoryxRoudiApp_Child roudi(iox::IceoryxConfig().setDefaults());

    roudi.setVariableRun(false);

    uint8_t result = roudi.callRun();

    EXPECT_EQ(result, EXIT_SUCCESS);
}

TEST_F(IceoryxRoudiApp_test, VerifyConstructorWithEmptyConfigSetRunVariableToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a193ef0-b6c5-4e5b-998d-7f86102814e0");

    const std::string expectedOutput = "A IceoryxConfig without segments was specified! Please provide a valid config!";

    iox::IceoryxConfig config;

    IceoryxRoudiApp_Child roudi(config);

    EXPECT_FALSE(roudi.getVariableRun());

    iox::testing::TestingLogger::checkLogMessageIfLogLevelIsSupported(
        iox::log::LogLevel::ERROR, [&](const auto& logMessages) {
            ASSERT_THAT(logMessages.size(), Eq(1U));
            EXPECT_THAT(logMessages[0], HasSubstr(expectedOutput));
        });
}

TEST_F(IceoryxRoudiApp_test, VerifyConstructorUsingConfigWithSegmentWithoutMemPoolSetRunVariableToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "542ff7f7-9365-40a4-a7ed-e67ba5735b9e");

    const std::string expectedOutput =
        "A IceoryxConfig with segments without mempools was specified! Please provide a valid config!";

    iox::mepoo::MePooConfig mempoolConfig;
    auto currentGroup = PosixGroup::getGroupOfCurrentProcess();

    iox::IceoryxConfig config;

    config.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mempoolConfig});

    IceoryxRoudiApp_Child roudi(config);

    EXPECT_FALSE(roudi.getVariableRun());

    iox::testing::TestingLogger::checkLogMessageIfLogLevelIsSupported(
        iox::log::LogLevel::ERROR, [&](const auto& logMessages) {
            ASSERT_THAT(logMessages.size(), Eq(1U));
            EXPECT_THAT(logMessages[0], HasSubstr(expectedOutput));
        });
}
} // namespace
