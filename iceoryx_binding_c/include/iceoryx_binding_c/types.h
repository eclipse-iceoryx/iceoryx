// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 Apex.AI Inc. All rights reserved.
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

#ifndef IOX_BINDING_C_TYPES_H
#define IOX_BINDING_C_TYPES_H

#include "iceoryx_binding_c/iceoryx_binding_c_deployment.h"
#include "internal/c2cpp_binding.h"

#if defined(__APPLE__)
#define CALCULATE_STORAGE_SIZE_FOR_LISTENER(numberOfAttachments)                                                       \
    (144 + numberOfAttachments * 168 - 8 * (((numberOfAttachments + 1) / 2) - 1))
#elif defined(_WIN32)
#define CALCULATE_STORAGE_SIZE_FOR_LISTENER(numberOfAttachments)                                                       \
    (168 + numberOfAttachments * 176 - 8 * (((numberOfAttachments + 1) / 2) - 1))
#else
#define CALCULATE_STORAGE_SIZE_FOR_LISTENER(numberOfAttachments) (((128 + numberOfAttachments * 140) / 8) * 8)
#endif

#if defined(_WIN32)
#define CALCULATE_STORAGE_SIZE_FOR_WAITSET(numberOfAttachments) (552 + numberOfAttachments * 168)
#else
#define CALCULATE_STORAGE_SIZE_FOR_WAITSET(numberOfAttachments) (552 + numberOfAttachments * 184)
#endif

#define IOX_C_CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT 8
#define IOX_C_CHUNK_NO_USER_HEADER_SIZE 0
#define IOX_C_CHUNK_NO_USER_HEADER_ALIGNMENT 1

/// The issue iox-308: https://github.com/eclipse-iceoryx/iceoryx/issues/308
/// was created to explore other options then a magic number to create
/// the structs of a specific size in C.

/// The size and the alignment of all structs are verified by the
/// binding c integration test iox_types_test

struct iox_ws_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(WaitSet) / 8
    /// @note see iceoryx_binding_c_deployment.h.in for calculation of the size
    uint64_t do_not_touch_me[CALCULATE_STORAGE_SIZE_FOR_WAITSET(IOX_BUILD_GENERATED_MAX_NUMBER_OF_NOTIFIERS) / 8];
};
typedef struct iox_ws_storage_t_ iox_ws_storage_t;

struct iox_user_trigger_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(UserTrigger) / 8
#if defined(__APPLE__)
    uint64_t do_not_touch_me[15];
#elif defined(_WIN32)
    uint64_t do_not_touch_me[16];
#else
    uint64_t do_not_touch_me[12];
#endif
};
typedef struct iox_user_trigger_storage_t_ iox_user_trigger_storage_t;

struct iox_sub_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(cpp2c_Subscriber) / 8
#if defined(__APPLE__)
    uint64_t do_not_touch_me[16];
#elif defined(_WIN32)
    uint64_t do_not_touch_me[17];
#else
    uint64_t do_not_touch_me[13];
#endif
};
typedef struct iox_sub_storage_t_ iox_sub_storage_t;

struct iox_pub_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(cpp2c_Publisher) / 8
    uint64_t do_not_touch_me[1];
};
typedef struct iox_pub_storage_t_ iox_pub_storage_t;

struct iox_listener_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(Listener) / 8
    /// @note see iceoryx_binding_c_deployment.h.in for calculation of the size
    uint64_t do_not_touch_me[CALCULATE_STORAGE_SIZE_FOR_LISTENER(IOX_BUILD_GENERATED_MAX_NUMBER_OF_NOTIFIERS) / 8];
};
typedef struct iox_listener_storage_t_ iox_listener_storage_t;

/// @brief handle of the chunk header
typedef struct
{
    // could be empty but then we get `struct has no members` warning
    uint8_t do_not_touch_me[1];
} iox_chunk_header_t;

/// @brief has exactly the size required to store the underlying object of iox_client_t
typedef struct
{
    // the value of the array size is the result of the following formula:
    // sizeof(UntypedClient) / 8
#if defined(__APPLE__)
    uint64_t do_not_touch_me[22];
#elif defined(_WIN32)
    uint64_t do_not_touch_me[23];
#else
    uint64_t do_not_touch_me[19];
#endif
} iox_client_storage_t;

/// @brief has exactly the size required to store the underlying object of iox_server_t
typedef struct
{
    // the value of the array size is the result of the following formula:
    // sizeof(UntypedServer) / 8
#if defined(__APPLE__)
    uint64_t do_not_touch_me[22];
#elif defined(_WIN32)
    uint64_t do_not_touch_me[23];
#else
    uint64_t do_not_touch_me[19];
#endif
} iox_server_storage_t;

/// @brief has exactly the size required to store the underlying object of iox_service_discovery_t
struct iox_service_discovery_storage_t
{
    // the value of the array size is the result of the following formula:
    // sizeof(ServiceDiscovery) / 8
#if defined(__APPLE__)
    uint64_t do_not_touch_me[30];
#elif defined(_WIN32)
    uint64_t do_not_touch_me[33];
#else
    uint64_t do_not_touch_me[24];
#endif
};
typedef struct iox_service_discovery_storage_t iox_service_discovery_storage_t;

#endif
