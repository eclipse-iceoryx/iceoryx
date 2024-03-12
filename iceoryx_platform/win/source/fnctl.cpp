// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/handle_translator.hpp"
#include "iceoryx_platform/logging.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"
#include "iceoryx_platform/windows.hpp"

#include <io.h>
#include <share.h>
#include <sys/stat.h>

#include <sstream>

int iox_open(const char* pathname, int flags, mode_t mode)
{
    int fd;
    _sopen_s(&fd, pathname, flags, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    return fd;
}

int iox_ext_open(const char* pathname, int flags, mode_t mode)
{
    auto handle = Win32Call(CreateFileA,
                            pathname,
                            GENERIC_WRITE,
                            0,
                            static_cast<LPSECURITY_ATTRIBUTES>(NULL),
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            static_cast<HANDLE>(NULL))
                      .value;

    if (handle == INVALID_HANDLE_VALUE)
    {
        std::stringstream stream;
        stream << "unable to create file \"" << pathname << "\"";
        IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, stream.str().c_str());

        errno = EWOULDBLOCK;
        return -1;
    }

    return HandleTranslator::getInstance().add(handle);
}

int iox_fcntl2(int, int)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_fcntl2' is not implemented in windows!");
    errno = ENOSYS;
    return -1;
}

int iox_fcntl3(int, int, int)
{
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'iox_fcntl3' is not implemented in windows!");
    errno = ENOSYS;
    return -1;
}
