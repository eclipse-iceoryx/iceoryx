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

#ifndef IOX_BINDING_C_RUNNABLE_H
#define IOX_BINDING_C_RUNNABLE_H

#include "iceoryx_binding_c/internal/c2cpp_binding.h"

typedef CLASS RunnableData* iox_runnable_t;

/// @brief creates a runnable in the shared memory
/// @param[in] runnableName name of the runnable
/// @return handle to the runnable
iox_runnable_t iox_runnable_create(const char* const runnableName);

/// @brief removes a runnable from the shared memory
/// @param[in] self handle to the runnable
void iox_runnable_destroy(iox_runnable_t const self);

/// @brief acquires the name of the runnable
/// @param[in] self handle to the runnable
/// @param[in] name pointer to a memory location where the name can be written to
/// @param[in] nameCapacity size of the memory location where the name is written to
/// @return the actual length of the runnable name, if the return value is greater
///         then nameCapacity the name is truncated
uint64_t iox_runnable_get_name(iox_runnable_t const self, char* const name, const uint64_t nameCapacity);

/// @brief acquires the name of the process in which the runnable is stored
/// @param[in] self handle to the runnable
/// @param[in] name pointer to a memory location where the name can be written to
/// @param[in] nameCapacity size of the memory location where the name is written to
/// @return the actual length of the process name, if the return value is greater
///         then nameCapacity the name is truncated
uint64_t iox_runnable_get_process_name(iox_runnable_t const self, char* const name, const uint64_t nameCapacity);

#endif
