// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

// ======================================== Helpers ======================================== //


// ======================================== Fixture ======================================== //
class TomlGatewayConfigParserTest : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

// ======================================== Tests ======================================== //
TEST_F(TomlGatewayConfigParserTest, PassesValidationIfValidCharactersUsedInServiceDescription)
{
    // ===== Setup
    // Prepare configuration to test with
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntry = cpptoml::make_table();
    serviceEntry->insert("service", "service");
    serviceEntry->insert("instance", "instance");
    serviceEntry->insert("event", "event");
    serviceEntry->insert("size", 0);
    serviceArray->push_back(serviceEntry);

    auto serviceEntryUppercase = cpptoml::make_table();
    serviceEntryUppercase->insert("service", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceEntryUppercase->insert("instance", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceEntryUppercase->insert("event", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    serviceEntryUppercase->insert("size", 0);
    serviceArray->push_back(serviceEntryUppercase);

    auto serviceEntryLowercase = cpptoml::make_table();
    serviceEntryLowercase->insert("service", "abcdefghijklmnopqrstuvwxyz");
    serviceEntryLowercase->insert("instance", "abcdefghijklmnopqrstuvwxyz");
    serviceEntryLowercase->insert("event", "abcdefghijklmnopqrstuvwxyz");
    serviceEntryLowercase->insert("size", 0);
    serviceArray->push_back(serviceEntryLowercase);

    auto serviceEntryNumbers = cpptoml::make_table();
    serviceEntryNumbers->insert("service", "Number1234567890");
    serviceEntryNumbers->insert("instance", "Number1234567890");
    serviceEntryNumbers->insert("event", "Number1234567890");
    serviceEntryNumbers->insert("size", 0);
    serviceArray->push_back(serviceEntryNumbers);

    auto serviceEntryUnderscore = cpptoml::make_table();
    serviceEntryUnderscore->insert("service", "service_name");
    serviceEntryUnderscore->insert("instance", "instance_name");
    serviceEntryUnderscore->insert("event", "event_name");
    serviceEntryUnderscore->insert("size", 0);
    serviceArray->push_back(serviceEntryUnderscore);

    auto serviceEntryBeginsWithUnderscore = cpptoml::make_table();
    serviceEntryBeginsWithUnderscore->insert("service", "_service_name");
    serviceEntryBeginsWithUnderscore->insert("instance", "_instance_name");
    serviceEntryBeginsWithUnderscore->insert("event", "_event_name");
    serviceEntryBeginsWithUnderscore->insert("size", 0);
    serviceArray->push_back(serviceEntryBeginsWithUnderscore);

    toml->insert("services", serviceArray);

    // ===== Test
    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(false, result.has_error());
}

TEST_F(TomlGatewayConfigParserTest, FailsValidationIfNoServiceNameInServiceDescription)
{
    // ===== Setup
    // Prepare configuration to test with
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoServiceName = cpptoml::make_table();
    serviceEntryNoServiceName->insert("instance", "instance");
    serviceEntryNoServiceName->insert("event", "event");
    serviceEntryNoServiceName->insert("size", 0);
    serviceArray->push_back(serviceEntryNoServiceName);
    toml->insert("services", serviceArray);

    // ===== Test
    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(true, result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(iox::config::TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
    }
}

TEST_F(TomlGatewayConfigParserTest, FailsValidationIfNoInstanceNameInServiceDescription)
{
    // ===== Setup
    // Prepare configuration to test with
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoInstanceName = cpptoml::make_table();
    serviceEntryNoInstanceName->insert("service", "service");
    serviceEntryNoInstanceName->insert("event", "event");
    serviceEntryNoInstanceName->insert("size", 0);
    serviceArray->push_back(serviceEntryNoInstanceName);
    toml->insert("services", serviceArray);

    // ===== Test
    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(true, result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(iox::config::TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
    }
}

TEST_F(TomlGatewayConfigParserTest, FailsValidationIfNoEventNameInServiceDescription)
{
    // ===== Setup
    // Prepare configuration to test with
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceEntryNoEventName = cpptoml::make_table();
    serviceEntryNoEventName->insert("service", "service");
    serviceEntryNoEventName->insert("instance", "instance");
    serviceEntryNoEventName->insert("size", 0);
    serviceArray->push_back(serviceEntryNoEventName);
    toml->insert("services", serviceArray);

    // ===== Test
    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(true, result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(iox::config::TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION, result.get_error());
    }
}

TEST_F(TomlGatewayConfigParserTest, FailsValidationIfServiceDescriptionBeginsWithNumber)
{
    // ===== Setup
    // Prepare configuration to test with
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();

    auto serviceBeginsWithNumber = cpptoml::make_table();
    serviceBeginsWithNumber->insert("service", "0000");
    serviceBeginsWithNumber->insert("instance", "0000");
    serviceBeginsWithNumber->insert("event", "0000");
    serviceBeginsWithNumber->insert("size", 0);
    serviceArray->push_back(serviceBeginsWithNumber);
    toml->insert("services", serviceArray);

    // ===== Test
    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(true, result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(iox::config::TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
    }
}

TEST_F(TomlGatewayConfigParserTest, FailsValidationIfHyphenInServiceDescription)
{
    // ===== Setup
    // Prepare configuration to test with
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();
    auto serviceEntry = cpptoml::make_table();
    serviceEntry->insert("service", "service-name");
    serviceEntry->insert("instance", "instance-name");
    serviceEntry->insert("event", "event-name");
    serviceEntry->insert("size", 0);
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);

    // ===== Test
    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(true, result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(iox::config::TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION, result.get_error());
    }
}

TEST_F(TomlGatewayConfigParserTest, FailsValidationIfNoServicesInConfig)
{
    // ===== Setup
    // Prepare configuration to test with
    auto toml = cpptoml::make_table();

    // ===== Test
    auto result = iox::config::StubbedTomlGatewayConfigParser::validate(*toml);
    EXPECT_EQ(true, result.has_error());
    if (result.has_error())
    {
        EXPECT_EQ(iox::config::TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION, result.get_error());
    }
}
