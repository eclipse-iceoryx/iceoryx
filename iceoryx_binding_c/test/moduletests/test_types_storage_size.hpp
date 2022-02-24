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

inline void assertIceoryxBindingCStorageSizes()
{
    static_assert(sizeof(iox::popo::WaitSet<>) == sizeof(iox_ws_storage_t), "iox_ws_storage_t size mismatch");
    static_assert(alignof(iox::popo::WaitSet<>) == alignof(iox_ws_storage_t), "iox_ws_storage_t alignment mismatch");

    static_assert(sizeof(iox::popo::Listener) == sizeof(iox_listener_storage_t),
                  "iox_listener_storage_t storage size mismatch");
    static_assert(alignof(iox::popo::Listener) == alignof(iox_listener_storage_t),
                  "iox_listener_storage_t alignment mismatch");

    static_assert(sizeof(iox::popo::UserTrigger) == sizeof(iox_user_trigger_storage_t),
                  "iox_user_trigger_storage_t storage size mismatch");
    static_assert(alignof(iox::popo::UserTrigger) == alignof(iox_user_trigger_storage_t),
                  "iox_user_trigger_storage_t alignment mismatch");

    static_assert(sizeof(cpp2c_Subscriber) == sizeof(iox_sub_storage_t), "iox_sub_storage_t storage size mismatch");
    static_assert(alignof(cpp2c_Subscriber) == alignof(iox_sub_storage_t), "iox_sub_storage_t alignment mismatch");

    static_assert(sizeof(cpp2c_Publisher) == sizeof(iox_pub_storage_t), "iox_pub_storage_t storage size mismatch");
    static_assert(alignof(cpp2c_Publisher) == alignof(iox_pub_storage_t), "iox_pub_storage_t alignment mismatch");

    static_assert(sizeof(iox::popo::UntypedClient) == sizeof(iox_client_storage_t),
                  "iox_client_storage_t storage size mismatch");
    static_assert(alignof(iox::popo::UntypedClient) == alignof(iox_client_storage_t),
                  "iox_client_storage_t alignment mismatch");

    static_assert(sizeof(iox::popo::UntypedServer) == sizeof(iox_server_storage_t),
                  "iox_server_storage_t storage size mismatch");
    static_assert(alignof(iox::popo::UntypedServer) == alignof(iox_server_storage_t),
                  "iox_server_storage_t alignment mismatch");

    static_assert(sizeof(iox::runtime::ServiceDiscovery) == sizeof(iox_service_discovery_storage_t),
                  "iox_service_discovery_storage_t storage size mismatch");
    static_assert(alignof(iox::runtime::ServiceDiscovery) == alignof(iox_service_discovery_storage_t),
                  "iox_service_discovery_storage_t alignment mismatch");
}

#endif // IOX_TEST_TYPES_STORAGE_SIZE_HPP
