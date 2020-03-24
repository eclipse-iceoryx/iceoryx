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

#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/unistd.hpp"
#include "iceoryx_utils/platform/win32_errorHandling.hpp"


#include <cstdio>
#include <string>
#include <sys/stat.h>


#define MAP_SHARED 0
#define MAP_FAILED 1
#define PROT_READ 3
#define PROT_WRITE 4

inline void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    DWORD desiredAccess = FILE_MAP_ALL_ACCESS;
    DWORD fileOffsetHigh = 0;
    DWORD fileOffsetLow = 0;
    DWORD numberOfBytesToMap = length;

    void* mappedObject = MapViewOfFile(
        HandleTranslator::getInstance().get(fd), desiredAccess, fileOffsetHigh, fileOffsetLow, numberOfBytesToMap);

    if (mappedObject == nullptr)
    {
        PrintLastErrorToConsole();
    }

    return mappedObject;
}

inline int munmap(void* addr, size_t length)
{
    if (UnmapViewOfFile(addr))
    {
        return 0;
    }

    PrintLastErrorToConsole();
    return -1;
}

inline int shm_open(const char* name, int oflag, mode_t mode)
{
    static constexpr DWORD MAXIMUM_SIZE_HIGH = 0;
    static constexpr DWORD MAXIMUM_SIZE_LOW = 256;

    HANDLE sharedMemoryHandle{nullptr};
    DWORD access = (oflag & O_RDWR) ? PAGE_READWRITE : PAGE_READONLY;

    if (oflag & O_CREAT) // O_EXCL
    {
        sharedMemoryHandle =
            CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, access, MAXIMUM_SIZE_HIGH, MAXIMUM_SIZE_LOW, name);

        if (oflag & O_EXCL && PrintLastErrorToConsole() == ERROR_ALREADY_EXISTS)
        {
            if (sharedMemoryHandle != nullptr)
            {
                CloseHandle(sharedMemoryHandle);
            }
            return -1;
        }
    }
    else
    {
        sharedMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, name);

        if (PrintLastErrorToConsole() != 0)
        {
            if (sharedMemoryHandle != nullptr)
            {
                CloseHandle(sharedMemoryHandle);
            }
            return -1;
        }
    }

    return HandleTranslator::getInstance().add(sharedMemoryHandle);
}

inline int shm_unlink(const char* name)
{
    return 0;
}
