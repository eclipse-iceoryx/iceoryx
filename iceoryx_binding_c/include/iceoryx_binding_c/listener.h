// Copyright (c) 2021 Apex.AI Inc. All rights reserved.
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

#ifndef IOX_BINDING_C_LISTENER_H
#define IOX_BINDING_C_LISTENER_H

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"

typedef CLASS Listener* iox_listener_t;

iox_listener_t iox_listener_init(iox_listener_storage_t* self);

void iox_listener_deinit(iox_listener_t const self);

ENUM iox_ListenerResult iox_listener_attach_subscriber_event(iox_listener_t const self,
                                                             iox_sub_t const subscriber,
                                                             const ENUM iox_SubscriberEvent subscriberEvent,
                                                             void (*callback)(iox_sub_t));
ENUM iox_ListenerResult iox_listener_attach_user_trigger_event(iox_listener_t const self,
                                                               iox_user_trigger_t const userTrigger,
                                                               void (*callback)(iox_user_trigger_t));

void iox_listener_detach_subscriber_event(iox_listener_t const self,
                                          iox_sub_t const subscriber,
                                          const ENUM iox_SubscriberEvent subscriberEvent);
void iox_listener_detach_user_trigger_event(iox_listener_t const self, iox_user_trigger_t const userTrigger);

#endif
