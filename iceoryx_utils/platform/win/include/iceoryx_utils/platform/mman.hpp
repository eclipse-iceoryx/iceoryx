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
#include "iceoryx_utils/platform/win32-error.hpp"
#include "iceoryx_utils/platform/windows.hpp"


#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <vector>


#define MAP_SHARED 0
#define MAP_FAILED 1
#define PROT_READ 3
#define PROT_WRITE 4

class HandleTranslator
{
  public:
    static HandleTranslator& getInstance() noexcept
    {
        static HandleTranslator globalHandleTranslator;
        return globalHandleTranslator;
    }

    HANDLE get(const int handle) const noexcept
    {
        return m_handleList[static_cast<size_t>(handle)].windowsHandle;
    }

    HANDLE get(const std::string& handleName) const noexcept
    {
        for (auto& handle : m_handleList)
        {
            if (handle.name == handleName)
            {
                return handle.windowsHandle;
            }
        }
        return nullptr;
    }

    int add(HANDLE handle, const std::string& handleName) noexcept
    {
        for (int64_t limit = m_handleList.size(), k = 0; k < limit; ++k)
        {
            if (m_handleList[k].windowsHandle == nullptr)
            {
                m_handleList[k].windowsHandle = handle;
                return k;
            }
        }

        m_handleList.emplace_back(handle_t{handle, handleName});
        return m_handleList.size() - 1;
    }

    void remove(HANDLE handle) noexcept
    {
        for (auto& handle : m_handleList)
        {
            if (RtlCompareMemory(&handle.windowsHandle, &handle, sizeof(void*)) == sizeof(void*))
            {
                handle.windowsHandle = nullptr;
                handle.name.clear();
                break;
            }
        }
    }

    void remove(const std::string& handleName) noexcept
    {
        for (auto& handle : m_handleList)
        {
            if (handleName == handle.name)
            {
                handle.windowsHandle = nullptr;
                handle.name.clear();
                break;
            }
        }
    }

  private:
    struct handle_t
    {
        HANDLE windowsHandle;
        std::string name;
    };
    std::vector<handle_t> m_handleList;
};

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
        return nullptr;
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

    HANDLE sharedMemoryHandle;
    DWORD access = (oflag & O_RDWR) ? PAGE_READWRITE : PAGE_READONLY;

    if (oflag & O_CREAT)
    {
        sharedMemoryHandle =
            CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, access, MAXIMUM_SIZE_HIGH, MAXIMUM_SIZE_LOW, name);
    }
    else
    {
        sharedMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, name);
    }

    if (sharedMemoryHandle == nullptr)
    {
        PrintLastErrorToConsole();
        return -1;
    }

    return HandleTranslator::getInstance().add(sharedMemoryHandle, name);
}

inline int shm_unlink(const char* name)
{
    if (CloseHandle(HandleTranslator::getInstance().get(name)) == 0)
    {
        return -1;
    }
    return 0;
}
