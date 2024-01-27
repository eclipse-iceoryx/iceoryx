// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace runtime
{
ServiceDiscovery::ServiceDiscovery() noexcept
{
}

void ServiceDiscovery::update()
{
    // allows us to use update and hence findService concurrently
    std::lock_guard<std::mutex> lock(m_serviceRegistryMutex);
    m_serviceRegistrySubscriber.take().and_then([&](popo::Sample<const roudi::ServiceRegistry>& serviceRegistrySample) {
        *m_serviceRegistry = *serviceRegistrySample;
    });
}

void ServiceDiscovery::findService(const optional<capro::IdString_t>& service,
                                   const optional<capro::IdString_t>& instance,
                                   const optional<capro::IdString_t>& event,
                                   const function_ref<void(const capro::ServiceDescription&)> callableForEach,
                                   const popo::MessagingPattern pattern) noexcept
{
    update();

    switch (pattern)
    {
    case popo::MessagingPattern::PUB_SUB:
    {
        m_serviceRegistry->find(
            service, instance, event, [&](const roudi::ServiceRegistry::ServiceDescriptionEntry& serviceEntry) {
                if (serviceEntry.publisherCount > 0)
                {
                    callableForEach(serviceEntry.serviceDescription);
                }
            });
        break;
    }
    case popo::MessagingPattern::REQ_RES:
    {
        m_serviceRegistry->find(
            service, instance, event, [&](const roudi::ServiceRegistry::ServiceDescriptionEntry& serviceEntry) {
                if (serviceEntry.serverCount > 0)
                {
                    callableForEach(serviceEntry.serviceDescription);
                }
            });
        break;
    }
    default:
    {
        IOX_LOG(WARN, "ServiceDiscovery could not perform search due to unknown MessagingPattern!");
        IOX_REPORT(PoshError::POSH__SERVICE_DISCOVERY_UNKNOWN_MESSAGE_PATTERN_PROVIDED, iox::er::RUNTIME_ERROR);
    }
    }
}

void ServiceDiscovery::enableEvent(popo::TriggerHandle&& triggerHandle, const ServiceDiscoveryEvent event) noexcept
{
    switch (event)
    {
    case ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED:
    {
        m_serviceRegistrySubscriber.enableEvent(std::move(triggerHandle), popo::SubscriberEvent::DATA_RECEIVED);
        break;
    }
    default:
    {
        IOX_LOG(WARN, "ServiceDiscovery::enableEvent() called with unknown event!");
        IOX_REPORT(PoshError::POSH__SERVICE_DISCOVERY_UNKNOWN_EVENT_PROVIDED, iox::er::RUNTIME_ERROR);
    }
    }
}

void ServiceDiscovery::disableEvent(const ServiceDiscoveryEvent event) noexcept
{
    switch (event)
    {
    case ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED:
    {
        m_serviceRegistrySubscriber.disableEvent(popo::SubscriberEvent::DATA_RECEIVED);
        break;
    }
    default:
    {
        IOX_LOG(WARN, "ServiceDiscovery::disableEvent() called with unknown event!");
        IOX_REPORT(PoshError::POSH__SERVICE_DISCOVERY_UNKNOWN_EVENT_PROVIDED, iox::er::RUNTIME_ERROR);
    }
    }
}

void ServiceDiscovery::invalidateTrigger(const uint64_t uniqueTriggerId)
{
    m_serviceRegistrySubscriber.invalidateTrigger(uniqueTriggerId);
}

popo::WaitSetIsConditionSatisfiedCallback
ServiceDiscovery::getCallbackForIsStateConditionSatisfied(const popo::SubscriberState state)
{
    return m_serviceRegistrySubscriber.getCallbackForIsStateConditionSatisfied(state);
}

} // namespace runtime
} // namespace iox
