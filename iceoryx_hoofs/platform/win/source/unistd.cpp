// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/platform/unistd.hpp"
#include "iceoryx_hoofs/platform/handle_translator.hpp"
#include "iceoryx_hoofs/platform/win32_errorHandling.hpp"

int ftruncate(int fildes, off_t length)
{
    return 0;
}

long sysconf(int name)
{
    if (name == _SC_PAGESIZE)
    {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        return systemInfo.dwPageSize;
    }
    return 0;
}

int iox_close(int fd)
{
    HANDLE handle = HandleTranslator::getInstance().get(fd);
    if (handle == nullptr)
    {
        return 0;
    }

    auto success = Win32Call(CloseHandle, handle).value;
    HandleTranslator::getInstance().remove(fd);
    if (success == 0)
    {
        return -1;
    }
    return 0;
}
