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

#pragma once

#include "iceoryx_posh/roudi/roudi_cmd_line_parser.hpp"

namespace iox
{
namespace roudi
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
    void parse(int argc,
               char* argv[],
               const CmdLineArgumentParsingMode cmdLineParsingMode = CmdLineArgumentParsingMode::ALL) noexcept override;

    ConfigFilePathString_t getConfigFilePath() const;

  protected:
    ConfigFilePathString_t m_customConfigFilePath;
};

} // namespace roudi
} // namespace iox
