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

#include "iceoryx_platform/unistd.hpp"
#include "iceoryx_platform/handle_translator.hpp"
#include "iceoryx_platform/mman.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"


int iox_ftruncate(int fildes, off_t length)
{
    internal_iox_shm_set_size(fildes, length);
    return 0;
}

int iox_access(const char* pathname, int mode)
{
    return _access(pathname, mode);
}

int iox_unlink(const char* pathname)
{
    return _unlink(pathname);
}

long iox_sysconf(int name)
{
    if (name == IOX_SC_PAGESIZE)
    {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        return systemInfo.dwPageSize;
    }
    return 0;
}

int iox_close(int fd)
{
    return _close(fd);
}

int iox_ext_close(int fd)
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

int iox_fchown(int fd, iox_uid_t owner, iox_gid_t group)
{
    return 0;
}

iox_off_t iox_lseek(int fd, iox_off_t offset, int whence)
{
    return _lseek(fd, offset, whence);
}

iox_ssize_t iox_read(int fd, void* buf, size_t count)
{
    return _read(fd, buf, count);
}

iox_ssize_t iox_write(int fd, const void* buf, size_t count)
{
    return _write(fd, buf, count);
}

iox_gid_t iox_getgid(void)
{
    return 0;
}

iox_uid_t iox_geteuid(void)
{
    return 0;
}
