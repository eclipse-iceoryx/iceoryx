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

#include "iceoryx_posh/gateway/toml_gateway_config_parser.hpp"
#include "stubs/stub_toml_gateway_config_parser.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

using namespace iox::config;

// ======================================== Helpers ======================================== //
const std::string TestFile = "gwconfig_test.tmp";
#ifndef _WIN32
const std::string TempPath = "/tmp";
const std::string TestFilePath = TempPath + "/" + TestFile;
#else
const std::string TempPath = std::getenv("TEMP");
const std::string TestFilePath = TempPath + "\\" + TestFile;
#endif

// ======================================== Fixture ======================================== //
class TomlGatewayConfigParserTest : public Test
{
  public:
    void SetUp(){};
    void TearDown()
    {
        if (std::remove(TestFilePath.c_str()) != 0)
        {
            std::cerr << "Failed to remove temporary file '" << TestFilePath
                      << "'. You'll have to remove it by yourself.";
        }
    };

    void CreateTmpTomlFile(std::shared_ptr<cpptoml::table> toml)
    {
        std::fstream fs(TestFilePath, std::fstream::out | std::fstream::trunc);
        if (fs.std::fstream::is_open())
        {
            fs << *toml;
        }
        fs.close();
    }
};

// ======================================== Tests ======================================== //
TEST_F(TomlGatewayConfigParserTest, ValidCharactersUsedInServiceDescriptionReturnNoValidateError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    serviceEntry->insert("service", "service");
    serviceEntry->insert("instance", "instance");
    serviceEntry->insert("event", "event");
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(false, result.has_error());
}

TEST_F(TomlGatewayConfigParserTest, UppercaseCharactersUsedInServiceDescriptionReturnNoValidateError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryUppercase = cpptoml::make_table();
    serviceEntryUppercase->insert("service", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceEntryUppercase->insert("instance", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceEntryUppercase->insert("event", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceArray->push_back(serviceEntryUppercase);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(false, result.has_error());
}

TEST_F(TomlGatewayConfigParserTest, LowercaseCharactersUsedInServiceDescriptionReturnNoValidateError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryLowercase = cpptoml::make_table();
    serviceEntryLowercase->insert("service", "abcdefghijklmnopqrstuvwxyz");
    serviceEntryLowercase->insert("instance", "abcdefghijklmnopqrstuvwxyz");
    serviceEntryLowercase->insert("event", "abcdefghijklmnopqrstuvwxyz");
    serviceArray->push_back(serviceEntryLowercase);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(false, result.has_error());
}

TEST_F(TomlGatewayConfigParserTest, NumbersUsedInServiceDescriptionReturnNoValidateError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNumbers = cpptoml::make_table();
    serviceEntryNumbers->insert("service", "Number1234567890");
    serviceEntryNumbers->insert("instance", "Number1234567890");
    serviceEntryNumbers->insert("event", "Number1234567890");
    serviceArray->push_back(serviceEntryNumbers);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(false, result.has_error());
}

TEST_F(TomlGatewayConfigParserTest, UnderscoreCharactersUsedInServiceDescriptionReturnNoValidateError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryUnderscore = cpptoml::make_table();
    serviceEntryUnderscore->insert("service", "service_name");
    serviceEntryUnderscore->insert("instance", "instance_name");
    serviceEntryUnderscore->insert("event", "event_name");
    serviceArray->push_back(serviceEntryUnderscore);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(false, result.has_error());
}

TEST_F(TomlGatewayConfigParserTest, ServiceEntryBeginsWithUnderscoresUsedInServiceDescriptionReturnNoValidateError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryBeginsWithUnderscore = cpptoml::make_table();
    serviceEntryBeginsWithUnderscore->insert("service", "_service_name");
    serviceEntryBeginsWithUnderscore->insert("instance", "_instance_name");
    serviceEntryBeginsWithUnderscore->insert("event", "_event_name");
    serviceArray->push_back(serviceEntryBeginsWithUnderscore);
    toml->insert("services", serviceArray);

    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(false, result.has_error());
}

TEST_F(TomlGatewayConfigParserTest, NoServiceNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
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

TEST_F(TomlGatewayConfigParserTest, NoInstanceNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
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

TEST_F(TomlGatewayConfigParserTest, NoEventNameInServiceDescriptionReturnIncompleteServiceDescriptionError)
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

TEST_F(TomlGatewayConfigParserTest, ServiceDescriptionBeginsWithNumberReturnInvalidServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceBeginsWithNumber = cpptoml::make_table();
    serviceBeginsWithNumber->insert("service", "0000");
    serviceBeginsWithNumber->insert("instance", "0000");
    serviceBeginsWithNumber->insert("event", "0000");
    serviceArray->push_back(serviceBeginsWithNumber);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserTest, HyphenInServiceDescriptionReturnInvalidServiceDescriptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryWithHyphen = cpptoml::make_table();
    serviceEntryWithHyphen->insert("service", "service-name");
    serviceEntryWithHyphen->insert("instance", "instance-name");
    serviceEntryWithHyphen->insert("event", "event-name");
    serviceArray->push_back(serviceEntryWithHyphen);
    toml->insert("services", serviceArray);

    auto result = StubbedTomlGatewayConfigParser::validate(*toml);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserTest, NoServicesInConfigReturnIncompleteConfigurationError)
{
    auto toml = cpptoml::make_table();

    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION, result.get_error());
}

TEST_F(TomlGatewayConfigParserTest, ParseWithoutParameterTakeDefaultPathReturnNoError)
{
    auto result = TomlGatewayConfigParser::parse();
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_TRUE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest, ParseWithEmptyPathReturnEmptyConfig)
{
    iox::roudi::ConfigFilePathString_t path = "";

    auto result = TomlGatewayConfigParser::parse(path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_TRUE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest, ValidServiceDescriptionInTomlConfigFileReturnNotEmptyConfigObjectAndNoParseError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    serviceEntry->insert("service", "service");
    serviceEntry->insert("instance", "instance");
    serviceEntry->insert("event", "event");
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest,
       ParseServiceDescriptionWithUppercaseInTomlConfigFileReturnNotEmptyConfigObjectAndNoParseError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryUppercase = cpptoml::make_table();
    serviceEntryUppercase->insert("service", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceEntryUppercase->insert("instance", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceEntryUppercase->insert("event", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceArray->push_back(serviceEntryUppercase);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest,
       ParseServiceDescriptionWithLowercaseInTomlConfigFileReturnReturnNotEmptyConfigObjectAndNoParseError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryLowercase = cpptoml::make_table();
    serviceEntryLowercase->insert("service", "abcdefghijklmnopqrstuvwxyz");
    serviceEntryLowercase->insert("instance", "abcdefghijklmnopqrstuvwxyz");
    serviceEntryLowercase->insert("event", "abcdefghijklmnopqrstuvwxyz");
    serviceArray->push_back(serviceEntryLowercase);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest,
       NumbersUsedInServiceDescriptionInTomlConfigFileReturnNotEmptyConfigObjectAndNoParseError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNumbers = cpptoml::make_table();
    serviceEntryNumbers->insert("service", "Number1234567890");
    serviceEntryNumbers->insert("instance", "Number1234567890");
    serviceEntryNumbers->insert("event", "Number1234567890");
    serviceArray->push_back(serviceEntryNumbers);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest,
       UnderscoreCharactersUsedInServiceDescriptionInTomlFileReturnNotEmptyConfigObjectAndNoParseError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryUnderscore = cpptoml::make_table();
    serviceEntryUnderscore->insert("service", "service_name");
    serviceEntryUnderscore->insert("instance", "instance_name");
    serviceEntryUnderscore->insert("event", "event_name");
    serviceArray->push_back(serviceEntryUnderscore);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest,
       ServiceEntryBeginsWithUnderscoresUsedInTomlConfigFileReturnNotEmptyConfigObjectAndNoParseError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryBeginsWithUnderscore = cpptoml::make_table();
    serviceEntryBeginsWithUnderscore->insert("service", "_service_name");
    serviceEntryBeginsWithUnderscore->insert("instance", "_instance_name");
    serviceEntryBeginsWithUnderscore->insert("event", "_event_name");
    serviceArray->push_back(serviceEntryBeginsWithUnderscore);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);
    GatewayConfig config = result.value();

    EXPECT_FALSE(result.has_error());
    EXPECT_FALSE(config.m_configuredServices.empty());
}

TEST_F(TomlGatewayConfigParserTest,
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


TEST_F(TomlGatewayConfigParserTest,
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

TEST_F(TomlGatewayConfigParserTest,
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

TEST_F(TomlGatewayConfigParserTest,
       ParseWithServiceDescriptionBeginsWithNumberInTomlConfigFileReturnInvalidServiceDesciptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceBeginsWithNumber = cpptoml::make_table();
    serviceBeginsWithNumber->insert("service", "0000");
    serviceBeginsWithNumber->insert("instance", "0000");
    serviceBeginsWithNumber->insert("event", "0000");
    serviceArray->push_back(serviceBeginsWithNumber);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserTest,
       ParseWithHyphenInServiceDescriptionInTomlConfigFileReturnInvalidServiceDesciptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryWithHyphen = cpptoml::make_table();
    serviceEntryWithHyphen->insert("service", "service-name");
    serviceEntryWithHyphen->insert("instance", "instance-name");
    serviceEntryWithHyphen->insert("event", "event-name");
    serviceArray->push_back(serviceEntryWithHyphen);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserTest,
       ParseServiceDescriptionWithInvalidCharactersInTomlConfigFileReturnInvalidServiceDesciptionError)
{
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryWithInvalidCharacters = cpptoml::make_table();
    serviceEntryWithInvalidCharacters->insert("service", "這場考試_!*#:");
    serviceEntryWithInvalidCharacters->insert("instance", "這場考試_!*#:");
    serviceEntryWithInvalidCharacters->insert("event", "這場考試_!*#:");
    serviceArray->push_back(serviceEntryWithInvalidCharacters);
    toml->insert("services", serviceArray);
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
}

TEST_F(TomlGatewayConfigParserTest, ParseWithoutServicesConfigurationInTomlConfigFileReturnIncompleteConfigurationError)
{
    auto toml = cpptoml::make_table();
    CreateTmpTomlFile(toml);

    iox::roudi::ConfigFilePathString_t Path =
        iox::roudi::ConfigFilePathString_t(iox::cxx::TruncateToCapacity, TestFilePath);
    auto result = TomlGatewayConfigParser::parse(Path);

    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION, result.get_error());
}
