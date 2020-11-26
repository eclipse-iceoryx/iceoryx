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

#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/popo/trigger_state.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/trigger_state.h"
}

#include <type_traits>

uint64_t iox_trigger_state_get_trigger_id(iox_trigger_state_t const self)
{
    return self->getTriggerId();
}

bool iox_trigger_state_does_originate_from_subscriber(iox_trigger_state_t const self, iox_sub_t const subscriber)
{
    return self->doesOriginateFrom(subscriber);
}

bool iox_trigger_state_does_originate_from_user_trigger(iox_trigger_state_t const self,
                                                        iox_user_trigger_t const user_trigger)
{
    return self->doesOriginateFrom(user_trigger);
}

iox_sub_t iox_trigger_state_get_subscriber_origin(iox_trigger_state_t const self)
{
    return self->getOrigin<cpp2c_Subscriber>();
}

iox_user_trigger_t iox_trigger_state_get_user_trigger_origin(iox_trigger_state_t const self)
{
    return self->getOrigin<UserTrigger>();
}

void iox_trigger_state_call(iox_trigger_state_t const self)
{
    (*self)();
}

