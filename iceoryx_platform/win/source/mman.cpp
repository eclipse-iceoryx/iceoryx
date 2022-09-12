// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_platform/handle_translator.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_platform/win32_errorHandling.hpp"

#include <iostream>
#include <mutex>
#include <set>
#include <string>

static std::set<std::string> openedSharedMemorySegments;
static std::mutex openedSharedMemorySegmentsMutex;

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    DWORD desiredAccess = FILE_MAP_ALL_ACCESS;
    DWORD fileOffsetHigh = 0;
    DWORD fileOffsetLow = 0;
    DWORD numberOfBytesToMap = length;

    auto printErrorMessage = [&] {
        std::cerr << "Failed to map file mapping into process space with mmap( addr = " << std::hex << addr << std::dec
                  << ", length = " << length << ", [always assume PROT_READ | PROT_WRITE] prot = " << prot
                  << ", [always assume MAP_SHARED] flags = " << flags << ", fd = " << fd
                  << ", [always assume 0] offset = " << offset << ")" << std::endl;
    };

    void* mappedObject = Win32Call(MapViewOfFile,
                                   HandleTranslator::getInstance().get(fd),
                                   desiredAccess,
                                   fileOffsetHigh,
                                   fileOffsetLow,
                                   numberOfBytesToMap)
                             .value;

    if (mappedObject == nullptr)
    {
        printErrorMessage();
        return nullptr;
    }

    // windows only reserves memory but does not allocate it right away (see SEC_RESERVE in iox_shm_open)
    // this call actually allocates the right amount of bytes
    mappedObject = Win32Call(VirtualAlloc, mappedObject, numberOfBytesToMap, MEM_COMMIT, PAGE_READWRITE).value;
    if (mappedObject == nullptr)
    {
        printErrorMessage();
        return nullptr;
    }

    return mappedObject;
}

int munmap(void* addr, size_t length)
{
    if (Win32Call(UnmapViewOfFile, addr).value)
    {
        return 0;
    }
    else
    {
        std::cerr << "Failed to unmap memory region with munmap( addr = " << std::hex << addr << std::dec
                  << ", length = " << length << ")" << std::endl;
    }

    return -1;
}

int iox_shm_open(const char* name, int oflag, mode_t mode)
{
    HANDLE sharedMemoryHandle{nullptr};

    auto printErrorMessage = [&] {
        std::cerr << "Failed to create shared memory with iox_shm_open( name = " << name
                  << ", [only consider O_CREAT and O_EXCL] oflag = " << oflag
                  << ", [always assume read, write, execute for everyone] mode = " << mode << ")" << std::endl;
    };


    if (oflag & O_CREAT)
    {
        // we do not yet support ACL and rights for data partitions in windows
        // DWORD access = (oflag & O_RDWR) ? PAGE_READWRITE : PAGE_READONLY;
        DWORD access = PAGE_READWRITE | SEC_RESERVE;
        DWORD MAXIMUM_SIZE_LOW = static_cast<DWORD>(iox::platform::win32::IOX_MAXIMUM_SUPPORTED_SHM_SIZE & 0xFFFFFFFF);
        DWORD MAXIMUM_SIZE_HIGH =
            static_cast<DWORD>((iox::platform::win32::IOX_MAXIMUM_SUPPORTED_SHM_SIZE >> 32) & 0xFFFFFFFF);

        auto result = Win32Call(CreateFileMapping,
                                static_cast<HANDLE>(INVALID_HANDLE_VALUE),
                                static_cast<LPSECURITY_ATTRIBUTES>(nullptr),
                                static_cast<DWORD>(access),
                                static_cast<DWORD>(MAXIMUM_SIZE_HIGH),
                                static_cast<DWORD>(MAXIMUM_SIZE_LOW),
                                static_cast<LPCSTR>(name));
        sharedMemoryHandle = result.value;

        if (sharedMemoryHandle == nullptr)
        {
            errno = EACCES;
            printErrorMessage();
            return HandleTranslator::INVALID_LINUX_FD;
        }

        if (oflag & O_EXCL && result.error == ERROR_ALREADY_EXISTS)
        {
            errno = EEXIST;
            if (sharedMemoryHandle != nullptr)
            {
                Win32Call(CloseHandle, sharedMemoryHandle).value;
            }
            return HandleTranslator::INVALID_LINUX_FD;
        }
    }
    else
    {
        auto result = Win32Call(OpenFileMapping,
                                static_cast<DWORD>(FILE_MAP_ALL_ACCESS),
                                static_cast<BOOL>(false),
                                static_cast<LPCSTR>(name));

        sharedMemoryHandle = result.value;

        if (sharedMemoryHandle == nullptr)
        {
            errno = ENOENT;
            return HandleTranslator::INVALID_LINUX_FD;
        }

        if (result.error != 0)
        {
            printErrorMessage();
            errno = EACCES;
            Win32Call(CloseHandle, sharedMemoryHandle);
            return HandleTranslator::INVALID_LINUX_FD;
        }
    }

    {
        std::lock_guard<std::mutex> lock(openedSharedMemorySegmentsMutex);
        openedSharedMemorySegments.insert(name);
    }
    return HandleTranslator::getInstance().add(sharedMemoryHandle);
}

int iox_shm_unlink(const char* name)
{
    std::lock_guard<std::mutex> lock(openedSharedMemorySegmentsMutex);
    auto iter = openedSharedMemorySegments.find(name);
    if (iter != openedSharedMemorySegments.end())
    {
        openedSharedMemorySegments.erase(iter);
        return 0;
    }

    errno = ENOENT;
    return -1;
}
