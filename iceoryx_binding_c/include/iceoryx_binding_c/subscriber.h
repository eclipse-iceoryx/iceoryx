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

CLASS SubscriberPortData*
iox_sub_create(const char* const service, const char* const instance, const char* const event, uint64_t historyRequest);
void iox_sub_destroy(CLASS SubscriberPortData* const self);
void iox_sub_subscribe(CLASS SubscriberPortData* const self, const uint64_t queueCapacity);
void iox_sub_unsubscribe(CLASS SubscriberPortData* const self);
ENUM iox_SubscribeState iox_sub_getSubscriptionState(CLASS SubscriberPortData* const self);
ENUM iox_popo_ChunkReceiveResult iox_sub_getChunk(CLASS SubscriberPortData* const self, const void** const);
void iox_sub_releaseChunk(CLASS SubscriberPortData* const self, const void* const);
void iox_sub_releaseQueuedChunks(CLASS SubscriberPortData* const self);
bool iox_sub_hasNewChunks(CLASS SubscriberPortData* const self);
bool iox_sub_hasLostChunks(CLASS SubscriberPortData* const self);
bool iox_sub_attachConditionVariable(CLASS SubscriberPortData* const self);
bool iox_sub_detachConditionVariable(CLASS SubscriberPortData* const self);
bool iox_sub_isConditionVariableAttached(CLASS SubscriberPortData* const self,
                                         CLASS ConditionVariableData* const cvHandle);

#endif

