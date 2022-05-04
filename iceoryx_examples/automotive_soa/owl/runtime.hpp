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

namespace owl
{
class Runtime
{
  public:
    static Runtime& GetInstance(const core::String& name) noexcept
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

    kom::ServiceHandleContainer<kom::ProxyHandleType> FindService(kom::ServiceIdentifier& serviceIdentifier,
                                                                  kom::InstanceIdentifier& instanceIdentifier) noexcept
    {
        kom::ServiceHandleContainer<kom::ProxyHandleType> iceoryxServiceContainer;

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
        kom::ServiceHandleContainer<kom::ProxyHandleType> autosarServiceContainer;
        if (verifyThatServiceIsComplete(iceoryxServiceContainer))
        {
            autosarServiceContainer.push_back({serviceIdentifier, instanceIdentifier});
        }

        return autosarServiceContainer;
    }

    kom::FindServiceHandle StartFindService(kom::FindServiceHandler<kom::ProxyHandleType> handler,
                                            kom::ServiceIdentifier& serviceIdentifier,
                                            kom::InstanceIdentifier& instanceIdentifier) noexcept
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

        return kom::FindServiceHandle({serviceIdentifier, instanceIdentifier});
    }

    void StopFindService(kom::FindServiceHandle handle) noexcept
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

    bool verifyThatServiceIsComplete(kom::ServiceHandleContainer<kom::ProxyHandleType>& container)
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
        // Requirements (not implemented, see below)
        // 1) Has the availability of one of the registered services changed?
        // 2) If yes, call the user-defined callback

        for (auto& callback : self->m_callbacks)
        {
            auto container =
                self->FindService(callback.second.m_serviceIdentifier, callback.second.m_instanceIdentifier);
            // Typically there should be a check for container.empty() and the callback should only be called when the
            // availability of the specific service has changed. However, to notify the user in the ara::com example
            // about a service, which has disappeared we call the callback on ANY change of the service registry
            (callback.first)(
                container,
                kom::FindServiceHandle({callback.second.m_serviceIdentifier, callback.second.m_instanceIdentifier}));
        }
    }

    iox::runtime::ServiceDiscovery m_discovery;
    iox::popo::Listener m_listener;
    // A vector is not the optimal data structure but used here for simplicity
    iox::cxx::vector<iox::cxx::pair<kom::FindServiceHandler<kom::ProxyHandleType>, kom::FindServiceHandle>,
                     iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>
        m_callbacks;
};
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
