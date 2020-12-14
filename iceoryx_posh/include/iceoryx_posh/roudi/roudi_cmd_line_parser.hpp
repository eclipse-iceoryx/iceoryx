// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ROUDI_CMD_LINE_PARSER_HPP
#define IOX_POSH_ROUDI_ROUDI_CMD_LINE_PARSER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/log/logcommon.hpp"

namespace iox
{
namespace config
{
class CmdLineParser
{
  public:
    enum class CmdLineArgumentParsingMode
    {
        ALL,
        ONE
    };

    CmdLineParser() noexcept = default;
    virtual ~CmdLineParser() noexcept = default;
    CmdLineParser& operator=(const CmdLineParser& other) = delete;
    CmdLineParser(const CmdLineParser& other) = delete;
    CmdLineParser& operator=(CmdLineParser&&) = delete;
    CmdLineParser(CmdLineParser&& other) = delete;

    /// @brief process the passed command line arguments
    /// @param[in] argc forwarding of command line arguments
    /// @param[in] argv forwarding of command line arguments
    /// @param[in] cmdLineParsingMode selects to parse a single option or all options
    virtual void parse(int argc,
                       char* argv[],
                       const CmdLineArgumentParsingMode cmdLineParsingMode = CmdLineArgumentParsingMode::ALL) noexcept;

    void printParameters() const noexcept;

    bool getRun() const noexcept;
    iox::log::LogLevel getLogLevel() const noexcept;
    roudi::MonitoringMode getMonitoringMode() const noexcept;
    version::CompatibilityCheckLevel getCompatibilityCheckLevel() const noexcept;
    cxx::optional<uint16_t> getUniqueRouDiId() const noexcept;
    units::Duration getProcessKillDelay() const noexcept;

  protected:
    bool m_run{true};
    iox::log::LogLevel m_logLevel{iox::log::LogLevel::kWarn};
    roudi::MonitoringMode m_monitoringMode{roudi::MonitoringMode::ON};
    version::CompatibilityCheckLevel m_compatibilityCheckLevel{version::CompatibilityCheckLevel::PATCH};
    cxx::optional<uint16_t> m_uniqueRouDiId;
    units::Duration m_processKillDelay{roudi::PROCESS_DEFAULT_KILL_DELAY};
};

} // namespace config
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_CMD_LINE_PARSER_HPP
