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

#include "owl/runtime.hpp"

namespace owl
{
Runtime& Runtime::GetInstance(const core::String& name) noexcept
{
    iox::runtime::PoshRuntime::initRuntime(iox::RuntimeName_t(iox::cxx::TruncateToCapacity, name));
    return GetInstance();
}

Runtime& Runtime::GetInstance() noexcept
{
    static Runtime runtime;
    return runtime;
}

kom::ServiceHandleContainer<kom::ProxyHandleType>
Runtime::FindService(const kom::ServiceIdentifier& serviceIdentifier,
                     const kom::InstanceIdentifier& instanceIdentifier) noexcept
{
    //! [searching for iceoryx services]
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
    //! [searching for iceoryx services]

    // We need to make sure that all four internal services representing 'MinimalSkeleton' are available
    //! [verify iceoryx mapping]
    kom::ServiceHandleContainer<kom::ProxyHandleType> autosarServiceContainer;
    if (verifyThatServiceIsComplete(iceoryxServiceContainer))
    {
        autosarServiceContainer.push_back({serviceIdentifier, instanceIdentifier});
    }

    return autosarServiceContainer;
    //! [verify iceoryx mapping]
}

kom::FindServiceCallbackHandle
Runtime::EnableFindServiceCallback(const kom::FindServiceCallback<kom::ProxyHandleType> handler,
                                   const kom::ServiceIdentifier& serviceIdentifier,
                                   const kom::InstanceIdentifier& instanceIdentifier) noexcept
{
    // Duplicate entries for the same service are allowed
    if (!m_callbacks.push_back(CallbackEntryType(
            handler, {serviceIdentifier, instanceIdentifier}, NumberOfAvailableServicesOnLastSearch())))
    {
        std::cerr << "Callback vector capacity exceeded, did not EnableFindServiceCallback!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    //! [attach discovery to listener]
    if (m_callbacks.size() == 1)
    {
        auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
        m_listener.attachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
            .expect("Unable to attach discovery!");
    }
    //! [attach discovery to listener]

    return kom::FindServiceCallbackHandle({serviceIdentifier, instanceIdentifier});
}

void Runtime::DisableFindServiceCallback(const kom::FindServiceCallbackHandle handle) noexcept
{
    auto iter = std::find_if(m_callbacks.begin(), m_callbacks.end(), [&](auto& element) {
        return std::get<1>(element).GetServiceId() == handle.GetServiceId()
               && std::get<1>(element).GetInstanceId() == handle.GetInstanceId();
    });
    if (iter != m_callbacks.end())
    {
        m_callbacks.erase(iter);
    }

    if (m_callbacks.empty())
    {
        m_listener.detachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED);
    }
}

bool Runtime::verifyThatServiceIsComplete(const kom::ServiceHandleContainer<kom::ProxyHandleType>& container) noexcept
{
    // The service level of AUTOSAR Adaptive is not available in iceoryx, instead every publisher and server is
    // considered as a service. A ara::com binding implementer would typically query the AUTOSAR meta model here, to
    // find out if all event, fields and methods of a service are available. For the example we assume that the
    // 'MinimalSkeleton' service is complete when the container contains the four iceoryx services:
    //
    // 1.    EventPublisher: MinimalSkeleton, Instance, Event  (iox::popo::Publisher)
    // 2. a) FieldPublisher: MinimalSkeleton, Instance, Field  (iox::popo::Publisher)
    // 2. b) FieldPublisher: MinimalSkeleton, Instance, Field  (iox::popo::Server)
    // 3.    MethodServer:   MinimalSkeleton, Instance, Method (iox::popo::Server)

    return (container.size() == NUMBER_OF_ALL_SERVICES);
}

void Runtime::invokeCallback(iox::runtime::ServiceDiscovery*, Runtime* self) noexcept
{
    if (self == nullptr)
    {
        std::cerr << "Callback was invoked with Runtime* being a nullptr!" << std::endl;
        return;
    }
    // Has the availability of one of the registered services changed?
    //! [perform FindService]
    for (auto& callback : self->m_callbacks)
    {
        auto container =
            self->FindService(std::get<1>(callback).m_serviceIdentifier, std::get<1>(callback).m_instanceIdentifier);

        auto numberOfAvailableServicesOnCurrentSearch = container.size();
        auto& numberOfAvailableServicesOnLastSearch = std::get<2>(callback);
        //! [perform FindService]

        auto executeCallback = [&]() {
            (std::get<0>(callback))(container,
                                    kom::FindServiceCallbackHandle({std::get<1>(callback).m_serviceIdentifier,
                                                                    std::get<1>(callback).m_instanceIdentifier}));
            numberOfAvailableServicesOnLastSearch.emplace(numberOfAvailableServicesOnCurrentSearch);
        };

        // If the service is available for the first time
        //! [first execution conditions]
        if (!numberOfAvailableServicesOnLastSearch.has_value() && numberOfAvailableServicesOnCurrentSearch != 0)
        {
            executeCallback();
        }
        //! [first execution conditions]

        // If the service was available before and current number of services has changed
        //! [second execution conditions]
        if (numberOfAvailableServicesOnLastSearch.has_value()
            && numberOfAvailableServicesOnLastSearch.value() != numberOfAvailableServicesOnCurrentSearch)
        {
            executeCallback();
        }
        //! [second execution conditions]
    }
}
} // namespace owl
