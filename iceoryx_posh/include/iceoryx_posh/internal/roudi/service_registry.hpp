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
#ifndef IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP
#define IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"


#include <cstdint>
#include <utility>

namespace iox
{
namespace roudi
{
class ServiceRegistry
{
  public:
    enum class Error
    {
        SERVICE_REGISTRY_FULL,
    };

    using ReferenceCounter_t = uint64_t;

    struct ServiceDescriptionEntry
    {
        ServiceDescriptionEntry(const capro::ServiceDescription& serviceDescription);

        capro::ServiceDescription serviceDescription{};
        ReferenceCounter_t count{1U};
    };

    /// @todo #415 #1074 set limits properly and define location for the limits,
    ///       e.g posh_types.hpp
    static constexpr uint32_t MAX_SERVICE_DESCRIPTIONS = iox::MAX_PUBLISHERS;

    using ServiceDescriptionVector_t = cxx::vector<ServiceDescriptionEntry, MAX_SERVICE_DESCRIPTIONS>;

    /// @brief Adds given service description to registry
    /// @param[in] serviceDescription, service to be added
    /// @return ServiceRegistryError, error wrapped in cxx::expected
    cxx::expected<Error> add(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Removes given service description from registry if service is found,
    ///        in case of multiple occurrences only one occurrence is removed
    /// @param[in] serviceDescription, service to be removed
    void remove(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Removes given service description from registry if service is found,
    ///        all occurences are removed
    /// @param[in] serviceDescription, service to be removed
    void purge(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Searches for given service description in registry
    /// @param[in] searchResult, reference to the vector which will be filled with the results
    /// @param[in] service, string or wildcard (= iox::cxx::nullopt) to search for
    /// @param[in] instance, string or wildcard (= iox::cxx::nullopt) to search for
    /// @param[in] event, string or wildcard (= iox::cxx::nullopt) to search for
    void find(ServiceDescriptionVector_t& searchResult,
              const cxx::optional<capro::IdString_t>& service,
              const cxx::optional<capro::IdString_t>& instance,
              const cxx::optional<capro::IdString_t>& event) const noexcept;

    /// @copydoc ServiceDiscovery::findService
    void find(const cxx::optional<capro::IdString_t>& service,
              const cxx::optional<capro::IdString_t>& instance,
              const cxx::optional<capro::IdString_t>& event,
              cxx::function_ref<void(const ServiceDescriptionEntry&)> callable) const noexcept;

    /// @todo #415 this may not be needed later or we can move applyToAll to the public interface,
    ///       (we want to avoid large containers on the stack)
    /// @brief Returns all service descriptions as copy
    /// @return ServiceDescriptionVector_t, copy of complete service registry
    const ServiceDescriptionVector_t getServices() const noexcept;

  private:
    using Entry_t = cxx::optional<ServiceDescriptionEntry>;
    using ServiceDescriptionContainer_t = cxx::vector<Entry_t, MAX_SERVICE_DESCRIPTIONS>;

    static constexpr uint32_t NO_INDEX = MAX_SERVICE_DESCRIPTIONS;

    ServiceDescriptionContainer_t m_serviceDescriptions;

    // store the last known free Index (if any is known)
    // we could use a queue (or stack) here since they are not optimal
    // for the filling pattern of a vector (prefer entries close to the front)
    uint32_t m_freeIndex{NO_INDEX};

  private:
    uint32_t findIndex(const capro::ServiceDescription& serviceDescription) const noexcept;

    void getAll(ServiceDescriptionVector_t& searchResult) const noexcept;

    void applyToAll(cxx::function_ref<void(const ServiceDescriptionEntry&)> callable) const noexcept;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP
