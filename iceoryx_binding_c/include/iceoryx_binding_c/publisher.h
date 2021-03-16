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

#ifndef IOX_BINDING_C_PUBLISHER_H
#define IOX_BINDING_C_PUBLISHER_H

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/service_description.h"
#include "iceoryx_binding_c/types.h"

/// @brief publisher handle
typedef struct cpp2c_Publisher* iox_pub_t;

/// @brief options to be set for a publisher
typedef struct
{
    // size of the history chunk queue
    uint64_t historyCapacity;

    // name of the node the publisher belongs to
    // nullptr indicates that the default node name is used
    const char* nodeName;

    // The option whether the publisher should already be offered when creating it
    bool offerOnCreate;

    // this value will be set exclusively by iox_pub_options_init
    // and is not supposed to be modified otherwise
    uint64_t initCheck;
} iox_pub_options_t;

/// @brief initialize publisher options to default values
/// @param[in] options pointer to options to be initialized,
///                    emit warning if it is a null pointer
/// @attention This must always be called on a newly created options struct to
///            prevent uninitialized values. The options may get extended
///            in the future.
void iox_pub_options_init(iox_pub_options_t* options);

/// @brief check whether the publisher options were initialized by iox_pub_options_init
/// @param[in] options pointer to options to be checked
/// @return true if options are not null and were initialized, false otherwise
bool iox_pub_options_is_initialized(const iox_pub_options_t* const options);

/// @brief creates a publisher handle
/// @param[in] self pointer to preallocated memory of size = sizeof(iox_pub_storage_t)
/// @param[in] service serviceString
/// @param[in] instance instanceString
/// @param[in] event eventString
/// @param[in] options publisher options set by the user,
///                    if it is a null pointer default options are used
/// @return handle of the publisher
iox_pub_t iox_pub_init(iox_pub_storage_t* self,
                       const char* const service,
                       const char* const instance,
                       const char* const event,
                       const iox_pub_options_t* const options);

/// @brief removes a publisher handle
/// @param[in] self the handle which should be removed
void iox_pub_deinit(iox_pub_t const self);

/// @brief allocates a chunk in the shared memory
/// @param[in] self handle of the publisher
/// @param[in] chunk pointer in which a pointer to the allocated chunk is stored
/// @param[in] payloadSize size of the allocated chunk
/// @return on success it returns AllocationResult_SUCCESS otherwise a value which
///         describes the error
ENUM iox_AllocationResult iox_pub_loan_chunk(iox_pub_t const self, void** const chunk, const uint32_t payloadSize);

/// @brief releases ownership of a previously allocated chunk without sending it
/// @param[in] self handle of the publisher
/// @param[in] chunk chunk which should be free'd
void iox_pub_release_chunk(iox_pub_t const self, void* const chunk);

/// @brief sends a previously allocated chunk
/// @param[in] self handle of the publisher
/// @param[in] chunk chunk which should be send
void iox_pub_publish_chunk(iox_pub_t const self, void* const chunk);

/// @brief returns the previously sended chunk
/// @param[in] self handle of the publisher
/// @return nullptr if no chunk was previously send otherwise a pointer to the
///           previous chunk
const void* iox_pub_loan_previous_chunk(iox_pub_t const self);

/// @brief offers the service
/// @param[in] self handle of the publisher
void iox_pub_offer(iox_pub_t const self);

/// @brief stop offering the service
/// @param[in] self handle of the publisher
void iox_pub_stop_offer(iox_pub_t const self);

/// @brief is the service still offered
/// @param[in] self handle of the publisher
/// @return true is the service is offered otherwise false
bool iox_pub_is_offered(iox_pub_t const self);

/// @brief does the service have subscribers
/// @param[in] self handle of the publisher
/// @return true if there are subscribers otherwise false
bool iox_pub_has_subscribers(iox_pub_t const self);

/// @brief returns the service description of the publisher
/// @param[in] self handle to the publisher
/// @return the service description
iox_service_description_t iox_pub_get_service_description(iox_pub_t const self);
#endif
