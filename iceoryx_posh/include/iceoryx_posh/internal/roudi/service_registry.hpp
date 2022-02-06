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
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_posh/capro/service_description.hpp"

#include <cstdint>
#include <map>
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
        ServiceDescriptionEntry(const capro::ServiceDescription& desc, ReferenceCounter_t count)
            : serviceDescription(desc)
            , referenceCounter(count)
        {
        }
        capro::ServiceDescription serviceDescription{};
        ReferenceCounter_t referenceCounter = 0U;
    };

    /// @todo #415 should be connected with iox::MAX_NUMBER_OF_SERVICES
    static constexpr uint32_t MAX_SERVICE_DESCRIPTIONS = 2000U;
    static constexpr uint32_t NO_INDEX = MAX_SERVICE_DESCRIPTIONS;


    using ServiceDescriptionVector_t = cxx::vector<ServiceDescriptionEntry, MAX_SERVICE_DESCRIPTIONS>;

    using Entry_t = cxx::optional<ServiceDescriptionEntry>;
    using ServiceDescriptionContainer_t = cxx::vector<Entry_t, MAX_SERVICE_DESCRIPTIONS>;

    /// @brief Adds given service description to registry
    /// @param[in] serviceDescription, service to be added
    /// @return ServiceRegistryError, error wrapped in cxx::expected
    cxx::expected<Error> add(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Removes given service description from registry if service is found,
    ///        in case of multiple occurences only one is removed
    /// @param[in] serviceDescription, service to be removed
    void remove(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Removes given service description from registry if service is found,
    ///        all occurences are removed
    /// @param[in] serviceDescription, service to be removed
    void removeAll(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief Searches for given service description in registry
    /// @param[in] searchResult, reference to the vector which will be filled with the results
    /// @param[in] service, string or wildcard (= iox::cxx::nullopt) to search for
    /// @param[in] instance, string or wildcard (= iox::cxx::nullopt) to search for
    /// @param[in] event, string or wildcard (= iox::cxx::nullopt) to search for
    void find(ServiceDescriptionVector_t& searchResult,
              const cxx::optional<capro::IdString_t>& service,
              const cxx::optional<capro::IdString_t>& instance,
              const cxx::optional<capro::IdString_t>& event) const noexcept;

    /// @brief Returns all service descriptions as copy
    /// @return ServiceDescriptionVector_t, copy of complete service registry
    const ServiceDescriptionVector_t getServices() const noexcept;

  private:
    ServiceDescriptionContainer_t m_serviceDescriptions;
    // ServiceDescriptionPool_t m_serviceDescriptions;

    uint32_t m_freeIndex{NO_INDEX};

    uint32_t find(const capro::ServiceDescription& serviceDescription)
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

    struct Wildcard
    {
    };

    void find_(const capro::IdString_t& service, Wildcard, Wildcard, ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry && entry->serviceDescription.getServiceIDString() == service)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }

    void find_(Wildcard, const capro::IdString_t& instance, Wildcard, ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry && entry->serviceDescription.getInstanceIDString() == instance)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }

    void find_(Wildcard, Wildcard, const capro::IdString_t& event, ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry && entry->serviceDescription.getEventIDString() == event)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }

    void find_(const capro::IdString_t& service,
               const capro::IdString_t& instance,
               Wildcard,
               ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry && entry->serviceDescription.getServiceIDString() == service
                && entry->serviceDescription.getInstanceIDString() == instance)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }

    void find_(const capro::IdString_t& service,
               Wildcard,
               const capro::IdString_t& event,
               ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry && entry->serviceDescription.getServiceIDString() == service
                && entry->serviceDescription.getEventIDString() == event)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }

    void find_(Wildcard,
               const capro::IdString_t& instance,
               const capro::IdString_t& event,
               ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry && entry->serviceDescription.getInstanceIDString() == instance
                && entry->serviceDescription.getEventIDString() == event)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }

    void find_(const capro::IdString_t& service,
               const capro::IdString_t& instance,
               const capro::IdString_t& event,
               ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry && entry->serviceDescription.getServiceIDString() == service
                && entry->serviceDescription.getInstanceIDString() == instance
                && entry->serviceDescription.getEventIDString() == event)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }

    void find_all(ServiceDescriptionVector_t& searchResult) const
    {
        for (auto& entry : m_serviceDescriptions)
        {
            if (entry)
            {
                searchResult.emplace_back(*entry);
            }
        }
    }
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_SERVICE_REGISTRY_HPP
