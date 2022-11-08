// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_platform/win32_errorHandling.hpp"

#include <iostream>
#include <mutex>

int __PrintLastErrorToConsole(const char* functionName, const char* file, const int line) noexcept
{
    static std::mutex coutMutex;
    constexpr uint64_t BUFFER_SIZE{2048u};
    int lastError = GetLastError();
    if (lastError != 0)
    {
        char buffer[BUFFER_SIZE];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      lastError,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      buffer,
                      BUFFER_SIZE - 1,
                      NULL);

        coutMutex.lock();
        fprintf(stderr, "< Win32API Error > %s:%d { %s } [ %d ] ::: %s", file, line, functionName, lastError, buffer);
        coutMutex.unlock();
    }
    return lastError;
}
