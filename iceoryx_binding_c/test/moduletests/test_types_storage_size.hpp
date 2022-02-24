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


#ifndef IOX_TEST_TYPES_STORAGE_SIZE_HPP
#define IOX_TEST_TYPES_STORAGE_SIZE_HPP

#include "iceoryx_binding_c/internal/cpp2c_publisher.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

extern "C" {
#include "iceoryx_binding_c/types.h"
}

#include "test.hpp"

namespace iox_test_binding_c_types_storage_size
{
using namespace ::testing;

/* ######################################################
 * Please add a test with this function in all test files
 * ######################################################*/

inline void testBindingCTypesStorageSizes()
{
    SCOPED_TRACE("WaitSet storage size");
    EXPECT_THAT(sizeof(iox::popo::WaitSet<>), Le(sizeof(iox_ws_storage_t)));
    EXPECT_THAT(alignof(iox::popo::WaitSet<>), Le(alignof(iox_ws_storage_t)));

    SCOPED_TRACE("Listener storage size");
    EXPECT_THAT(sizeof(iox::popo::Listener), Le(sizeof(iox_listener_storage_t)));
    EXPECT_THAT(alignof(iox::popo::Listener), Le(alignof(iox_listener_storage_t)));

    SCOPED_TRACE("UserTrigger storage size");
    EXPECT_THAT(sizeof(iox::popo::UserTrigger), Le(sizeof(iox_user_trigger_storage_t)));
    EXPECT_THAT(alignof(iox::popo::UserTrigger), Le(alignof(iox_user_trigger_storage_t)));

    SCOPED_TRACE("cpp2c_Subscriber storage size");
    EXPECT_THAT(sizeof(cpp2c_Subscriber), Le(sizeof(iox_sub_storage_t)));
    EXPECT_THAT(alignof(cpp2c_Subscriber), Le(alignof(iox_sub_storage_t)));

    SCOPED_TRACE("cpp2c_Publisher storage size");
    EXPECT_THAT(sizeof(cpp2c_Publisher), Le(sizeof(iox_pub_storage_t)));
    EXPECT_THAT(alignof(cpp2c_Publisher), Le(alignof(iox_pub_storage_t)));

    SCOPED_TRACE("UntypedClient storage size");
    EXPECT_THAT(sizeof(iox::popo::UntypedClient), Le(sizeof(iox_client_storage_t)));
    EXPECT_THAT(alignof(iox::popo::UntypedClient), Le(alignof(iox_client_storage_t)));

    SCOPED_TRACE("UntypedServer storage size");
    EXPECT_THAT(sizeof(iox::popo::UntypedServer), Le(sizeof(iox_server_storage_t)));
    EXPECT_THAT(alignof(iox::popo::UntypedServer), Le(alignof(iox_server_storage_t)));

    SCOPED_TRACE("ServiceDiscovery storage size");
    EXPECT_THAT(sizeof(iox::runtime::ServiceDiscovery), Le(sizeof(iox_service_discovery_storage_t)));
    EXPECT_THAT(alignof(iox::runtime::ServiceDiscovery), Le(alignof(iox_service_discovery_storage_t)));
}

#endif // IOX_TEST_TYPES_STORAGE_SIZE_HPP

} // namespace
