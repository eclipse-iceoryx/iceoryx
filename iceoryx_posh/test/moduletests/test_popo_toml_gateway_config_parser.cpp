// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/gateway/toml_gateway_config_parser.hpp"
#include "stubs/stub_toml_gateway_config_parser.hpp"

#include "iceoryx/tests/posh/moduletests/test_input_path.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Helpers ======================================== //
namespace
{
using ParseErrorInputFile_t = std::pair<iox::config::TomlGatewayConfigParseError, iox::roudi::ConfigFilePathString_t>;
};

// ======================================== Fixture ======================================== //
class TomlGatewayConfigParserTest : public TestWithParam<ParseErrorInputFile_t>
{
  public:
    void SetUp()
    {
        // get file path via cmake
        m_configFilePath = iox::testing::TEST_INPUT_PATH;
    };
    void TearDown(){};

    iox::roudi::ConfigFilePathString_t m_configFilePath;
};

/// we require INSTANTIATE_TEST_CASE since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

INSTANTIATE_TEST_CASE_P(ValidTest,
                        TomlGatewayConfigParser_Test,
                        ::testing::Values(std::make_tuple("validcharacters", false),
                                          std::make_tuple("UPPERCASECHARACTERS", false),
                                          std::make_tuple("lowercasecharacters", false),
                                          std::make_tuple("Number1234567890", false),
                                          std::make_tuple("Under_score_Characters", false),
                                          std::make_tuple("_BeginsWithUnderscore", false),
                                          std::make_tuple("Hyphen-InService", true),
                                          std::make_tuple("1234567890", true),
                                          std::make_tuple("這場考試_!*#:", true)));

#pragma GCC diagnostic pop

// ======================================== Tests ======================================== //
TEST_P(TomlGatewayConfigParser_Test, CheckCharactersUsedInServiceDescription)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    std::string stringentry = std::get<0>(GetParam());
    serviceEntry->insert("service", stringentry);
    serviceEntry->insert("instance", stringentry);
    serviceEntry->insert("event", stringentry);
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    ASSERT_EQ(std::get<1>(GetParam()), result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
    }
}

TEST_F(TomlGatewayConfigParser_Test, NoServiceNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoServiceName = cpptoml::make_table();
    serviceEntryNoServiceName->insert("instance", "instance");
    serviceEntryNoServiceName->insert("event", "event");
    serviceArray->push_back(serviceEntryNoServiceName);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParser_Test, NoInstanceNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoInstanceName = cpptoml::make_table();
    serviceEntryNoInstanceName->insert("service", "service");
    serviceEntryNoInstanceName->insert("event", "event");
    serviceArray->push_back(serviceEntryNoInstanceName);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParser_Test, NoEventNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoEventName = cpptoml::make_table();
    serviceEntryNoEventName->insert("service", "service");
    serviceEntryNoEventName->insert("instance", "instance");
    serviceArray->push_back(serviceEntryNoEventName);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParser_Test, NoServicesInConfigReturnIncompleteConfigurationError)
{
    auto toml = cpptoml::make_table();

    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION, result.get_error());
}

TEST_F(TomlGatewayConfigParser_Test, ParseWithoutParameterTakeDefaultPathReturnNoError)
{
    auto result = TomlGatewayConfigParser::parse();
    ASSERT_FALSE(result.has_error());

    GatewayConfig config = result.value();
    EXPECT_TRUE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParser_Test, ParseWithEmptyPathReturnEmptyConfig)
{
    iox::roudi::ConfigFilePathString_t path = "";

    auto result = TomlGatewayConfigParser::parse(path);
    ASSERT_FALSE(result.has_error());

    GatewayConfig config = result.value();
    EXPECT_TRUE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParser_Test,
       ParseWithoutServiceNameInServiceDescriptionInTomlConfigFileReturnIncompleteServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoServiceName = cpptoml::make_table();
    serviceEntryNoServiceName->insert("instance", "instance");
    serviceEntryNoServiceName->insert("event", "event");
    serviceArray->push_back(serviceEntryNoServiceName);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParser_Test,
       ParseWithoutInstanceNameInServiceDescriptionInTomlConfigFileReturnIncompleteServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoInstanceName = cpptoml::make_table();
    serviceEntryNoInstanceName->insert("service", "service");
    serviceEntryNoInstanceName->insert("event", "event");
    serviceArray->push_back(serviceEntryNoInstanceName);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParser_Test,
       ParseWithoutEventNameInServiceDescriptionInTomlConfigFileReturnIncompleteServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoEventName = cpptoml::make_table();
    serviceEntryNoEventName->insert("service", "service");
    serviceEntryNoEventName->insert("instance", "instance");
    serviceArray->push_back(serviceEntryNoEventName);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParser_Test,
       ParseWithoutServicesConfigurationInTomlConfigFileReturnIncompleteConfigurationError)
{
    auto toml = cpptoml::make_table();
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION, result.get_error());
}

TEST_P(TomlGatewayConfigParser_Test, CheckCharactersUsedForServiceDescriptionInTomlConfigFile)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    std::string stringentry = std::get<0>(GetParam());
    serviceEntry->insert("service", stringentry);
    serviceEntry->insert("instance", stringentry);
    serviceEntry->insert("event", stringentry);
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_EQ(std::get<1>(GetParam()), result.has_error());

    if (!result.has_error())
    {
        GatewayConfig config = result.value();
        EXPECT_FALSE(config.m_configuredServices.empty());
    }
    else
    {
        EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
    }
}

/// we require INSTANTIATE_TEST_CASE_P since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
INSTANTIATE_TEST_CASE_P(ParseAllMalformedInputConfigFiles,
                        TomlGatewayConfigParserTest,
                        Values(ParseErrorInputFile_t{iox::config::TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION,
                                                     "popo_toml_gateway_error_incomplete_configuration.toml"},
                               ParseErrorInputFile_t{iox::config::TomlGatewayConfigParseError::EXCEPTION_IN_PARSER,
                                                     "toml_parser_exception.toml"}));


#pragma GCC diagnostic pop

TEST_P(TomlGatewayConfigParserTest, ParseMalformedInputFileCausesError)
{
    const auto parseErrorInputFile = GetParam();

    m_configFilePath.append(iox::cxx::TruncateToCapacity, parseErrorInputFile.second);

    auto result = iox::config::TomlGatewayConfigParser::parse(m_configFilePath);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(parseErrorInputFile.first, result.get_error());
}
