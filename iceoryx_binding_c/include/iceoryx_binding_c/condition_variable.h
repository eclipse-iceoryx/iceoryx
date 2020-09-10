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

#ifndef IOX_BINDING_C_CONDITION_VARIABLE_H
#define IOX_BINDING_C_CONDITION_VARIABLE_H

/// @brief condition variable handle
typedef struct ConditionVariableData* cond_var_t;

cond_var_t iox_cond_var_create();
void iox_cond_var_destroy(cond_var_t const self);

#endif
