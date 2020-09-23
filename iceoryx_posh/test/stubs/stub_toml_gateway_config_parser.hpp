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

#ifndef IOX_POSH_STUBS_TOML_GATEWAY_CONFIG_PARSER_HPP
#define IOX_POSH_STUBS_TOML_GATEWAY_CONFIG_PARSER_HPP

#include "iceoryx_posh/gateway/toml_gateway_config_parser.hpp"

#include "iceoryx_utils/cxx/expected.hpp"

namespace iox
{
namespace config
{
///
/// @brief The StubbedTomlGatewayConfigParser class exposes protected methods in the TomlGatewayConfigParser os that
/// they can be tested.
///
class StubbedTomlGatewayConfigParser : public TomlGatewayConfigParser
{
  public:
    static cxx::expected<TomlGatewayConfigParseError> validate(const cpptoml::table& parsedToml) noexcept
    {
        return TomlGatewayConfigParser::validate(parsedToml);
    }
};

} // namespace config
} // namespace iox

#endif // IOX_POSH_STUBS_TOML_GATEWAY_CONFIG_PARSER_HPP
