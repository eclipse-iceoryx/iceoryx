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
#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_utils/log/logging.hpp"
#include "iceoryx_utils/log/logmanager.hpp"
#include "iceoryx_utils/platform/getopt.hpp"
#include "iceoryx_utils/platform/pthread.hpp"
#include "iceoryx_utils/platform/resource.hpp"
#include "iceoryx_utils/platform/semaphore.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"

#include "stdio.h"
#include <signal.h>

namespace iox
{
namespace roudi
{
// using unnamed namespace to keep the functions in this translation unit
namespace
{
iox::roudi::RouDiApp* g_RouDiApp;
} // unnamed namespace

void RouDiApp::roudiSigHandler(int32_t signal) noexcept
{
    if (g_RouDiApp)
    {
        if (signal == SIGHUP)
        {
            LogWarn() << "SIGHUP not supported by RouDi";
        }
        // Post semaphore to exit
        g_RouDiApp->m_semaphore.post();
    }
}

void RouDiApp::registerSigHandler() noexcept
{
    /// @todo smart_c all the things

    // Save the pointer to self
    g_RouDiApp = this;

    // register sigHandler for SIGINT, SIGTERM and SIGHUP
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = roudiSigHandler;
    act.sa_flags = 0;
    if (-1 == sigaction(SIGINT, &act, NULL))
    {
        LogError() << "Calling sigaction() failed";
        std::terminate();
    }

    if (-1 == sigaction(SIGTERM, &act, NULL))
    {
        LogError() << "Calling sigaction() failed";
        std::terminate();
    }

    if (-1 == sigaction(SIGHUP, &act, NULL))
    {
        LogError() << "Calling sigaction() failed";
        std::terminate();
    }
}

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

RouDiApp::RouDiApp(const config::CmdLineParser& cmdLineParser, const RouDiConfig_t& config) noexcept
    : RouDiApp(config)
{
    setCmdLineParserResults(cmdLineParser);
    init();
}

RouDiApp::RouDiApp(const RouDiConfig_t& config) noexcept
    : m_run(checkAndOptimizeConfig(config))
    , m_config(config)
{
}

bool RouDiApp::checkAndOptimizeConfig(const RouDiConfig_t& config) noexcept
{
    if (config.m_sharedMemorySegments.empty())
    {
        LogError() << "A RouDiConfig without segments was specified! Please provide a valid config!";
        return false;
    }

    for (const auto& segment : config.m_sharedMemorySegments)
    {
        if (segment.m_mempoolConfig.m_mempoolConfig.empty())
        {
            LogError() << "A RouDiConfig with segments without mempools was specified! Please provide a valid config!";
            return false;
        }
    }

    return true;
}

RouDiConfig_t RouDiApp::generateConfigFromMePooConfig(const mepoo::MePooConfig* mePooConfig) noexcept
{
    RouDiConfig_t defaultConfig;
    defaultConfig.setDefaults();
    if (mePooConfig)
    {
        defaultConfig.m_sharedMemorySegments.front().m_mempoolConfig.m_mempoolConfig.clear();
        for (auto entry : *mePooConfig->getMemPoolConfig())
        {
            defaultConfig.m_sharedMemorySegments.front().m_mempoolConfig.m_mempoolConfig.push_back({entry});
        }
    }

    return defaultConfig;
}

void RouDiApp::init() noexcept
{
    // be silent if not running
    if (m_run)
    {
        iox::log::LogManager::GetLogManager().SetDefaultLogLevel(m_logLevel);

        registerSigHandler();
    }
}

bool RouDiApp::waitForSignal() const noexcept
{
    return m_semaphore.wait();
}

void RouDiApp::setCmdLineParserResults(const config::CmdLineParser& cmdLineParser) noexcept
{
    m_monitoringMode = cmdLineParser.getMonitoringMode();
    m_logLevel = cmdLineParser.getLogLevel();
    // the "and" is intentional, just in case the the provided RouDiConfig_t is empty
    m_run &= cmdLineParser.getRun();
    m_compatibilityCheckLevel = cmdLineParser.getCompatibilityCheckLevel();
    auto uniqueId = cmdLineParser.getUniqueRouDiId();
    if (uniqueId)
    {
        popo::internal::setUniqueRouDiId(*uniqueId);
    }
}

void RouDiApp::parseCmdLineArguments(int argc,
                                     char* argv[],
                                     config::CmdLineParser::CmdLineArgumentParsingMode cmdLineParsingMode
                                     [[gnu::unused]]) noexcept
{
    /// @todo Remove this from RouDi once the deprecated c'tors taking argc and argv have been removed
    config::CmdLineParser cmdLineParser;
    cmdLineParser.parse(argc, argv);
    setCmdLineParserResults(cmdLineParser);
}

} // namespace roudi
} // namespace iox
