// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/service_registry.hpp"

namespace iox
{
namespace roudi
{
ServiceRegistry::ServiceDescriptionEntry::ServiceDescriptionEntry(const capro::ServiceDescription& serviceDescription)
    : serviceDescription(serviceDescription)
{
}

expected<void, ServiceRegistry::Error> ServiceRegistry::add(const capro::ServiceDescription& serviceDescription,
                                                            ReferenceCounter_t ServiceDescriptionEntry::*count)
{
    auto index = findIndex(serviceDescription);
    if (index != NO_INDEX)
    {
        // multiple entries with the same service descripion are possible
        // and we just increase the count in this case (multi-set semantics)
        // entry exists, increment counter
        auto& entry = m_serviceDescriptions[index];
        ((*entry).*count)++;
        m_dataChanged = true;
        return ok();
    }

    // entry does not exist, find a free slot if it exists

    // fast path to a free slot (which was occupied by previously removed entry),
    // prefer to fill entries close to the front
    if (m_freeIndex != NO_INDEX)
    {
        auto& entry = m_serviceDescriptions[m_freeIndex];
        entry.emplace(serviceDescription);
        (*entry).*count = 1U;
        m_dataChanged = true;
        m_freeIndex = NO_INDEX;
        return ok();
    }

    // search from start
    for (auto& entry : m_serviceDescriptions)
    {
        if (!entry)
        {
            entry.emplace(serviceDescription);
            (*entry).*count = 1U;
            m_dataChanged = true;
            return ok();
        }
    }

    // append new entry at the end (the size only grows up to capacity)
    if (m_serviceDescriptions.emplace_back())
    {
        auto& entry = m_serviceDescriptions.back();
        entry.emplace(serviceDescription);
        (*entry).*count = 1U;
        m_dataChanged = true;
        return ok();
    }

    return err(Error::SERVICE_REGISTRY_FULL);
}

expected<void, ServiceRegistry::Error>
ServiceRegistry::addPublisher(const capro::ServiceDescription& serviceDescription) noexcept
{
    return add(serviceDescription, &ServiceDescriptionEntry::publisherCount);
}

expected<void, ServiceRegistry::Error>
ServiceRegistry::addServer(const capro::ServiceDescription& serviceDescription) noexcept
{
    return add(serviceDescription, &ServiceDescriptionEntry::serverCount);
}

void ServiceRegistry::removePublisher(const capro::ServiceDescription& serviceDescription) noexcept
{
    auto index = findIndex(serviceDescription);
    if (index != NO_INDEX)
    {
        auto& entry = m_serviceDescriptions[index];

        if (entry && entry->publisherCount >= 1U)
        {
            if (--entry->publisherCount == 0U && entry->serverCount == 0)
            {
                entry.reset();
                // reuse the slot in the next insertion
                m_freeIndex = index;
                m_dataChanged = true;
            }
        }
    }
}

void ServiceRegistry::removeServer(const capro::ServiceDescription& serviceDescription) noexcept
{
    auto index = findIndex(serviceDescription);
    if (index != NO_INDEX)
    {
        auto& entry = m_serviceDescriptions[index];

        if (entry && entry->serverCount >= 1U)
        {
            if (--entry->serverCount == 0U && entry->publisherCount == 0)
            {
                entry.reset();
                // reuse the slot in the next insertion
                m_freeIndex = index;
                m_dataChanged = true;
            }
        }
    }
}

void ServiceRegistry::purge(const capro::ServiceDescription& serviceDescription) noexcept
{
    auto index = findIndex(serviceDescription);
    if (index != NO_INDEX)
    {
        auto& entry = m_serviceDescriptions[index];
        entry.reset();
        // reuse the slot in the next insertion
        m_freeIndex = index;
        m_dataChanged = true;
    }
}

void ServiceRegistry::find(const optional<capro::IdString_t>& service,
                           const optional<capro::IdString_t>& instance,
                           const optional<capro::IdString_t>& event,
                           function_ref<void(const ServiceDescriptionEntry&)> callable) const noexcept
{
    for (auto& entry : m_serviceDescriptions)
    {
        if (entry)
        {
            bool match = (service) ? (entry->serviceDescription.getServiceIDString() == *service) : true;
            match &= (instance) ? (entry->serviceDescription.getInstanceIDString() == *instance) : true;
            match &= (event) ? (entry->serviceDescription.getEventIDString() == *event) : true;

            if (match)
            {
                callable(*entry);
            }
        }
    }
}

uint32_t ServiceRegistry::findIndex(const capro::ServiceDescription& serviceDescription) const noexcept
{
    for (uint32_t i = 0; i < m_serviceDescriptions.size(); ++i)
    {
        auto& entry = m_serviceDescriptions[i];
        if (entry && entry->serviceDescription == serviceDescription)
        {
            return i;
        }
    }
    return NO_INDEX;
}

void ServiceRegistry::forEach(function_ref<void(const ServiceDescriptionEntry&)> callable) const noexcept
{
    for (auto& entry : m_serviceDescriptions)
    {
        if (entry)
        {
            callable(*entry);
        }
    }
}

bool ServiceRegistry::hasDataChangedSinceLastCall() noexcept
{
    auto dataChanged = m_dataChanged;
    m_dataChanged = false;
    return dataChanged;
}

} // namespace roudi
} // namespace iox
