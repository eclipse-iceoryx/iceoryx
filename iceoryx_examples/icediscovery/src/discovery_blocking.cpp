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

#include "discovery_blocking.hpp"

namespace discovery
{
//! [service discovery singleton]
ServiceDiscovery& serviceDiscovery()
{
    static ServiceDiscovery instance;
    return instance;
}
//! [service discovery singleton]

Discovery::Discovery()
    : m_discovery(&serviceDiscovery())
{
    auto errorHandler = [](auto&) {
        std::cerr << "failed to attach to waitset" << std::endl;
        std::terminate();
    };

    //! [attach waitset]
    m_waitset.attachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED)
        .or_else(errorHandler);
    //! [attach waitset]
}

//! [wait until change]
void Discovery::waitUntilChange()
{
    do
    {
        auto notificationVector = m_waitset.wait();
        for (auto& notification : notificationVector)
        {
            if (notification->doesOriginateFrom(m_discovery))
            {
                return;
            }
        }
    } while (m_blocking);
}
//! [wait until change]

//! [unblock wait]
void Discovery::unblockWait() volatile noexcept
{
    m_blocking = false;
    // could also unblock with a dedicated condition to unblock the wait but that requires more code
    // (additional trigger) and is not necessary if it is only supposed to happen once
    m_waitset.markForDestruction();
}
//! [unblock wait]

//! [findService]
ServiceContainer Discovery::findService(const iox::optional<iox::capro::IdString_t>& service,
                                        const iox::optional<iox::capro::IdString_t>& instance,
                                        const iox::optional<iox::capro::IdString_t>& event)
{
    ServiceContainer result;
    auto filter = [&](const iox::capro::ServiceDescription& s) { result.emplace_back(s); };
    m_discovery->findService(service, instance, event, filter, iox::popo::MessagingPattern::PUB_SUB);
    return result;
}
//! [findService]

} // namespace discovery
