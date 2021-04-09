// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_binding_c/internal/cpp2c_waitset.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/wait_set.h"
}

static uint64_t event_info_vector_to_c_array(const WaitSet<>::EventInfoVector& triggerVector,
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

    for (uint64_t i = 0U; i < eventInfoArraySize; ++i)
    {
        eventInfoArray[i] = triggerVector[i];
    }

    return eventInfoArraySize;
}

iox_ws_t iox_ws_init(iox_ws_storage_t* self)
{
    if (self == nullptr)
    {
        LogWarn() << "wait set initialization skipped - null pointer provided for iox_ws_storage_t";
        return nullptr;
    }
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
    return event_info_vector_to_c_array(
        self->timedWait(units::Duration(timeout)), eventInfoArray, eventInfoArrayCapacity, missedElements);
}

uint64_t iox_ws_wait(iox_ws_t const self,
                     iox_event_info_t* const eventInfoArray,
                     const uint64_t eventInfoArrayCapacity,
                     uint64_t* missedElements)
{
    return event_info_vector_to_c_array(self->wait(), eventInfoArray, eventInfoArrayCapacity, missedElements);
}

uint64_t iox_ws_size(iox_ws_t const self)
{
    return self->size();
}

uint64_t iox_ws_capacity(iox_ws_t const self)
{
    return self->capacity();
}

iox_WaitSetResult iox_ws_attach_subscriber_state(iox_ws_t const self,
                                                 iox_sub_t const subscriber,
                                                 const iox_SubscriberState subscriberState,
                                                 const uint64_t eventId,
                                                 void (*callback)(iox_sub_t))
{
    auto result = self->attachState(*subscriber, c2cpp::subscriberState(subscriberState), eventId, callback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.get_error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_subscriber_event(iox_ws_t const self,
                                                 iox_sub_t const subscriber,
                                                 const iox_SubscriberEvent subscriberEvent,
                                                 const uint64_t eventId,
                                                 void (*callback)(iox_sub_t))
{
    auto result = self->attachEvent(*subscriber, c2cpp::subscriberEvent(subscriberEvent), eventId, callback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.get_error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_user_trigger_event(iox_ws_t const self,
                                                   iox_user_trigger_t const userTrigger,
                                                   const uint64_t eventId,
                                                   void (*callback)(iox_user_trigger_t))
{
    auto result = self->attachEvent(*userTrigger, eventId, callback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.get_error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

void iox_ws_detach_subscriber_event(iox_ws_t const self,
                                    iox_sub_t const subscriber,
                                    const iox_SubscriberEvent subscriberEvent)
{
    self->detachEvent(*subscriber, c2cpp::subscriberEvent(subscriberEvent));
}

void iox_ws_detach_subscriber_state(iox_ws_t const self,
                                    iox_sub_t const subscriber,
                                    const iox_SubscriberState subscriberState)
{
    self->detachState(*subscriber, c2cpp::subscriberState(subscriberState));
}

void iox_ws_detach_user_trigger_event(iox_ws_t const self, iox_user_trigger_t const userTrigger)
{
    self->detachEvent(*userTrigger);
}
