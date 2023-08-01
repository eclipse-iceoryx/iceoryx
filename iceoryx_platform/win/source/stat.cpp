// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_platform/handle_translator.hpp"
#include "iceoryx_platform/mman.hpp"

int iox_fstat(int fildes, iox_stat* buf)
{
    HANDLE handle = HandleTranslator::getInstance().get(fildes);
    if (handle == INVALID_HANDLE_VALUE)
    {
        int ret_val = _fstat64(fildes, buf);
        buf->st_mode = std::numeric_limits<decltype(buf->st_mode)>::max();
        return ret_val;
    }

    auto size = internal_iox_shm_get_size(fildes);
    if (size != -1)
    {
        buf->st_size = size;
        return 0;
    }

    buf->st_size = Win32Call(GetFileSize, (HANDLE)handle, (LPDWORD)NULL).value;
    return 0;
}

int iox_fchmod(int fildes, iox_mode_t mode)
{
    return 0;
}
