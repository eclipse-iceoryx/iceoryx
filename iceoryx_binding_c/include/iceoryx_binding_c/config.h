// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

uint32_t iox_cfg_max_publishers();
uint32_t iox_cfg_max_subscribers_per_publisher();
uint32_t iox_cfg_max_chunks_allocated_per_publisher_simultaneously();
uint64_t iox_cfg_max_publisher_history();

uint32_t iox_cfg_max_subscribers();
uint32_t iox_cfg_max_chunks_held_per_subscriber_simultaneously();
uint32_t iox_cfg_max_subscriber_queue_capacity();

uint32_t iox_cfg_max_number_of_condition_variables();
uint32_t iox_cfg_max_number_of_notifiers_per_condition_variable();
uint32_t iox_cfg_max_number_of_attachments_per_waitset();
uint32_t iox_cfg_max_number_of_events_per_listener();

uint32_t iox_cfg_max_number_of_mempools();
uint32_t iox_cfg_max_shm_segments();
uint32_t iox_cfg_max_number_of_memory_provider();
uint32_t iox_cfg_max_number_of_memory_blocks_per_memory_provider();

uint32_t iox_cfg_chunk_default_user_payload_alignment();
uint32_t iox_cfg_no_user_header_size();
uint32_t iox_cfg_no_user_header_alignment();

uint32_t iox_cfg_max_process_number();
uint32_t iox_cfg_max_number_of_services();
uint32_t iox_cfg_max_runtime_name_length();


#endif
