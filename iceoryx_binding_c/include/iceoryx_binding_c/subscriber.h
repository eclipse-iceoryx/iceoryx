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

#ifndef IOX_BINDING_C_SUBSCRIBER_H_
#define IOX_BINDING_C_SUBSCRIBER_H_

#include "iceoryx_binding_c/internal/c2cpp_bridge.h"
#include "iceoryx_binding_c/types.h"

CLASS SubscriberPortData* subscriber_new();
void subscriber_delete(CLASS SubscriberPortData* const self);
void subscriber_subscribe(CLASS SubscriberPortData* const self, const uint64_t queueCapacity);
void subscriber_unsubscribe(CLASS SubscriberPortData* const self);
ENUM subscriber_SubscriptionState subscriber_getSubscriptionState(CLASS SubscriberPortData* const self);
ENUM subscriber_AllocateError subscriber_getChunk(CLASS SubscriberPortData* const self,
                                                  const CLASS ChunkHeader** const);
void subscriber_releaseChunk(CLASS SubscriberPortData* const self, const CLASS ChunkHeader* const);
void subscriber_releaseQueuedChunks(CLASS SubscriberPortData* const self);
bool subscriber_hasNewChunks(CLASS SubscriberPortData* const self);
bool subscriber_hasLostChunks(CLASS SubscriberPortData* const self);
void subscriber_attachConditionVariable(CLASS SubscriberPortData* const self);
void subscriber_detachConditionVariable(CLASS SubscriberPortData* const self);
bool subscriber_isConditionVariableAttached(CLASS SubscriberPortData* const self);

#endif

