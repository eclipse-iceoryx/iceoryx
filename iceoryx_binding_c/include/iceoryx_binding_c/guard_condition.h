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

/// @brief guard condition handle
typedef struct GuardCondition* guard_cond_t;

void iox_guard_cond_init(guard_cond_t const self);

/// @todo naming?
void iox_guard_cond_deinit(guard_cond_t const self);

void iox_guard_cond_set_trigger(guard_cond_t const self);
bool iox_guard_cond_has_triggered(guard_cond_t const self);
void iox_guard_cond_reset_trigger(guard_cond_t const self);

#endif
