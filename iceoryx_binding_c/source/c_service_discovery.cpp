// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by Michael Bentley <mikebentley15@gmail.com>. All rights reserved.
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

#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_service_description_translation.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iox/assertions.hpp"
#include "iox/optional.hpp"

using namespace iox;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/service_discovery.h"
}

iox_service_discovery_t iox_service_discovery_init(iox_service_discovery_storage_t* self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    auto* me = new ServiceDiscovery();
    self->do_not_touch_me[0] = reinterpret_cast<uint64_t>(me);
    return me;
}

void iox_service_discovery_deinit(iox_service_discovery_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    delete self;
}

uint64_t iox_service_discovery_find_service(iox_service_discovery_t const self,
                                            const char* const service,
                                            const char* const instance,
                                            const char* const event,
                                            iox_service_description_t* const serviceContainer,
                                            const uint64_t serviceContainerCapacity,
                                            uint64_t* missedServices,
                                            const enum iox_MessagingPattern pattern)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceContainer != nullptr, "'serviceContainer' must not be a 'nullptr'");
    IOX_ENFORCE(missedServices != nullptr, "'missedServices' must not be a 'nullptr'");

    optional<capro::IdString_t> maybeService;
    if (service != nullptr)
    {
        maybeService.emplace(TruncateToCapacity, service);
    }
    optional<capro::IdString_t> maybeInstance;
    if (instance != nullptr)
    {
        maybeInstance.emplace(TruncateToCapacity, instance);
    }
    optional<capro::IdString_t> maybeEvent;
    if (event != nullptr)
    {
        maybeEvent.emplace(TruncateToCapacity, event);
    }

    uint64_t currentSize = 0U;
    auto filter = [&](const capro::ServiceDescription& s) {
        if (currentSize < serviceContainerCapacity)
        {
            serviceContainer[currentSize] = TranslateServiceDescription(s);
            ++currentSize;
        }
        else
        {
            ++(*missedServices);
        }
    };
    self->findService(maybeService, maybeInstance, maybeEvent, filter, c2cpp::messagingPattern(pattern));

    return currentSize;
}

void iox_service_discovery_find_service_apply_callable(iox_service_discovery_t const self,
                                                       const char* const service,
                                                       const char* const instance,
                                                       const char* const event,
                                                       void (*callable)(const iox_service_description_t),
                                                       const enum iox_MessagingPattern pattern)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(callable != nullptr, "'callable' must not be a 'nullptr'");

    optional<capro::IdString_t> maybeService;
    if (service != nullptr)
    {
        maybeService.emplace(TruncateToCapacity, service);
    }
    optional<capro::IdString_t> maybeInstance;
    if (instance != nullptr)
    {
        maybeInstance.emplace(TruncateToCapacity, instance);
    }
    optional<capro::IdString_t> maybeEvent;
    if (event != nullptr)
    {
        maybeEvent.emplace(TruncateToCapacity, event);
    }

    auto filter = [&](const capro::ServiceDescription& s) { callable(TranslateServiceDescription(s)); };
    self->findService(maybeService, maybeInstance, maybeEvent, filter, c2cpp::messagingPattern(pattern));
}

void iox_service_discovery_find_service_apply_callable_with_context_data(
    iox_service_discovery_t const self,
    const char* const service,
    const char* const instance,
    const char* const event,
    void (*callable)(const iox_service_description_t, void*),
    void* const contextData,
    const enum iox_MessagingPattern pattern)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(callable != nullptr, "'callable' must not be a 'nullptr'");

    optional<capro::IdString_t> maybeService;
    if (service != nullptr)
    {
        maybeService.emplace(TruncateToCapacity, service);
    }
    optional<capro::IdString_t> maybeInstance;
    if (instance != nullptr)
    {
        maybeInstance.emplace(TruncateToCapacity, instance);
    }
    optional<capro::IdString_t> maybeEvent;
    if (event != nullptr)
    {
        maybeEvent.emplace(TruncateToCapacity, event);
    }

    auto filter = [&](const capro::ServiceDescription& s) { callable(TranslateServiceDescription(s), contextData); };
    self->findService(maybeService, maybeInstance, maybeEvent, filter, c2cpp::messagingPattern(pattern));
}
