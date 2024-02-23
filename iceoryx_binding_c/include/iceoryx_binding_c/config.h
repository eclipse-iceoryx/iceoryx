// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_BINDING_C_CONFIG_H
#define IOX_BINDING_C_CONFIG_H

#include <stdint.h>

/// @brief returns maximum supported amount of publishers
uint32_t iox_cfg_max_publishers(void);

/// @brief returns maximum amount of subscribers which can be subscribed to one publisher
uint32_t iox_cfg_max_subscribers_per_publisher(void);

/// @brief returns maximum amount of samples a publisher can acquire at the same time with loan
uint32_t iox_cfg_max_chunks_allocated_per_publisher_simultaneously(void);

/// @brief returns maximum history size for a publisher (e.g. samples which are hold back so
//         that new subscribers can acquire past data)
uint64_t iox_cfg_max_publisher_history(void);

/// @brief returns maximum supported amount of subscribers
uint32_t iox_cfg_max_subscribers(void);

/// @brief returns the maximum amount of samples which a subscriber can hold without releasing them
uint32_t iox_cfg_max_chunks_held_per_subscriber_simultaneously(void);

/// @brief returns the maximum subscriber queue capacity which is used when the publisher delivers samples to the
/// subscriber if the queue capacity is reached new samples will discard old samples
uint32_t iox_cfg_max_subscriber_queue_capacity(void);

/// @brief returns the maximum supported amount of condition variables. this determines how many listeners and waitsets
/// can be used in one iceoryx system
uint32_t iox_cfg_max_number_of_condition_variables(void);

/// @brief returns the maximum supported amount of notifiers per condition variable. this determines how many
/// attachments a listener or waitset can have
uint32_t iox_cfg_max_number_of_notifiers_per_condition_variable(void);

/// @brief returns the maximum amount of attachments per waitset
/// @note is less or equal to iox_cfg_max_number_of_notifiers_per_condition_variable
uint32_t iox_cfg_max_number_of_attachments_per_waitset(void);

/// @brief returns the maximum amount of evens per listener
/// @note is less or equal to iox_cfg_max_number_of_notifiers_per_condition_variable
uint32_t iox_cfg_max_number_of_events_per_listener(void);

/// @brief returns the maximum amount of mempools for roudi. restricts also the number of mempools in the roudi config
/// file
uint32_t iox_cfg_max_number_of_mempools(void);

/// @brief returns the maximum number of shared memory segments. restricts also the number of configurable segments in
/// the roudi config file
uint32_t iox_cfg_max_shm_segments(void);

/// @brief returns the maximum supported amount of shared memory providers
uint32_t iox_cfg_max_number_of_memory_provider(void);

/// @brief returns the maximum supported amount of memory blocks per shared memory provider
uint32_t iox_cfg_max_number_of_memory_blocks_per_memory_provider(void);

/// @brief returns the alignment of the user payload when it is not set explicitly
uint32_t iox_cfg_chunk_default_user_payload_alignment(void);

/// @brief returns the size of the user header when no user header is requested by the user
uint32_t iox_cfg_no_user_header_size(void);

/// @brief returns the alignment of the user header when no user header is requested by the user
uint32_t iox_cfg_no_user_header_alignment(void);

/// @brief returns the maximum supported amount of processes which can register at roudi by initializing the posh
/// runtime
uint32_t iox_cfg_max_process_number(void);

/// @brief returns the maximum number of services that are supported byt the service registry
uint32_t iox_cfg_service_registry_capacity(void);

/// @brief returns the maximum number of services a findservice call can return
uint32_t iox_cfg_max_findservice_result_size(void);

/// @brief returns the maximum runtime name length
uint32_t iox_cfg_max_runtime_name_length(void);

#if defined(__APPLE__)
/// @brief the maximum size of a node name string + \0 terminator
#define IOX_CONFIG_NODE_NAME_SIZE 86
#else
/// @brief the maximum size of a node name string + \0 terminator
#define IOX_CONFIG_NODE_NAME_SIZE 88
#endif

/// @brief the maximum size of a service description string identifier + \0 terminator
#define IOX_CONFIG_SERVICE_STRING_SIZE 101

#endif // IOX_BINDING_C_CONFIG_H
