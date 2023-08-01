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

/// @brief server handle
typedef CLASS UntypedServer* iox_server_t;

/// @brief options to be set for a server
typedef struct
{
    /// @brief size of the request queue
    uint64_t requestQueueCapacity;

    /// @brief name of the node the server belongs to
    char nodeName[IOX_CONFIG_NODE_NAME_SIZE];

    /// @brief Indicates if the server should be connected when created
    bool offerOnCreate;

    /// @brief Sets whether the client blocks when the server request queue is full
    ENUM iox_QueueFullPolicy requestQueueFullPolicy;

    /// @brief Sets whether the server blocks when the client response queue is full
    ENUM iox_ConsumerTooSlowPolicy clientTooSlowPolicy;

    /// @brief this value will be set exclusively by 'iox_server_options_init' and is not supposed to be modified
    /// otherwise
    uint64_t initCheck;
} iox_server_options_t;

/// @brief initialize server options to default values
/// @param[in] options pointer to options to be initialized,
///                    emit warning if it is a null pointer
/// @attention This must always be called on a newly created options struct to
///            prevent uninitialized values. The options may get extended
///            in the future.
void iox_server_options_init(iox_server_options_t* const options);

/// @brief check whether the server options were initialized by iox_server_options_init
/// @param[in] options pointer to options to be checked
/// @return true if options are not null and were initialized, false otherwise
bool iox_server_options_is_initialized(const iox_server_options_t* const options);

/// @brief creates a server handle
/// @param[in] self pointer to preallocated memory of size = sizeof(iox_server_storage_t)
/// @param[in] service serviceString
/// @param[in] instance instanceString
/// @param[in] event eventString
/// @param[in] options server options set by the user,
///                    if it is a null pointer default options are used
/// @return handle of the server
iox_server_t iox_server_init(iox_server_storage_t* self,
                             const char* const service,
                             const char* const instance,
                             const char* const event,
                             const iox_server_options_t* const options);

/// @brief removes a server handle
/// @param[in] self the handle which should be removed
void iox_server_deinit(iox_server_t const self);

/// @brief retrieve a received request
/// @param[in] self handle to the server
/// @param[in] payload pointer in which the pointer to the user-payload of the request is stored
/// @return if a chunk could be received it returns ChunkReceiveResult_SUCCESS otherwise
///         an enum which describes the error
ENUM iox_ServerRequestResult iox_server_take_request(iox_server_t const self, const void** const payload);

/// @brief release a previously acquired request (via iox_server_take_request)
/// @param[in] self handle to the server
/// @param[in] payload pointer to the user-payload of chunk which should be released
void iox_server_release_request(iox_server_t const self, const void* const payload);

/// @brief allocates a response in the shared memory
/// @param[in] self handle of the server
/// @param[in] requestPayload pointer to the payload of the received request
/// @param[in] payload pointer in which a pointer to the user-payload of the allocated chunk is stored
/// @param[in] payloadSize user-payload size of the allocated request
/// @return on success it returns AllocationResult_SUCCESS otherwise a value which
///         describes the error
/// @note for the user-payload alignment 'IOX_C_CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT' is used
///       for a custom user-payload alignment please use 'iox_server_loan_aligned_response'
ENUM iox_AllocationResult iox_server_loan_response(iox_server_t const self,
                                                   const void* const requestPayload,
                                                   void** const payload,
                                                   const uint32_t payloadSize);

/// @brief allocates a response in the shared memory
/// @param[in] self handle of the server
/// @param[in] requestPayload pointer to the payload of the received request
/// @param[in] payload pointer in which a pointer to the user-payload of the allocated chunk is stored
/// @param[in] payloadSize user-payload size of the allocated request
/// @param[in] payloadAlignment user-payload alignment of the allocated request
/// @return on success it returns AllocationResult_SUCCESS otherwise a value which
///         describes the error
ENUM iox_AllocationResult iox_server_loan_aligned_response(iox_server_t const self,
                                                           const void* const requestPayload,
                                                           void** const payload,
                                                           const uint32_t payloadSize,
                                                           const uint32_t payloadAlignment);

/// @brief sends a previously loaned response
/// @param[in] self handle of the server
/// @param[in] payload pointer to the user-payload of the response which should be send
/// @return on success it returns ServerSendResult_SUCCESS otherwise a value which
///         describes the error
ENUM iox_ServerSendResult iox_server_send(iox_server_t const self, void* const payload);

/// @brief releases ownership of a previously allocated loaned response without sending it
/// @param[in] self handle of the server
/// @param[in] payload pointer to the user-payload of the loaned request which should be free'd
void iox_server_release_response(iox_server_t const self, void* const payload);

/// @brief returns the service description of the server
/// @param[in] self handle to the server
/// @return the service description
iox_service_description_t iox_server_get_service_description(iox_server_t const self);

/// @brief offers the servers service
/// @param[in] self handle to the server
void iox_server_offer(iox_server_t const self);

/// @brief stops offering the servers service
/// @param[in] self handle to the server
void iox_server_stop_offer(iox_server_t const self);

/// @brief is the server currently offering?
/// @param[in] self handle to the server
/// @return true if the server is offering, otherwise false
bool iox_server_is_offered(iox_server_t const self);

/// @brief are clients connected to the server?
/// @param[in] self handle to the server
/// @return true if the server has connected clients, otherwise false
bool iox_server_has_clients(iox_server_t const self);

/// @brief are requests from clients available?
/// @param[in] self handle to the server
/// @return true if the requests are available to take, otherwise false
bool iox_server_has_requests(iox_server_t const self);

/// @brief were requests missed?
/// @param[in] self handle to the server
/// @return true if there are lost requests due to overflowing queue, otherwise false
bool iox_server_has_missed_requests(iox_server_t const self);

/// @brief release a previously acquired request (via iox_server_take_request)
/// @param[in] self handle to the server
void iox_server_release_queued_requests(iox_server_t const self);

#endif
