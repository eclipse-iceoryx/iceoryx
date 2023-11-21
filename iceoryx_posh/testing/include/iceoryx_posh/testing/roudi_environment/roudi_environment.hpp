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

#ifndef IOX_POSH_ROUDI_ENVIRONMENT_ROUDI_ENVIRONMENT_HPP
#define IOX_POSH_ROUDI_ENVIRONMENT_ROUDI_ENVIRONMENT_HPP

#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "iox/detail/deprecation_marker.hpp"

#include <chrono>

namespace iox
{
namespace roudi
{
/// @deprecated Deprecated in 3.0, removed in 4.0, please port to 'iox::roudi_env::RouDiEnv'
class IOX_DEPRECATED_SINCE(3, "Please port to 'iox::roudi_env::RouDiEnv'") RouDiEnvironment : public roudi_env::RouDiEnv
{
  public:
    using ParentType = roudi_env::RouDiEnv;
    using ParentType::ParentType;
    using ParentType::operator=;

    /// @deprecated Deprecated in 3.0, removed in 4.0, please port to 'setDiscoveryLoopWaitToFinishTimeout'
    IOX_DEPRECATED_SINCE(3, "Please port to 'setDiscoveryLoopWaitToFinishTimeout'")
    void SetInterOpWaitingTime(const std::chrono::milliseconds& v) noexcept
    {
        setDiscoveryLoopWaitToFinishTimeout(units::Duration::fromMilliseconds(v.count()));
    }

    /// @deprecated Deprecated in 3.0, removed in 4.0, please port to 'triggerDiscoveryLoopAndWaitToFinish'
    IOX_DEPRECATED_SINCE(3, "Please port to 'triggerDiscoveryLoopAndWaitToFinish'") void InterOpWait() noexcept
    {
        triggerDiscoveryLoopAndWaitToFinish();
    }

    /// @deprecated Deprecated in 3.0, removed in 4.0, please port to 'cleanupAppResources'
    IOX_DEPRECATED_SINCE(3, "Please port to 'cleanupAppResources'")
    void CleanupAppResources(const RuntimeName_t& name) noexcept
    {
        cleanupAppResources(name);
    }
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ENVIRONMENT_ROUDI_ENVIRONMENT_HPP
