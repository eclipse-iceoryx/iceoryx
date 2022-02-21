// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_BINDING_C_SERVER_H
#define IOX_BINDING_C_SERVER_H

#include "iceoryx_binding_c/config.h"
#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/service_description.h"
#include "iceoryx_binding_c/types.h"

typedef CLASS UntypedServer* iox_server_t;

typedef struct
{
    uint64_t requestQueueCapacity;
    char nodeName[IOX_CONFIG_NODE_NAME_SIZE];
    bool offerOnCreate;
    ENUM iox_QueueFullPolicy requestQueueFullPolicy;
    ENUM iox_ConsumerTooSlowPolicy clientTooSlowPolicy;
    uint64_t initCheck;
} iox_server_options_t;

void iox_server_options_init(iox_server_options_t* const options);
void iox_server_options_is_initialized(const iox_server_options_t* const options);

iox_server_t iox_server_init(iox_server_storage_t* self,
                             const char* const service,
                             const char* const instance,
                             const char* const event,
                             const iox_server_options_t* const options);
void iox_server_deinit(iox_server_t const self);

iox_ServerRequestResult iox_server_take_request(iox_server_t const self, const void** const payload);
void iox_server_release_request(iox_server_t const self, const void* const payload);
iox_AllocationResult iox_server_loan_response(iox_server_t const self,
                                              const void* const requestPayload,
                                              void** const payload,
                                              const uint32_t payloadSize);
iox_AllocationResult iox_server_loan_aligned_response(iox_server_t const self,
                                                      const void* const requestPayload,
                                                      void** const payload,
                                                      const uint32_t payloadSize,
                                                      const uint32_t payloadAlignment);
void iox_server_send(iox_server_t const self, void* const payload);
void iox_server_release_response(iox_server_t const self, void* const payload);

iox_service_description_t iox_server_get_service_description(iox_server_t const self);

void iox_server_offer(iox_server_t const self);
void iox_server_stop_offer(iox_server_t const self);
bool iox_server_is_offered(iox_server_t const self);
bool iox_server_has_clients(iox_server_t const self);
bool iox_server_has_requests(iox_server_t const self);
bool iox_server_has_missed_requests(iox_server_t const self);
void iox_server_release_queued_requests(iox_server_t const self);

#endif
