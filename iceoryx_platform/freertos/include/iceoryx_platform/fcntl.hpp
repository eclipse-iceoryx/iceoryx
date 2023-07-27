// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
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

#ifndef IOX_HOOFS_FREERTOS_PLATFORM_FCNTL_HPP
#define IOX_HOOFS_FREERTOS_PLATFORM_FCNTL_HPP

#include <fcntl.h>

/* lock operations for flock(2) */
#define LOCK_SH 0x01 /* shared file lock */
#define LOCK_EX 0x02 /* exclusive file lock */
#define LOCK_NB 0x04 /* don't block when locking */
#define LOCK_UN 0x08 /* unlock file */

int iox_open(const char* pathname, int flags, mode_t mode);
int iox_ext_open(const char* pathname, int flags, mode_t mode);

#endif // IOX_HOOFS_FREERTOS_PLATFORM_FCNTL_HPP
