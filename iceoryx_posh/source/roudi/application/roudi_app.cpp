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

#include "iceoryx_posh/roudi/roudi_app.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/roudi/roudi_multi_process.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_utils/log/logging.hpp"
#include "iceoryx_utils/log/logmanager.hpp"
#include "iceoryx_utils/platform/getopt.hpp"
#include "iceoryx_utils/platform/pthread.hpp"
#include "iceoryx_utils/platform/resource.hpp"
#include "iceoryx_utils/platform/semaphore.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "iceoryx_versions.hpp"

#include "stdio.h"
#include <signal.h>

namespace iox
{
namespace roudi
{
// using unnamed namespace to keep the functions in this translation unit
namespace
{
static posix::Semaphore g_runSemaphore{[] {
    auto runSemaphore = posix::Semaphore::create(ROUDI_APP_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0);
    if (runSemaphore.has_error())
    {
        std::cerr << "Unable to create runSemaphore \"" << ROUDI_APP_SEMAPHORE_NAME
                  << "\", either RouDi is already running or someone else has already created this semaphore."
                  << std::endl;
        errorHandler(Error::kROUDI_APPLICATION__SEMAPHORE_PRESENT_ROUDI_ALREADY_RUNNING);
    }

    return std::move(*runSemaphore);
}()};

void roudiSigHandler(int /* sig */) noexcept
{
    g_runSemaphore.post(); // post semaphore to exit the application
}

void registerSigHandler() noexcept
{
    // register sigHandler for SIGINT and SIGTERM
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = roudiSigHandler;
    act.sa_flags = 0;
    if (-1 == sigaction(SIGINT, &act, NULL))
    {
        LogError() << "Calling sigaction() failed";
        exit(EXIT_FAILURE);
    }

    if (-1 == sigaction(SIGTERM, &act, NULL))
    {
        LogError() << "Calling sigaction() failed";
        exit(EXIT_FAILURE);
    }
}

} // unnamed namespace

RouDiApp::RouDiApp(int argc, char* argv[], const mepoo::MePooConfig* mePooConfig) noexcept
    : RouDiApp(argc, argv, generateConfigFromMePooConfig(mePooConfig))
{
}

RouDiApp::RouDiApp(int argc, char* argv[], const RouDiConfig_t& config) noexcept
    : RouDiApp(config)
{
    parseCmdLineArguments(argc, argv);

    init();
}

RouDiApp::RouDiApp(const RouDiConfig_t& config) noexcept
    : m_config(config)
{
    m_config.optimize();
}

RouDiApp::RouDiApp(int argc, char* argv[], RouDiConfigFileParser* configFileParser) noexcept
    : m_configFileParser(configFileParser)
{
    m_config.setDefaults();

    parseCmdLineArguments(argc, argv);

    init();
}

RouDiConfig_t RouDiApp::generateConfigFromMePooConfig(const mepoo::MePooConfig* mePooConfig) noexcept
{
    RouDiConfig_t defaultConfig;
    defaultConfig.setDefaults();
    if (mePooConfig)
    {
        defaultConfig.m_sharedMemorySegments[0].m_mempoolConfig.m_mempoolConfig.clear();
        for (auto entry : *mePooConfig->getMemPoolConfig())
        {
            defaultConfig.m_sharedMemorySegments[0].m_mempoolConfig.m_mempoolConfig.push_back({entry});
        }
    }

    /// @todo the best guess mapping address as long as we do not have introduced relative pointers
    defaultConfig.roudi.m_sharedMemoryBaseAddressOffset = 0x3E80000000ull;
    // the payload segments are now relocatable, therefore placement check can be omitted
    defaultConfig.roudi.m_verifySharedMemoryPlacement = false;

    return defaultConfig;
}

void RouDiApp::init() noexcept
{
    iox::log::LogManager::GetLogManager().SetDefaultLogLevel(m_logLevel);

    registerSigHandler();
}

void RouDiApp::waitToFinish() noexcept
{
    g_runSemaphore.wait();
}

void RouDiApp::parseCmdLineArguments(int argc, char* argv[], CmdLineArgumentParsingMode cmdLineParsingMode) noexcept
{
    constexpr option longOptions[] = {{"help", no_argument, nullptr, 'h'},
                                      {"version", no_argument, nullptr, 'v'},
                                      {"monitoring-mode", required_argument, nullptr, 'm'},
                                      {"log-level", required_argument, nullptr, 'l'},
                                      {"config-file", required_argument, nullptr, 'c'},
                                      {nullptr, 0, nullptr, 0}};

    // colon after shortOption means it requires an argument, two colons mean optional argument
    constexpr const char* shortOptions = "hvm:l:b:c:";
    int index;
    int opt{-1};
    while (opt = getopt_long(argc, argv, shortOptions, longOptions, &index), opt != -1)
    {
        switch (opt)
        {
        case 'h':
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "-h, --help                        Display help." << std::endl;
            std::cout << "-v, --version                     Display version." << std::endl;
            std::cout << "-m, --monitoring-mode <MODE>      Set process alive monitoring mode." << std::endl;
            std::cout << "                                  <MODE> {on, off}" << std::endl;
            std::cout << "                                  default = 'on'" << std::endl;
            std::cout << "                                  on: enables monitoring for all processes" << std::endl;
            std::cout << "                                  off: disables monitoring for all processes" << std::endl;
            std::cout << "-l, --log-level <LEVEL>           Set log level." << std::endl;
            std::cout << "                                  <LEVEL> {off, fatal, error, warning, info, debug, verbose}"
                      << std::endl;
            std::cout << "-c, --config-file                 Path to the RouDi Config File."
                         "                                  Have a look at the documentation for the format."
                      << std::endl;
            m_run = false;
            break;
        case 'v':
            std::cout << "RouDi version: " << ICEORYX_LATEST_RELEASE_VERSION << std::endl;
            std::cout << "Build date: " << ICEORYX_BUILDDATE << std::endl;
            m_run = false;
            break;

        case 'm': {
            if (strcmp(optarg, "on") == 0)
            {
                m_monitoringMode = MonitoringMode::ON;
            }
            else if (strcmp(optarg, "off") == 0)
            {
                m_monitoringMode = MonitoringMode::OFF;
            }
            else
            {
                m_run = false;
                LogError() << "Options for monitoring-mode are 'on' and 'off'!";
            }
            break;
        }

        case 'l': {
            if (strcmp(optarg, "off") == 0)
            {
                m_logLevel = iox::log::LogLevel::kOff;
            }
            else if (strcmp(optarg, "fatal") == 0)
            {
                m_logLevel = iox::log::LogLevel::kFatal;
            }
            else if (strcmp(optarg, "error") == 0)
            {
                m_logLevel = iox::log::LogLevel::kError;
            }
            else if (strcmp(optarg, "warning") == 0)
            {
                m_logLevel = iox::log::LogLevel::kWarn;
            }
            else if (strcmp(optarg, "info") == 0)
            {
                m_logLevel = iox::log::LogLevel::kInfo;
            }
            else if (strcmp(optarg, "debug") == 0)
            {
                m_logLevel = iox::log::LogLevel::kDebug;
            }
            else if (strcmp(optarg, "verbose") == 0)
            {
                m_logLevel = iox::log::LogLevel::kVerbose;
            }
            else
            {
                m_run = false;
                LogError()
                    << "Options for log-level are 'off', 'fatal', 'error', 'waring', 'info', 'debug' and 'verbose'!";
            }
            break;
        }

        case 'c': {
            if (!m_configFileParser)
            {
                LogFatal() << "Config File Parsing is not implemented!";
                exit(EXIT_FAILURE);
            }
            else
            {
                using ParseResult = cxx::expected<RouDiConfig_t, RouDiConfigFileParseError>;
                m_configFileParser->parse(ConfigFilePathString_t(cxx::UnsafeCheckPreconditions, optarg))
                    .on_success([this](ParseResult& parseResult) { this->m_config = *parseResult; })
                    .on_error([](ParseResult& parseResult) {
                        iox::LogFatal() << "Couldn't parse config file. Error: "
                                        << cxx::convertEnumToString(ROUDI_CONFIG_FILE_PARSE_ERROR_STRINGS,
                                                                    parseResult.get_error());
                        exit(EXIT_FAILURE);
                    });
            }
            break;
        }

        default: {
            m_run = false;
        }
        };

        if (cmdLineParsingMode == CmdLineArgumentParsingMode::ONE)
        {
            break;
        }
    }
}

} // namespace roudi
} // namespace iox
