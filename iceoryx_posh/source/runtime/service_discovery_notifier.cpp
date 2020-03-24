// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/runtime/service_discovery_notifier.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include <algorithm>


namespace iox
{
namespace runtime
{
ServiceDiscoveryNotifier::ServiceDiscoveryNotifier(const std::string& name,
                                                   const std::atomic<uint64_t>* serviceRegistryChangeCounter) noexcept
    : m_appName(name)
    , m_serviceRegistryChangeCounter(serviceRegistryChangeCounter)
    , m_changeCountAtLastServiceDiscovery(m_serviceRegistryChangeCounter->load(std::memory_order_relaxed))
    , m_serviceDiscoveryTimerActive(false)
    , m_handleCounter(0)
{
}


cxx::expected<FindServiceHandle, Error> ServiceDiscoveryNotifier::startFindService(const FindServiceHandler& handler,
                                                                                   const IdString& serviceId) noexcept
{
    struct ServiceHandlerDescription serviceHandleDescriptor;

    // Scope for m_queueAccessMutex
    {
        // Synchronise access to m_serviceDiscoveryTimerActive & m_handlers
        std::lock_guard<std::mutex> g(m_queueAccessMutex);
        if (!m_serviceDiscoveryTimerActive)
        {
            m_serviceDiscoveryTimerActive = true;
            m_serviceDiscovery.start(posix::Timer::RunMode::PERIODIC);
        }

        // Ensure that this function is called only once per service, unless StopFindService() is called.
        // If it's called for the same service, it returns the already registered handle.
        // Callback handler is not updated from the new request.
        auto found =
            std::find_if(m_handlers.begin(), m_handlers.end(), [&serviceId](const ServiceHandlerDescription& it) {
                return (serviceId == it.serviceId);
            });
        // Given service is already registered, return the handler
        if (found != m_handlers.end())
        {
            return cxx::success<FindServiceHandle>(found->handle);
        }

        // No room to accomodate new handlers
        if (m_handlers.size() == m_handlers.capacity())
        {
            LogError() << "Could not register FindServiceHandler for " << serviceId << " \n";
            errorHandler(Error::kPOSH__SERVICE_DISCOVERY_FIND_SERVICE_CALLBACKS_CONTAINER_OVERFLOW,
                         nullptr,
                         ErrorLevel::MODERATE);
            return cxx::error<Error>(Error::kPOSH__SERVICE_DISCOVERY_FIND_SERVICE_CALLBACKS_CONTAINER_OVERFLOW);
        }

        serviceHandleDescriptor = {handler, serviceId, InstanceContainer(), m_handleCounter};
        // Compute the handle for next request
        m_handleCounter++;

        auto status = PoshRuntime::getInstance(m_appName).findService({serviceId, iox::capro::AnyInstanceString},
                                                                      serviceHandleDescriptor.instances);
        // If there is any error, then clear all the instances found. In case of
        // POSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW, it is not possible to calculate the change in instances,
        // in a reliable manner.
        if (status.has_error())
        {
            serviceHandleDescriptor.instances.clear();
        }
        m_handlers.push_back(serviceHandleDescriptor);
    }

    // handler should be called immediately with the available instances, upon startFindService() call
    handler(serviceHandleDescriptor.instances, serviceHandleDescriptor.handle);

    return cxx::success<FindServiceHandle>(serviceHandleDescriptor.handle);
}

void ServiceDiscoveryNotifier::stopFindService(const FindServiceHandle handle) noexcept
{
    std::lock_guard<std::mutex> g(m_queueAccessMutex);
    auto found = std::find_if(m_handlers.begin(), m_handlers.end(), [handle](const ServiceHandlerDescription& it) {
        return (it.handle == handle);
    });

    if (found != m_handlers.end())
    {
        m_handlers.erase(found);
    }

    // No services to monitor. Stop the timer
    if (m_handlers.size() == 0)
    {
        m_serviceDiscovery.stop();
        m_serviceDiscoveryTimerActive = false;
    }
}

void ServiceDiscoveryNotifier::serviceDiscoveryNotifier() noexcept
{
    // This runs periodically!
    auto current = m_serviceRegistryChangeCounter->load(std::memory_order_relaxed);
    if (current != m_changeCountAtLastServiceDiscovery)
    {
        m_changeCountAtLastServiceDiscovery = current;

        // Synchronise access to m_handlers
        std::lock_guard<std::mutex> g(m_queueAccessMutex);
        // Do service discovery for all the registered services, If there are changes, then call the handler
        for (auto& element : m_handlers)
        {
            InstanceContainer lastKnownInstances(element.instances);
            element.instances.clear();

            auto status = PoshRuntime::getInstance(m_appName).findService(
                {element.serviceId, iox::capro::AnyInstanceString}, element.instances);

            // reset instances to last known instances, in case of error (callback is not fired)
            // In case of POSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW, it is not possible to calculate the
            // change in instances, in a reliable manner.
            if (status.has_error())
            {
                element.instances = lastKnownInstances;
            }

            // Sizes differ, hence instances has changed
            if (lastKnownInstances.size() != element.instances.size())
            {
                element.callbackHandler(element.instances, element.handle);
            }
            // Compute for change of instances
            else
            {
                InstanceContainer currentInstances(element.instances);

                std::sort(lastKnownInstances.begin(), lastKnownInstances.end());
                std::sort(currentInstances.begin(), currentInstances.end());

                for (auto first = lastKnownInstances.begin(), second = currentInstances.begin();
                     first != lastKnownInstances.end();
                     first++, second++)
                {
                    if (*first != *second)
                    {
                        element.callbackHandler(element.instances, element.handle);
                        break;
                    }
                }
            }
        }
    }
}

} // namespace runtime
} // namespace iox
