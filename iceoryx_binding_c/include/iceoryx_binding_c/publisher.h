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

#ifndef IOX_BINDING_C_PUBLISHER_H_
#define IOX_BINDING_C_PUBLISHER_H_

#include "iceoryx_binding_c/internal/c2cpp_bridge.h"
#include "iceoryx_binding_c/types.h"

CLASS PublisherPortData* Publisher_new();
void Publisher_delete(CLASS PublisherPortData* const self);
iox_popo_AllocationError
Publisher_allocateChunk(CLASS PublisherPortData* const self, void** const chunk, const uint32_t payloadSize);
void Publisher_freeChunk(CLASS PublisherPortData* const self, void* const chunk);
void Publisher_sendChunk(CLASS PublisherPortData* const self, void* const chunk);
const void* Publisher_getLastChunk(CLASS PublisherPortData* const self);
void Publisher_offer(CLASS PublisherPortData* const self);
void Publisher_stopOffer(CLASS PublisherPortData* const self);
bool Publisher_isOffered(CLASS PublisherPortData* const self);
bool Publisher_hasSubscribers(CLASS PublisherPortData* const self);

#endif
