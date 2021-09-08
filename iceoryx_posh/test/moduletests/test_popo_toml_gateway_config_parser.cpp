// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx/tests/posh/moduletests/test_input_path.hpp"
#include "iceoryx_posh/gateway/toml_gateway_config_parser.hpp"
#include "stubs/stub_toml_gateway_config_parser.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

using namespace iox::config;

// ======================================== Helpers ======================================== //
namespace
{
using ParseErrorInputFile_t = std::pair<iox::config::TomlGatewayConfigParseError, iox::roudi::ConfigFilePathString_t>;
using CheckCharactersValidity_t = std::pair<std::string, bool>;
}; // namespace

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

class TomlGatewayConfigParserSuiteTest : public TestWithParam<CheckCharactersValidity_t>
{
  public:
    void SetUp()
    {
        // get file path via cmake
        m_configFilePath = iox::testing::TEST_INPUT_PATH;
    };
    void TearDown(){};

    iox::roudi::ConfigFilePathString_t m_configFilePath;
    void CreateTmpTomlFile(std::shared_ptr<cpptoml::table> toml)
    {
        m_configFilePath.append(iox::cxx::TruncateToCapacity, "generated_gateway_config.toml");
        std::fstream fs(m_configFilePath, std::fstream::out | std::fstream::trunc);
        if (fs.std::fstream::is_open())
        {
            fs << *toml;
        }
        else
        {
            ASSERT_STREQ("expected open fstream", "fstream not open");
        }
        fs.close();
    }
};

// ======================================== Tests ======================================== //
/// we require INSTANTIATE_TEST_CASE since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

INSTANTIATE_TEST_CASE_P(ValidTest,
                        TomlGatewayConfigParserSuiteTest,
                        ::testing::Values(CheckCharactersValidity_t{"validcharacters", false},
                                          CheckCharactersValidity_t{"UPPERCASECHARACTERS", false},
                                          CheckCharactersValidity_t{"lowercasecharacters", false},
                                          CheckCharactersValidity_t{"Number1234567890", false},
                                          CheckCharactersValidity_t{"Under_score_Characters", false},
                                          CheckCharactersValidity_t{"_BeginsWithUnderscore", false},
                                          CheckCharactersValidity_t{"Hyphen-InService", true},
                                          CheckCharactersValidity_t{"1234567890", true},
                                          CheckCharactersValidity_t{"這場考試_!*#:", true}));


#pragma GCC diagnostic pop

TEST_P(TomlGatewayConfigParserSuiteTest, CheckCharactersUsedInServiceDescription)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    const auto charactersValidity = GetParam();

    std::string stringentry = charactersValidity.first;
    serviceEntry->insert("service", stringentry);
    serviceEntry->insert("instance", stringentry);
    serviceEntry->insert("event", stringentry);
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    ASSERT_EQ(charactersValidity.second, result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
    }
}

TEST_P(TomlGatewayConfigParserSuiteTest, CheckCharactersUsedForServiceDescriptionToParseInTomlConfigFile)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    const auto charactersValidity = GetParam();

    std::string stringentry = charactersValidity.first;
    serviceEntry->insert("service", stringentry);
    serviceEntry->insert("instance", stringentry);
    serviceEntry->insert("event", stringentry);
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);

    ASSERT_EQ(charactersValidity.second, result.has_error());
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

TEST_F(TomlGatewayConfigParserSuiteTest, NoServiceNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
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

TEST_F(TomlGatewayConfigParserSuiteTest, NoInstanceNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
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

TEST_F(TomlGatewayConfigParserSuiteTest, NoEventNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
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

TEST_F(TomlGatewayConfigParserSuiteTest, NoServicesInConfigReturnIncompleteConfigurationError)
{
    auto toml = cpptoml::make_table();

    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION, result.get_error());
}

// Without argument the iceoryx default config in /etc/iceoryx/gateway_config.toml is used. Then this
// test fails on every machine which is using such a config.
TEST_F(TomlGatewayConfigParserSuiteTest, DISABLED_ParseWithoutParameterTakeDefaultPathReturnNoError)
{
    auto result = TomlGatewayConfigParser::parse();
    ASSERT_FALSE(result.has_error());

    GatewayConfig config = result.value();
    EXPECT_TRUE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserSuiteTest, ParseWithEmptyPathReturnEmptyConfig)
{
    iox::roudi::ConfigFilePathString_t path = "";

    auto result = TomlGatewayConfigParser::parse(path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_TRUE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserSuiteTest,
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

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserSuiteTest,
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

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserSuiteTest,
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

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserSuiteTest,
       ParseWithoutServicesConfigurationInTomlConfigFileReturnIncompleteConfigurationError)
{
    auto toml = cpptoml::make_table();
    CreateTmpTomlFile(toml);

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION, result.get_error());
}

TEST_F(TomlGatewayConfigParserSuiteTest, DuplicatedServicesDescriptionInTomlFileReturnOnlyOneEntry)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    serviceEntry->insert("service", "service");
    serviceEntry->insert("instance", "instance");
    serviceEntry->insert("event", "event");

    auto serviceEntry1 = cpptoml::make_table();
    serviceEntry1->insert("service", "service");
    serviceEntry1->insert("instance", "instance");
    serviceEntry1->insert("event", "event");
    serviceArray->push_back(serviceEntry1);

    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);
    GatewayConfig config = result.value();
    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());
    EXPECT_EQ(config.m_configuredServices.size(), 1);
}

TEST_F(TomlGatewayConfigParserSuiteTest, ParseValidConfigFileWithMaximumAllowedNumberOfConfiguredServicesReturnNoError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    for (uint32_t i = 1U; i <= iox::MAX_GATEWAY_SERVICES; ++i)
    {
        auto serviceEntry = cpptoml::make_table();
        std::string stringentry = "validservice" + std::to_string(i);
        serviceEntry->insert("service", stringentry);
        serviceEntry->insert("instance", stringentry);
        serviceEntry->insert("event", stringentry);
        serviceArray->push_back(serviceEntry);
    }

    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);
    GatewayConfig config = result.value();

    EXPECT_EQ(config.m_configuredServices.size(), iox::MAX_GATEWAY_SERVICES);
    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());

    uint32_t count = 1;
    for (auto configuredService : config.m_configuredServices)
    {
        std::string stringentry = "validservice" + std::to_string(count);
        EXPECT_EQ(configuredService.m_serviceDescription.getServiceIDString(), stringentry);
        EXPECT_EQ(configuredService.m_serviceDescription.getInstanceIDString(), stringentry);
        EXPECT_EQ(configuredService.m_serviceDescription.getEventIDString(), stringentry);
        ++count;
    }
}

TEST_F(TomlGatewayConfigParserSuiteTest,
       ParseValidConfigFileWithMoreThanMaximumAllowedNumberOfConfiguredServicesReturnError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();
    auto serviceEntry = cpptoml::make_table();

    for (uint32_t i = 1U; i <= iox::MAX_GATEWAY_SERVICES + 1U; ++i)
    {
        std::string stringentry = "validservice" + std::to_string(i);
        serviceEntry->insert("service", stringentry);
        serviceEntry->insert("instance", stringentry);
        serviceEntry->insert("event", stringentry);
        serviceArray->push_back(serviceEntry);
    }


    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    auto result = TomlGatewayConfigParser::parse(m_configFilePath);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error(), MAXIMUM_NUMBER_OF_ENTRIES_EXCEEDED);
}

/// we require INSTANTIATE_TEST_CASE_P since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
INSTANTIATE_TEST_CASE_P(
    ParseAllMalformedInputConfigFiles,
    TomlGatewayConfigParserTest,
    Values(ParseErrorInputFile_t{iox::config::TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION,
                                 "generated_gateway_config.toml"},
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
