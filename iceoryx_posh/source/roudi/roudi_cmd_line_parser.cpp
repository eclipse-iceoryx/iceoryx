// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/roudi/roudi_cmd_line_parser.hpp"
#include "iceoryx_dust/cxx/convert.hpp"
#include "iceoryx_versions.hpp"
#include "iox/logging.hpp"

#include "iceoryx_platform/getopt.hpp"
#include <iostream>

namespace iox
{
namespace config
{
expected<CmdLineArgs_t, CmdLineParserResult>
CmdLineParser::parse(int argc, char* argv[], const CmdLineArgumentParsingMode cmdLineParsingMode) noexcept
{
    constexpr option LONG_OPTIONS[] = {{"help", no_argument, nullptr, 'h'},
                                       {"version", no_argument, nullptr, 'v'},
                                       {"monitoring-mode", required_argument, nullptr, 'm'},
                                       {"log-level", required_argument, nullptr, 'l'},
                                       {"unique-roudi-id", required_argument, nullptr, 'u'},
                                       {"compatibility", required_argument, nullptr, 'x'},
                                       {"kill-delay", required_argument, nullptr, 'k'},
                                       {nullptr, 0, nullptr, 0}};

    // colon after shortOption means it requires an argument, two colons mean optional argument
    constexpr const char* SHORT_OPTIONS = "hvm:l:u:x:k:";
    int32_t index;
    int32_t opt{-1};
    while ((opt = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &index), opt != -1))
    {
        switch (opt)
        {
        case 'h':
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "-h, --help                        Display help." << std::endl;
            std::cout << "-v, --version                     Display version." << std::endl;
            std::cout << "-u, --unique-roudi-id <INT>       Set the unique RouDi id." << std::endl;
            std::cout << "-m, --monitoring-mode <MODE>      Set process alive monitoring mode." << std::endl;
            std::cout << "                                  <MODE> {on, off}" << std::endl;
            std::cout << "                                  default = 'off'" << std::endl;
            std::cout << "                                  on: enables monitoring for all processes" << std::endl;
            std::cout << "                                  off: disables monitoring for all processes" << std::endl;
            std::cout << "-l, --log-level <LEVEL>           Set log level." << std::endl;
            std::cout << "                                  <LEVEL> {off, fatal, error, warning, info, debug, trace}"
                      << std::endl;
            std::cout << "-x, --compatibility               Set compatibility check level between runtime and RouDi."
                      << std::endl;
            std::cout << "                                  off: no check" << std::endl;
            std::cout << "                                  major: same major version " << std::endl;
            std::cout << "                                  minor: same minor version + major check" << std::endl;
            std::cout << "                                  patch: same patch version + minor check" << std::endl;
            std::cout << "                                  commitId: same commit ID + patch check" << std::endl;
            std::cout << "                                  buildDate: same build date + commId check" << std::endl;
            std::cout << "-k, --kill-delay <UINT>           Sets the delay when RouDi sends SIG_KILL, if apps"
                      << std::endl;
            std::cout << "                                  have't responded after trying SIG_TERM first, in seconds."
                      << std::endl;

            m_run = false;
            break;
        case 'v':
            std::cout << "RouDi version: " << ICEORYX_LATEST_RELEASE_VERSION << std::endl;
            std::cout << "Build date: " << ICEORYX_BUILDDATE << std::endl;
            std::cout << "Commit ID: " << ICEORYX_SHA1 << std::endl;
            m_run = false;
            break;
        case 'u':
        {
            uint16_t roudiId{0u};
            constexpr uint64_t MAX_ROUDI_ID = ((1 << 16) - 1);
            if (!cxx::convert::fromString(optarg, roudiId))
            {
                IOX_LOG(ERROR) << "The RouDi id must be in the range of [0, " << MAX_ROUDI_ID << "]";
                m_run = false;
            }

            m_uniqueRouDiId.emplace(roudiId);
            break;
        }
        case 'm':
        {
            if (strcmp(optarg, "on") == 0)
            {
                m_monitoringMode = roudi::MonitoringMode::ON;
            }
            else if (strcmp(optarg, "off") == 0)
            {
                m_monitoringMode = roudi::MonitoringMode::OFF;
            }
            else
            {
                m_run = false;
                IOX_LOG(ERROR) << "Options for monitoring-mode are 'on' and 'off'!";
            }
            break;
        }
        case 'l':
        {
            if (strcmp(optarg, "off") == 0)
            {
                m_logLevel = iox::log::LogLevel::OFF;
            }
            else if (strcmp(optarg, "fatal") == 0)
            {
                m_logLevel = iox::log::LogLevel::FATAL;
            }
            else if (strcmp(optarg, "error") == 0)
            {
                m_logLevel = iox::log::LogLevel::ERROR;
            }
            else if (strcmp(optarg, "warning") == 0)
            {
                m_logLevel = iox::log::LogLevel::WARN;
            }
            else if (strcmp(optarg, "info") == 0)
            {
                m_logLevel = iox::log::LogLevel::INFO;
            }
            else if (strcmp(optarg, "debug") == 0)
            {
                m_logLevel = iox::log::LogLevel::DEBUG;
            }
            else if (strcmp(optarg, "trace") == 0)
            {
                m_logLevel = iox::log::LogLevel::TRACE;
            }
            else
            {
                m_run = false;
                IOX_LOG(ERROR) << "Options for log-level are 'off', 'fatal', 'error', 'warning', 'info', 'debug' and "
                                  "'trace'!";
            }
            break;
        }
        case 'k':
        {
            uint32_t processKillDelayInSeconds{0u};
            constexpr uint64_t MAX_PROCESS_KILL_DELAY = std::numeric_limits<uint32_t>::max();
            if (!cxx::convert::fromString(optarg, processKillDelayInSeconds))
            {
                IOX_LOG(ERROR) << "The process kill delay must be in the range of [0, " << MAX_PROCESS_KILL_DELAY
                               << "]";
                m_run = false;
            }
            else
            {
                m_processKillDelay = units::Duration::fromSeconds(processKillDelayInSeconds);
            }
            break;
        }
        case 'x':
        {
            if (strcmp(optarg, "off") == 0)
            {
                m_compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::OFF;
            }
            else if (strcmp(optarg, "major") == 0)
            {
                m_compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::MAJOR;
            }
            else if (strcmp(optarg, "minor") == 0)
            {
                m_compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::MINOR;
            }
            else if (strcmp(optarg, "patch") == 0)
            {
                m_compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::PATCH;
            }
            else if (strcmp(optarg, "commitId") == 0)
            {
                m_compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::COMMIT_ID;
            }
            else if (strcmp(optarg, "buildDate") == 0)
            {
                m_compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::BUILD_DATE;
            }
            else
            {
                m_run = false;
                IOX_LOG(ERROR)
                    << "Options for compatibility are 'off', 'major', 'minor', 'patch', 'commitId' and 'buildDate'!";
            }
            break;
        }
        default:
        {
            // CmdLineParser did not understand the parameters, don't run
            m_run = false;
            return error<CmdLineParserResult>(CmdLineParserResult::UNKNOWN_OPTION_USED);
        }
        };

        if (cmdLineParsingMode == CmdLineArgumentParsingMode::ONE)
        {
            break;
        }
    }
    return success<CmdLineArgs_t>(CmdLineArgs_t{m_monitoringMode,
                                                m_logLevel,
                                                m_compatibilityCheckLevel,
                                                m_processKillDelay,
                                                m_uniqueRouDiId,
                                                m_run,
                                                iox::roudi::ConfigFilePathString_t("")});
} // namespace roudi
} // namespace config
} // namespace iox
