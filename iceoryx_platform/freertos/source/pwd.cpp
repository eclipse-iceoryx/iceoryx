// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
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

#include "iceoryx_platform/pwd.hpp"

struct passwd* getpwnam(const char*)
{
    static char value[] = "iceoryx_freertos_dummy";
    static struct passwd dummy;
    dummy.pw_name = value;
    dummy.pw_passwd = value;
    dummy.pw_uid = 0;
    dummy.pw_gid = 0;
    dummy.pw_gecos = 0;
    dummy.pw_dir = value;
    dummy.pw_shell = value;
    return &dummy;
}

struct passwd* getpwuid(iox_uid_t)
{
    static char value[] = "iceoryx_freertos_dummy";
    static struct passwd dummy;
    dummy.pw_name = value;
    dummy.pw_passwd = value;
    dummy.pw_uid = 0;
    dummy.pw_gid = 0;
    dummy.pw_gecos = 0;
    dummy.pw_dir = value;
    dummy.pw_shell = value;
    return &dummy;
}
