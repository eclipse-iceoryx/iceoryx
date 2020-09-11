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
#include "iceoryx_binding_c/condition_variable.h"
#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/subscriber.h"

/// @brief wait set handle
typedef struct WaitSet* wait_set_t;

void iox_wait_set_init(wait_set_t const self, iox_cond_var_t const conditionVariable);
void iox_wait_set_deinit(wait_set_t const self);

ENUM iox_WaitSetResult iox_wait_set_attach_condition(wait_set_t const self, iox_cond_t const condition);

bool iox_wait_set_detach_condition(wait_set_t const self, iox_cond_t const condition);

void iox_wait_set_detach_all_conditions(wait_set_t const self);

void iox_wait_set_timed_wait(wait_set_t const self,
                             struct timespec timeout,
                             iox_cond_t* const conditionArray,
                             const uint64_t conditionArrayCapacity,
                             uint64_t& conditionArraySize,
                             uint64_t& missedElements);

void iox_wait_set_wait(wait_set_t const self,
                       iox_cond_t* const conditionArray,
                       const uint64_t conditionArrayCapacity,
                       uint64_t& conditionArraySize,
                       uint64_t& missedElements);


#endif
