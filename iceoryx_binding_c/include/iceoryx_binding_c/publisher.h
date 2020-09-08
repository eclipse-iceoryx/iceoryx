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

#ifndef IOX_BINDING_C_PUBLISHER_H
#define IOX_BINDING_C_PUBLISHER_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/types.h"

CLASS PublisherPortData*
iox_pub_create(const char* service, const char* instance, const char* event, const uint64_t historyCapacity);
void iox_pub_destroy(CLASS PublisherPortData* const self);
ENUM iox_popo_AllocationResult iox_pub_allocateChunk(CLASS PublisherPortData* const self,
                                                     void** const chunk,
                                                     const uint32_t payloadSize);
void iox_pub_freeChunk(CLASS PublisherPortData* const self, void* const chunk);
void iox_pub_sendChunk(CLASS PublisherPortData* const self, void* const chunk);
const void* iox_pub_tryGetPreviousChunk(CLASS PublisherPortData* const self);
void iox_pub_offer(CLASS PublisherPortData* const self);
void iox_pub_stopOffer(CLASS PublisherPortData* const self);
bool iox_pub_isOffered(CLASS PublisherPortData* const self);
bool iox_pub_hasSubscribers(CLASS PublisherPortData* const self);

#endif
