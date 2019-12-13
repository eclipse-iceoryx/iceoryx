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

#pragma once

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/cxx/set.hpp"

#include <cstdint>
#include <map>

namespace iox
{
namespace roudi
{
class ServiceRegistry
{
  public:
    static constexpr uint32_t MAX_INSTANCES_PER_SERVICE = 100;
    using CaproIdString_t = capro::ServiceDescription::IdString;
    using InstanceSet_t = cxx::vector<CaproIdString_t, MAX_INSTANCES_PER_SERVICE>;
    struct instance_t
    {
        InstanceSet_t instanceSet;
    };
    using serviceMap_t = std::map<CaproIdString_t, instance_t>;

    void add(const CaproIdString_t& service, const CaproIdString_t& instance);
    void remove(const CaproIdString_t& service, const CaproIdString_t& instance);
    void find(InstanceSet_t& instances,
              const CaproIdString_t& service,
              const CaproIdString_t& instance = capro::AnyInstanceString) const;
    const serviceMap_t& getServiceMap() const;

  private:
    mutable serviceMap_t m_serviceMap;
};
} // namespace roudi
} // namespace iox
