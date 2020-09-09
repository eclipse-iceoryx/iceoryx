// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ROUDI_CONFIG_TOML_FILE_PROVIDER_HPP
#define IOX_POSH_ROUDI_ROUDI_CONFIG_TOML_FILE_PROVIDER_HPP

#include "iceoryx_posh/roudi/roudi_config_file_provider.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"

namespace iox
{
namespace config
{
static constexpr char defaultConfigFilePath[] = "/etc/iceoryx/roudi_config.toml";

class TomlRouDiConfigFileProvider : public iox::roudi::RouDiConfigFileProvider
{
  public:
    TomlRouDiConfigFileProvider(CmdLineParserConfigFileOption& cmdLineParserValue);

    iox::cxx::expected<iox::RouDiConfig_t, iox::roudi::RouDiConfigFileParseError> parse() override;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_CONFIG_TOML_FILE_PROVIDER_HPP
