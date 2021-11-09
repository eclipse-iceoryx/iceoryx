// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
    // Forbid duplicate service descriptions entries
    for (auto& element : m_serviceDescriptionVector)
    {
        if (element.serviceDescription == serviceDescription)
        {
            // Due to n:m communication we don't store twice but increase the reference counter
            element.referenceCounter++;
            return cxx::success<>();
        }
    }
    uint64_t referenceCounter = 1U;
    if (!m_serviceDescriptionVector.push_back({serviceDescription, referenceCounter}))
    {
        return cxx::error<Error>(Error::SERVICE_REGISTRY_FULL);
    }

    auto serviceIndex = m_serviceDescriptionVector.size() - 1;
    m_serviceMap.insert({serviceDescription.getServiceIDString(), serviceIndex});
    m_instanceMap.insert({serviceDescription.getInstanceIDString(), serviceIndex});
    return cxx::success<>();
}

bool ServiceRegistry::remove(const capro::ServiceDescription& serviceDescription) noexcept
{
    bool removedElement{false};
    uint64_t index = 0U;
    for (auto iterator = m_serviceDescriptionVector.begin(); iterator != m_serviceDescriptionVector.end();)
    {
        auto& element = m_serviceDescriptionVector[index].serviceDescription;
        if (element == serviceDescription)
        {
            auto& refCounter = m_serviceDescriptionVector[index].referenceCounter;
            --refCounter;
            if (refCounter == 0)
            {
                m_serviceDescriptionVector.erase(iterator);
                removedElement = true;
            }
            else
            {
                return true;
            }
            // There can't be more than one element
            break;
        }
        ++index;
        ++iterator;
    }

    auto removeIndexFromMap = [](std::multimap<capro::IdString_t, uint64_t>& map, uint64_t index) {
        for (auto it = map.begin(); it != map.end();)
        {
            if (it->second == index)
            {
                it = map.erase(it);
                continue;
            }
            else if (it->second > index)
            {
                // update index due to removed element
                it->second--;
            }
            it++;
        }
    };

    if (removedElement)
    {
        removeIndexFromMap(m_serviceMap, index);
        removeIndexFromMap(m_instanceMap, index);
    }

    return removedElement;
}

void ServiceRegistry::find(ServiceDescriptionVector_t& searchResult,
                           const capro::IdString_t& service,
                           const capro::IdString_t& instance) const noexcept
{
    cxx::vector<uint64_t, MAX_SERVICE_DESCRIPTIONS> intersection;

    // Find (K1, K2)
    // O(log n + log n + max(#PossibleServices + #possiblesInstances) + #intersection)
    if (instance != Wildcard && service != Wildcard)
    {
        cxx::vector<uint64_t, MAX_SERVICE_DESCRIPTIONS> possibleServices;
        cxx::vector<uint64_t, MAX_SERVICE_DESCRIPTIONS> possibleInstances;

        auto rangeServiceMap = m_serviceMap.equal_range(service);
        for (auto entry = rangeServiceMap.first; entry != rangeServiceMap.second; ++entry)
        {
            possibleServices.push_back(entry->second);
        }

        auto rangeInstanceMap = m_instanceMap.equal_range(instance);
        for (auto entry = rangeInstanceMap.first; entry != rangeInstanceMap.second; ++entry)
        {
            possibleInstances.push_back(entry->second);
        }

        ::std::set_intersection(possibleServices.begin(),
                                possibleServices.end(),
                                possibleInstances.begin(),
                                possibleInstances.end(),
                                ::std::back_inserter(intersection));

        for (auto& value : intersection)
        {
            searchResult.push_back(m_serviceDescriptionVector[value]);
        }
    }
    // Find (*, K2)
    // O(log n + #result)
    else if (service == Wildcard && instance != Wildcard)
    {
        auto range = m_instanceMap.equal_range(instance);
        for (auto entry = range.first; entry != range.second; ++entry)
        {
            searchResult.push_back(m_serviceDescriptionVector[entry->second]);
        }
    }
    // Find (K1, *)
    // O(log n + #result)
    else if (instance == Wildcard && service != Wildcard)
    {
        auto range = m_serviceMap.equal_range(service);
        for (auto entry = range.first; entry != range.second; ++entry)
        {
            searchResult.push_back(m_serviceDescriptionVector[entry->second]);
        }
    }
    else
    {
        // Find (*, *)
        // O(n)
        searchResult = m_serviceDescriptionVector;
    }
}

const ServiceRegistry::ServiceDescriptionVector_t ServiceRegistry::getServices() const noexcept
{
    return m_serviceDescriptionVector;
}
} // namespace roudi
} // namespace iox
