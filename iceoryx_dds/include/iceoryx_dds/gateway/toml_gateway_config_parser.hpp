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

#ifndef IOX_DDS_GATEWAY_FILE_CONFIG_PARSER_HPP
#define IOX_DDS_GATEWAY_FILE_CONFIG_PARSER_HPP

#include "iceoryx_dds/gateway/gateway_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/expected.hpp"

#include "cpptoml.h"


namespace iox
{
namespace dds
{
enum TomlGatewayConfigParseError
{
    FILE_NOT_FOUND,
    INCOMPLETE_CONFIGURATION,
    INCOMPLETE_SERVICE_DESCRIPTION,
    INVALID_SERVICE_DESCRIPTION
};

static constexpr const char DEFAULT_CONFIG_FILE_PATH[] = "/etc/iceoryx/gateway_config.toml";

static constexpr const char REGEX_VALID_CHARACTERS[] = "^[a-zA-Z_][a-zA-Z0-9_]*$";

static constexpr const char GATEWAY_CONFIG_SERVICE_TABLE_NAME[] = "services";
static constexpr const char GATEWAY_CONFIG_SERVICE_NAME[] = "service";
static constexpr const char GATEWAY_CONFIG_SERVICE_INSTANCE_NAME[] = "instance";
static constexpr const char GATEWAY_CONFIG_SERVICE_EVENT_NAME[] = "event";
static constexpr const char GATEWAY_CONFIG_SERVICE_PAYLOAD_SIZE[] = "size";

///
/// @brief The TomlGatewayConfigParser class provides methods for parsing gateway configs from toml text files.
///
class TomlGatewayConfigParser
{
  public:
    static iox::cxx::expected<GatewayConfig, TomlGatewayConfigParseError> parse();
    static iox::cxx::expected<GatewayConfig, TomlGatewayConfigParseError> parse(ConfigFilePathString_t path);

  protected:
    static iox::cxx::expected<TomlGatewayConfigParseError> validate(const cpptoml::table& parsedToml) noexcept;

  private:
    static bool hasInvalidCharacter(std::string s) noexcept;
};

} // namespace dds
} // namespace iox

#endif // IOX_DDS_GATEWAY_FILE_CONFIG_PARSER_HPP
