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

#include "iceoryx_hoofs/log/logcommon.hpp"
#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"
#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/cmd_line_args.hpp"

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
    /// @brief Method passed to the OS signal handler
    static void roudiSigHandler(int32_t signal) noexcept;

    /// @brief C'tor with command line parser, which has already parsed the command line parameters
    /// @param[in] cmdLineParser reference to a command line parser object
    /// @param[in] config the configuration to use
    RouDiApp(const config::CmdLineArgs_t& cmdLineArgs, const RouDiConfig_t& config) noexcept;

    virtual ~RouDiApp() noexcept {};

    /// @brief interface to start the execution of the RouDi daemon
    /// @return Return code for programm execution
    virtual uint8_t run() noexcept = 0;

  protected:
    /// @brief Tells the OS which signals shall be hooked
    void registerSigHandler() noexcept;

    /// @brief waits for the next signal to RouDi daemon
    bool waitForSignal() noexcept;

    iox::log::LogLevel m_logLevel{iox::log::LogLevel::kWarn};
    roudi::MonitoringMode m_monitoringMode{roudi::MonitoringMode::ON};
    bool m_run{true};
    RouDiConfig_t m_config;

    posix::Semaphore m_semaphore =
        std::move(posix::Semaphore::create(posix::CreateUnnamedSingleProcessSemaphore, 0u)
                      .or_else([](posix::SemaphoreError&) {
                          errorHandler(Error::kROUDI_APP__FAILED_TO_CREATE_SEMAPHORE, nullptr, ErrorLevel::FATAL);
                      })
                      .value());
    version::CompatibilityCheckLevel m_compatibilityCheckLevel{version::CompatibilityCheckLevel::PATCH};
    units::Duration m_processKillDelay{roudi::PROCESS_DEFAULT_KILL_DELAY};

  private:
    bool checkAndOptimizeConfig(const RouDiConfig_t& config) noexcept;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_APP_HPP
