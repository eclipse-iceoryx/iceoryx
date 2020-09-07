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
#ifndef IOX_UTILS_WIN_PLATFORM_UNISTD_HPP
#define IOX_UTILS_WIN_PLATFORM_UNISTD_HPP

#include "iceoryx_utils/platform/types.hpp"
#include "iceoryx_utils/platform/windows.hpp"

#include <cstdint>
#include <io.h>
#include <vector>

#define _SC_PAGESIZE 1
#define STDERR_FILENO 2

using useconds_t = uint64_t;

class HandleTranslator
{
  public:
    static HandleTranslator& getInstance() noexcept;
    HANDLE get(const int handle) const noexcept;
    int add(HANDLE handle) noexcept;
    void remove(int handle) noexcept;

  private:
    struct handle_t
    {
        HANDLE windowsHandle;
    };
    std::vector<handle_t> m_handleList;
};


int ftruncate(int fildes, off_t length);
long sysconf(int name);
int closePlatformFileHandle(int fd);

#endif // IOX_UTILS_WIN_PLATFORM_UNISTD_HPP
