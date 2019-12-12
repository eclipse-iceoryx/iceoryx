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

#include "iceoryx_posh/internal/roudi/service_registry.hpp"

namespace iox
{
namespace roudi
{
void ServiceRegistry::add(const CaproIdString_t& service, const CaproIdString_t& instance)
{
    cxx::set::add(m_serviceMap[service].instanceSet, instance);
}

void ServiceRegistry::remove(const CaproIdString_t& service, const CaproIdString_t& instance)
{
    cxx::set::remove(m_serviceMap[service].instanceSet, instance);
}

void ServiceRegistry::find(InstanceSet_t& instances,
                           const CaproIdString_t& service,
                           const CaproIdString_t& instance) const
{
    if (instance == capro::AnyInstanceString)
    {
        for (auto& instance : m_serviceMap[service].instanceSet)
        {
            instances.push_back(instance);
        }
    }
    else
    {
        auto& instanceSet = m_serviceMap[service].instanceSet;
        auto iter = std::find(instanceSet.begin(), instanceSet.end(), instance);
        if (iter != instanceSet.end())
        {
            instances.push_back(*iter);
        }
    }
}

const ServiceRegistry::serviceMap_t& ServiceRegistry::getServiceMap() const
{
    return m_serviceMap;
}
} // namespace roudi
} // namespace iox
