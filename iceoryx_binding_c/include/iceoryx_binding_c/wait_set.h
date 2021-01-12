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

#ifndef IOX_BINDING_C_WAIT_SET_H
#define IOX_BINDING_C_WAIT_SET_H

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/event_info.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"

#include <time.h>

/// @brief wait set handle
typedef CLASS cpp2c_WaitSet* iox_ws_t;

/// @brief initialize wait set handle
/// @param[in] self pointer to preallocated memory of size = sizeof(iox_ws_storage_t)
/// @return handle to wait set
iox_ws_t iox_ws_init(iox_ws_storage_t* self);

/// @brief deinitialize wait set handle
/// @param[in] self the handle which should be deinitialized
void iox_ws_deinit(iox_ws_t const self);

/// @brief waits until an event occurred or the timeout was reached
/// @param[in] self handle to the wait set
/// @param[in] timeout duration how long this method should wait
/// @param[in] eventArray preallocated memory to an array of iox_event_info_t in which
///             the event infos, which are describing the occurred event, can be written to
/// @param[in] eventInfoArrayCapacity the capacity of the preallocated eventInfoArray
/// @param[in] missedElements if the eventInfoArray has insufficient size the number of missed elements
///             which could not be written into the array are stored here
/// @return number of elements which were written into the eventInfoArray
uint64_t iox_ws_timed_wait(iox_ws_t const self,
                           struct timespec timeout,
                           iox_event_info_t* const eventInfoArray,
                           const uint64_t eventInfoArrayCapacity,
                           uint64_t* missedElements);

/// @brief waits until an event occurred
/// @param[in] self handle to the wait set
/// @param[in] eventInfoArray preallocated memory to an array of iox_event_info_t in which
///             the event infos, which are describing the occurred event, can be written to
/// @param[in] eventInfoArrayCapacity the capacity of the preallocated eventInfoArray
/// @param[in] missedElements if the eventInfoArray has insufficient size the number of missed elements
///             which could not be written into the array are stored here
/// @return number of elements which were written into the eventInfoArray
uint64_t iox_ws_wait(iox_ws_t const self,
                     iox_event_info_t* const eventInfoArray,
                     const uint64_t eventInfoArrayCapacity,
                     uint64_t* missedElements);

/// @brief returns the number of registered events
uint64_t iox_ws_size(iox_ws_t const self);

/// @brief returns the maximum amount of events which can be registered at the waitset
uint64_t iox_ws_capacity(iox_ws_t const self);

/// @brief attaches a subscriber event to a waitset
/// @param[in] self handle to the waitset
/// @param[in] subscriber the subscriber of the event which should be attached
/// @param[in] subscriberEvent the event which should be attached
/// @param[in] eventId an arbitrary id which will be tagged to the event
/// @param[in] callback a callback which is attached to the event
/// @return if the attaching was successfull it returns WaitSetResult_SUCCESS, otherwise
///             an enum which describes the error
ENUM iox_WaitSetResult iox_ws_attach_subscriber_event(iox_ws_t const self,
                                                      iox_sub_t const subscriber,
                                                      const ENUM iox_SubscriberEvent subscriberEvent,
                                                      const uint64_t eventId,
                                                      void (*callback)(iox_sub_t));

/// @brief attaches a user trigger event to a waitset
/// @param[in] self handle to the waitset
/// @param[in] userTrigger the user trigger of the event which should be attached
/// @param[in] eventId an arbitrary id which will be tagged to the event
/// @param[in] callback a callback which is attached to the event
/// @return if the attaching was successfull it returns WaitSetResult_SUCCESS, otherwise
///             an enum which describes the error
ENUM iox_WaitSetResult iox_ws_attach_user_trigger_event(iox_ws_t const self,
                                                        iox_user_trigger_t const userTrigger,
                                                        const uint64_t eventId,
                                                        void (*callback)(iox_user_trigger_t));

/// @brief detaches a subscriber event from a waitset
/// @param[in] self handle to the waitset
/// @param[in] subscriber the subscriber from which the event should be detached
/// @param[in] subscriberEvent the event which should be detached from the subscriber
void iox_ws_detach_subscriber_event(iox_ws_t const self,
                                    iox_sub_t const subscriber,
                                    const ENUM iox_SubscriberEvent subscriberEvent);

/// @brief detaches a user trigger event from a waitset
/// @param[in] self handle to the waitset
/// @param[in] usertrigger the user trigger which should be detached
void iox_ws_detach_user_trigger_event(iox_ws_t const self, iox_user_trigger_t const userTrigger);

#endif
