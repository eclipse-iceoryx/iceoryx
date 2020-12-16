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
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/trigger_info.h"
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

/// @brief waits until a trigger was triggered or the timeout was reached
/// @param[in] self handle to the wait set
/// @param[in] timeout duration how long this method should wait
/// @param[in] triggerArray preallocated memory to an array of iox_cond_t in which
///             the triggers which were triggered can be written to
/// @param[in] triggerArrayCapacity the capacity of the preallocated triggerArray
/// @param[in] missedElements if the triggerArray has insufficient size the number of missed elements
///             which could not be written into the array are stored here
/// @return number of elements which were written into the triggerArray
uint64_t iox_ws_timed_wait(iox_ws_t const self,
                           struct timespec timeout,
                           iox_trigger_info_t const triggerArray,
                           const uint64_t triggerArrayCapacity,
                           uint64_t* missedElements);

/// @brief waits until a trigger was triggered
/// @param[in] self handle to the wait set
/// @param[in] triggerArray preallocated memory to an array of iox_cond_t in which
///             the triggers which were triggered can be written to
/// @param[in] triggerArrayCapacity the capacity of the preallocated triggerArray
/// @param[in] missedElements if the triggerArray has insufficient size the number of missed elements
///             which could not be written into the array are stored here
/// @return number of elements which were written into the triggerArray
uint64_t iox_ws_wait(iox_ws_t const self,
                     iox_trigger_info_t const triggerArray,
                     const uint64_t triggerArrayCapacity,
                     uint64_t* missedElements);

/// @brief returns the number of acquired triggers
uint64_t iox_ws_size(iox_ws_t const self);

/// @brief returns the maximum amount of triggers which can be acquired from the waitset
uint64_t iox_ws_trigger_capacity(iox_ws_t const self);

#endif
