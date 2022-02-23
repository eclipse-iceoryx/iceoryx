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

/// @brief request header handle
typedef CLASS RequestHeader* iox_request_header_t;

/// @brief const request header handle
typedef const CLASS RequestHeader* iox_const_request_header_t;

/// @brief extract the request header from a given payload
/// @param[in] payload the pointer to the payload
/// @return a handle to the request header
iox_request_header_t iox_request_header_from_payload(void* const payload);

/// @brief extract the request header from a given payload
/// @param[in] payload the pointer to the payload
/// @return a handle to the request header
iox_const_request_header_t iox_request_header_from_payload_const(const void* const payload);

/// @brief set the sequence id of the request header
/// @param[in] self handle to the request header
/// @param[in] sequenceId the new sequence id value
void iox_request_header_set_sequence_id(iox_request_header_t const self, const int64_t sequenceId);

/// @brief returns the rpc header version
/// @param[in] self handle to the request header
/// @return rpc header version
uint8_t iox_request_header_get_rpc_header_version(iox_request_header_t const self);

/// @brief returns the rpc header version
/// @param[in] self handle to the request header
/// @return rpc header version
uint8_t iox_request_header_get_rpc_header_version_const(iox_const_request_header_t const self);

/// @brief returns the sequence id
/// @param[in] self handle to the request header
/// @return sequence id
int64_t iox_request_header_get_sequence_id(iox_request_header_t const self);

/// @brief returns the sequence id
/// @param[in] self handle to the request header
/// @return sequence id
int64_t iox_request_header_get_sequence_id_const(iox_const_request_header_t const self);

/// @brief returns a pointer to the user payload
/// @param[in] self handle to the request header
/// @return pointer to the payload
void* iox_request_header_get_user_payload(iox_request_header_t const self);

/// @brief returns a pointer to the user payload
/// @param[in] self handle to the request header
/// @return pointer to the payload
const void* iox_request_header_get_user_payload_const(iox_const_request_header_t const self);

/// @brief returns a pointer to the chunk header
/// @param[in] self handle to the request header
/// @return pointer to the chunk header
iox_chunk_header_t* iox_request_header_get_chunk_header(iox_request_header_t const self);

/// @brief returns a pointer to the chunk header
/// @param[in] self handle to the request header
/// @return pointer to the chunk header
const iox_chunk_header_t* iox_request_header_get_chunk_header_const(iox_const_request_header_t const self);

#endif
