// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"

#include "iceoryx_platform/getopt.hpp"
#include <iostream>

namespace iox
{
namespace config
{
expected<CmdLineArgs_t, CmdLineParserResult> CmdLineParserConfigFileOption::parse(
    int argc, char* argv[], const CmdLineArgumentParsingMode cmdLineParsingMode) noexcept
{
    constexpr option LONG_OPTIONS[] = {{"help", no_argument, nullptr, 'h'},
                                       {"config-file", required_argument, nullptr, 'c'},
                                       {nullptr, 0, nullptr, 0}};

    // colon after shortOption means it requires an argument, two colons mean optional argument
    constexpr const char* SHORT_OPTIONS = ":hc:";
    int index;
    int32_t opt{-1};
    while (opt = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &index), opt != -1)
    {
        switch (opt)
        {
        case 'h':
        {
            // we want to parse the help option again, therefore we need to decrement the option index of getopt
            optind--;
            auto result = CmdLineParser::parse(argc, argv);
            if (result.has_error())
            {
                return err(result.error());
            }
            std::cout << std::endl;
            std::cout << "Config File Option:" << std::endl;
            std::cout << "-c, --config-file                 Path to the RouDi Config File." << std::endl;
            std::cout << "                                  Have a look at the documentation for the format."
                      << std::endl;
            std::cout << "                                  If option is not given, fallbacks in descending order:"
                      << std::endl;
            std::cout << "                                  1) /etc/iceoryx/roudi_config.toml" << std::endl;
            std::cout << "                                  2) hard-coded config" << std::endl;
            m_cmdLineArgs.run = false;
            break;
        }
        case 'c':
        {
            m_cmdLineArgs.configFilePath = roudi::ConfigFilePathString_t(TruncateToCapacity, optarg);
            break;
        }
        default:
        {
            // we want to parse the help option again, therefore we need to decrement the option index of getopt
            optind--;
            auto result = CmdLineParser::parse(argc, argv, CmdLineArgumentParsingMode::ONE);
            if (result.has_error())
            {
                return result;
            }
        }
        };

        if (cmdLineParsingMode == CmdLineArgumentParsingMode::ONE)
        {
            break;
        }
    }
    return ok(m_cmdLineArgs);
}

} // namespace config
} // namespace iox
