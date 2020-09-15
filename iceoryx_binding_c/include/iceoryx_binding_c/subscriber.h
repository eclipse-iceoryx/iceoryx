// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_BINDING_C_SUBSCRIBER_H
#define IOX_BINDING_C_SUBSCRIBER_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/types.h"

/// @brief creates a subscriber handle in the default runtime runnable
/// @param[in] service serviceString
/// @param[in] instance instanceString
/// @param[in] event eventString
/// @param[in] historyCapacity size of the history chunk queue
/// @return handle of the subscriber
CLASS SubscriberPortData*
iox_sub_create(const char* const service, const char* const instance, const char* const event, uint64_t historyRequest);

/// @brief removes a subscriber handle
/// @param[in] self the handle which should be removed
void iox_sub_destroy(CLASS SubscriberPortData* const self);

/// @brief subscribes to the service
/// @param[in] self handle to the subscriber
/// @param[in] queueCapacity size of the receiver queue
void iox_sub_subscribe(CLASS SubscriberPortData* const self, const uint64_t queueCapacity);

/// @brief unsubscribes from a service
/// @param[in] self handle to the subscriber
void iox_sub_unsubscribe(CLASS SubscriberPortData* const self);

/// @brief what is the subscription state?
/// @param[in] self handle to the subscriber
/// @return SubscribeState_SUBSCRIBED when successfully subscribed otherwise an enum which
///         describes the current state
ENUM iox_SubscribeState iox_sub_get_subscription_state(CLASS SubscriberPortData* const self);

/// @brief retrieve a received chunk
/// @param[in] self handle to the subscriber
/// @param[in] chunk pointer in which the pointer to the chunk is stored
/// @return if a chunk could be received it returns ChunkReceiveResult_SUCCESS otherwise
///         an enum which describes the error
ENUM iox_popo_ChunkReceiveResult iox_sub_get_chunk(CLASS SubscriberPortData* const self, const void** const chunk);

/// @brief release a previously acquired chunk (via iox_sub_getChunk)
/// @param[in] self handle to the subscriber
/// @param[in] chunk pointer to the chunk which should be released
void iox_sub_release_chunk(CLASS SubscriberPortData* const self, const void* const chunk);

/// @brief release all chunks which are stored in the chunk queue
/// @param[in] self handle to the subscriber
void iox_sub_release_queued_chunks(CLASS SubscriberPortData* const self);

/// @brief are new chunks available?
/// @param[in] self handle to the subscriber
/// @return true if there are new chunks otherwise false
bool iox_sub_has_new_chunks(CLASS SubscriberPortData* const self);

/// @brief are chunks lost?
/// @param[in] self handle to the subscriber
/// @return true if there are lost chunks otherwise false
bool iox_sub_has_lost_chunks(CLASS SubscriberPortData* const self);

bool iox_sub_attach_condition_variable(CLASS SubscriberPortData* const self);
bool iox_sub_detach_condition_variable(CLASS SubscriberPortData* const self);
bool iox_sub_is_condition_variable_attached(CLASS SubscriberPortData* const self,
                                            CLASS ConditionVariableData* const cvHandle);

#endif
