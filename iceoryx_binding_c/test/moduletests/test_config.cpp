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
    ::testing::Test::RecordProperty("TEST_ID", "d62b1b3c-60c6-461b-8ca4-80c850fa65a0");
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
    EXPECT_EQ(iox_cfg_max_number_of_notifiers_per_condition_variable(), iox::MAX_NUMBER_OF_NOTIFIERS);
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
    EXPECT_EQ(iox_cfg_service_registry_capacity(), iox::SERVICE_REGISTRY_CAPACITY);
    EXPECT_EQ(iox_cfg_max_findservice_result_size(), iox::MAX_FINDSERVICE_RESULT_SIZE);
    EXPECT_EQ(iox_cfg_max_runtime_name_length(), iox::MAX_RUNTIME_NAME_LENGTH);

    constexpr uint64_t ZERO_TERMINATOR_SIZE = 1U;
    EXPECT_EQ(IOX_CONFIG_NODE_NAME_SIZE, iox::NodeName_t::capacity() + ZERO_TERMINATOR_SIZE);

    EXPECT_EQ(IOX_CONFIG_SERVICE_STRING_SIZE, iox::capro::IdString_t::capacity() + ZERO_TERMINATOR_SIZE);
}
} // namespace
