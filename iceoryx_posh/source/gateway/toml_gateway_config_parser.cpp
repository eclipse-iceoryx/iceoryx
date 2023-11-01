// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 by Apex AI Inc. All rights reserved.
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
#include "iox/file_reader.hpp"
#include "iox/into.hpp"
#include "iox/logging.hpp"
#include "iox/std_string_support.hpp"

#include <cpptoml.h>
#include <limits> // workaround for missing include in cpptoml.h
#include <regex>

iox::expected<iox::config::GatewayConfig, iox::config::TomlGatewayConfigParseError>
iox::config::TomlGatewayConfigParser::parse(const roudi::ConfigFilePathString_t& path) noexcept
{
    iox::config::GatewayConfig config;

    // Set defaults if no path provided.
    if (path.size() == 0)
    {
        IOX_LOG(WARN, "Invalid file path provided. Falling back to built-in config.");
        config.setDefaults();
        return iox::ok(config);
    }

    /// @todo iox-#1718 Replace with C++17 std::filesystem::exists()
    iox::FileReader configFile(into<std::string>(path), "", FileReader::ErrorMode::Ignore);
    if (!configFile.isOpen())
    {
        IOX_LOG(WARN, "Gateway config file not found at: '" << path << "'. Falling back to built-in config.");
        config.setDefaults();
        return iox::ok(config);
    }

    IOX_LOG(INFO, "Using gateway config at: " << path);

    std::ifstream fileStream{path.c_str()};
    if (!fileStream.is_open())
    {
        IOX_LOG(ERROR, "Could not open config file from path '" << path << "'");
        return iox::err(iox::config::TomlGatewayConfigParseError::FILE_OPEN_FAILED);
    }

    auto parseResult = TomlGatewayConfigParser::parse(fileStream, config);
    if (parseResult.has_error())
    {
        return iox::err(parseResult.error());
    }

    return iox::ok(config);
}

iox::expected<iox::config::GatewayConfig, iox::config::TomlGatewayConfigParseError>
iox::config::TomlGatewayConfigParser::parse(std::istream& stream) noexcept
{
    GatewayConfig config;
    auto parseResult = TomlGatewayConfigParser::parse(stream, config);
    if (parseResult.has_error())
    {
        return iox::err(parseResult.error());
    }

    return iox::ok(config);
}

iox::expected<void, iox::config::TomlGatewayConfigParseError>
iox::config::TomlGatewayConfigParser::parse(std::istream& stream, GatewayConfig& config) noexcept
{
    std::shared_ptr<cpptoml::table> parsedToml{nullptr};
    try
    {
        cpptoml::parser p{stream};
        parsedToml = p.parse();
    }
    catch (const std::exception& parserException)
    {
        auto parserError = iox::config::TomlGatewayConfigParseError::EXCEPTION_IN_PARSER;
        auto errorStringIndex = static_cast<uint64_t>(parserError);
        IOX_LOG(WARN,
                iox::config::TOML_GATEWAY_CONFIG_FILE_PARSE_ERROR_STRINGS[errorStringIndex] << ": "
                                                                                            << parserException.what());

        return iox::err(parserError);
    }

    auto result = validate(*parsedToml);
    if (result.has_error())
    {
        return iox::err(result.error());
    }

    // Prepare config object
    auto serviceArray = parsedToml->get_table_array(GATEWAY_CONFIG_SERVICE_TABLE_NAME);
    for (const auto& service : *serviceArray)
    {
        GatewayConfig::ServiceEntry entry;
        auto serviceName = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_NAME);
        auto instance = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_INSTANCE_NAME);
        auto event = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_EVENT_NAME);
        entry.m_serviceDescription = iox::capro::ServiceDescription(into<lossy<iox::capro::IdString_t>>(*serviceName),
                                                                    into<lossy<iox::capro::IdString_t>>(*instance),
                                                                    into<lossy<iox::capro::IdString_t>>(*event));
        config.m_configuredServices.push_back(entry);
    }

    return iox::ok();
}

iox::expected<void, iox::config::TomlGatewayConfigParseError>
iox::config::TomlGatewayConfigParser::validate(const cpptoml::table& parsedToml) noexcept
{
    // Check for expected fields
    auto serviceArray = parsedToml.get_table_array(GATEWAY_CONFIG_SERVICE_TABLE_NAME);
    if (!serviceArray)
    {
        return iox::err(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION);
    }

    if (serviceArray->get().size() > iox::MAX_GATEWAY_SERVICES)
    {
        return iox::err(TomlGatewayConfigParseError::MAXIMUM_NUMBER_OF_ENTRIES_EXCEEDED);
    }

    for (const auto& service : *serviceArray)
    {
        auto serviceName = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_NAME);
        auto instance = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_INSTANCE_NAME);
        auto event = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_EVENT_NAME);

        // check for incomplete service descriptions
        if (!serviceName || !instance || !event)
        {
            return iox::err(TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION);
        }

        // check for invalid characters in strings
        if (hasInvalidCharacter(*serviceName) || hasInvalidCharacter(*instance) || hasInvalidCharacter(*event))
        {
            return iox::err(TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION);
        }
    }

    return iox::ok();
}

bool iox::config::TomlGatewayConfigParser::hasInvalidCharacter(const std::string& s) noexcept
{
    // See: https://design.ros2.org/articles/topic_and_service_names.html
    const std::regex regex(REGEX_VALID_CHARACTERS);
    auto isInvalid = !std::regex_match(s, regex);
    return isInvalid;
}
