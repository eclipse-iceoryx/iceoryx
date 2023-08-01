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

#include "iceoryx_platform/mman.hpp"
#include "iceoryx_platform/errno.hpp"
#include "iceoryx_platform/shm_file.hpp"

#include "FreeRTOS.h"

#include <algorithm>
#include <cstring>

int iox_shm_open(const char* name, int, mode_t)
{
    std::lock_guard<std::mutex> lock{ShmFile::openFilesMutex};
    const auto iter =
        std::find_if(std::begin(ShmFile::openFiles), std::end(ShmFile::openFiles), [name](const ShmFile& f) {
            return std::strncmp(name, f.name(), ShmFile::MAX_NAME_LENGTH) == 0;
        });
    if (iter == std::end(ShmFile::openFiles))
    {
        const auto iter_empty = std::find_if(
            std::begin(ShmFile::openFiles), std::end(ShmFile::openFiles), [](const ShmFile& f) { return f.empty(); });
        configASSERT(iter_empty != std::end(ShmFile::openFiles));
        *iter_empty = ShmFile{name};
        return iter_empty->fd();
    }
    else
    {
        return iter->fd();
    }
}

int iox_shm_unlink(const char* name)
{
    std::lock_guard<std::mutex> lock{ShmFile::openFilesMutex};
    const auto iter =
        std::find_if(std::begin(ShmFile::openFiles), std::end(ShmFile::openFiles), [name](const ShmFile& f) {
            return std::strncmp(name, f.name(), ShmFile::MAX_NAME_LENGTH) == 0;
        });
    if (iter != std::end(ShmFile::openFiles))
    {
        *iter = ShmFile{};
        return 0;
    }
    else
    {
        FreeRTOS_errno = ENOENT;
        return -1;
    }
}

int iox_shm_close(int)
{
    // We do all the closing in unlink
    return 0;
}

void* mmap(void*, size_t length, int, int, int fd, off_t)
{
    std::lock_guard<std::mutex> lock{ShmFile::openFilesMutex};
    const auto iter = std::find_if(
        std::begin(ShmFile::openFiles), std::end(ShmFile::openFiles), [fd](const ShmFile& f) { return f.fd() == fd; });

    configASSERT(iter != std::end(ShmFile::openFiles));
    configASSERT(iter->size() == length);
    configASSERT(iter->ptr() != nullptr);
    return iter->ptr();
}

int munmap(void*, size_t)
{
    return 0;
}
