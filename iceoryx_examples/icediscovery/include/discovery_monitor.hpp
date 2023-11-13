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

#ifndef IOX_EXAMPLES_DISCOVERY_MONITORING_HPP
#define IOX_EXAMPLES_DISCOVERY_MONITORING_HPP

#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "iox/function.hpp"
#include "iox/optional.hpp"

#include <vector>

namespace discovery
{
using ServiceDiscovery = iox::runtime::ServiceDiscovery;
using ServiceContainer = std::vector<iox::capro::ServiceDescription>;

ServiceDiscovery& serviceDiscovery();

/// @brief User defined discovery with monitoring functionality that allows
/// to execute a callback in a background thread whenever the discovery changes.
class Discovery
{
  public:
    Discovery();

    /// @brief register a callback to be executed on any service discovery change
    /// @param callback callback with signature void(ServiceDiscovery& discovery)
    /// @note callback is executed in a background thread
    /// @note signature of callback is due to the listener signature requirement
    ///       but we could hide this from the user (by e.g. accessing ServiceRegistry via singleton
    ///       and passing it)
    template <typename Callback>
    void registerCallback(const Callback& callback);

    /// @brief deregister the active callback (if any)
    void deregisterCallback();

    /// @brief get all services matching a findService query
    /// @note invokes findService of the native iceoryx ServiceDiscovery API
    ServiceContainer findService(const iox::optional<iox::capro::IdString_t>& service,
                                 const iox::optional<iox::capro::IdString_t>& instance,
                                 const iox::optional<iox::capro::IdString_t>& event);

  private:
    using callback_t = iox::function<void(Discovery&)>;

    ServiceDiscovery* m_discovery{nullptr};
    iox::popo::Listener m_listener;

    /// @note currently only one callback can be active (and there is no need to have more
    /// as we only have one event at the ServiceDiscovery to attach to - SERVICE_REGISTRY_CHANGED)
    iox::optional<callback_t> m_callback;

    static void invokeCallback(ServiceDiscovery* discovery, Discovery* self);
};

//! [registerCallback]
template <typename Callback>
void Discovery::registerCallback(const Callback& callback)
//! [registerCallback]
{
    m_callback.emplace(callback);
    auto errorHandler = [](auto) {
        std::cerr << "failed to attach to listener" << std::endl;
        std::exit(EXIT_FAILURE);
    };

    //! [attach listener]
    auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
    m_listener.attachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
        .or_else(errorHandler);
    //! [attach listener]
}

} // namespace discovery

#endif // IOX_EXAMPLES_DISCOVERY_BLOCKING_HPP
