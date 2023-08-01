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

#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/shm_file.hpp"

#include <algorithm>

mode_t umask(mode_t)
{
    return mode_t{};
}

int iox_fstat(int fildes, iox_stat* buf)
{
    buf->st_uid = 0;
    buf->st_gid = 0;
    buf->st_mode = 0777;

    std::lock_guard<std::mutex> lock{ShmFile::openFilesMutex};
    const auto iter = std::find_if(std::begin(ShmFile::openFiles),
                                   std::end(ShmFile::openFiles),
                                   [fildes](const ShmFile& f) { return f.fd() == fildes; });
    configASSERT(iter != std::end(ShmFile::openFiles));
    buf->st_size = iter->size();
    return 0;
}

int iox_fchmod(int, iox_mode_t)
{
    return 0;
}
