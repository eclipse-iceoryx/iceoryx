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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_ROUDI_ROUDI_CONFIG_HPP
#define IOX_POSH_ROUDI_ROUDI_CONFIG_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"

#include <cstdint>

namespace iox
{
namespace config
{
struct RouDiConfig
{
    /// @brief The domain ID which is used to tie the iceoryx resources to when created in the file system
    DomainId domainId{DEFAULT_DOMAIN_ID};
    /// @brief The unique RouDi id used for the unique port id in order to distinguish between remote and local ports
    roudi::UniqueRouDiId uniqueRouDiId{roudi::DEFAULT_UNIQUE_ROUDI_ID};
    /// @brief Specifies whether RouDi is running in the same address space as the applications as it is the case with
    /// RouDiEnv
    bool sharesAddressSpaceWithApplications{false};
    /// @brief the log level used by RouDi
    iox::log::LogLevel logLevel{iox::log::LogLevel::INFO};
    /// @brief Specifies whether RouDi monitors the process for abnormal termination
    roudi::MonitoringMode monitoringMode{roudi::MonitoringMode::OFF};
    /// @brief Specifies to which level the compatibility of applications trying to register with RouDi should be
    /// checked
    version::CompatibilityCheckLevel compatibilityCheckLevel{version::CompatibilityCheckLevel::PATCH};
    /// @brief Sets the delay in seconds before RouDi sends SIGTERM to running applications at shutdown
    units::Duration processTerminationDelay{roudi::PROCESS_DEFAULT_TERMINATION_DELAY};
    /// @brief Sets the delay in seconds before RouDi sends SIGKILL to application which did not respond to the initial
    /// SIGTERM signal
    units::Duration processKillDelay{roudi::PROCESS_DEFAULT_KILL_DELAY};

    // have some spare chunks to still deliver introspection data in case there are multiple subscribers to the data
    // which are caching different samples; could probably be reduced to 2 with the instruction to not cache the
    // introspection samples
    /// @brief The number of memory chunks used per introspection topic
    uint32_t introspectionChunkCount{10};

    /// @brief the number of memory chunks used for discovery
    uint32_t discoveryChunkCount{10};

    RouDiConfig& setDefaults() noexcept;
    RouDiConfig& optimize() noexcept;
};
} // namespace config
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_CONFIG_HPP
