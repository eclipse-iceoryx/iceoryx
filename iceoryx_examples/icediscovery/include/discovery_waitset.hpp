// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#pragma once

#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

namespace discovery
{
using ServiceDiscovery = iox::runtime::ServiceDiscovery;
using ServiceContainer = iox::runtime::ServiceContainer;

ServiceDiscovery& serviceDiscovery();

/// @brief User defined discovery with wait functionality that allows
/// to wait until some (discovery related) condition is true.
class Discovery
{
  public:
    Discovery();

    /// @brief wait until service availability changes AND some condition evaluates to true
    /// @note condition must be bool(void) (we can enforce this later ...)
    /// @note blocks the current thread, can be unblocked by unblockWait (as a final action)
    template <typename Condition>
    bool waitUntil(const Condition& discoveryCondition);

    /// @brief wait for any change of the registry since the last update
    void waitUntilChange();

    /// @brief unblock any wait
    /// @note not reversible, i.e. after this call no further waiting with e.g. waitUntil is possible
    void unblockWait();

    /// @brief get all services matching a findService query
    /// @note invokes findService of the native iceoryx ServiceDiscovery API
    ServiceContainer findService(const iox::cxx::optional<iox::capro::IdString_t>& service,
                                 const iox::cxx::optional<iox::capro::IdString_t>& instance,
                                 const iox::cxx::optional<iox::capro::IdString_t>& event);

  private:
    ServiceDiscovery* m_discovery{nullptr};
    iox::popo::WaitSet<1> m_waitset;
    bool m_blocking{true};

    void update();
};

template <typename Condition>
bool Discovery::waitUntil(const Condition& discoveryCondition)
{
    update();
    do
    {
        // 1) we have current discovery data (almost, as it can have changed again already)
        // condition holds?
        bool result = discoveryCondition();
        if (result)
        {
            // 2) condition held and we return (without mutex to protect condition changes
            // there is no way to guarantee it still holds)
            return true;
        }
        else
        {
            if (!m_blocking)
            {
                return false;
            }
        }
        // 3) condition did not hold but it may hold if we use the latest discovery data
        //    which may have arrived in the meantime

        // 4) this does not wait if there is new discovery data (and hence we try again immediately)
        waitUntilChange();
        // 5) discovery data changed, check condition again (even if unblocked)
    } while (true);

    return false;
}

} // namespace discovery
