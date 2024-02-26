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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_ROUDI_ROUDI_CONFIG_TOML_FILE_PROVIDER_HPP
#define IOX_POSH_ROUDI_ROUDI_CONFIG_TOML_FILE_PROVIDER_HPP

#include "iceoryx_posh/roudi/cmd_line_args.hpp"
#include "iceoryx_posh/roudi/roudi_config_file_provider.hpp"
#include "iox/expected.hpp"

#include <istream>

namespace iox
{
namespace config
{
static constexpr char defaultConfigFilePath[] = "/etc/iceoryx/roudi_config.toml";

class TomlRouDiConfigFileProvider : public iox::roudi::RouDiConfigFileProvider
{
  public:
    TomlRouDiConfigFileProvider(iox::config::CmdLineArgs_t& cmdLineArgs) noexcept;

    iox::expected<iox::IceoryxConfig, iox::roudi::RouDiConfigFileParseError> parse() noexcept override;

    static iox::expected<iox::IceoryxConfig, iox::roudi::RouDiConfigFileParseError>
    parse(std::istream& stream) noexcept;
};
} // namespace config
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_CONFIG_TOML_FILE_PROVIDER_HPP
