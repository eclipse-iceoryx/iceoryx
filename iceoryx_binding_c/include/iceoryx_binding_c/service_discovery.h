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

#ifndef IOX_BINDING_C_SERVICE_DISCOVERY_H
#define IOX_BINDING_C_SERVICE_DISCOVERY_H

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/types.h"
#include "service_description.h"

/// @brief service discovery handle
typedef IOX_C_CLASS ServiceDiscovery* iox_service_discovery_t;

/// @brief initializes a service discovery from a storage struct pointer
/// @param[in] self pointer to raw memory which can hold a service discovery
/// @return an initialized iox_service_discovery_t
iox_service_discovery_t iox_service_discovery_init(iox_service_discovery_storage_t* self);

/// @brief after using an iox_service_discovery_t it must be cleaned up with this function
/// @param[in] self the service discovery which should be deinitialized
void iox_service_discovery_deinit(iox_service_discovery_t const self);

/// @brief Searches all services with the given messaging pattern that match the provided service description
/// @param[in] self handle of the service discovery
/// @param[in] service service string to search for, a nullptr corresponds to a wildcard
/// @param[in] instance instance string to search for, a nullptr corresponds to a wildcard
/// @param[in] event event string to search for, a nullptr corresponds to a wildcard
/// @param[in] serviceContainer preallocated memory to an array of iox_service_description_t in which the matching
/// services can be written
/// @param[in] serviceContainerCapacity the capacity of the preallocated serviceContainer
/// @param[in] missedServices if the serviceContainer has insufficient size the number of missed services which could
/// not be written into the serviceContainer are stored here
/// @param[in] pattern messaging pattern of the service to search
/// @return the number of services which were written into the serviceContainer
uint64_t iox_service_discovery_find_service(iox_service_discovery_t const self,
                                            const char* const service,
                                            const char* const instance,
                                            const char* const event,
                                            iox_service_description_t* const serviceContainer,
                                            const uint64_t serviceContainerCapacity,
                                            uint64_t* missedServices,
                                            const enum iox_MessagingPattern pattern);

/// @brief Searches all services with the given messaging pattern that match the provided service description and
/// applies a function to each of them
/// @param[in] self handle of the service discovery
/// @param[in] service service string to search for, a nullptr corresponds to a wildcard
/// @param[in] instance instance string to search for, a nullptr corresponds to a wildcard
/// @param[in] event event string to search for, a nullptr corresponds to a wildcard
/// @param[in] callable to apply to all matching services
/// @param[in] pattern messaging pattern of the service to search
void iox_service_discovery_find_service_apply_callable(iox_service_discovery_t const self,
                                                       const char* const service,
                                                       const char* const instance,
                                                       const char* const event,
                                                       void (*callable)(const iox_service_description_t),
                                                       const enum iox_MessagingPattern pattern);

/// @brief Searches all services with the given messaging pattern that match the provided service description and
/// applies a function to each of them. A second parameter for the function can be provided as contextData.
/// @param[in] self handle of the service discovery
/// @param[in] service service string to search for, a nullptr corresponds to a wildcard
/// @param[in] instance instance string to search for, a nullptr corresponds to a wildcard
/// @param[in] event event string to search for, a nullptr corresponds to a wildcard
/// @param[in] callable to apply to all matching services
/// @param[in] contextData a void pointer which is provided as second argument to the callback
/// @param[in] pattern messaging pattern of the service to search
void iox_service_discovery_find_service_apply_callable_with_context_data(
    iox_service_discovery_t const self,
    const char* const service,
    const char* const instance,
    const char* const event,
    void (*callable)(const iox_service_description_t, void*),
    void* const contextData,
    const enum iox_MessagingPattern pattern);

#endif
