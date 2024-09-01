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

#ifndef IOX_BINDING_C_CLIENT_H
#define IOX_BINDING_C_CLIENT_H

#include "iceoryx_binding_c/config.h"
#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/service_description.h"
#include "iceoryx_binding_c/types.h"

/// @brief client handle
typedef IOX_C_CLASS UntypedClient* iox_client_t;

/// @brief options to be set for a client
typedef struct
{
    /// @brief size of the response queue
    uint64_t responseQueueCapacity;

    /// @brief name of the node the client belongs to
    char nodeName[IOX_CONFIG_NODE_NAME_SIZE];

    /// @brief Indicates if the client should be connected when created
    bool connectOnCreate;

    /// @brief Sets whether the server blocks when the client response queue is full
    enum iox_QueueFullPolicy responseQueueFullPolicy;

    /// @brief Sets whether the client blocks when the server request queue is full
    enum iox_ConsumerTooSlowPolicy serverTooSlowPolicy;

    /// @brief this value will be set exclusively by 'iox_client_options_init' and is not supposed to be modified
    /// otherwise
    uint64_t initCheck;
} iox_client_options_t;

/// @brief initialize client options to default values
/// @param[in] options pointer to options to be initialized,
///                    emit warning if it is a null pointer
/// @attention This must always be called on a newly created options struct to
///            prevent uninitialized values. The options may get extended
///            in the future.
void iox_client_options_init(iox_client_options_t* options);

/// @brief check whether the client options were initialized by iox_client_options_init
/// @param[in] options pointer to options to be checked
/// @return true if options are not null and were initialized, false otherwise
bool iox_client_options_is_initialized(const iox_client_options_t* const options);

/// @brief creates a client handle
/// @param[in] self pointer to preallocated memory of size = sizeof(iox_client_storage_t)
/// @param[in] service serviceString
/// @param[in] instance instanceString
/// @param[in] event eventString
/// @param[in] options client options set by the user,
///                    if it is a null pointer default options are used
/// @return handle of the client
iox_client_t iox_client_init(iox_client_storage_t* self,
                             const char* const service,
                             const char* const instance,
                             const char* const event,
                             const iox_client_options_t* const options);

/// @brief removes a client handle
/// @param[in] self the handle which should be removed
void iox_client_deinit(iox_client_t const self);

/// @brief allocates a request in the shared memory
/// @param[in] self handle of the client
/// @param[in] payload pointer in which a pointer to the user-payload of the allocated chunk is stored
/// @param[in] payloadSize user-payload size of the allocated request
/// @return on success it returns AllocationResult_SUCCESS otherwise a value which
///         describes the error
/// @note for the user-payload alignment 'IOX_C_CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT' is used
///       for a custom user-payload alignment please use 'iox_client_loan_aligned_request'
enum iox_AllocationResult
iox_client_loan_request(iox_client_t const self, void** const payload, const uint64_t payloadSize);

/// @brief allocates a request in the shared memory with a custom alignment for the user-payload
/// @param[in] self handle of the client
/// @param[in] payload pointer in which a pointer to the user-payload of the allocated request is stored
/// @param[in] payloadSize user-payload size of the allocated request
/// @param[in] payloadAlignment user-payload alignment of the allocated request
/// @return on success it returns AllocationResult_SUCCESS otherwise a value which
///         describes the error
enum iox_AllocationResult iox_client_loan_aligned_request(iox_client_t const self,
                                                          void** const payload,
                                                          const uint64_t payloadSize,
                                                          const uint32_t payloadAlignment);


/// @brief releases ownership of a previously allocated loaned request without sending it
/// @param[in] self handle of the client
/// @param[in] payload pointer to the user-payload of the loaned request which should be free'd
void iox_client_release_request(iox_client_t const self, void* const payload);

/// @brief sends a previously loaned request
/// @param[in] self handle of the client
/// @param[in] payload pointer to the user-payload of the request which should be send
/// @return on success it returns ClientSendResult_SUCCESS otherwise a value which
///         describes the error
enum iox_ClientSendResult iox_client_send(iox_client_t const self, void* const payload);

/// @brief connects to the service
/// @param[in] self handle to the client
void iox_client_connect(iox_client_t const self);

/// @brief disconnects from the service
/// @param[in] self handle to the client
void iox_client_disconnect(iox_client_t const self);

/// @brief what is the connection state?
/// @param[in] self handle to the client
/// @return ConnectionState_CONNECTED when successfully connected otherwise an enum which
///         describes the current state
enum iox_ConnectionState iox_client_get_connection_state(iox_client_t const self);

/// @brief retrieve a received respone
/// @param[in] self handle to the client
/// @param[in] payload pointer in which the pointer to the user-payload of the response is stored
/// @return if a chunk could be received it returns ChunkReceiveResult_SUCCESS otherwise
///         an enum which describes the error
enum iox_ChunkReceiveResult iox_client_take_response(iox_client_t const self, const void** const payload);

/// @brief release a previously acquired response (via iox_client_take_response)
/// @param[in] self handle to the client
/// @param[in] payload pointer to the user-payload of chunk which should be released
void iox_client_release_response(iox_client_t const self, const void* const payload);

/// @brief release all responses which are stored in the chunk queue
/// @param[in] self handle to the client
void iox_client_release_queued_responses(iox_client_t const self);

/// @brief are new responses available?
/// @param[in] self handle to the client
/// @return true if there are responses, otherwise false
bool iox_client_has_responses(iox_client_t const self);

/// @brief were responses missed?
/// @param[in] self handle to the client
/// @return true if there are lost responses due to overflowing queue, otherwise false
bool iox_client_has_missed_responses(iox_client_t const self);

/// @brief returns the service description of the client
/// @param[in] self handle to the client
/// @return the service description
iox_service_description_t iox_client_get_service_description(iox_client_t const self);
#endif
