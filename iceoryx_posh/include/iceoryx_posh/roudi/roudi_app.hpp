// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ROUDI_APP_HPP
#define IOX_POSH_ROUDI_ROUDI_APP_HPP

#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/cmd_line_args.hpp"
#include "iox/logging.hpp"

#include <cstdint>
#include <cstdio>

namespace iox
{
namespace roudi
{
/// @brief base class for RouDi daemons
class RouDiApp
{
  public:
    /// @brief C'tor with command line parser, which has already parsed the command line parameters
    /// @param[in] cmdLineParser reference to a command line parser object
    /// @param[in] config the configuration to use
    RouDiApp(const config::CmdLineArgs_t& cmdLineArgs, const RouDiConfig_t& config) noexcept;

    virtual ~RouDiApp() noexcept {};

    /// @brief interface to start the execution of the RouDi daemon
    /// @return Return code for programm execution
    virtual uint8_t run() noexcept = 0;

  protected:
    /// @brief waits for the next signal to RouDi daemon
    [[deprecated("in 3.0, removed in 4.0, use iox::posix::waitForTerminationRequest() from "
                 "'iceoryx_dust/posix_wrapper/signal_watcher.hpp'")]] bool
    waitForSignal() noexcept;

    iox::log::LogLevel m_logLevel{iox::log::LogLevel::WARN};
    roudi::MonitoringMode m_monitoringMode{roudi::MonitoringMode::ON};
    bool m_run{true};
    RouDiConfig_t m_config;

    version::CompatibilityCheckLevel m_compatibilityCheckLevel{version::CompatibilityCheckLevel::PATCH};
    units::Duration m_processKillDelay{roudi::PROCESS_DEFAULT_KILL_DELAY};

  private:
    bool checkAndOptimizeConfig(const RouDiConfig_t& config) noexcept;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_APP_HPP
