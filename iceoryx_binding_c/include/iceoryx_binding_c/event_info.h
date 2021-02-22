// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_BINDING_C_EVENT_INFO_H
#define IOX_BINDING_C_EVENT_INFO_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/user_trigger.h"

/// @brief event info handle
typedef const CLASS EventInfo* iox_event_info_t;

/// @brief returns the id of the event
/// @param[in] self handle to event info
/// @return eventId
uint64_t iox_event_info_get_event_id(iox_event_info_t const self);

/// @brief does the event originate from a certain subscriber
/// @param[in] self handle to event info
/// @param[in] subscriber handle to the subscriber in question
/// @return true if the event originates from the subscriber, otherwise false
bool iox_event_info_does_originate_from_subscriber(iox_event_info_t const self, iox_sub_t const subscriber);

/// @brief does the event originate from a certain user trigger
/// @param[in] self handle to event info
/// @param[in] user_trigger handle to the user trigger in question
/// @return true if the event originates from the user trigger, otherwise false
bool iox_event_info_does_originate_from_user_trigger(iox_event_info_t const self,
                                                     iox_user_trigger_t const user_trigger);

/// @brief acquires the handle of the subscriber origin
/// @param[in] self handle to event info
/// @return the handle to the subscriber if the event originated from a subscriber, otherwise NULL
iox_sub_t iox_event_info_get_subscriber_origin(iox_event_info_t const self);

/// @brief acquires the handle of the user trigger origin
/// @param[in] self handle to event info
/// @return the handle to the user trigger if the event originated from a user trigger, otherwise NULL
iox_user_trigger_t iox_event_info_get_user_trigger_origin(iox_event_info_t const self);

/// @brief calls the callback of the event
/// @param[in] self handle to event info
void iox_event_info_call(iox_event_info_t const self);

#endif
