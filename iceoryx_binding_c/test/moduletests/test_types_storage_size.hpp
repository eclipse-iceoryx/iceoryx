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

#include <iostream>

#define IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(CPP_TYPE, C_STORAGE)                                            \
    EXPECT_THAT(sizeof(C_STORAGE), Eq(sizeof(CPP_TYPE)));                                                              \
    EXPECT_THAT(alignof(C_STORAGE), Eq(alignof(CPP_TYPE)));

inline void checkIceoryxBindingCStorageSizes()
{
    using namespace ::testing;

    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(iox::popo::WaitSet<>, iox_ws_storage_t);
    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(iox::popo::Listener, iox_listener_storage_t);
    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(iox::popo::UserTrigger, iox_user_trigger_storage_t);
    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(cpp2c_Subscriber, iox_sub_storage_t);
    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(cpp2c_Publisher, iox_pub_storage_t);
    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(iox::popo::UntypedClient, iox_client_storage_t);
    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(iox::popo::UntypedServer, iox_server_storage_t);
    IOX_BINDING_C_CHECK_STORAGE_SIZE_AND_ALIGNMENT(iox::runtime::ServiceDiscovery, iox_service_discovery_storage_t);
}


#endif // IOX_TEST_TYPES_STORAGE_SIZE_HPP
