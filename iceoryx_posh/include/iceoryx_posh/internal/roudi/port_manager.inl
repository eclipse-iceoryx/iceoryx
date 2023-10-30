// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_INTERNAL_ROUDI_PORT_MANAGER_INL
#define IOX_POSH_INTERNAL_ROUDI_PORT_MANAGER_INL

#include "iceoryx_posh/internal/roudi/port_manager.hpp"

namespace iox
{
namespace roudi
{
template <typename T, std::enable_if_t<std::is_same<T, iox::build::OneToManyPolicy>::value>*>
inline optional<RuntimeName_t>
PortManager::doesViolateCommunicationPolicy(const capro::ServiceDescription& service) noexcept
{
    // check if the publisher is already in the list
    auto& publisherPorts = m_portPool->getPublisherPortDataList();
    auto port = publisherPorts.begin();
    while (port != publisherPorts.end())
    {
        popo::PublisherPortRouDi publisherPort(port.to_ptr());

        if (service == publisherPort.getCaProServiceDescription())
        {
            if (publisherPort.toBeDestroyed())
            {
                destroyPublisherPort(port.to_ptr());
                port = publisherPorts.begin();
                continue;
            }
            return make_optional<RuntimeName_t>(port->m_runtimeName);
        }
        else
        {
            ++port;
        }
    }
    return nullopt;
}

template <typename T, std::enable_if_t<std::is_same<T, iox::build::ManyToManyPolicy>::value>*>
inline optional<RuntimeName_t> PortManager::doesViolateCommunicationPolicy(const capro::ServiceDescription&) noexcept
{
    // Duplicates are allowed when using n:m policy
    return nullopt;
}

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_INTERNAL_ROUDI_PORT_MANAGER_INL
