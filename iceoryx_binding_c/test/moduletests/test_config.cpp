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

extern "C" {
#include "iceoryx_binding_c/config.h"
}

#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include "test.hpp"

namespace
{
using namespace iox;

TEST(iox_cfg, valuesAreCorrectlyConnected)
{
    EXPECT_EQ(iox_cfg_max_publishers(), iox::MAX_PUBLISHERS);
    EXPECT_EQ(iox_cfg_max_subscribers_per_publisher(), iox::MAX_SUBSCRIBERS_PER_PUBLISHER);
    EXPECT_EQ(iox_cfg_max_chunks_allocated_per_publisher_simultaneously(),
              iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY);
    EXPECT_EQ(iox_cfg_max_publisher_history(), iox::MAX_PUBLISHER_HISTORY);
    EXPECT_EQ(iox_cfg_max_subscribers(), iox::MAX_SUBSCRIBERS);
    EXPECT_EQ(iox_cfg_max_chunks_held_per_subscriber_simultaneously(),
              iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY);
    EXPECT_EQ(iox_cfg_max_subscriber_queue_capacity(), iox::MAX_SUBSCRIBER_QUEUE_CAPACITY);
    EXPECT_EQ(iox_cfg_max_number_of_condition_variables(), iox::MAX_NUMBER_OF_CONDITION_VARIABLES);
    EXPECT_EQ(iox_cfg_max_number_of_notifiers_per_condition_variable(),
              iox::MAX_NUMBER_OF_NOTIFIERS_PER_CONDITION_VARIABLE);
    EXPECT_EQ(iox_cfg_max_number_of_attachments_per_waitset(), iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET);
    EXPECT_EQ(iox_cfg_max_number_of_events_per_listener(), iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER);
    EXPECT_EQ(iox_cfg_max_number_of_mempools(), iox::MAX_NUMBER_OF_MEMPOOLS);
    EXPECT_EQ(iox_cfg_max_shm_segments(), iox::MAX_SHM_SEGMENTS);
    EXPECT_EQ(iox_cfg_max_number_of_memory_provider(), iox::MAX_NUMBER_OF_MEMORY_PROVIDER);
    EXPECT_EQ(iox_cfg_max_number_of_memory_blocks_per_memory_provider(),
              iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER);
    EXPECT_EQ(iox_cfg_chunk_default_user_payload_alignment(), iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
    EXPECT_EQ(iox_cfg_no_user_header_size(), iox::CHUNK_NO_USER_HEADER_SIZE);
    EXPECT_EQ(iox_cfg_no_user_header_alignment(), iox::CHUNK_NO_USER_HEADER_ALIGNMENT);
    EXPECT_EQ(iox_cfg_max_process_number(), iox::MAX_PROCESS_NUMBER);
    EXPECT_EQ(iox_cfg_max_number_of_services(), iox::MAX_NUMBER_OF_SERVICES);
    EXPECT_EQ(iox_cfg_max_runtime_name_length(), iox::MAX_RUNTIME_NAME_LENGTH);
}
} // namespace
