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

#ifndef IOX_BINDING_C_WAIT_SET_H
#define IOX_BINDING_C_WAIT_SET_H

#include "iceoryx_binding_c/condition.h"
#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"

#include <time.h>

/// @brief wait set handle
typedef struct WaitSet* iox_wait_set_t;

/// @brief initialize wait set handle
/// @param[in] self pointer to preallocated memory of size = sizeof(iox_wait_set_storage_t)
/// @return handle to wait set
iox_wait_set_t iox_wait_set_init(iox_wait_set_storage_t* self);

/// @brief deinitialize wait set handle
/// @param[in] self the handle which should be deinitialized
void iox_wait_set_deinit(iox_wait_set_t const self);

/// @brief Attach a condition to your wait set. If you would like to destroy the condition
///         you have to detach it first from the wait set.
/// @param[in] self handle to the wait set
/// @param[in] condition the condition you would like to attach
/// @return if the attachment was successful it returns WaitSetResult_SUCCESS, otherwise
///          an enum which describes the error
ENUM iox_WaitSetResult iox_wait_set_attach_condition(iox_wait_set_t const self, iox_cond_t const condition);

/// @brief detaches a condition.
/// @param[in] self handle to the wait set
/// @param[in] condition the condition you would like to detach
/// @return true if the condition could be detached, otherwise false for instance when the
///         condition was not attached before
bool iox_wait_set_detach_condition(iox_wait_set_t const self, iox_cond_t const condition);

/// @brief detaches all conditions
/// @param[in] self handle to the wait set
void iox_wait_set_detach_all_conditions(iox_wait_set_t const self);

/// @brief waits until a condition was triggered or the timeout was reached
/// @param[in] self handle to the wait set
/// @param[in] timeout duration how long this method should wait
/// @param[in] conditionArray preallocated memory to an array of iox_cond_t in which
///             the conditions which were triggered can be written to
/// @param[in] conditionArrayCapacity the capacity of the preallocated conditionArray
/// @param[in] missedElements if the conditionArray has insufficient size the number of missed elements
///             which could not be written into the array are stored here
/// @return number of elements which were written into the conditionArray
uint64_t iox_wait_set_timed_wait(iox_wait_set_t const self,
                                 struct timespec timeout,
                                 iox_cond_t* const conditionArray,
                                 const uint64_t conditionArrayCapacity,
                                 uint64_t* missedElements);

/// @brief waits until a condition was triggered
/// @param[in] self handle to the wait set
/// @param[in] conditionArray preallocated memory to an array of iox_cond_t in which
///             the conditions which were triggered can be written to
/// @param[in] conditionArrayCapacity the capacity of the preallocated conditionArray
/// @param[in] missedElements if the conditionArray has insufficient size the number of missed elements
///             which could not be written into the array are stored here
/// @return number of elements which were written into the conditionArray
uint64_t iox_wait_set_wait(iox_wait_set_t const self,
                           iox_cond_t* const conditionArray,
                           const uint64_t conditionArrayCapacity,
                           uint64_t* missedElements);


#endif
