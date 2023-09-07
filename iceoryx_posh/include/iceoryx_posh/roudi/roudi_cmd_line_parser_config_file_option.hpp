// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ROUDI_CMD_LINE_PARSER_CONFIG_FILE_OPTION_HPP
#define IOX_POSH_ROUDI_ROUDI_CMD_LINE_PARSER_CONFIG_FILE_OPTION_HPP

#include "iceoryx_posh/roudi/roudi_cmd_line_parser.hpp"

namespace iox
{
namespace config
{
class CmdLineParserConfigFileOption : public CmdLineParser
{
  public:
    CmdLineParserConfigFileOption() noexcept = default;
    virtual ~CmdLineParserConfigFileOption() noexcept = default;
    CmdLineParserConfigFileOption& operator=(const CmdLineParserConfigFileOption& other) = delete;
    CmdLineParserConfigFileOption(const CmdLineParserConfigFileOption& other) = delete;
    CmdLineParserConfigFileOption& operator=(CmdLineParserConfigFileOption&&) = delete;
    CmdLineParserConfigFileOption(CmdLineParserConfigFileOption&& other) = delete;

    /// @brief process the passed command line arguments
    /// @param[in] argc forwarding of command line arguments
    /// @param[in] argv forwarding of command line arguments
    /// @param[in] cmdLineParsingMode selects to parse a single option or all options
    /// @param[out] Result wrapped in an expected, either the parsed arguments as CmdLineArgs_t struct or
    /// CmdLineParserResult
    expected<CmdLineArgs_t, CmdLineParserResult>
    parse(int argc,
          char* argv[],
          const CmdLineArgumentParsingMode cmdLineParsingMode = CmdLineArgumentParsingMode::ALL) noexcept override;
};

} // namespace config
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_CMD_LINE_PARSER_CONFIG_FILE_OPTION_HPP
