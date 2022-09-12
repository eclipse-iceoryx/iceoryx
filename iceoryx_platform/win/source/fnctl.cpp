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

#include "iceoryx_hoofs/platform/fcntl.hpp"
#include "iceoryx_hoofs/platform/handle_translator.hpp"
#include "iceoryx_hoofs/platform/win32_errorHandling.hpp"

int iox_open(const char* pathname, int flags, mode_t mode)
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
        fprintf(stderr, "unable to create file \"%s\"\n", pathname);
        errno = EWOULDBLOCK;
        return -1;
    }

    return HandleTranslator::getInstance().add(handle);
}
