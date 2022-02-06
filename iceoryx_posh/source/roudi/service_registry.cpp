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

#include <algorithm>
#include <iterator>

namespace iox
{
namespace roudi
{
cxx::expected<ServiceRegistry::Error> ServiceRegistry::add(const capro::ServiceDescription& serviceDescription) noexcept
{
    auto index = find(serviceDescription);
    if (index != NO_INDEX)
    {
        // entry exists, increment counter
        auto& entry = m_serviceDescriptions[index];
        entry->referenceCounter++;
        return cxx::success<>();
    }

    // entry does not exist, find a free slot if it exists

    // fast path to a free slot (previously removed)
    // note: we prefer entries close to the front
    if (m_freeIndex != NO_INDEX)
    {
        auto& entry = m_serviceDescriptions[m_freeIndex];
        entry.emplace(serviceDescription, 1);
        m_freeIndex = NO_INDEX;
        return cxx::success<>();
    }

    // search from start
    for (auto& entry : m_serviceDescriptions)
    {
        if (!entry)
        {
            entry.emplace(serviceDescription, 1);
            return cxx::success<>();
        }
    }

    // append new entry at the end
    if (m_serviceDescriptions.emplace_back())
    {
        auto& entry = m_serviceDescriptions.back();
        entry.emplace(serviceDescription, 1);
        return cxx::success<>();
    }

    return cxx::error<Error>(Error::SERVICE_REGISTRY_FULL);
}

void ServiceRegistry::remove(const capro::ServiceDescription& serviceDescription) noexcept
{
    auto index = find(serviceDescription);
    if (index != NO_INDEX)
    {
        auto& entry = m_serviceDescriptions[index];

        if (entry->referenceCounter > 1)
        {
            entry->referenceCounter--;
        }
        else
        {
            entry.reset();
            m_freeIndex = index;
        }
    }
}

void ServiceRegistry::removeAll(const capro::ServiceDescription& serviceDescription) noexcept
{
    auto index = find(serviceDescription);
    if (index != NO_INDEX)
    {
        auto& entry = m_serviceDescriptions[index];
        entry.reset();
        m_freeIndex = index;
    }
}

void ServiceRegistry::find(ServiceDescriptionVector_t& searchResult,
                           const cxx::optional<capro::IdString_t>& service,
                           const cxx::optional<capro::IdString_t>& instance,
                           const cxx::optional<capro::IdString_t>& event) const noexcept
{
    // all search cases have complexity O(n)

    constexpr Wildcard WILDCARD;

    // as little case-checking as possible, as much compile time dispatch via tag types as possible
    if (service)
    {
        if (instance)
        {
            if (event)
            {
                return find_(*service, *instance, *event, searchResult);
            }
            return find_(*service, *instance, WILDCARD, searchResult);
        }
        if (event)
        {
            return find_(*service, WILDCARD, *event, searchResult);
        }
        return find_(*service, WILDCARD, WILDCARD, searchResult);
    }

    if (instance)
    {
        if (event)
        {
            return find_(WILDCARD, *instance, *event, searchResult);
        }
        return find_(WILDCARD, *instance, WILDCARD, searchResult);
    }
    if (event)
    {
        return find_(WILDCARD, WILDCARD, *event, searchResult);
    }

    return find_all(searchResult);
}

const ServiceRegistry::ServiceDescriptionVector_t ServiceRegistry::getServices() const noexcept
{
    ServiceDescriptionVector_t allEntries;
    find_all(allEntries);
    return allEntries;
}
} // namespace roudi
} // namespace iox
