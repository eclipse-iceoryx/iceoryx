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

/// cxx::multimap<cxx::pair<K1,K2>,V>
/// cxx::vector<K2>


/// cxx::multimap<K1,K2>
/// cxx::multimap<K2,V>


/// cxx::vector<ServiceDescription>


//<Radar,FrontLeft>,  Object
//<Radar,FrontRight>, Object
//<Radar,FrontLeft>,  Speed
//<Video,FrontLeft>,  Speed


// if (instance == capro::IdString_t(capro::AnyInstanceString))
// {
// for (auto& instance : m_serviceMap[make_pair(service,instance)])
// {
// instances.push_back();
// }
// }

namespace iox
{
namespace roudi
{
void ServiceRegistry::add(const capro::ServiceDescription& serviceDescription)
{
    m_serviceMapOld.insert(
        {std::make_pair(serviceDescription.getServiceIDString(), serviceDescription.getInstanceIDString()),
         serviceDescription.getEventIDString()});
    // cxx::set::add(m_serviceMap[service].instanceSet, instance);
}

void ServiceRegistry::remove(const capro::ServiceDescription& serviceDescription)
{
    cxx::set::remove(m_serviceMapOld,
                     {std::make_pair(serviceDescription.getServiceIDString(), serviceDescription.getInstanceIDString()),
                      serviceDescription.getEventIDString()});
}

void ServiceRegistry::find(ServiceDescriptionVector_t& searchResult,
                           const capro::IdString_t& service,
                           const capro::IdString_t& instance) const
{
    // Attempt A
    if (instance != capro::IdString_t(Wildcard)
        && service != capro::IdString_t(Wildcard))
    {
        // O(log n)
        auto range = m_serviceMapOld.equal_range(std::make_pair(service, instance));
        // O(#result)
        for (auto entry = range.first; entry != range.second; ++entry)
        {
            searchResult.push_back(capro::ServiceDescription(service, instance, entry->second));
        }
    }
    else
    {
        cxx::vector<capro::IdString_t, MAX_SERVICE_DESCRIPTIONS> availableServices;
        cxx::vector<capro::IdString_t, MAX_SERVICE_DESCRIPTIONS> availableInstances;

        // Grep for all available service strings
        if (service == capro::IdString_t(Wildcard))
        {
            for (auto& entry : m_serviceMapOld)
            {
                availableServices.push_back(entry.first.first);
            }
        }
        else
        {
            availableServices.push_back(service);
        }

        // Grep for all available instance strings
        if (instance == capro::IdString_t(Wildcard))
        {
            for (auto& entry : m_serviceMapOld)
            {
                availableInstances.push_back(entry.first.second);
            }
        }
        else
        {
            availableInstances.push_back(instance);
        }

        // O(n)
        for (auto& service : availableServices)
        {
            // O(n)
            for (auto& instance : availableInstances)
            {
                // O(log n)
                auto range = m_serviceMapOld.equal_range(std::make_pair(service, instance));
                for (auto entry = range.first; entry != range.second; ++entry)
                {
                    searchResult.push_back(capro::ServiceDescription(service, instance, entry->second));
                }
            }
        }

        //////////////////////////

        // Find (K1, K2)
        // O(log n)

        // Find (K1, *) <- aktuelle Implementierung ist für diesen Fall optimiert
        // m = available K2
        // O(n + n * m * log n)

        // Find (*, *)
        // p = available K1
        // m = available K2
        // O(n + n + (p * m * log n))

        // Questions:
        // * Can we just stored IDs aka integers instead of strings?
        //   * How would this affect the choice for the underlying data structure?
        //   * Can we force users to always use IDs?
        //   * Can we remove the integer values? What are they used for?
        //   * Where is the ClassHash function?
        // * What happens if we create a ServiceDescription(AnyServiceString, AnyInstanceString, "foo")?
        //   * Forbid this pattern?

        // Temporary data structure
        // map<K1, idx> k1;
        // map<K2, idx> k2;
        // vector<tuple<String, String, String>> serviceDescriptionVector;

        //////////////////////////


        // Attempt B
        cxx::vector<uint64_t, MAX_SERVICE_DESCRIPTIONS> intersection;

        // Find (K1, K2)
        if (instance != capro::IdString_t(Wildcard)
            && service != capro::IdString_t(Wildcard))
        {
            // O(log n)
            cxx::vector<uint64_t, MAX_SERVICE_DESCRIPTIONS> possibleServices;
            auto range = m_serviceMap.equal_range(service);
            for (auto entry = range.first; entry != range.second; ++entry)
            {
                possibleServices.push_back(entry.second);
            }

            // O(log n)
            cxx::vector<uint64_t, MAX_SERVICE_DESCRIPTIONS> possibleInstances;
            auto range = m_instanceMap.equal_range(instance);
            for (auto entry = range.first; entry != range.second; ++entry)
            {
                possibleInstances.push_back(entry.second);
            }

            // O(max(#possibleServices,#possiblesInstances)) = O(n)
            std::set_intersection(possibleServices.begin(),
                                  possibleServices.end(),
                                  possibleInstances.begin(),
                                  possibleInstances.end(),
                                  std::back_inserter(intersection));

            for (auto& value : intersection)
            {
                searchResult.push_back(capro::ServiceDescription(service, instance, m_serviceDescriptionVector[value]));
            }
        }
        else
        {
            // Find (*, K2)
            if (service == capro::IdString_t(capro::AnyServiceString)
                && instance != capro::IdString_t(capro::AnyInstanceString))
            {
                // O(log n)
                auto range = m_instanceMap.equal_range(instance);
                // O(#result)
                for (auto entry = range.first; entry != range.second; ++entry)
                {
                    searchResult.push_back(
                        capro::ServiceDescription(FOOBAR, instance, m_serviceDescriptionVector[entry->second]));
                }
            }
            // Find (K1, *)
            else if (instance == capro::IdString_t(capro::AnyInstanceString)
                     && service != capro::IdString_t(capro::AnyServiceString))
            {
                // O(log n)
                auto range = m_serviceMap.equal_range(service);
                // O(#result)
                for (auto entry = range.first; entry != range.second; ++entry)
                {
                    possibleServices.
                        capro::ServiceDescription(FOOBAR, instance, m_serviceDescriptionVector[entry->second]));
                }
            }
            // Find (*, *)
            searchResult = m_serviceDescriptionVector;
        }

        // Find (K1, K2)
        // O(log n + log n + #maxPossibleServices + #intersection)

        // Find (K1, *) <- bisheriger Implementierung
        // O(log n)

        // Find (*, *)
        // O(n)
    }

    const ServiceRegistry::serviceMap_t& ServiceRegistry::getServiceMap() const
    {
        return m_serviceMap;
    }
} // namespace roudi
} // namespace roudi
