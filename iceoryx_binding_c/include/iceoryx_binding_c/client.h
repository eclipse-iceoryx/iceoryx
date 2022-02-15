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

#ifndef IOX_BINDING_C_CLIENT_H
#define IOX_BINDING_C_CLIENT_H

#include "iceoryx_binding_c/config.h"
#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/types.h"

typedef iox_client_storage_t* iox_client_t;

typedef struct
{
    uint64_t responseQueueCapacity;
    char nodeName[IOX_CONFIG_NODE_NAME_SIZE];
    bool connectOnCreate;
    ENUM iox_QueueFullPolicy responseQueueFullPolicy;
    ENUM iox_ConsumerTooSlowPolicy serverTooSlowPolicy;
    uint64_t initCheck;
} iox_client_options_t;

void iox_client_options_init(iox_client_options_t* options);
bool iox_client_options_is_initialized(const iox_client_options_t* const options);

iox_client_t iox_client_init(iox_client_storage_t* self,
                             const char* const service,
                             const char* const instance,
                             const char* const event,
                             const iox_client_options_t* const options);

void iox_client_deinit(iox_client_t const self);

ENUM iox_AllocationResult iox_client_loan(iox_client_t const self,
                                          void** const userPayload,
                                          const uint32_t userPayloadSize,
                                          const uint32_t userPayloadAlignment);

void iox_client_release_request(iox_client_t const self, void* const userPayload);

void iox_client_send(iox_client_t const self, void* const userPayload);

void iox_client_connect(iox_client_t const self);

void iox_client_disconnect(iox_client_t const self);

ENUM iox_ConnectionState iox_client_get_connection_state(iox_client_t const self);

iox_ChunkReceiveResult iox_client_take(iox_client_t const self, const void** const userPayload);

void iox_client_release_response(iox_client_t const self, void* const userPayload);

void iox_client_release_queued_responses(iox_client_t const self);

bool iox_client_has_responses(iox_client_t const self);

bool iox_client_has_missed_responses(iox_client_t const self);


#endif
