// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_platform/string.hpp"

#ifndef IOX_PLATFORM_OVERRIDE_STRING_ALL

#include <cerrno>

#ifndef IOX_PLATFORM_OVERRIDE_STRING_STRERROR_R

namespace
{
[[maybe_unused]] char* strerror_r_gnu_xsi_unificaton(const int returnCode [[maybe_unused]], char* buf)
{
    return buf;
}

[[maybe_unused]] char* strerror_r_gnu_xsi_unificaton(char* msg, char* buf [[maybe_unused]])
{
    return msg;
}

} // namespace

char* iox_gnu_strerror_r(int errnum, char* buf, size_t buflen)
{
    return strerror_r_gnu_xsi_unificaton(strerror_r(errnum, buf, buflen), buf);
}

#endif

#endif
