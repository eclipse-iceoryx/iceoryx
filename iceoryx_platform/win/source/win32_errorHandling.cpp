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
#include "iceoryx_platform/logging.hpp"

#include <cstdio>
#include <mutex>

int __PrintLastErrorToConsole(const char* functionName, const char* file, const int line) noexcept
{
    constexpr uint64_t BUFFER_SIZE{2048u};
    int lastError = GetLastError();
    if (lastError != 0)
    {
        char buffer[BUFFER_SIZE];
        sprintf(buffer, "< Win32API Error > [%d] ::: ", lastError);
        size_t used_buffer_size = strlen(buffer);
        constexpr size_t NULL_TERMINATOR_SIZE{1};
        size_t remaining_buffer_size = BUFFER_SIZE - used_buffer_size - NULL_TERMINATOR_SIZE;

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                      NULL,
                      lastError,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      &buffer[used_buffer_size],
                      remaining_buffer_size,
                      NULL);

        iox_platform_detail_log(file, line, functionName, IOX_PLATFORM_LOG_LEVEL_ERROR, buffer);
    }
    return lastError;
}
