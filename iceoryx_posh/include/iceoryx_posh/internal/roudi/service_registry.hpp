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
#ifndef IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP
#define IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP

#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cxx/set.hpp"
#include "iceoryx_posh/capro/service_description.hpp"

#include <cstdint>
#include <map>

namespace iox
{
namespace roudi
{
static const capro::IdString_t Wildcard{"*"};
class ServiceRegistry
{
  public:
    static constexpr uint32_t MAX_INSTANCES_PER_SERVICE = 100u;
    using InstanceSet_t = cxx::vector<capro::IdString_t, MAX_INSTANCES_PER_SERVICE>;
    struct instance_t
    {
        InstanceSet_t instanceSet;
    };
    using serviceMap_t = std::map<capro::IdString_t, instance_t>;

    void add(const capro::IdString_t& service, const capro::IdString_t& instance);
    void remove(const capro::IdString_t& service, const capro::IdString_t& instance);
    void find(InstanceSet_t& instances,
              const capro::IdString_t& service,
              const capro::IdString_t& instance = Wildcard) const;
    const serviceMap_t& getServiceMap() const;

  private:
    mutable serviceMap_t m_serviceMap;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP
