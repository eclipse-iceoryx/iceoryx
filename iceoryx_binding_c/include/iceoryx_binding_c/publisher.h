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

/// @brief creates a publisher handle in the default runtime runnable
/// @param[in] service serviceString
/// @param[in] instance instanceString
/// @param[in] event eventString
/// @param[in] historyCapacity size of the history chunk queue
/// @return handle of the publisher
CLASS PublisherPortData*
iox_pub_create(const char* service, const char* instance, const char* event, const uint64_t historyCapacity);

/// @brief removes a publisher handle
/// @param[in] self the handle which should be removed
void iox_pub_destroy(CLASS PublisherPortData* const self);

/// @brief allocates a chunk in the shared memory
/// @param[in] self handle of the publisher
/// @param[in] chunk pointer in which a pointer to the allocated chunk is stored
/// @param[in] payloadSize size of the allocated chunk
/// @return on success it returns AllocationResult_SUCCESS otherwise a value which
///         describes the error
ENUM iox_popo_AllocationResult iox_pub_allocate_chunk(CLASS PublisherPortData* const self,
                                                      void** const chunk,
                                                      const uint32_t payloadSize);

/// @brief frees a previously allocated chunk without sending it
/// @param[in] self handle of the publisher
/// @param[in] chunk chunk which should be free'd
void iox_pub_free_chunk(CLASS PublisherPortData* const self, void* const chunk);

/// @brief sends a previously allocated chunk
/// @param[in] self handle of the publisher
/// @param[in] chunk chunk which should be send
void iox_pub_send_chunk(CLASS PublisherPortData* const self, void* const chunk);

/// @brief returns the previously sended chunk
/// @param[in] self handle of the publisher
/// @return nullptr if no chunk was previously send otherwise a pointer to the
///           previous chunk
const void* iox_pub_try_get_previous_chunk(CLASS PublisherPortData* const self);

/// @brief offers the service
/// @param[in] self handle of the publisher
void iox_pub_offer(CLASS PublisherPortData* const self);

/// @brief stop offering the service
/// @param[in] self handle of the publisher
void iox_pub_stop_offer(CLASS PublisherPortData* const self);

/// @brief is the service still offered
/// @param[in] self handle of the publisher
/// @return true is the service is offered otherwise false
bool iox_pub_is_offered(CLASS PublisherPortData* const self);

/// @brief does the service have subscribers
/// @param[in] self handle of the publisher
/// @return true if there are subscribers otherwise false
bool iox_pub_has_subscribers(CLASS PublisherPortData* const self);

#endif
