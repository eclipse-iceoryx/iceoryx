// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <memory>

namespace iox
{
namespace roudi_env
{
RouDiEnv::RouDiEnv(MainCTor) noexcept
{
}

RouDiEnv::RouDiEnv(const DomainId domainId, const IceoryxConfig& config) noexcept
    : RouDiEnv(MainCTor{})
{
    if (domainId == DEFAULT_DOMAIN_ID)
    {
        m_runtimes.emplace();
    }

    auto adjustedConfig = config;
    adjustedConfig.domainId = domainId;
    adjustedConfig.sharesAddressSpaceWithApplications = true;

    m_roudiComponents =
        std::unique_ptr<roudi::IceOryxRouDiComponents>(new roudi::IceOryxRouDiComponents(adjustedConfig));
    m_roudiApp = std::unique_ptr<roudi::RouDi>(
        new roudi::RouDi(m_roudiComponents->rouDiMemoryManager, m_roudiComponents->portManager, adjustedConfig));
}

RouDiEnv::RouDiEnv(const IceoryxConfig& config) noexcept
    : RouDiEnv(config.domainId, config)
{
}

RouDiEnv::~RouDiEnv() noexcept
{
    cleanupRuntimes();
}

void RouDiEnv::setDiscoveryLoopWaitToFinishTimeout(const units::Duration timeout) noexcept
{
    m_discoveryLoopWaitToFinishTimeout = timeout;
}

void RouDiEnv::triggerDiscoveryLoopAndWaitToFinish() noexcept
{
    m_roudiApp->triggerDiscoveryLoopAndWaitToFinish(m_discoveryLoopWaitToFinishTimeout);
}

void RouDiEnv::cleanupAppResources(const RuntimeName_t& name) noexcept
{
    if (m_runtimes.has_value())
    {
        m_runtimes->eraseRuntime(name);
    }
}

uint64_t RouDiEnv::numberOfActiveRuntimeTestInterfaces() noexcept
{
    if (m_runtimes.has_value())
    {
        return m_runtimes->activeRuntimeCount();
    }

    return 0;
}

void RouDiEnv::cleanupRuntimes() noexcept
{
    if (m_runtimes.has_value())
    {
        m_runtimes->cleanupRuntimes();
    }
}

} // namespace roudi_env
} // namespace iox
