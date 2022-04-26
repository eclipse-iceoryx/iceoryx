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

#ifndef IOX_EXAMPLES_ARA_COM_RUNTIME_HPP
#define IOX_EXAMPLES_ARA_COM_RUNTIME_HPP

#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cxx/pair.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "types.hpp"

#include <string>

namespace ara
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

    ara::com::ServiceHandleContainer<com::ProxyHandleType>
    FindService(ara::com::ServiceIdentifier& serviceIdentifier,
                ara::com::InstanceIdentifier& instanceIdentifier) noexcept
    {
        ara::com::ServiceHandleContainer<com::ProxyHandleType> iceoryxServiceContainer;

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
        ara::com::ServiceHandleContainer<com::ProxyHandleType> araServiceContainer;
        if (verifyThatServiceIsComplete(iceoryxServiceContainer))
        {
            araServiceContainer.push_back({serviceIdentifier, instanceIdentifier});
        }

        return araServiceContainer;
    }

    ara::com::FindServiceHandle StartFindService(ara::com::FindServiceHandler<com::ProxyHandleType> handler,
                                                 ara::com::ServiceIdentifier& serviceIdentifier,
                                                 ara::com::InstanceIdentifier& instanceIdentifier) noexcept
    {
        /// @todo Are duplicate entries allowed?
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

        return ara::com::FindServiceHandle({serviceIdentifier, instanceIdentifier});
    }

    void StopFindService(ara::com::FindServiceHandle handle) noexcept
    {
        /// @todo use unique integer id in FindServiceHandle for easier adding/removal?
        /// auto iter = std::find(m_callbacks.begin(), m_callbacks.end(), handle);
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
            std::cout << "detached!" << std::endl;
        }
    }

  private:
    explicit Runtime() noexcept = default;

    bool verifyThatServiceIsComplete(ara::com::ServiceHandleContainer<com::ProxyHandleType>& container)
    {
        // The service level of AUTOSAR Adaptive is not available in iceoryx, instead every publisher and server is
        // considered as a service. A ara::com binding implementer would typically query the AUTOSAR meta model here, to
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
                             ara::com::FindServiceHandle(
                                 {callback.second.m_serviceIdentifier, callback.second.m_instanceIdentifier}));
        }
    }

    iox::runtime::ServiceDiscovery m_discovery;
    iox::popo::Listener m_listener;
    iox::cxx::vector<iox::cxx::pair<ara::com::FindServiceHandler<com::ProxyHandleType>, ara::com::FindServiceHandle>,
                     iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>
        m_callbacks;
};
} // namespace ara

#endif // IOX_EXAMPLES_ARA_COM_RUNTIME_HPP
