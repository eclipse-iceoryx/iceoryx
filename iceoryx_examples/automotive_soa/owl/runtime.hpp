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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP

#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cxx/pair.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "types.hpp"

#include <string>

namespace owl
{
class Runtime
{
  public:
    static Runtime& GetInstance(const std::string& name) noexcept
    {
        iox::runtime::PoshRuntime::initRuntime(iox::RuntimeName_t(iox::cxx::TruncateToCapacity, name));
        static Runtime runtime;
        return runtime;
    }

    static Runtime& GetInstance() noexcept
    {
        static Runtime runtime;
        return runtime;
    }

    owl::kom::ServiceHandleContainer<kom::ProxyHandleType>
    FindService(owl::kom::ServiceIdentifier& serviceIdentifier,
                owl::kom::InstanceIdentifier& instanceIdentifier) noexcept
    {
        owl::kom::ServiceHandleContainer<kom::ProxyHandleType> iceoryxServiceContainer;

        m_discovery.findService(
            serviceIdentifier,
            instanceIdentifier,
            iox::cxx::nullopt,
            [&](auto& service) {
                iceoryxServiceContainer.push_back({service.getEventIDString(), service.getInstanceIDString()});
            },
            iox::popo::MessagingPattern::PUB_SUB);

        m_discovery.findService(
            serviceIdentifier,
            instanceIdentifier,
            iox::cxx::nullopt,
            [&](auto& service) {
                iceoryxServiceContainer.push_back({service.getEventIDString(), service.getInstanceIDString()});
            },
            iox::popo::MessagingPattern::REQ_RES);

        // We need to make sure that all three internal services representing 'MinimalSkeleton' are available
        owl::kom::ServiceHandleContainer<kom::ProxyHandleType> autosarServiceContainer;
        if (verifyThatServiceIsComplete(iceoryxServiceContainer))
        {
            autosarServiceContainer.push_back({serviceIdentifier, instanceIdentifier});
        }

        return autosarServiceContainer;
    }

    owl::kom::FindServiceHandle StartFindService(owl::kom::FindServiceHandler<kom::ProxyHandleType> handler,
                                                 owl::kom::ServiceIdentifier& serviceIdentifier,
                                                 owl::kom::InstanceIdentifier& instanceIdentifier) noexcept
    {
        /// @todo #1332 Are duplicate entries allowed?
        m_callbacks.push_back({handler, {serviceIdentifier, instanceIdentifier}});

        if (m_callbacks.size() == 1)
        {
            auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
            m_listener.attachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
                .or_else([](auto) {
                    std::cerr << "unable to attach discovery" << std::endl;
                    std::exit(EXIT_FAILURE);
                });
        }

        return owl::kom::FindServiceHandle({serviceIdentifier, instanceIdentifier});
    }

    void StopFindService(owl::kom::FindServiceHandle handle) noexcept
    {
        auto iter = m_callbacks.begin();
        for (; iter != m_callbacks.end(); iter++)
        {
            if (iter->second.GetServiceId() == handle.GetServiceId()
                && iter->second.GetInstanceId() == handle.GetInstanceId())
            {
                break;
            }
        }
        if (iter != m_callbacks.end())
        {
            m_callbacks.erase(iter);
        }

        if (m_callbacks.empty())
        {
            m_listener.detachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED);
        }
    }

  private:
    explicit Runtime() noexcept = default;

    bool verifyThatServiceIsComplete(owl::kom::ServiceHandleContainer<kom::ProxyHandleType>& container)
    {
        // The service level of AUTOSAR Adaptive is not available in iceoryx, instead every publisher and server is
        // considered as a service. A owl::kom binding implementer would typically query the AUTOSAR meta model here, to
        // find out if all event, fields and methods of a service are available. For the example we assume that the
        // 'MinimalSkeleton' service is complete when the container contains the three iceoryx services:
        //
        // 1. EventPublisher: MinimalSkeleton, Instance, Event
        // 2. FieldPublisher: MinimalSkeleton, Instance, Field
        // 3. MethodServer:   MinimalSkeleton, Instance, Method

        if (container.size() == 3U)
        {
            return true;
        }
        return false;
    }

    static void invokeCallback(iox::runtime::ServiceDiscovery*, Runtime* self)
    {
        // 1) Has the availability of one of the registered services changed?
        // 2) If yes, call the user-defined callback
        for (auto& callback : self->m_callbacks)
        {
            auto container =
                self->FindService(callback.second.m_serviceIdentifier, callback.second.m_instanceIdentifier);
            // if (container.empty())
            // {
            //     continue;
            // }
            (callback.first)(container,
                             owl::kom::FindServiceHandle(
                                 {callback.second.m_serviceIdentifier, callback.second.m_instanceIdentifier}));
        }
    }

    iox::runtime::ServiceDiscovery m_discovery;
    iox::popo::Listener m_listener;
    iox::cxx::vector<iox::cxx::pair<owl::kom::FindServiceHandler<kom::ProxyHandleType>, owl::kom::FindServiceHandle>,
                     iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>
        m_callbacks;
};
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
