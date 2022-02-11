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
#ifndef IOX_POSH_RUNTIME_SERVICE_DISCOVERY_HPP
#define IOX_POSH_RUNTIME_SERVICE_DISCOVERY_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace runtime
{
class ServiceDiscovery
{
  public:
    ServiceDiscovery() noexcept = default;
    ServiceDiscovery(const ServiceDiscovery&) = delete;
    ServiceDiscovery& operator=(const ServiceDiscovery&) = delete;
    ServiceDiscovery(ServiceDiscovery&&) = delete;
    ServiceDiscovery& operator=(ServiceDiscovery&&) = delete;
    ~ServiceDiscovery() noexcept = default;

    /// @brief Searches all services that match the provided service description
    /// @param[in] service service string to search for, a nullopt corresponds to a wildcard
    /// @param[in] instance instance string to search for, a nullopt corresponds to a wildcard
    /// @param[in] event event string to search for, a nullopt corresponds to a wildcard
    /// @return ServiceContainer
    /// ServiceContainer: on success, container that is filled with all matching instances
    /// FindServiceError: if any, encountered during the operation
    ServiceContainer findService(const cxx::optional<capro::IdString_t>& service,
                                 const cxx::optional<capro::IdString_t>& instance,
                                 const cxx::optional<capro::IdString_t>& event) noexcept;

    /// @brief Searches all services that match the provided service description and applies a function to each of them
    /// @param[in] service service string to search for, a nullopt corresponds to a wildcard
    /// @param[in] instance instance string to search for, a nullopt corresponds to a wildcard
    /// @param[in] event event string to search for, a nullopt corresponds to a wildcard
    /// @param[in] callable to apply to all matching services
    void findService(const cxx::optional<capro::IdString_t>& service,
                     const cxx::optional<capro::IdString_t>& instance,
                     const cxx::optional<capro::IdString_t>& event,
                     const cxx::function_ref<void(const ServiceContainer&)>& callable) noexcept;

    friend iox::popo::NotificationAttorney;

  private:
    void enableEvent(popo::TriggerHandle&& triggerHandle, const popo::SubscriberEvent event) noexcept;
    void disableEvent(const popo::SubscriberEvent event) noexcept;
    void invalidateTrigger(const uint64_t uniqueTriggerId);
    iox::popo::WaitSetIsConditionSatisfiedCallback
    getCallbackForIsStateConditionSatisfied(const popo::SubscriberState state);
    roudi::ServiceRegistry m_serviceRegistry;
    popo::Subscriber<roudi::ServiceRegistry> m_serviceRegistrySubscriber{
        roudi::serviceRegistryService, {1U, 1U, iox::NodeName_t("Service Registry"), true}};
};


} // namespace runtime

} // namespace iox

#endif // IOX_POSH_RUNTIME_SERVICE_DISCOVERY_HPP
