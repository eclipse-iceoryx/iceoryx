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

#include "iceoryx_binding_c/internal/cpp2c_publisher.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"


using namespace iox;
using namespace iox::popo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/types.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;

TEST(iox_types_test, WaitSetStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b010bf2-9ba7-401f-a0c0-fb5d802f33d6");
    EXPECT_THAT(sizeof(WaitSet<>), Le(sizeof(iox_ws_storage_t)));
    EXPECT_THAT(alignof(WaitSet<>), Le(alignof(iox_ws_storage_t)));
}

TEST(iox_types_test, ListenerStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "612beb6a-a778-4cc6-94eb-42856c2fee6f");
    EXPECT_THAT(sizeof(Listener), Le(sizeof(iox_listener_storage_t)));
    EXPECT_THAT(alignof(Listener), Le(alignof(iox_listener_storage_t)));
}

TEST(iox_types_test, UserTriggerStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "53e7081c-fa0d-4f2d-9dbf-81e7803cbcc6");
    EXPECT_THAT(sizeof(UserTrigger), Le(sizeof(iox_user_trigger_storage_t)));
    EXPECT_THAT(alignof(UserTrigger), Le(alignof(iox_user_trigger_storage_t)));
}

TEST(iox_types_test, cpp2c_SubscriberStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "7fbe6908-0a4b-44f8-b369-4998dab2c7ba");
    EXPECT_THAT(sizeof(cpp2c_Subscriber), Le(sizeof(iox_sub_storage_t)));
    EXPECT_THAT(alignof(cpp2c_Subscriber), Le(alignof(iox_sub_storage_t)));
}

TEST(iox_types_test, cpp2c_PublisherStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b216e64-1310-4016-961d-fc54ab1ef6ea");
    EXPECT_THAT(sizeof(cpp2c_Publisher), Le(sizeof(iox_pub_storage_t)));
    EXPECT_THAT(alignof(cpp2c_Publisher), Le(alignof(iox_pub_storage_t)));
}

TEST(iox_types_test, ClientStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "70aa551d-4220-4389-8cae-cd3c5314b2b0");
    EXPECT_THAT(sizeof(UntypedClient), Le(sizeof(iox_client_storage_t)));
    EXPECT_THAT(alignof(UntypedClient), Le(alignof(iox_client_storage_t)));
}

TEST(iox_types_test, ServerStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "ccebf221-8f51-4f55-a94e-9348174a37d0");
    EXPECT_THAT(sizeof(UntypedServer), Le(sizeof(iox_server_storage_t)));
    EXPECT_THAT(alignof(UntypedServer), Le(alignof(iox_server_storage_t)));
}

TEST(iox_types_test, ServiceDiscoveryStorageSizeFits)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fd3af84-ca73-4e38-a061-5e4638f48b77");
    EXPECT_THAT(sizeof(ServiceDiscovery), Le(sizeof(iox_service_discovery_storage_t)));
    EXPECT_THAT(alignof(ServiceDiscovery), Le(alignof(iox_service_discovery_storage_t)));
}
} // namespace
