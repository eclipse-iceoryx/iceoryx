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

#ifndef IOX_BINDING_C_TRIGGER_STATE_H
#define IOX_BINDING_C_TRIGGER_STATE_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/user_trigger.h"

/// @brief trigger state handle
typedef CLASS TriggerState* iox_trigger_state_t;

/// @brief returns the id of the trigger
/// @param[in] self handle to trigger state
/// @return triggerId
uint64_t iox_trigger_state_get_trigger_id(iox_trigger_state_t const self);

/// @brief does the trigger originate from a certain subscriber
/// @param[in] self handle to trigger state
/// @param[in] subscriber handle to the subscriber in question
/// @return true if the trigger originates from the subscriber, otherwise false
bool iox_trigger_state_does_originate_from_subscriber(iox_trigger_state_t const self, iox_sub_t const subscriber);

/// @brief does the trigger originate from a certain user trigger
/// @param[in] self handle to trigger state
/// @param[in] user_trigger handle to the user trigger in question
/// @return true if the trigger originates from the user trigger, otherwise false
bool iox_trigger_state_does_originate_from_user_trigger(iox_trigger_state_t const self,
                                                        iox_user_trigger_t const user_trigger);

/// @brief acquires the handle of the subscriber origin
/// @param[in] self handle to trigger state
/// @return the handle to the subscriber if the trigger originated from a subscriber, otherwise NULL
iox_sub_t iox_trigger_state_get_subscriber_origin(iox_trigger_state_t const self);

/// @brief acquires the handle of the user trigger origin
/// @param[in] self handle to trigger state
/// @return the handle to the user trigger if the trigger originated from a user trigger, otherwise NULL
iox_user_trigger_t iox_trigger_state_get_user_trigger_origin(iox_trigger_state_t const self);

/// @brief calls the callback of the trigger
/// @param[in] self handle to trigger state
void iox_trigger_state_call(iox_trigger_state_t const self);

#endif
