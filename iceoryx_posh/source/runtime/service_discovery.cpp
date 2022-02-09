// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/runtime/service_discovery.hpp"

namespace iox
{
namespace runtime
{
cxx::expected<ServiceContainer, FindServiceError>
ServiceDiscovery::findService(const cxx::optional<capro::IdString_t>& service,
                              const cxx::optional<capro::IdString_t>& instance,
                              const cxx::optional<capro::IdString_t>& event) noexcept
{
    capro::IdString_t serviceString;
    capro::IdString_t instanceString;
    capro::IdString_t eventString;
    bool isServiceWildcard = !service;
    bool isInstanceWildcard = !instance;
    bool isEventWildcard = !event;

    if (!isServiceWildcard)
    {
        serviceString = service.value();
    }
    if (!isInstanceWildcard)
    {
        instanceString = instance.value();
    }
    if (!isEventWildcard)
    {
        eventString = event.value();
    }

    ServiceContainer serviceContainer;

    // @todo #415 Get data and fill serviceContainter

    return {cxx::success<ServiceContainer>(serviceContainer)};
}

void ServiceDiscovery::findService(const cxx::optional<capro::IdString_t>& service,
                                   const cxx::optional<capro::IdString_t>& instance,
                                   const cxx::optional<capro::IdString_t>& event,
                                   const cxx::function_ref<void(const ServiceContainer&)>& callable) noexcept
{
    if (!callable)
    {
        return;
    }

    /// @todo #415 change implementation once PR #1088 is merged
    auto searchResult = findService(service, instance, event);
    if (!searchResult.has_error())
    {
        callable(searchResult.value());
    }
}
} // namespace runtime
} // namespace iox
