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

#include <iostream>

#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

inline void PrintLastErrorToConsole() noexcept
{
    auto lastError = GetLastError();
    if (lastError == 0)
    {
        return;
    }

    char buffer[2048];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  lastError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  buffer,
                  2048,
                  NULL);

    std::cerr << "error ( " << lastError << " ) :: " << buffer << std::endl;
}
