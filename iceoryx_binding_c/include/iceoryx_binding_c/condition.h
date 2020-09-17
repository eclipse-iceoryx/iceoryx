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

#ifndef IOX_BINDING_C_CONDITION_H
#define IOX_BINDING_C_CONDITION_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"

/// @brief condition handle
typedef CLASS Condition* iox_cond_t;

/// @brief Was the condition fulfilled since last call.
/// @param[in] self handle to condition
/// @return returns true if the condition was fulfilled, otherwise false
bool iox_cond_has_triggered(iox_cond_t const self);

/// @brief is a condition variable attached
/// @param[in] self handle to condition
/// @return returns true if the condition is attached, otherwise false
bool iox_cond_is_condition_variable_attached(iox_cond_t const self);

#endif
