// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_CMD_LINE_ARGS_HPP
#define IOX_POSH_ROUDI_CMD_LINE_ARGS_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iox/log/logstream.hpp"

#include <cstdint>

namespace iox
{
namespace config
{
struct CmdLineArgs_t
{
    roudi::MonitoringMode monitoringMode{roudi::MonitoringMode::OFF};
    iox::log::LogLevel logLevel{iox::log::LogLevel::INFO};
    version::CompatibilityCheckLevel compatibilityCheckLevel{version::CompatibilityCheckLevel::PATCH};
    units::Duration processKillDelay{roudi::PROCESS_DEFAULT_KILL_DELAY};
    optional<uint16_t> uniqueRouDiId{nullopt};
    bool run{true};
    roudi::ConfigFilePathString_t configFilePath;
};

inline iox::log::LogStream& operator<<(iox::log::LogStream& logstream, const CmdLineArgs_t& cmdLineArgs) noexcept
{
    logstream << "Log level: " << cmdLineArgs.logLevel << "\n";
    logstream << "Monitoring mode: " << cmdLineArgs.monitoringMode << "\n";
    logstream << "Compatibility check level: " << cmdLineArgs.compatibilityCheckLevel << "\n";
    cmdLineArgs.uniqueRouDiId.and_then([&logstream](auto& id) { logstream << "Unique RouDi ID: " << id << "\n"; })
        .or_else([&logstream] { logstream << "Unique RouDi ID: < unset >\n"; });
    logstream << "Process kill delay: " << cmdLineArgs.processKillDelay.toSeconds() << " s\n";
    if (!cmdLineArgs.configFilePath.empty())
    {
        logstream << "Config file used is: " << cmdLineArgs.configFilePath;
    }
    else
    {
        logstream << "Config file used is: < none >";
    }
    return logstream;
}
} // namespace config
} // namespace iox

#endif // IOX_POSH_ROUDI_CMD_LINE_ARGS_HPP
