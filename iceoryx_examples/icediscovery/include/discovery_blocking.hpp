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

#ifndef IOX_EXAMPLES_DISCOVERY_BLOCKING_HPP
#define IOX_EXAMPLES_DISCOVERY_BLOCKING_HPP

#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iox/atomic.hpp"

#include <vector>

namespace discovery
{
using ServiceDiscovery = iox::runtime::ServiceDiscovery;
using ServiceContainer = std::vector<iox::capro::ServiceDescription>;

ServiceDiscovery& serviceDiscovery();

/// @brief User defined discovery with wait functionality that allows
/// to wait until some (discovery related) condition is true.
class Discovery
{
  public:
    Discovery();

    /// @brief wait until service availability changes AND some condition evaluates to true
    /// @param condition condition with signature bool(void)
    /// @return true if the condition held, false otherwise (i.e. was unblocked)
    /// @note blocks the current thread, can be unblocked by unblockWait (as a final action)
    template <typename Condition>
    bool waitUntil(const Condition& condition);

    /// @brief wait for any change of the registry since the last update
    void waitUntilChange();

    /// @brief unblock any wait
    /// @note not reversible, i.e. after this call no further waiting with e.g. waitUntil is possible
    void unblockWait() volatile noexcept;

    /// @brief get all services matching a findService query
    /// @return ServiceContainer, containing the found services
    /// @note invokes findService of the native iceoryx ServiceDiscovery API
    ServiceContainer findService(const iox::optional<iox::capro::IdString_t>& service,
                                 const iox::optional<iox::capro::IdString_t>& instance,
                                 const iox::optional<iox::capro::IdString_t>& event);

  private:
    ServiceDiscovery* m_discovery{nullptr};
    iox::popo::WaitSet<1> m_waitset;
    iox::concurrent::Atomic<bool> m_blocking{true};
};

//! [wait until condition]
template <typename Condition>
bool Discovery::waitUntil(const Condition& condition)
{
    do
    {
        // 1) does the condition hold?
        bool result = condition();
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
//! [wait until condition]

} // namespace discovery

#endif // IOX_EXAMPLES_DISCOVERY_BLOCKING_HPP
