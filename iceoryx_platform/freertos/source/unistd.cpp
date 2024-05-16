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

#include "iceoryx_platform/unistd.hpp"
#include "iceoryx_platform/shm_file.hpp"

#include "FreeRTOS.h"

#include <algorithm>

int iox_close(int)
{
    // The files in ShmFile::openFiles only should be destroyed in iox_shm_unlink.
    // So, we dont need to do anything here.
    return 0;
}

int iox_ext_close(int)
{
    return 0;
}

int iox_ftruncate(int fd, off_t length)
{
    std::lock_guard<std::mutex> lock{ShmFile::openFilesMutex};
    const auto iter = std::find_if(
        std::begin(ShmFile::openFiles), std::end(ShmFile::openFiles), [fd](const ShmFile& f) { return f.fd() == fd; });
    configASSERT(iter != std::end(ShmFile::openFiles));
    return iter->ftruncate(length) ? 0 : -1;
}

long iox_sysconf(int)
{
    // This is only ever used to find the page size. Lets just return 4 kB as usual, even though there is no paging on
    // FreeRTOS
    return 4096;
}

int iox_fchown(int, iox_uid_t, iox_gid_t)
{
    return 0;
}

int iox_access(const char*, int)
{
    return 0;
}

int iox_unlink(const char*)
{
    return 0;
}

iox_off_t iox_lseek(int, iox_off_t offset, int)
{
    return offset;
}

iox_ssize_t iox_read(int, void*, size_t)
{
    return 0;
}

iox_ssize_t iox_write(int, const void*, size_t)
{
    return 0;
}

iox_gid_t iox_getgid(void)
{
    // Lets just say that on FreeRTOS, all group IDs are 1
    return 1;
}

iox_uid_t iox_geteuid(void)
{
    // Lets just say that on FreeRTOS, all user IDs are 1
    return 1;
}
