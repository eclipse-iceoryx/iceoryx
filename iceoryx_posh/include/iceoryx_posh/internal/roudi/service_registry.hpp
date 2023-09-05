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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/expected.hpp"
#include "iox/function_ref.hpp"
#include "iox/optional.hpp"
#include "iox/vector.hpp"


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

    static constexpr uint32_t CAPACITY = iox::SERVICE_REGISTRY_CAPACITY;

    using ReferenceCounter_t = uint64_t;

    struct ServiceDescriptionEntry
    {
        ServiceDescriptionEntry(const capro::ServiceDescription& serviceDescription);

        capro::ServiceDescription serviceDescription;

        // note that we can have publishers and servers with the same ServiceDescription
        // and using the counters we save space
        ReferenceCounter_t publisherCount{0U};
        ReferenceCounter_t serverCount{0U};
    };

    /// @brief Adds a given publisher service description to registry
    /// @param[in] serviceDescription, service to be added
    /// @return ServiceRegistryError, error wrapped in expected
    expected<void, Error> addPublisher(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Removes a given publisher service description from registry if service is found,
    ///        in case of multiple occurrences only one occurrence is removed
    /// @param[in] serviceDescription, service to be removed
    void removePublisher(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Adds a given server service description to registry
    /// @param[in] serviceDescription, service to be added
    /// @return ServiceRegistryError, error wrapped in expected
    expected<void, Error> addServer(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Removes a given server service description from registry if service is found,
    ///        in case of multiple occurrences only one occurrence is removed
    /// @param[in] serviceDescription, service to be removed
    void removeServer(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Removes given service description from registry if service is found,
    ///        all occurences are removed
    /// @param[in] serviceDescription, service to be removed
    void purge(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Searches for given service description in registry
    /// @param[in] service, string or wildcard (= iox::nullopt) to search for
    /// @param[in] instance, string or wildcard (= iox::nullopt) to search for
    /// @param[in] event, string or wildcard (= iox::nullopt) to search for
    /// @param[in] callable, callable to apply to each matching entry
    void find(const optional<capro::IdString_t>& service,
              const optional<capro::IdString_t>& instance,
              const optional<capro::IdString_t>& event,
              function_ref<void(const ServiceDescriptionEntry&)> callable) const noexcept;

    /// @brief Applies a callable to all entries
    /// @param[in] callable, callable to apply to each entry
    /// @note Can be used to obtain all entries or count them
    void forEach(function_ref<void(const ServiceDescriptionEntry&)> callable) const noexcept;

    /// @brief Checks whether the registry data changed since the last time this method was called
    /// @return true when the registry changed since the last call, false otherwise
    bool hasDataChangedSinceLastCall() noexcept;

  private:
    using Entry_t = optional<ServiceDescriptionEntry>;
    using ServiceDescriptionContainer_t = vector<Entry_t, CAPACITY>;

    static constexpr uint32_t NO_INDEX = CAPACITY;

    ServiceDescriptionContainer_t m_serviceDescriptions;

    // store the last known free Index (if any is known)
    // we could use a queue (or stack) here since they are not optimal
    // for the filling pattern of a vector (prefer entries close to the front)
    uint32_t m_freeIndex{NO_INDEX};

    bool m_dataChanged{true}; // initially true in order to also get notified of the empty registry

  private:
    uint32_t findIndex(const capro::ServiceDescription& serviceDescription) const noexcept;


    expected<void, Error> add(const capro::ServiceDescription& serviceDescription,
                              ReferenceCounter_t ServiceDescriptionEntry::*count);
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP
