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
    struct ServiceHandlerDescriptor serviceHandleDescriptor;

    // Scope for m_serviceDescriptorsMutex
    {
        // Synchronise access to m_serviceDiscoveryTimerActive & m_serviceDescriptors
        std::lock_guard<std::mutex> g(m_serviceDescriptorsMutex);
        if (!m_serviceDiscoveryTimerActive)
        {
            m_serviceDiscoveryTimerActive = true;
            m_serviceDiscovery.start(posix::Timer::RunMode::PERIODIC);
        }

        // Ensure that this function is called only once per service, unless StopFindService() is called.
        // If it's called for the same service, it returns the already registered handle.
        // Callback handler is not updated from the new request.
        auto found =
            std::find_if(m_serviceDescriptors.begin(),
                         m_serviceDescriptors.end(),
                         [&serviceId](const ServiceHandlerDescriptor& it) { return (serviceId == it.serviceId); });
        // Given service is already registered, return the handle
        if (found != m_serviceDescriptors.end())
        {
            return cxx::success<FindServiceHandle>(found->handle);
        }

        // No room to accomodate new handlers
        if (m_serviceDescriptors.size() == m_serviceDescriptors.capacity())
        {
            LogError() << "Could not register FindServiceHandler for " << serviceId << " \n";
            errorHandler(Error::kPOSH__SERVICE_DISCOVERY_FIND_SERVICE_CALLBACKS_CONTAINER_OVERFLOW,
                         nullptr,
                         ErrorLevel::MODERATE);
            return cxx::error<Error>(Error::kPOSH__SERVICE_DISCOVERY_FIND_SERVICE_CALLBACKS_CONTAINER_OVERFLOW);
        }

        serviceHandleDescriptor = {handler, serviceId, InstanceContainer(), m_handleCounter};
        m_serviceDescriptors.push_back(serviceHandleDescriptor);

        // Compute the handle for next request
        m_handleCounter++;

        // Trigger service discovery loop, in the next iteration of cyclic thread serviceDiscoveryNotifier
        // If there are any instances available for this service, handler will be triggered from
        // serviceDiscoveryNotifier
        m_triggerDiscoveryLoop = true;
    }

    return cxx::success<FindServiceHandle>(serviceHandleDescriptor.handle);
}

void ServiceDiscoveryNotifier::processStopFindServiceRequests() noexcept
{
    auto guardedStoppedHandles = m_threadSafeStoppedHandles.GetScopeGuard();
    for (auto handle = guardedStoppedHandles->begin(); handle != guardedStoppedHandles->end(); ++handle)
    {
        auto found = std::find_if(
            m_serviceDescriptors.begin(),
            m_serviceDescriptors.end(),
            [handle](const ServiceHandlerDescriptor& descriptor) { return (descriptor.handle == *handle); });

        if (found != m_serviceDescriptors.end())
        {
            m_serviceDescriptors.erase(found);
        }
    }
    // No services to monitor. Stop the timer
    if (m_serviceDescriptors.size() == 0)
    {
        m_serviceDiscovery.stop();
        m_serviceDiscoveryTimerActive = false;
    }
    guardedStoppedHandles->clear();
}

void ServiceDiscoveryNotifier::stopFindService(const FindServiceHandle handle) noexcept
{
    m_threadSafeStoppedHandles->push_back(handle);
}

bool ServiceDiscoveryNotifier::checkForInstanceChange(ServiceHandlerDescriptor& descriptor) const noexcept
{
    InstanceContainer lastKnownInstances(descriptor.instances);
    descriptor.instances.clear();

    auto status = PoshRuntime::getInstance(m_appName).findService({descriptor.serviceId, iox::capro::AnyInstanceString},
                                                                  descriptor.instances);

    // reset instances to last known instances, in case of error (callback is not fired)
    // In case of POSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW, it is not possible to calculate the
    // change in instances, in a reliable manner.
    if (status.has_error())
    {
        descriptor.instances = lastKnownInstances;
    }

    // Sizes differ, hence instances has changed
    if (lastKnownInstances.size() != descriptor.instances.size())
    {
        return true;
    }
    // Compute for change of instances

    InstanceContainer currentInstances(descriptor.instances);

    std::sort(lastKnownInstances.begin(), lastKnownInstances.end());
    std::sort(currentInstances.begin(), currentInstances.end());

    for (auto first = lastKnownInstances.begin(), second = currentInstances.begin(); first != lastKnownInstances.end();
         first++, second++)
    {
        if (*first != *second)
        {
            return true;
        }
    }
    return false;
}

void ServiceDiscoveryNotifier::serviceDiscoveryNotifier() noexcept
{
    iox::cxx::vector<size_t, MAX_START_FIND_SERVICE_CALLBACKS> changedServiceDescriptors;
    {
        // This runs periodically !
        std::lock_guard<std::mutex> g(m_serviceDescriptorsMutex);
        processStopFindServiceRequests();

        auto currentValue = m_serviceRegistryChangeCounter->load(std::memory_order_relaxed);
        // Run discovery loop either because any service state changes in RouDi or new service is registered for
        // notification
        if ((currentValue != m_changeCountAtLastServiceDiscovery) || m_triggerDiscoveryLoop)
        {
            m_changeCountAtLastServiceDiscovery = currentValue;
            m_triggerDiscoveryLoop = false;
            // Do service discovery for all the registered services, If there are changes, then call the handler
            for (size_t i = 0; i < m_serviceDescriptors.size(); ++i)
            {
                if (checkForInstanceChange(m_serviceDescriptors[i]))
                {
                    changedServiceDescriptors.push_back(i);
                }
            }
        }
    }

    for (size_t i = 0; i < changedServiceDescriptors.size(); ++i)
    {
        auto& descriptor = m_serviceDescriptors[changedServiceDescriptors[i]];
        descriptor.callbackHandler(descriptor.instances, descriptor.handle);
    }
}

} // namespace runtime
} // namespace iox
