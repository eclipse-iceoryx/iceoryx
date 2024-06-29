// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_GW_TOML_FILE_CONFIG_PARSER_HPP
#define IOX_POSH_GW_TOML_FILE_CONFIG_PARSER_HPP

#include "iceoryx_posh/gateway/gateway_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/expected.hpp"

#include <istream>

namespace cpptoml
{
class table;
}

namespace iox
{
namespace config
{
enum TomlGatewayConfigParseError
{
    FILE_OPEN_FAILED,
    INCOMPLETE_CONFIGURATION,
    INCOMPLETE_SERVICE_DESCRIPTION,
    INVALID_SERVICE_DESCRIPTION,
    EXCEPTION_IN_PARSER,
    MAXIMUM_NUMBER_OF_ENTRIES_EXCEEDED
};

constexpr const char* TOML_GATEWAY_CONFIG_FILE_PARSE_ERROR_STRINGS[] = { "FILE_OPEN_FAILED",
                                                                         "INCOMPLETE_CONFIGURATION",
                                                                         "INCOMPLETE_SERVICE_DESCRIPTION",
                                                                         "INVALID_SERVICE_DESCRIPTION",
                                                                         "EXCEPTION_IN_PARSER",
                                                                         "MAXIMUM_NUMBER_OF_ENTRIES_EXCEEDED" };

static constexpr const char REGEX_VALID_CHARACTERS[] = "^[a-zA-Z_][a-zA-Z0-9_]*$";

static constexpr const char DEFAULT_CONFIG_FILE_PATH[] = "/etc/iceoryx/gateway_config.toml";
static constexpr const char GATEWAY_CONFIG_SERVICE_TABLE_NAME[] = "services";
static constexpr const char GATEWAY_CONFIG_SERVICE_NAME[] = "service";
static constexpr const char GATEWAY_CONFIG_SERVICE_INSTANCE_NAME[] = "instance";
static constexpr const char GATEWAY_CONFIG_SERVICE_EVENT_NAME[] = "event";

///
/// @brief The TomlGatewayConfigParser class provides methods for parsing gateway configs from toml text files.
///
class TomlGatewayConfigParser
{
  public:
    static expected<GatewayConfig, TomlGatewayConfigParseError>
    parse(const roudi::ConfigFilePathString_t& path = roudi::ConfigFilePathString_t(DEFAULT_CONFIG_FILE_PATH)) noexcept;

    static expected<GatewayConfig, TomlGatewayConfigParseError> parse(std::istream& stream) noexcept;

  protected:
    static expected<void, TomlGatewayConfigParseError> parse(std::istream& stream, GatewayConfig& config) noexcept;
    static expected<void, TomlGatewayConfigParseError> validate(const cpptoml::table& parsedToml) noexcept;

  private:
    static bool hasInvalidCharacter(const std::string& s) noexcept;
};

} // namespace config
} // namespace iox

#endif // IOX_POSH_GW_TOML_FILE_CONFIG_PARSER_HPP
