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

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/internal/cxx/set.hpp"
#include "iceoryx_posh/capro/service_description.hpp"

#include <cstdint>
#include <map>
#include <utility>

namespace iox
{
namespace roudi
{
static const capro::IdString_t Wildcard{"*"};
class ServiceRegistry
{
  public:
    enum class ServiceRegistryError
    {
        INVALID_STATE,
        SERVICE_DESCRIPTION_ALREADY_ADDED,
        SERVICE_REGISTRY_FULL,
    };
    static constexpr uint32_t MAX_SERVICE_DESCRIPTIONS = 100U;
    using ServiceDescriptionVector_t = cxx::vector<capro::ServiceDescription, MAX_SERVICE_DESCRIPTIONS>;

    cxx::expected<ServiceRegistryError> add(const capro::ServiceDescription& serviceDescription);
    bool remove(const capro::ServiceDescription& serviceDescription);
    void find(ServiceDescriptionVector_t& searchResult,
              const capro::IdString_t& service = Wildcard,
              const capro::IdString_t& instinstanceance = Wildcard) const;
    const ServiceDescriptionVector_t getAllServices() const;

  private:
    ::std::multimap<capro::IdString_t, uint64_t> m_serviceMap;
    ::std::multimap<capro::IdString_t, uint64_t> m_instanceMap;
    ServiceDescriptionVector_t m_serviceDescriptionVector;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP
