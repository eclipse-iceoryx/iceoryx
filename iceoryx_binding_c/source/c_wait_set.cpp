// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_waitset.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/wait_set.h"
}

static uint64_t trigger_vector_to_c_array(const WaitSet<>::EventInfoVector& triggerVector,
                                          iox_event_info_t* eventInfoArray,
                                          const uint64_t eventInfoArrayCapacity,
                                          uint64_t* missedElements)
{
    uint64_t eventInfoArraySize = 0U;
    uint64_t triggerVectorSize = triggerVector.size();
    if (triggerVectorSize > eventInfoArrayCapacity)
    {
        *missedElements = triggerVectorSize - eventInfoArrayCapacity;
        eventInfoArraySize = eventInfoArrayCapacity;
    }
    else
    {
        *missedElements = 0U;
        eventInfoArraySize = triggerVectorSize;
    }

    for (uint64_t i = 0; i < eventInfoArraySize; ++i)
    {
        eventInfoArray[i] = triggerVector[i];
    }

    return eventInfoArraySize;
}

iox_ws_t iox_ws_init(iox_ws_storage_t* self)
{
    new (self) cpp2c_WaitSet();
    return reinterpret_cast<iox_ws_t>(self);
}

void iox_ws_deinit(iox_ws_t const self)
{
    self->~cpp2c_WaitSet();
}

uint64_t iox_ws_timed_wait(iox_ws_t const self,
                           struct timespec timeout,
                           iox_event_info_t* const eventInfoArray,
                           const uint64_t eventInfoArrayCapacity,
                           uint64_t* missedElements)
{
    return trigger_vector_to_c_array(
        self->timedWait(units::Duration::nanoseconds(static_cast<unsigned long long int>(timeout.tv_nsec))
                        + units::Duration::seconds(static_cast<unsigned long long int>(timeout.tv_sec))),
        eventInfoArray,
        eventInfoArrayCapacity,
        missedElements);
}

uint64_t iox_ws_wait(iox_ws_t const self,
                     iox_event_info_t* const eventInfoArray,
                     const uint64_t eventInfoArrayCapacity,
                     uint64_t* missedElements)
{
    return trigger_vector_to_c_array(self->wait(), eventInfoArray, eventInfoArrayCapacity, missedElements);
}

uint64_t iox_ws_size(iox_ws_t const self)
{
    return self->size();
}

uint64_t iox_ws_capacity(iox_ws_t const self)
{
    return self->capacity();
}
