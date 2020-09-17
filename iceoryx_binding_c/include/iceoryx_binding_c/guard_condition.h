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

#ifndef IOX_BINDING_C_GUARD_CONDITION_H
#define IOX_BINDING_C_GUARD_CONDITION_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/types.h"

/// @brief guard condition handle
typedef CLASS GuardCondition* iox_guard_cond_t;

/// @brief initialize guard condition handle
/// @param[in] self pointer to preallocated memory of size = sizeof(iox_guard_cond_storage_t)
/// @return handle to guard condition
iox_guard_cond_t iox_guard_cond_init(iox_guard_cond_storage_t* self);

/// @brief deinitialize guard condition handle
/// @param[in] self handle to guard condition
void iox_guard_cond_deinit(iox_guard_cond_t const self);

/// @brief trigger a guard condition
/// @param[in] self handle to guard condition
void iox_guard_cond_trigger(iox_guard_cond_t const self);

/// @brief was the guard condition triggered
/// @param[in] self handle to guard condition
/// @return returns true if the guard condition was triggered, otherwise false
bool iox_guard_cond_has_triggered(iox_guard_cond_t const self);

/// @brief resets the guard condition triggering state. after that call
///         iox_guard_cond_has_triggered will return false until it was
///         triggered again
/// @param[in] self handle to guard condition
void iox_guard_cond_reset_trigger(iox_guard_cond_t const self);

#endif
