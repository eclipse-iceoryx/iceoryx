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

#ifndef IOX_POSH_ROUDI_ENV_ROUDI_ENV_HPP
#define IOX_POSH_ROUDI_ENV_ROUDI_ENV_HPP

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/roudi_env/runtime_test_interface.hpp"
#include "iox/duration.hpp"

namespace iox
{
namespace roudi
{
class RouDi;
}
namespace roudi_env
{

/// @brief A convenient way to create a 'RouDi' for integration tests
class RouDiEnv
{
  public:
    RouDiEnv(const DomainId domainId, const IceoryxConfig& config = MinimalIceoryxConfigBuilder().create()) noexcept;

    RouDiEnv(const IceoryxConfig& config = MinimalIceoryxConfigBuilder().create()) noexcept;
    virtual ~RouDiEnv() noexcept;

    RouDiEnv(RouDiEnv&& rhs) noexcept = default;
    RouDiEnv& operator=(RouDiEnv&& rhs) noexcept = default;

    RouDiEnv(const RouDiEnv&) = delete;
    RouDiEnv& operator=(const RouDiEnv&) = delete;

    void setDiscoveryLoopWaitToFinishTimeout(const units::Duration timeout) noexcept;
    void triggerDiscoveryLoopAndWaitToFinish() noexcept;

    void cleanupAppResources(const RuntimeName_t& name) noexcept;

    uint64_t numberOfActiveRuntimeTestInterfaces() noexcept;

  protected:
    /// @note this is due to ambiguity of the cTor with the default parameter
    struct MainCTor
    {
    };
    /// @brief for implementations on top of RouDiEnv
    RouDiEnv(MainCTor) noexcept;

    void cleanupRuntimes() noexcept;

  private:
    optional<RuntimeTestInterface> m_runtimes;
#if defined(__APPLE__)
    iox::units::Duration m_discoveryLoopWaitToFinishTimeout{iox::units::Duration::fromMilliseconds(1000)};
#else
    iox::units::Duration m_discoveryLoopWaitToFinishTimeout{iox::units::Duration::fromMilliseconds(200)};
#endif
    std::unique_ptr<roudi::IceOryxRouDiComponents> m_roudiComponents;
    std::unique_ptr<roudi::RouDi> m_roudiApp;
};

} // namespace roudi_env
} // namespace iox

#endif // IOX_POSH_ROUDI_ENV_ROUDI_ENV_HPP
