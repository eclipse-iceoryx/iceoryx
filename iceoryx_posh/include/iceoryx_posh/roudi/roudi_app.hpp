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

#pragma once

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/log/logcommon.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{

/// @brief base class for RouDi daemons
class RouDiApp
{
  public:
    /// @brief This controls the process alive monitoring. Upon timeout a monitored process is removed
    /// and its resources made available. The process can then start and register itself again.
    /// Contrarily, unmonitored processes can be restarted but registration will fail.
    /// Once extended Runlevel Management is operational it will detect process absence and
    /// those processes can register again.
    /// ON - all processes are monitored
    /// OFF - no process is monitored
    enum class MonitoringMode
    {
        ON,
        OFF
    };

    /// @deprecated Please port to RouDiConfig_t
    /// @todo use the [[gnu::deprecated]] attribute in the next release
    static RouDiConfig_t generateConfigFromMePooConfig(const mepoo::MePooConfig* mePooConfig) noexcept;
    /// @deprecated Please port to RouDiConfig_t
    /// @todo use the [[gnu::deprecated]] attribute in the next release
    RouDiApp(int argc, char* argv[], const mepoo::MePooConfig* mePooConfig = nullptr) noexcept;

    /// @brief contructor to create a RouDi daemon with a given config
    /// @param[in] argc forwarding of command line arguments
    /// @param[in] argv forwarding of command line arguments
    /// @param[in] config the configuration to use
    RouDiApp(int argc, char* argv[], const RouDiConfig_t& config) noexcept;

    virtual ~RouDiApp() noexcept {};

    /// @brief interface to start the execution of the RouDi daemon
    virtual void run() noexcept = 0;

  protected:
    enum class CmdLineArgumentParsingMode
    {
        ALL,
        ONE
    };

    /// @brief this is needed for the child classes to extend the parseCmdLineArguments function
    /// @param[in] config the configuration to use
    RouDiApp(const RouDiConfig_t& config) noexcept;

    /// @brief initialize the RouDi daemon
    void init() noexcept;

    /// @brief waits for termination of RouDi daemon
    void waitToFinish() noexcept;

    /// @brief process the passed command line arguments
    /// @param[in] argc forwarding of command line arguments
    /// @param[in] argv forwarding of command line arguments
    /// @param[in] cmdLineParsingMode the parser to use
    void
    parseCmdLineArguments(int argc,
                          char* argv[],
                          CmdLineArgumentParsingMode cmdLineParsingMode = CmdLineArgumentParsingMode::ALL) noexcept;

    bool m_run{true};
    iox::log::LogLevel m_logLevel{iox::log::LogLevel::kWarn};
    MonitoringMode m_monitoringMode{MonitoringMode::ON};
    RouDiConfig_t m_config;
};

} // namespace roudi
} // namespace iox
