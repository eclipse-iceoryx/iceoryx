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
#ifndef IOX_POSH_ROUDI_ROUDI_APP_HPP
#define IOX_POSH_ROUDI_ROUDI_APP_HPP

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser.hpp"
#include "iceoryx_posh/roudi/roudi_config_file_provider.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iceoryx_utils/log/logcommon.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

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

    /// @deprecated Will be deprecated soon, please port to RouDiApp(const CmdLineParser&, const RouDiConfig_T&)
    static RouDiConfig_t generateConfigFromMePooConfig(const mepoo::MePooConfig* mePooConfig) noexcept;

    /// @deprecated Please port to RouDiApp(const CmdLineParser&, const RouDiConfig_T&)
    [[gnu::deprecated]]  RouDiApp(int argc, char* argv[], const mepoo::MePooConfig* mePooConfig = nullptr) noexcept;

    /// @deprecated Will be deprecated soon, please port to RouDiApp(const CmdLineParser&, const RouDiConfig_T&)
    RouDiApp(int argc, char* argv[], const RouDiConfig_t& config) noexcept;

    /// @brief C'tor with command line parser, which has already parsed the command line parameters
    /// @param[in] cmdLineParser reference to a command line parser object
    /// @param[in] config the configuration to use
    RouDiApp(const config::CmdLineParser& cmdLineParser, const RouDiConfig_t& config) noexcept;

    virtual ~RouDiApp() noexcept {};

    /// @brief interface to start the execution of the RouDi daemon
    virtual void run() noexcept = 0;

  protected:
    /// @brief this is needed for the child classes for custom CmdLineParser
    /// @param[in] config the configuration to use
    RouDiApp(const RouDiConfig_t& config) noexcept;

    /// @brief Tells the OS which signals shall be hooked
    void registerSigHandler() noexcept;

    void parseCmdLineArguments(int argc,
                               char* argv[],
                               config::CmdLineParser::CmdLineArgumentParsingMode cmdLineParsingMode =
                                   config::CmdLineParser::CmdLineArgumentParsingMode::ALL) noexcept;

    /// @brief Extracts from CmdLineParser and sets them
    void setCmdLineParserResults(const config::CmdLineParser& cmdLineParser) noexcept;

    /// @brief initialize the RouDi daemon
    void init() noexcept;

    /// @brief waits for the next signal to RouDi daemon
    bool waitForSignal() const noexcept;

    iox::log::LogLevel m_logLevel{iox::log::LogLevel::kWarn};
    config::MonitoringMode m_monitoringMode{config::MonitoringMode::ON};
    bool m_run{true};
    RouDiConfig_t m_config;

    posix::Semaphore m_semaphore = std::move(posix::Semaphore::create(0u)
                                                 .or_else([](posix::SemaphoreError&) {
                                                     std::cerr << "Unable to create the semaphore for RouDi"
                                                               << std::endl;
                                                     std::terminate();
                                                 })
                                                 .get_value());
    version::CompatibilityCheckLevel m_compatibilityCheckLevel{version::CompatibilityCheckLevel::PATCH};

  private:
    bool checkAndOptimizeConfig(const RouDiConfig_t& config) noexcept;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_APP_HPP
