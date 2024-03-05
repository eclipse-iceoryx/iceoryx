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
#include "iceoryx_versions.hpp"
#include "iox/detail/convert.hpp"
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
                                       {"domain-id", required_argument, nullptr, 'd'},
                                       {"unique-roudi-id", required_argument, nullptr, 'u'},
                                       {"compatibility", required_argument, nullptr, 'x'},
                                       {"termination-delay", required_argument, nullptr, 't'},
                                       {"kill-delay", required_argument, nullptr, 'k'},
                                       {nullptr, 0, nullptr, 0}};

    // colon after shortOption means it requires an argument, two colons mean optional argument
    constexpr const char* SHORT_OPTIONS = "hvm:l:d:u:x:t:k:";
    int index;
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
            std::cout << "-d, --domain-id <UINT>            Set the Domain ID." << std::endl;
            std::cout << "                                  <UINT> 0..65535" << std::endl;
            std::cout << "                                  Experimental!" << std::endl;
            std::cout << "-u, --unique-roudi-id <UINT>      Set the unique RouDi ID." << std::endl;
            std::cout << "                                  <UINT> 0..65535" << std::endl;
            std::cout << "-m, --monitoring-mode <MODE>      Set process alive monitoring mode." << std::endl;
            std::cout << "                                  <MODE> {on, off}" << std::endl;
            std::cout << "                                  default = 'off'" << std::endl;
            std::cout << "                                  on: enables monitoring for all processes" << std::endl;
            std::cout << "                                  off: disables monitoring for all processes" << std::endl;
            std::cout << "-l, --log-level <LEVEL>           Set log level." << std::endl;
            std::cout << "                                  <LEVEL> {off, fatal, error, warning, info," << std::endl;
            std::cout << "                                  debug, trace}" << std::endl;
            std::cout << "                                  default = 'info'" << std::endl;
            std::cout << "-x, --compatibility               Set compatibility check level between runtime" << std::endl;
            std::cout << "                                  and RouDi. Value are" << std::endl;
            std::cout << "                                  off: no check" << std::endl;
            std::cout << "                                  major: same major version " << std::endl;
            std::cout << "                                  minor: same minor version + major check" << std::endl;
            std::cout << "                                  patch: same patch version + minor check" << std::endl;
            std::cout << "                                  commitId: same commit ID + patch check" << std::endl;
            std::cout << "                                  buildDate: same build date + commId check" << std::endl;
            std::cout << "                                  default = 'patch'" << std::endl;
            std::cout << "-t, --termination-delay <UINT>    Sets the delay in seconds before RouDi sends" << std::endl;
            std::cout << "                                  SIGTERM to running applications at shutdown." << std::endl;
            std::cout << "                                  When RouDi and the applications are running" << std::endl;
            std::cout << "                                  in an automated environment like" << std::endl;
            std::cout << "                                  launch_testing, where the framework takes" << std::endl;
            std::cout << "                                  care of the shutdown, this results in a race" << std::endl;
            std::cout << "                                  between RouDi and the framework in" << std::endl;
            std::cout << "                                  terminating the applications. To prevent this" << std::endl;
            std::cout << "                                  race, this parameter can be used to delay the" << std::endl;
            std::cout << "                                  raising of SIGTERM by a few seconds." << std::endl;
            std::cout << "                                  default = '0'" << std::endl;
            std::cout << "-k, --kill-delay <UINT>           Sets the delay in seconds before RouDi sends" << std::endl;
            std::cout << "                                  SIGKILL to application which did not respond" << std::endl;
            std::cout << "                                  to the initial SIGTERM signal." << std::endl;
            std::cout << "                                  default = '45'" << std::endl;

            m_cmdLineArgs.run = false;
            break;
        case 'v':
            std::cout << "RouDi version: " << ICEORYX_LATEST_RELEASE_VERSION << std::endl;
            std::cout << "Build date: " << ICEORYX_BUILDDATE << std::endl;
            std::cout << "Commit ID: " << ICEORYX_SHA1 << std::endl;
            m_cmdLineArgs.run = false;
            break;
        case 'd':
        {
            constexpr uint64_t MAX_DOMAIN_ID = ((1 << 16) - 1);
            auto maybeValue = convert::from_string<uint16_t>(optarg);
            if (!maybeValue.has_value())
            {
                IOX_LOG(ERROR, "The domain ID must be in the range of [0, " << MAX_DOMAIN_ID << "]");
                return err(CmdLineParserResult::INVALID_PARAMETER);
            }

            if (experimental::hasExperimentalPoshFeaturesEnabled())
            {
                m_cmdLineArgs.roudiConfig.domainId = DomainId{maybeValue.value()};
            }
            else
            {
                IOX_LOG(WARN,
                        "The domain ID is an experimental feature and iceoryx must be compiled with the "
                        "'IOX_EXPERIMENTAL_POSH' cmake option to use it!");
            }

            break;
        }
        case 'u':
        {
            constexpr uint64_t MAX_ROUDI_ID = ((1 << 16) - 1);
            auto maybeValue = convert::from_string<uint16_t>(optarg);
            if (!maybeValue.has_value())
            {
                IOX_LOG(ERROR, "The RouDi ID must be in the range of [0, " << MAX_ROUDI_ID << "]");
                return err(CmdLineParserResult::INVALID_PARAMETER);
            }

            m_cmdLineArgs.roudiConfig.uniqueRouDiId = roudi::UniqueRouDiId{maybeValue.value()};
            break;
        }
        case 'm':
        {
            if (strcmp(optarg, "on") == 0)
            {
                m_cmdLineArgs.roudiConfig.monitoringMode = roudi::MonitoringMode::ON;
            }
            else if (strcmp(optarg, "off") == 0)
            {
                m_cmdLineArgs.roudiConfig.monitoringMode = roudi::MonitoringMode::OFF;
            }
            else
            {
                IOX_LOG(ERROR, "Options for monitoring-mode are 'on' and 'off'!");
                return err(CmdLineParserResult::INVALID_PARAMETER);
            }
            break;
        }
        case 'l':
        {
            if (strcmp(optarg, "off") == 0)
            {
                m_cmdLineArgs.roudiConfig.logLevel = iox::log::LogLevel::OFF;
            }
            else if (strcmp(optarg, "fatal") == 0)
            {
                m_cmdLineArgs.roudiConfig.logLevel = iox::log::LogLevel::FATAL;
            }
            else if (strcmp(optarg, "error") == 0)
            {
                m_cmdLineArgs.roudiConfig.logLevel = iox::log::LogLevel::ERROR;
            }
            else if (strcmp(optarg, "warning") == 0)
            {
                m_cmdLineArgs.roudiConfig.logLevel = iox::log::LogLevel::WARN;
            }
            else if (strcmp(optarg, "info") == 0)
            {
                m_cmdLineArgs.roudiConfig.logLevel = iox::log::LogLevel::INFO;
            }
            else if (strcmp(optarg, "debug") == 0)
            {
                m_cmdLineArgs.roudiConfig.logLevel = iox::log::LogLevel::DEBUG;
            }
            else if (strcmp(optarg, "trace") == 0)
            {
                m_cmdLineArgs.roudiConfig.logLevel = iox::log::LogLevel::TRACE;
            }
            else
            {
                IOX_LOG(ERROR,
                        "Options for log-level are 'off', 'fatal', 'error', 'warning', 'info', 'debug' and 'trace'!");
                return err(CmdLineParserResult::INVALID_PARAMETER);
            }
            break;
        }
        case 't':
        {
            constexpr uint64_t MAX_PROCESS_TERMINATION_DELAY = std::numeric_limits<uint32_t>::max();
            auto maybeValue = convert::from_string<uint32_t>(optarg);
            if (!maybeValue.has_value())
            {
                IOX_LOG(ERROR,
                        "The process termination delay must be in the range of [0, " << MAX_PROCESS_TERMINATION_DELAY
                                                                                     << "]");
                return err(CmdLineParserResult::INVALID_PARAMETER);
            }

            m_cmdLineArgs.roudiConfig.processTerminationDelay = units::Duration::fromSeconds(maybeValue.value());
            break;
        }
        case 'k':
        {
            constexpr uint64_t MAX_PROCESS_KILL_DELAY = std::numeric_limits<uint32_t>::max();
            auto maybeValue = convert::from_string<uint32_t>(optarg);
            if (!maybeValue.has_value())
            {
                IOX_LOG(ERROR, "The process kill delay must be in the range of [0, " << MAX_PROCESS_KILL_DELAY << "]");
                return err(CmdLineParserResult::INVALID_PARAMETER);
            }

            m_cmdLineArgs.roudiConfig.processKillDelay = units::Duration::fromSeconds(maybeValue.value());
            break;
        }
        case 'x':
        {
            if (strcmp(optarg, "off") == 0)
            {
                m_cmdLineArgs.roudiConfig.compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::OFF;
            }
            else if (strcmp(optarg, "major") == 0)
            {
                m_cmdLineArgs.roudiConfig.compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::MAJOR;
            }
            else if (strcmp(optarg, "minor") == 0)
            {
                m_cmdLineArgs.roudiConfig.compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::MINOR;
            }
            else if (strcmp(optarg, "patch") == 0)
            {
                m_cmdLineArgs.roudiConfig.compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::PATCH;
            }
            else if (strcmp(optarg, "commitId") == 0)
            {
                m_cmdLineArgs.roudiConfig.compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::COMMIT_ID;
            }
            else if (strcmp(optarg, "buildDate") == 0)
            {
                m_cmdLineArgs.roudiConfig.compatibilityCheckLevel = iox::version::CompatibilityCheckLevel::BUILD_DATE;
            }
            else
            {
                IOX_LOG(ERROR,
                        "Options for compatibility are 'off', 'major', 'minor', 'patch', 'commitId' and 'buildDate'!");
                return err(CmdLineParserResult::INVALID_PARAMETER);
            }
            break;
        }
        default:
        {
            return err(CmdLineParserResult::UNKNOWN_OPTION_USED);
        }
        };

        if (cmdLineParsingMode == CmdLineArgumentParsingMode::ONE)
        {
            break;
        }
    }
    return ok(m_cmdLineArgs);
} // namespace roudi
} // namespace config
} // namespace iox
