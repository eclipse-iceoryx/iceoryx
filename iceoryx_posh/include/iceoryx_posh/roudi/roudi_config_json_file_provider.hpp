// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_POSH_ROUDI_ROUDI_CONFIG_JSON_FILE_PROVIDER_HPP
#define IOX_POSH_ROUDI_ROUDI_CONFIG_JSON_FILE_PROVIDER_HPP

#include "iceoryx_posh/roudi/roudi_config_file_provider.hpp"

#include "iceoryx_posh/roudi/cmd_line_args.hpp"

extern "C" {
#include "tiny-json.h"
}

namespace iox
{
namespace roudi
{
static constexpr char defaultConfigJsonPath[] = "/etc/iceoryx/roudi_config.json";

/// @brief Reader for Roudi Configuration using json config file
class JsonRouDiConfigFileProvider : public iox::roudi::RouDiConfigFileProvider
{
  private:
    constexpr static uint32_t NUMBER_OF_JSON_NODES = 1024;

    /// @brief reads a mempool configuration from a segment
    /// @return the Mempool configuration or a helpful error when it is misconfigured
    iox::cxx::expected<iox::mepoo::MePooConfig, iox::roudi::RouDiConfigFileParseError>
    getMempool(json_t const* segment);

  public:
    /// @brief constructor taking the comand-line options
    /// @param[in] cmdLineParserValue parsed command line arguments
    JsonRouDiConfigFileProvider(iox::config::CmdLineArgs_t& cmdLineArgs);
    /// @brief default deconstructor
    virtual ~JsonRouDiConfigFileProvider() = default;

    /// @brief Reads the manifest from a json file
    /// @code
    /// {
    ///   "general":
    ///    {
    ///     "version" : 1
    ///    },
    ///    "segment":[
    ///    {
    ///      "mempool":
    ///      [
    ///        {
    ///          "size":32,
    ///          "count":10000
    ///        }
    ///      ]
    ///    }
    /// }
    /// @endcode
    /// @return[in] the roudi config or a helpful error
    iox::cxx::expected<iox::RouDiConfig_t, iox::roudi::RouDiConfigFileParseError> parse() override;
};
} // namespace roudi
} // namespace iox

#endif /* IOX_POSH_ROUDI_ROUDI_CONFIG_JSON_FILE_PROVIDER_HPP */
