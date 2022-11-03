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
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iceoryx_posh/roudi/roudi_config_toml_file_provider.hpp"

#include "test.hpp"

#include <regex>

namespace
{
using namespace ::testing;

using iox::roudi::IceOryxRouDiApp;
using namespace iox::config;
using namespace iox;

class IceoryxRoudiApp_Child : public IceOryxRouDiApp
{
  public:
    IceoryxRoudiApp_Child(const config::CmdLineArgs_t& cmdLineArgs, const RouDiConfig_t& roudiConfig)
        : IceOryxRouDiApp(cmdLineArgs, roudiConfig)
    {
    }

    bool getVariableRun()
    {
        return m_run;
    }

    iox::log::LogLevel getLogLevel()
    {
        return m_logLevel;
    }

    roudi::MonitoringMode getMonitoringMode()
    {
        return m_monitoringMode;
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

    IceoryxRoudiApp_Child roudi(cmdLineArgs.value(), iox::RouDiConfig_t().setDefaults());

    EXPECT_TRUE(roudi.getVariableRun());
    EXPECT_EQ(roudi.getLogLevel(), iox::log::LogLevel::WARN);
    EXPECT_EQ(roudi.getMonitoringMode(), roudi::MonitoringMode::OFF);
}

TEST_F(IceoryxRoudiApp_test, CreateTwoRoudiAppIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "a095ea92-be03-4157-959a-72b1cb285b46");
    constexpr uint8_t NUMBER_OF_ARGS{1U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    args[0] = &appName[0];

    auto cmdLineArgs = cmdLineParser.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(cmdLineArgs.has_error());

    IceoryxRoudiApp_Child roudi(cmdLineArgs.value(), iox::RouDiConfig_t().setDefaults());

    IceoryxRoudiApp_Child roudiTest(cmdLineArgs.value(), iox::RouDiConfig_t().setDefaults());

    EXPECT_TRUE(roudiTest.getVariableRun());
}

TEST_F(IceoryxRoudiApp_test, VerifyRunMethodWithFalseConditionReturnExitSuccess)
{
    ::testing::Test::RecordProperty("TEST_ID", "e25e69a5-4d41-4020-85ca-9f585ac09919");
    constexpr uint8_t NUMBER_OF_ARGS{1U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    args[0] = &appName[0];

    auto cmdLineArgs = cmdLineParser.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(cmdLineArgs.has_error());

    IceoryxRoudiApp_Child roudi(cmdLineArgs.value(), iox::RouDiConfig_t().setDefaults());

    roudi.setVariableRun(false);

    uint8_t result = roudi.callRun();

    EXPECT_EQ(result, EXIT_SUCCESS);
}

TEST_F(IceoryxRoudiApp_test, ConstructorCalledWithArgUniqueIdTwoTimesReturnError)
{
    ::testing::Test::RecordProperty("TEST_ID", "72ec1d9e-7e29-4a9b-a8dd-cb4de82683cb");
    constexpr uint8_t NUMBER_OF_ARGS{3U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "--unique-roudi-id";
    char value[] = "4242";
    args[0] = &appName[0];
    args[1] = &option[0];
    args[2] = &value[0];

    auto cmdLineArgs = cmdLineParser.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(cmdLineArgs.has_error());

    iox::cxx::optional<iox::PoshError> detectedError;
    iox::cxx::optional<iox::ErrorLevel> detectedErrorLevel;
    auto errorHandlerGuard = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::PoshError>(
        [&](const iox::PoshError error, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            detectedErrorLevel.emplace(errorLevel);
        });

    IceoryxRoudiApp_Child roudi(cmdLineArgs.value(), iox::RouDiConfig_t().setDefaults());
    // we don't know if setUniqueRouDiId was called before, therefore ignore any error
    detectedError.reset();
    detectedErrorLevel.reset();

    IceoryxRoudiApp_Child roudiTest(cmdLineArgs.value(), iox::RouDiConfig_t().setDefaults());

    // now we know that setUniqueRouDiId was called and therefore the error handler must also be called
    ASSERT_TRUE(detectedError.has_value());
    ASSERT_TRUE(detectedErrorLevel.has_value());
    EXPECT_THAT(detectedError.value(),
                Eq(iox::PoshError::POPO__TYPED_UNIQUE_ID_ROUDI_HAS_ALREADY_DEFINED_CUSTOM_UNIQUE_ID));
    EXPECT_THAT(detectedErrorLevel.value(), Eq(iox::ErrorLevel::SEVERE));

    // reset unique RouDi ID
    iox::popo::UniquePortId::setUniqueRouDiId(iox::roudi::DEFAULT_UNIQUE_ROUDI_ID);
}

TEST_F(IceoryxRoudiApp_test, ConstructorCalledWithArgVersionSetRunVariableToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "207dd5ea-a00c-48f1-a8de-5ef5a0c5235b");
    constexpr uint8_t NUMBER_OF_ARGS{2U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    char option[] = "-v";
    args[0] = &appName[0];
    args[1] = &option[0];

    auto cmdLineArgs = cmdLineParser.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(cmdLineArgs.has_error());

    IceoryxRoudiApp_Child roudi(cmdLineArgs.value(), iox::RouDiConfig_t().setDefaults());

    EXPECT_FALSE(roudi.getVariableRun());
}

TEST_F(IceoryxRoudiApp_test, VerifyConstructorWithEmptyConfigSetRunVariableToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a193ef0-b6c5-4e5b-998d-7f86102814e0");
    constexpr uint8_t NUMBER_OF_ARGS{1U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    args[0] = &appName[0];
    const std::string expectedOutput = "A RouDiConfig without segments was specified! Please provide a valid config!";

    auto cmdLineArgs = cmdLineParser.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(cmdLineArgs.has_error());

    iox::RouDiConfig_t roudiConfig;

    IceoryxRoudiApp_Child roudi(cmdLineArgs.value(), roudiConfig);

    EXPECT_FALSE(roudi.getVariableRun());

    if (iox::testing::TestingLogger::doesLoggerSupportLogLevel(iox::log::LogLevel::ERROR))
    {
        auto logMessages = iox::testing::TestingLogger::getLogMessages();
        ASSERT_THAT(logMessages.size(), Eq(1U));
        EXPECT_THAT(logMessages[0], HasSubstr(expectedOutput));
    }
}

TEST_F(IceoryxRoudiApp_test, VerifyConstructorUsingConfigWithSegmentWithoutMemPoolSetRunVariableToFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "542ff7f7-9365-40a4-a7ed-e67ba5735b9e");
    constexpr uint8_t NUMBER_OF_ARGS{1U};
    char* args[NUMBER_OF_ARGS];
    char appName[] = "./foo";
    args[0] = &appName[0];
    const std::string expectedOutput =
        "A RouDiConfig with segments without mempools was specified! Please provide a valid config!";

    auto cmdLineArgs = cmdLineParser.parse(NUMBER_OF_ARGS, args);

    ASSERT_FALSE(cmdLineArgs.has_error());

    iox::mepoo::MePooConfig mempoolConfig;
    auto currentGroup = iox::posix::PosixGroup::getGroupOfCurrentProcess();

    iox::RouDiConfig_t roudiConfig;

    roudiConfig.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mempoolConfig});

    IceoryxRoudiApp_Child roudi(cmdLineArgs.value(), roudiConfig);

    EXPECT_FALSE(roudi.getVariableRun());

    if (iox::testing::TestingLogger::doesLoggerSupportLogLevel(iox::log::LogLevel::ERROR))
    {
        auto logMessages = iox::testing::TestingLogger::getLogMessages();
        ASSERT_THAT(logMessages.size(), Eq(1U));
        EXPECT_THAT(logMessages[0], HasSubstr(expectedOutput));
    }
}
} // namespace
