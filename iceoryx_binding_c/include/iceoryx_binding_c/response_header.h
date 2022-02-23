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

/// @brief response header handle
typedef CLASS ResponseHeader* iox_response_header_t;

/// @brief const response header handle
typedef const CLASS ResponseHeader* iox_const_response_header_t;

/// @brief extract the response header from a given payload
/// @param[in] payload the pointer to the payload
/// @return a handle to the response header
iox_response_header_t iox_response_header_from_payload(void* const payload);

/// @brief extract the response header from a given payload
/// @param[in] payload the pointer to the payload
/// @return a handle to the response header
iox_const_response_header_t iox_response_header_from_payload_const(const void* const payload);

/// @brief set the response header into server error state
/// @param[in] self handle to the response header
void iox_response_header_set_server_error(iox_response_header_t const self);

/// @brief is the response header in an error state
/// @param[in] self handle to the response header
/// @return true if it is in an error state, otherwise false
bool iox_response_header_has_server_error(iox_response_header_t const self);

/// @brief is the response header in an error state
/// @param[in] self handle to the response header
/// @return true if it is in an error state, otherwise false
bool iox_response_header_has_server_error_const(iox_const_response_header_t const self);

/// @brief returns the rpc header version
/// @param[in] self handle to the response header
/// @return rpc header version
uint8_t iox_response_header_get_rpc_header_version(iox_response_header_t const self);

/// @brief returns the rpc header version
/// @param[in] self handle to the response header
/// @return rpc header version
uint8_t iox_response_header_get_rpc_header_version_const(iox_const_response_header_t const self);

/// @brief returns the sequence id
/// @param[in] self handle to the response header
/// @return sequence id
int64_t iox_response_header_get_sequence_id(iox_response_header_t const self);

/// @brief returns the sequence id
/// @param[in] self handle to the response header
/// @return sequence id
int64_t iox_response_header_get_sequence_id_const(iox_const_response_header_t const self);

/// @brief returns a pointer to the user payload
/// @param[in] self handle to the response header
/// @return pointer to the payload
void* iox_response_header_get_user_payload(iox_response_header_t const self);

/// @brief returns a pointer to the user payload
/// @param[in] self handle to the response header
/// @return pointer to the payload
const void* iox_response_header_get_user_payload_const(iox_const_response_header_t const self);

/// @brief returns a pointer to the chunk header
/// @param[in] self handle to the response header
/// @return pointer to the chunk header
iox_chunk_header_t* iox_response_header_get_chunk_header(iox_response_header_t const self);

/// @brief returns a pointer to the chunk header
/// @param[in] self handle to the response header
/// @return pointer to the chunk header
const iox_chunk_header_t* iox_response_header_get_chunk_header_const(iox_const_response_header_t const self);

#endif
