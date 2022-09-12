// Copyright (c) 2019, 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/roudi_app.hpp"

#include "iceoryx_dust/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/memory_map.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logmanager.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "iceoryx_platform/getopt.hpp"
#include "iceoryx_platform/resource.hpp"
#include "iceoryx_platform/semaphore.hpp"
#include "iceoryx_platform/signal.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/cmd_line_args.hpp"

#include <cstdio>

namespace iox
{
namespace roudi
{
RouDiApp::RouDiApp(const config::CmdLineArgs_t& cmdLineArgs, const RouDiConfig_t& config) noexcept
    : m_logLevel(cmdLineArgs.logLevel)
    , m_monitoringMode(cmdLineArgs.monitoringMode)
    , m_run(checkAndOptimizeConfig(config))
    , m_config(config)
    , m_compatibilityCheckLevel(cmdLineArgs.compatibilityCheckLevel)
    , m_processKillDelay(cmdLineArgs.processKillDelay)
{
    // the "and" is intentional, just in case the the provided RouDiConfig_t is empty
    m_run &= cmdLineArgs.run;
    if (cmdLineArgs.uniqueRouDiId)
    {
        popo::UniquePortId::setUniqueRouDiId(cmdLineArgs.uniqueRouDiId.value());
    }

    // be silent if not running
    if (m_run)
    {
        iox::log::LogManager::GetLogManager().SetDefaultLogLevel(m_logLevel);

        LogVerbose() << "Command line parameters are:\n" << cmdLineArgs;
    }
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

bool RouDiApp::waitForSignal() noexcept
{
    iox::posix::waitForTerminationRequest();
    return true;
}

} // namespace roudi
} // namespace iox
