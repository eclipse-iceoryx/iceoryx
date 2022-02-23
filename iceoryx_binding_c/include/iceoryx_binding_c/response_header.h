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

#ifndef IOX_BINDING_C_RESPONSE_HEADER_H
#define IOX_BINDING_C_RESPONSE_HEADER_H

#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"

typedef CLASS ResponseHeader* iox_response_header_t;
typedef const CLASS ResponseHeader* iox_const_response_header_t;

iox_response_header_t iox_response_header_from_payload(void* const payload);
iox_const_response_header_t iox_response_header_from_payload_const(const void* const payload);

void iox_response_header_set_server_error(iox_response_header_t const self);
bool iox_response_header_has_server_error(iox_response_header_t const self);
bool iox_response_header_has_server_error_const(iox_const_response_header_t const self);

uint8_t iox_response_header_get_rpc_header_version(iox_response_header_t const self);
uint8_t iox_response_header_get_rpc_header_version(iox_response_header_t const self);
int64_t iox_response_header_get_sequence_id(iox_response_header_t const self);
int64_t iox_response_header_get_sequence_id_const(iox_const_response_header_t const self);
void* iox_response_header_get_user_payload(iox_response_header_t const self);
const void* iox_response_header_get_user_payload_const(iox_const_response_header_t const self);
iox_chunk_header_t* iox_response_header_get_chunk_header(iox_response_header_t const self);
const iox_chunk_header_t* iox_response_header_get_chunk_header_const(iox_const_response_header_t const self);

#endif
