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

#ifndef IOX_HOOFS_FREERTOS_PLATFORM_MMAN_HPP
#define IOX_HOOFS_FREERTOS_PLATFORM_MMAN_HPP

#include <sys/types.h>

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_FAILED (void*)-1
#define PROT_NONE 0
#define PROT_READ 3
#define PROT_WRITE 4

int iox_shm_open(const char* name, int oflag, mode_t mode);
int iox_shm_unlink(const char* name);
int iox_shm_close(int fd);

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void* addr, size_t length);

#endif // IOX_HOOFS_FREERTOS_PLATFORM_MMAN_HPP
