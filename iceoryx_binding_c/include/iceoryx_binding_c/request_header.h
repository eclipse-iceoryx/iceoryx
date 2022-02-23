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

#ifndef IOX_BINDING_C_REQUEST_HEADER_H
#define IOX_BINDING_C_REQUEST_HEADER_H

#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"

typedef CLASS RequestHeader* iox_request_header_t;

iox_request_header_t iox_request_header_from_payload(void* const payload) noexcept;
iox_request_header_t iox_request_header_from_const_payload(const void* const payload) noexcept;

void iox_request_header_set_sequence_id(iox_request_header_t const self, const int64_t sequenceId);
void iox_request_header_set_fire_and_forget(iox_request_header_t const self);
bool iox_request_header_is_fire_and_forget(iox_request_header_t const self);
uint8_t iox_request_header_get_header_version(iox_request_header_t const self);
int64_t iox_request_header_get_sequence_id(iox_request_header_t const self);
void* iox_request_header_get_user_payload(iox_request_header_t const self);
iox_chunk_header_t iox_request_header_get_chunk_header(iox_request_header_t const self);

#endif
