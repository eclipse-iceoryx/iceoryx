// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_GENERIC_PLATFORM_STDLIB_HPP
#define IOX_HOOFS_GENERIC_PLATFORM_STDLIB_HPP

#include <cstdlib>

/// @brief Implementation of 'getenv_s'
/// @param[out] actual_size_with_null of the value of the env variable including null-termination or 0 if the
/// environment variable does not exist
/// @param[out] buffer to store the value of the env variable
/// @param[in] buffer_capacity of the buffer to store the env variable
/// @param[in] name of the env variable
/// @return 0 on success or the errno value on error
/// @note Conditional thread safe as long as only 'iox_getenv', 'iox_setenv' and 'iox_unsetenv' are used and none of
/// 'getenv', 'setenv', 'unsetenv' and 'putenv' directly
int iox_getenv_s(size_t* actual_size_with_null, char* buffer, size_t buffer_capacity, const char* name);

/// @brief Implementation of 'setenv'
/// @param[in] name of the env variable
/// @param[in] value of the env variable
/// @param[in] overwrite will overwrite an existing env variable if the value is not 0
/// @return 0 on success or -1 on error with the errno set to indicate the error
/// @note Conditional thread safe as long as only 'iox_getenv', 'iox_setenv' and 'iox_unsetenv' are used and none of
/// 'getenv', 'setenv', 'unsetenv' and 'putenv' directly
int iox_setenv(const char* name, const char* value, int overwrite);

/// @brief Implementation of 'unsetenv'
/// @param[in] name of the env variable
/// @return 0 on success or -1 on error with the errno set to indicate the error
/// @note Conditional thread safe as long as only 'iox_getenv', 'iox_setenv' and 'iox_unsetenv' are used and none of
/// 'getenv', 'setenv', 'unsetenv' and 'putenv' directly
int iox_unsetenv(const char* name);

#endif // IOX_HOOFS_GENERIC_PLATFORM_STDLIB_HPP
