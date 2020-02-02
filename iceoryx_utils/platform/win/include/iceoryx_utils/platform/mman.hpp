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

#pragma once

#include <sys/stat.h>

#include "iceoryx_utils/platform/types.hpp"

#define MAP_SHARED 0
#define MAP_FAILED 1
#define PROT_READ 3
#define PROT_WRITE 4

inline void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return nullptr;
}

inline int munmap(void* addr, size_t length)
{
    return 0;
}

inline int shm_open(const char* name, int oflag, mode_t mode)
{
    return 0;
}

inline int shm_unlink(const char* name)
{
    return 0;
}
