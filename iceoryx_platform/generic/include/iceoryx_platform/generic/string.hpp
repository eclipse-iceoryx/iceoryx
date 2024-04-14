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

#ifndef IOX_PLATFORM_GENERIC_STRING_HPP
#define IOX_PLATFORM_GENERIC_STRING_HPP

#include <cstring>

/// @brief Implements the GNU version of 'strerror_r'
/// @param[in] errnum the error code to be converted to a string
/// @param[out] buf the buffer to store the error message in case there is no static error message available for the
/// given error code
/// @param[in] buflen the length of the buffer to store the error message
/// @return a pointer to a string containing the error message; this is either a pointer to an immutable static string
/// or the provided buffer
char* iox_gnu_strerror_r(int errnum, char* buf, size_t buflen);

#endif // IOX_PLATFORM_GENERIC_STRING_HPP
