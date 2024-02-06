// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/popo/notification_info.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iox/assertions.hpp"

using namespace iox;
using namespace iox::popo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/notification_info.h"
}

uint64_t iox_notification_info_get_notification_id(iox_notification_info_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->getNotificationId();
}

bool iox_notification_info_does_originate_from_subscriber(iox_notification_info_t const self,
                                                          iox_sub_t const subscriber)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");
    return self->doesOriginateFrom(subscriber);
}

bool iox_notification_info_does_originate_from_user_trigger(iox_notification_info_t const self,
                                                            iox_user_trigger_t const user_trigger)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(user_trigger != nullptr, "'user_trigger' must not be a 'nullptr'");
    return self->doesOriginateFrom(user_trigger);
}

bool iox_notification_info_does_originate_from_client(iox_notification_info_t const self, iox_client_t const client)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");
    return self->doesOriginateFrom(client);
}

bool iox_notification_info_does_originate_from_server(iox_notification_info_t const self, iox_server_t const server)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");
    return self->doesOriginateFrom(server);
}

bool iox_notification_info_does_originate_from_service_discovery(iox_notification_info_t const self,
                                                                 iox_service_discovery_t const serviceDiscovery)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceDiscovery != nullptr, "'serviceDiscovery' must not be a 'nullptr'");
    return self->doesOriginateFrom(serviceDiscovery);
}

iox_sub_t iox_notification_info_get_subscriber_origin(iox_notification_info_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->getOrigin<cpp2c_Subscriber>();
}

iox_user_trigger_t iox_notification_info_get_user_trigger_origin(iox_notification_info_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->getOrigin<UserTrigger>();
}

iox_client_t iox_notification_info_get_client_origin(iox_notification_info_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->getOrigin<UntypedClient>();
}

iox_server_t iox_notification_info_get_server_origin(iox_notification_info_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->getOrigin<UntypedServer>();
}

iox_service_discovery_t iox_notification_info_get_service_discovery_origin(iox_notification_info_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->getOrigin<ServiceDiscovery>();
}

void iox_notification_info_call(iox_notification_info_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    (*self)();
}
