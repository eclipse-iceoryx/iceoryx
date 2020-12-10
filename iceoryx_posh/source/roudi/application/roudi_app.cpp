// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#include "iceoryx_utils/platform/resource.hpp"
#include "iceoryx_utils/platform/semaphore.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/posix_wrapper/thread.hpp"

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
    // Save the pointer to self
    g_RouDiApp = this;

    // register sigHandler for SIGINT, SIGTERM and SIGHUP
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = roudiSigHandler;
    act.sa_flags = 0;
    if (cxx::makeSmartC(sigaction, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGINT, &act, nullptr)
            .hasErrors())
    {
        LogFatal() << "Calling sigaction() failed";
        errorHandler(Error::kROUDI_APP__COULD_NOT_REGISTER_SIGNALS, nullptr, ErrorLevel::FATAL);
        return;
    }

    if (cxx::makeSmartC(sigaction, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGTERM, &act, nullptr)
            .hasErrors())
    {
        LogFatal() << "Calling sigaction() failed";
        errorHandler(Error::kROUDI_APP__COULD_NOT_REGISTER_SIGNALS, nullptr, ErrorLevel::FATAL);
        return;
    }

    if (cxx::makeSmartC(sigaction, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGHUP, &act, nullptr)
            .hasErrors())
    {
        LogFatal() << "Calling sigaction() failed";
        errorHandler(Error::kROUDI_APP__COULD_NOT_REGISTER_SIGNALS, nullptr, ErrorLevel::FATAL);
        return;
    }
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
    return !m_semaphore.wait().has_error();
}

void RouDiApp::setCmdLineParserResults(const config::CmdLineParser& cmdLineParser) noexcept
{
    m_monitoringMode = cmdLineParser.getMonitoringMode();
    m_logLevel = cmdLineParser.getLogLevel();
    // the "and" is intentional, just in case the the provided RouDiConfig_t is empty
    m_run &= cmdLineParser.getRun();
    m_compatibilityCheckLevel = cmdLineParser.getCompatibilityCheckLevel();
    m_processKillDelay = cmdLineParser.getProcessKillDelay();
    auto uniqueId = cmdLineParser.getUniqueRouDiId();
    if (uniqueId)
    {
        popo::internal::setUniqueRouDiId(*uniqueId);
    }
}


} // namespace roudi
} // namespace iox
