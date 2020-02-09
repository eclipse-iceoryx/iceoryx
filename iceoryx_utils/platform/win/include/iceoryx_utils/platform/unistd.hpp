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

#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/windows.hpp"

#include <io.h>
#include <vector>

#define _SC_PAGESIZE 1
#define STDERR_FILENO 2

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

    int add(HANDLE handle) noexcept
    {
        for (int64_t limit = m_handleList.size(), k = 0; k < limit; ++k)
        {
            if (m_handleList[k].windowsHandle == nullptr)
            {
                m_handleList[k].windowsHandle = handle;
                return k;
            }
        }

        m_handleList.emplace_back(handle_t{handle});
        return m_handleList.size() - 1;
    }

    void remove(HANDLE handle) noexcept
    {
        for (auto& handle : m_handleList)
        {
            if (RtlCompareMemory(&handle.windowsHandle, &handle, sizeof(void*)) == sizeof(void*))
            {
                handle.windowsHandle = nullptr;
                break;
            }
        }
    }

  private:
    struct handle_t
    {
        HANDLE windowsHandle;
    };
    std::vector<handle_t> m_handleList;
};


inline int ftruncate(int fildes, off_t length)
{
    return 0;
}

inline long sysconf(int name)
{
    return 0;
}

inline int closePlatformFileHandle(int fd)
{
    printf("closing :: %d\n", fd);
    if (CloseHandle(HandleTranslator::getInstance().get(fd)) == 0)
    {
        return -1;
    }
    return 0;
}
