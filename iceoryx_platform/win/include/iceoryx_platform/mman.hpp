// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_MMAN_HPP
#define IOX_HOOFS_WIN_PLATFORM_MMAN_HPP

#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/unistd.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"

#include <cstdio>
#include <string>
#include <sys/stat.h>

// this header needs to be the last include in the file otherwise
// windows will define some macros which makes the code uncompilable
#include "iceoryx_platform/platform_correction.hpp"

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_FAILED (void*)-1
#define PROT_NONE 0
#define PROT_READ 3
#define PROT_WRITE 4

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);

int munmap(void* addr, size_t length);

int iox_shm_open(const char* name, int oflag, mode_t mode);

int iox_shm_unlink(const char* name);

int iox_shm_close(int fd);

void internal_iox_shm_set_size(int fd, off_t length);

off_t internal_iox_shm_get_size(int fd);
#endif // IOX_HOOFS_WIN_PLATFORM_MMAN_HPP
