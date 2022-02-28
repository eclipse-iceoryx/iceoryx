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

inline void verifyStorageSizeCalculationForListener()
{
    using namespace ::testing;
    using namespace ::iox::popo;

    EXPECT_THAT(sizeof(ListenerImpl<1>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(1)));
    EXPECT_THAT(sizeof(ListenerImpl<2>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(2)));
    EXPECT_THAT(sizeof(ListenerImpl<3>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(3)));
    EXPECT_THAT(sizeof(ListenerImpl<4>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(4)));
    EXPECT_THAT(sizeof(ListenerImpl<5>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(5)));
    EXPECT_THAT(sizeof(ListenerImpl<6>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(6)));
    EXPECT_THAT(sizeof(ListenerImpl<7>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(7)));
    EXPECT_THAT(sizeof(ListenerImpl<8>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(8)));

    EXPECT_THAT(sizeof(ListenerImpl<15>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(15)));
    EXPECT_THAT(sizeof(ListenerImpl<16>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(16)));
    EXPECT_THAT(sizeof(ListenerImpl<17>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(17)));

    EXPECT_THAT(sizeof(ListenerImpl<99>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(99)));
    EXPECT_THAT(sizeof(ListenerImpl<100>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(100)));
    EXPECT_THAT(sizeof(ListenerImpl<101>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(101)));

    EXPECT_THAT(sizeof(ListenerImpl<255>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(255)));
    EXPECT_THAT(sizeof(ListenerImpl<256>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(256)));
    EXPECT_THAT(sizeof(ListenerImpl<257>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(257)));

    EXPECT_THAT(sizeof(ListenerImpl<999>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(999)));
    EXPECT_THAT(sizeof(ListenerImpl<1000>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(1000)));
    EXPECT_THAT(sizeof(ListenerImpl<1001>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(1001)));

    EXPECT_THAT(sizeof(ListenerImpl<2047>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(2047)));
    EXPECT_THAT(sizeof(ListenerImpl<2048>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(2048)));
    EXPECT_THAT(sizeof(ListenerImpl<2049>), Eq(CALCULATE_STORAGE_SIZE_FOR_LISTENER(2049)));
}

inline void verifyStorageSizeCalculationForWaitSet()
{
    using namespace ::testing;
    using namespace iox::popo;

    EXPECT_THAT(sizeof(WaitSet<1>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(1)));
    EXPECT_THAT(sizeof(WaitSet<2>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(2)));
    EXPECT_THAT(sizeof(WaitSet<3>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(3)));
    EXPECT_THAT(sizeof(WaitSet<4>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(4)));
    EXPECT_THAT(sizeof(WaitSet<5>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(5)));
    EXPECT_THAT(sizeof(WaitSet<6>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(6)));
    EXPECT_THAT(sizeof(WaitSet<7>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(7)));
    EXPECT_THAT(sizeof(WaitSet<8>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(8)));

    EXPECT_THAT(sizeof(WaitSet<15>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(15)));
    EXPECT_THAT(sizeof(WaitSet<16>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(16)));
    EXPECT_THAT(sizeof(WaitSet<17>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(17)));

    EXPECT_THAT(sizeof(WaitSet<99>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(99)));
    EXPECT_THAT(sizeof(WaitSet<100>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(100)));
    EXPECT_THAT(sizeof(WaitSet<101>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(101)));

    EXPECT_THAT(sizeof(WaitSet<255>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(255)));
    EXPECT_THAT(sizeof(WaitSet<256>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(256)));
    EXPECT_THAT(sizeof(WaitSet<257>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(257)));

    EXPECT_THAT(sizeof(WaitSet<999>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(999)));
    EXPECT_THAT(sizeof(WaitSet<1000>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(1000)));
    EXPECT_THAT(sizeof(WaitSet<1001>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(1001)));

    EXPECT_THAT(sizeof(WaitSet<2047>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(2047)));
    EXPECT_THAT(sizeof(WaitSet<2048>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(2048)));
    EXPECT_THAT(sizeof(WaitSet<2049>), Eq(CALCULATE_STORAGE_SIZE_FOR_WAITSET(2049)));
}

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
